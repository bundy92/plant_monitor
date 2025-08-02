#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <nvs_flash.h>
#include <esp_http_client.h>
#include <cJSON.h>
#include <driver/gpio.h>
#include <driver/adc.h>
#include <driver/i2c.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>
#include "../config.h"

// Task handles
TaskHandle_t sensor_task_handle = NULL;
TaskHandle_t wifi_task_handle = NULL;

// Event group for WiFi
EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;

// ADC handles
adc_oneshot_unit_handle_t adc1_handle;
adc_cali_handle_t adc1_cali_handle = NULL;

// I2C handle
i2c_port_t i2c_port = I2C_MASTER_NUM;

// Sensor data structure
typedef struct {
    float temperature;
    float humidity;
    int soil_moisture;
    int light_level;
    uint32_t timestamp;
} sensor_data_t;

static const char *TAG = "PLANT_MONITOR";

// AHT10 sensor commands
#define AHT10_CMD_INITIALIZE    0xE1
#define AHT10_CMD_MEASURE       0xAC
#define AHT10_CMD_NORMAL        0xA8
#define AHT10_CMD_SOFT_RESET    0xBA

// Initialize I2C
esp_err_t i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    
    esp_err_t err = i2c_param_config(i2c_port, &conf);
    if (err != ESP_OK) {
        return err;
    }
    
    return i2c_driver_install(i2c_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

// Initialize AHT10 sensor
esp_err_t aht10_init(uint8_t sensor_addr)
{
    // Send soft reset command
    uint8_t reset_cmd[] = {AHT10_CMD_SOFT_RESET};
    esp_err_t ret = i2c_master_write_to_device(i2c_port, sensor_addr, reset_cmd, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "AHT10 reset failed");
        return ret;
    }
    
    vTaskDelay(pdMS_TO_TICKS(20)); // Wait for reset
    
    // Send initialization command
    uint8_t init_cmd[] = {AHT10_CMD_INITIALIZE, 0x08, 0x00};
    ret = i2c_master_write_to_device(i2c_port, sensor_addr, init_cmd, 3, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "AHT10 initialization failed");
        return ret;
    }
    
    vTaskDelay(pdMS_TO_TICKS(10)); // Wait for initialization
    
    ESP_LOGI(TAG, "AHT10 sensor initialized successfully");
    return ESP_OK;
}

// Read temperature and humidity from AHT10
esp_err_t aht10_read_sensor(uint8_t sensor_addr, float *temperature, float *humidity)
{
    // Send measurement command
    uint8_t measure_cmd[] = {AHT10_CMD_MEASURE, 0x33, 0x00};
    esp_err_t ret = i2c_master_write_to_device(i2c_port, sensor_addr, measure_cmd, 3, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "AHT10 measurement command failed");
        return ret;
    }
    
    vTaskDelay(pdMS_TO_TICKS(80)); // Wait for measurement
    
    // Read 6 bytes of data
    uint8_t data[6];
    ret = i2c_master_read_from_device(i2c_port, sensor_addr, data, 6, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "AHT10 read data failed");
        return ret;
    }
    
    // Check if measurement is ready
    if (data[0] & 0x80) {
        ESP_LOGE(TAG, "AHT10 measurement not ready");
        return ESP_ERR_INVALID_STATE;
    }
    
    // Calculate humidity (20-bit value)
    uint32_t humidity_raw = ((uint32_t)(data[1] & 0x0F) << 16) | ((uint32_t)data[2] << 8) | data[3];
    *humidity = (float)humidity_raw * 100.0 / 1048576.0; // Convert to percentage
    
    // Calculate temperature (20-bit value)
    uint32_t temp_raw = ((uint32_t)(data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5];
    *temperature = (float)temp_raw * 200.0 / 1048576.0 - 50.0; // Convert to Celsius
    
    return ESP_OK;
}

// WiFi event handler
static void event_handler(void* arg, esp_event_base_t event_base,
                         int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "WiFi disconnected, trying to reconnect...");
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// Initialize WiFi
void wifi_init_sta(void)
{
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                      ESP_EVENT_ANY_ID,
                                                      &event_handler,
                                                      NULL,
                                                      &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                      IP_EVENT_STA_GOT_IP,
                                                      &event_handler,
                                                      NULL,
                                                      &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");
}

// HTTP client event handler
esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP Client Error");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP Client Connected");
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP Client Finished");
            break;
        default:
            break;
    }
    return ESP_OK;
}

// Read analog sensor
int read_analog_sensor(adc_channel_t channel)
{
    int adc_raw = 0;
    int voltage = 0;
    esp_err_t ret = ESP_OK;
    
    ret = adc_oneshot_read(adc1_handle, channel, &adc_raw);
    if (ret == ESP_OK) {
        ret = adc_cali_raw_to_voltage(adc1_cali_handle, adc_raw, &voltage);
        if (ret == ESP_OK) {
            return voltage;
        }
    }
    return adc_raw; // Return raw value if calibration fails
}

// Read sensors
sensor_data_t read_sensors(void)
{
    sensor_data_t data = {0};
    
    // Read temperature and humidity from AHT10 sensor 1
    float temp1 = 0.0, hum1 = 0.0;
    esp_err_t ret = aht10_read_sensor(AHT10_SENSOR_1_ADDR, &temp1, &hum1);
    
    if (ret == ESP_OK) {
        data.temperature = temp1;
        data.humidity = hum1;
        ESP_LOGI(TAG, "AHT10 Sensor 1 - Temperature: %.2f°C, Humidity: %.2f%%", temp1, hum1);
    } else {
        ESP_LOGE(TAG, "Failed to read AHT10 sensor 1");
        data.temperature = 0.0;
        data.humidity = 0.0;
    }
    
    // Try to read from second AHT10 sensor if available
    float temp2 = 0.0, hum2 = 0.0;
    ret = aht10_read_sensor(AHT10_SENSOR_2_ADDR, &temp2, &hum2);
    if (ret == ESP_OK) {
        // Average the readings from both sensors
        data.temperature = (data.temperature + temp2) / 2.0;
        data.humidity = (data.humidity + hum2) / 2.0;
        ESP_LOGI(TAG, "AHT10 Sensor 2 - Temperature: %.2f°C, Humidity: %.2f%%", temp2, hum2);
    }
    
    // Read soil moisture (0-4095, higher = more moisture)
    data.soil_moisture = read_analog_sensor(SOIL_MOISTURE_PIN);
    
    // Read light level (0-4095, higher = more light)
    data.light_level = read_analog_sensor(LIGHT_SENSOR_PIN);
    
    data.timestamp = esp_timer_get_time() / 1000; // Convert to milliseconds
    
    ESP_LOGI(TAG, "=== Sensor Readings ===");
    ESP_LOGI(TAG, "Temperature: %.2f°C", data.temperature);
    ESP_LOGI(TAG, "Humidity: %.2f%%", data.humidity);
    ESP_LOGI(TAG, "Soil Moisture: %d", data.soil_moisture);
    ESP_LOGI(TAG, "Light Level: %d", data.light_level);
    ESP_LOGI(TAG, "=====================");
    
    return data;
}

// Send data to server
bool send_data_to_server(sensor_data_t *data)
{
    esp_http_client_config_t config = {
        .url = SERVER_URL,
        .event_handler = http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    
    // Create JSON payload
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "temperature", data->temperature);
    cJSON_AddNumberToObject(root, "humidity", data->humidity);
    cJSON_AddNumberToObject(root, "soil_moisture", data->soil_moisture);
    cJSON_AddNumberToObject(root, "light_level", data->light_level);
    cJSON_AddNumberToObject(root, "timestamp", data->timestamp);
    cJSON_AddStringToObject(root, "device_id", DEVICE_ID);
    
    char *json_string = cJSON_Print(root);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json_string, strlen(json_string));
    
    ESP_LOGI(TAG, "Sending data to server...");
    ESP_LOGI(TAG, "JSON: %s", json_string);
    
    esp_err_t err = esp_http_client_perform(client);
    int status_code = esp_http_client_get_status_code(client);
    
    if (err == ESP_OK && status_code == 200) {
        ESP_LOGI(TAG, "HTTP POST Status = %d", status_code);
        esp_http_client_cleanup(client);
        free(json_string);
        cJSON_Delete(root);
        return true;
    } else {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        free(json_string);
        cJSON_Delete(root);
        return false;
    }
}

// LED control
void set_led(bool state)
{
    gpio_set_level(LED_PIN, state);
}

void blink_led(int times)
{
    for (int i = 0; i < times; i++) {
        set_led(true);
        vTaskDelay(pdMS_TO_TICKS(100));
        set_led(false);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Sensor task
void sensor_task(void *pvParameters)
{
    uint32_t last_send_time = 0;
    
    while (1) {
        // Read sensors
        sensor_data_t data = read_sensors();
        
        // Check if it's time to send data
        uint32_t current_time = esp_timer_get_time() / 1000;
        if (current_time - last_send_time >= DATA_INTERVAL_MS) {
            // Wait for WiFi connection
            xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
            
            if (send_data_to_server(&data)) {
                // Blink LED once for successful transmission
                blink_led(1);
                last_send_time = current_time;
            } else {
                // Blink LED twice for failed transmission
                blink_led(2);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// WiFi task
void wifi_task(void *pvParameters)
{
    wifi_init_sta();
    
    // Wait for WiFi connection
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "WiFi connected successfully");
    
    // Blink LED to indicate successful startup
    blink_led(3);
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Plant Monitor Starting...");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    
    // Initialize I2C
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");
    
    // Initialize AHT10 sensors
    ESP_ERROR_CHECK(aht10_init(AHT10_SENSOR_1_ADDR));
    ESP_ERROR_CHECK(aht10_init(AHT10_SENSOR_2_ADDR));
    ESP_LOGI(TAG, "AHT10 sensors initialized successfully");
    
    // Initialize ADC
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
    
    // ADC1 config
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, SOIL_MOISTURE_PIN, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, LIGHT_SENSOR_PIN, &config));
    
    // ADC calibration - simplified for now
    adc1_cali_handle = NULL;
    
    // Create tasks
    xTaskCreate(&wifi_task, "wifi_task", 4096, NULL, 5, &wifi_task_handle);
    xTaskCreate(&sensor_task, "sensor_task", 4096, NULL, 5, &sensor_task_handle);
    
    ESP_LOGI(TAG, "Plant Monitor initialized successfully");
} 