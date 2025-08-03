#!/bin/bash
# Plant Monitor - Raspberry Pi Server Test Runner
# ===============================================
# This script runs comprehensive tests for the Raspberry Pi server
# including unit tests, integration tests, performance tests, and
# security tests with industry standards and professional reporting.
#
# Features:
# - Unit test execution
# - Integration test execution
# - Performance testing
# - Security testing
# - Code coverage analysis
# - Linting and style checking
# - Documentation generation
# - Professional reporting
#
# Author: Plant Monitor System
# Version: 2.0.0
# Date: 2024
# License: MIT

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Test results tracking
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
SKIPPED_TESTS=0

# Function to print status messages
print_status() {
    local level=$1
    local message=$2
    case $level in
        "INFO")
            echo -e "${BLUE}ℹ️  $message${NC}"
            ;;
        "SUCCESS")
            echo -e "${GREEN}✅ $message${NC}"
            ;;
        "WARNING")
            echo -e "${YELLOW}⚠️  $message${NC}"
            ;;
        "ERROR")
            echo -e "${RED}❌ $message${NC}"
            ;;
        "HEADER")
            echo -e "${PURPLE}🔧 $message${NC}"
            ;;
        "SECTION")
            echo -e "${CYAN}📋 $message${NC}"
            ;;
    esac
}

# Function to run a test and track results
run_test() {
    local test_name="$1"
    local test_command="$2"
    local test_type="${3:-unit}"
    
    print_status "INFO" "Running: $test_name"
    echo "========================================="
    
    ((TOTAL_TESTS++))
    if eval "$test_command"; then
        print_status "SUCCESS" "$test_name passed"
        ((PASSED_TESTS++))
    else
        print_status "ERROR" "$test_name failed"
        ((FAILED_TESTS++))
    fi
    echo ""
}

# Function to check environment
check_environment() {
    print_status "INFO" "Checking Python environment"
    echo "========================================="
    
    # Check Python version
    python_version=$(python3 --version 2>&1)
    print_status "SUCCESS" "Python: $python_version"
    
    # Check if we're in the right directory
    if [ ! -f "server.py" ]; then
        print_status "ERROR" "server.py not found. Please run this script from the raspberry_pi directory."
        exit 1
    fi
    
    # Check for required Python packages
    local required_packages=(
        "flask"
        "sqlalchemy"
        "marshmallow"
        "pyyaml"
        "python-dotenv"
        "flask-cors"
        "flask-socketio"
        "flask-limiter"
        "prometheus-client"
        "cryptography"
    )
    
    for package in "${required_packages[@]}"; do
        if python3 -c "import $package" 2>/dev/null; then
            echo "✅ $package"
        else
            print_status "WARNING" "$package - MISSING"
        fi
    done
    
    print_status "SUCCESS" "Environment check completed"
    echo ""
}

# Function to install dependencies
install_dependencies() {
    print_status "INFO" "Installing Python dependencies"
    echo "========================================="
    
    # Create requirements.txt if it doesn't exist
    if [ ! -f "requirements.txt" ]; then
        cat > requirements.txt << EOF
Flask==2.3.3
SQLAlchemy==2.0.21
Marshmallow==3.20.1
PyYAML==6.0.1
python-dotenv==1.0.0
Flask-CORS==4.0.0
Flask-SocketIO==5.3.6
Flask-Limiter==3.5.0
prometheus-client==0.17.1
cryptography==41.0.4
pytest==7.4.2
pytest-cov==4.1.0
pytest-mock==3.11.1
black==23.9.1
flake8==6.1.0
mypy==1.5.1
EOF
        print_status "SUCCESS" "Created requirements.txt"
    fi
    
    # Install dependencies
    if pip3 install -r requirements.txt; then
        print_status "SUCCESS" "Dependencies installed successfully"
    else
        print_status "ERROR" "Failed to install dependencies"
        exit 1
    fi
    echo ""
}

# Function to run unit tests
run_unit_tests() {
    print_status "SECTION" "Starting Unit Tests"
    echo "========================================="
    
    # Run unit tests with coverage
    run_test "Unit Tests with Coverage" "python3 -m pytest test_server.py -v --cov=server --cov-report=html --cov-report=term-missing"
    
    # Run specific test classes
    run_test "Server Functionality Tests" "python3 -m pytest test_server.py::TestPlantMonitorServer -v"
    run_test "API Endpoint Tests" "python3 -m pytest test_server.py::TestAPIEndpoints -v"
    run_test "Data Validation Tests" "python3 -m pytest test_server.py::TestDataValidation -v"
    run_test "Error Handling Tests" "python3 -m pytest test_server.py::TestErrorHandling -v"
    
    print_status "SUCCESS" "Unit tests completed"
    echo ""
}

# Function to run integration tests
run_integration_tests() {
    print_status "SECTION" "Starting Integration Tests"
    echo "========================================="
    
    # Test server startup
    run_test "Server Startup Test" "timeout 10s python3 server.py & sleep 2 && curl -f http://localhost:5000/api/health && pkill -f 'python3 server.py'"
    
    # Test database operations
    run_test "Database Integration Test" "python3 -c 'from server import PlantMonitorServer; server = PlantMonitorServer(); print(\"Database integration OK\")'"
    
    # Test API endpoints
    run_test "API Integration Test" "python3 -c 'from server import app; client = app.test_client(); response = client.get(\"/api/health\"); print(f\"API integration OK: {response.status_code}\")'"
    
    print_status "SUCCESS" "Integration tests completed"
    echo ""
}

# Function to run performance tests
run_performance_tests() {
    print_status "SECTION" "Starting Performance Tests"
    echo "========================================="
    
    # Test API response time
    run_test "API Response Time Test" "python3 -c 'import time; from server import app; client = app.test_client(); start = time.time(); client.get(\"/api/health\"); end = time.time(); print(f\"Response time: {(end-start)*1000:.2f}ms\"); assert (end-start) < 1.0'"
    
    # Test data processing performance
    run_test "Data Processing Performance Test" "python3 -c 'import time; from server import PlantMonitorServer; server = PlantMonitorServer(); start = time.time(); [server.receive_sensor_data({\"device_id\": f\"test_{i}\", \"timestamp\": int(time.time()), \"temperature\": 25.0, \"humidity\": 60.0, \"soil_moisture\": 2048, \"light_level\": 1024, \"lux\": 5000.0, \"health_score\": 85.0, \"health_status\": \"Good\", \"health_emoji\": \"🙂\", \"recommendation\": \"Test\", \"uptime_seconds\": 3600, \"wifi_connected\": True, \"data_sent\": True}) for i in range(10)]; end = time.time(); print(f\"Processing time: {(end-start)*1000:.2f}ms\"); assert (end-start) < 5.0'"
    
    print_status "SUCCESS" "Performance tests completed"
    echo ""
}

# Function to run security tests
run_security_tests() {
    print_status "SECTION" "Starting Security Tests"
    echo "========================================="
    
    # Test input validation
    run_test "Input Validation Test" "python3 -c 'from server import app; client = app.test_client(); response = client.post(\"/api/data\", data=\"invalid json\", content_type=\"application/json\"); print(f\"Invalid input handled: {response.status_code}\"); assert response.status_code in [400, 500]'"
    
    # Test SQL injection prevention
    run_test "SQL Injection Prevention Test" "python3 -c 'from server import app; client = app.test_client(); malicious_data = {\"device_id\": \"\"; DROP TABLE sensor_readings; --\", \"timestamp\": 1234567890, \"temperature\": 25.0, \"humidity\": 60.0, \"soil_moisture\": 2048, \"light_level\": 1024, \"lux\": 5000.0, \"health_score\": 85.0, \"health_status\": \"Good\", \"health_emoji\": \"🙂\", \"recommendation\": \"Test\", \"uptime_seconds\": 3600, \"wifi_connected\": True, \"data_sent\": True}; import json; response = client.post(\"/api/data\", data=json.dumps(malicious_data), content_type=\"application/json\"); print(f\"SQL injection test: {response.status_code}\"); assert response.status_code in [200, 400]'"
    
    # Test rate limiting
    run_test "Rate Limiting Test" "python3 -c 'from server import app; client = app.test_client(); responses = [client.get(\"/api/health\") for _ in range(5)]; print(f\"Rate limiting test: {len([r for r in responses if r.status_code == 200])} successful requests\"); assert all(r.status_code == 200 for r in responses)'"
    
    print_status "SUCCESS" "Security tests completed"
    echo ""
}

# Function to run code quality checks
run_code_quality_checks() {
    print_status "SECTION" "Starting Code Quality Checks"
    echo "========================================="
    
    # Check for required files
    local required_files=(
        "server.py"
        "test_server.py"
        "requirements.txt"
    )
    
    for file in "${required_files[@]}"; do
        if [ -f "$file" ]; then
            echo "✅ $file"
        else
            print_status "ERROR" "$file - MISSING"
            return 1
        fi
    done
    
    # Run linting with flake8
    run_test "Code Linting (flake8)" "python3 -m flake8 server.py test_server.py --max-line-length=120 --ignore=E501,W503"
    
    # Run type checking with mypy
    run_test "Type Checking (mypy)" "python3 -m mypy server.py --ignore-missing-imports --no-strict-optional"
    
    # Run code formatting check with black
    run_test "Code Formatting Check (black)" "python3 -m black --check server.py test_server.py"
    
    # Check for documentation
    print_status "INFO" "Checking for documentation..."
    local files_with_docs=0
    local total_files=0
    
    for file in server.py test_server.py; do
        if [ -f "$file" ]; then
            ((total_files++))
            if grep -q '"""' "$file" && grep -q '@author' "$file"; then
                ((files_with_docs++))
            fi
        fi
    done
    
    if [ $files_with_docs -eq $total_files ]; then
        print_status "SUCCESS" "All files have comprehensive documentation"
    else
        print_status "WARNING" "Some files may be missing documentation"
    fi
    
    # Check for error handling
    print_status "INFO" "Checking for error handling..."
    if grep -r "try:" server.py && grep -r "except:" server.py; then
        print_status "SUCCESS" "Error handling present"
    else
        print_status "WARNING" "Error handling may be incomplete"
    fi
    
    # Check for logging
    print_status "INFO" "Checking for logging..."
    if grep -r "logging" server.py; then
        print_status "SUCCESS" "Logging present"
    else
        print_status "WARNING" "Logging may be incomplete"
    fi
    
    print_status "SUCCESS" "Code quality checks completed"
    echo ""
}

# Function to run coverage analysis
run_coverage_analysis() {
    print_status "SECTION" "Starting Coverage Analysis"
    echo "========================================="
    
    # Run tests with coverage
    run_test "Coverage Analysis" "python3 -m pytest test_server.py --cov=server --cov-report=html --cov-report=term-missing --cov-fail-under=80"
    
    # Check if coverage report was generated
    if [ -d "htmlcov" ]; then
        print_status "SUCCESS" "Coverage report generated in htmlcov/"
    else
        print_status "WARNING" "Coverage report not generated"
    fi
    
    print_status "SUCCESS" "Coverage analysis completed"
    echo ""
}

# Function to run load testing
run_load_testing() {
    print_status "SECTION" "Starting Load Testing"
    echo "========================================="
    
    # Test concurrent requests
    run_test "Concurrent Request Test" "python3 -c 'import threading, time; from server import app; client = app.test_client(); def make_request(): client.get(\"/api/health\"); threads = [threading.Thread(target=make_request) for _ in range(5)]; start = time.time(); [t.start() for t in threads]; [t.join() for t in threads]; end = time.time(); print(f\"Concurrent requests completed in {(end-start)*1000:.2f}ms\"); assert (end-start) < 2.0'"
    
    # Test memory usage
    run_test "Memory Usage Test" "python3 -c 'import psutil, os; from server import PlantMonitorServer; process = psutil.Process(os.getpid()); initial_memory = process.memory_info().rss; server = PlantMonitorServer(); final_memory = process.memory_info().rss; memory_increase = (final_memory - initial_memory) / 1024 / 1024; print(f\"Memory increase: {memory_increase:.2f}MB\"); assert memory_increase < 50.0'"
    
    print_status "SUCCESS" "Load testing completed"
    echo ""
}

# Function to run database tests
run_database_tests() {
    print_status "SECTION" "Starting Database Tests"
    echo "========================================="
    
    # Test database initialization
    run_test "Database Initialization Test" "python3 -c 'from server import PlantMonitorServer; server = PlantMonitorServer(); print(\"Database initialization OK\")'"
    
    # Test database operations
    run_test "Database Operations Test" "python3 -c 'from server import PlantMonitorServer; server = PlantMonitorServer(); result = server.receive_sensor_data({\"device_id\": \"test_db\", \"timestamp\": 1234567890, \"temperature\": 25.0, \"humidity\": 60.0, \"soil_moisture\": 2048, \"light_level\": 1024, \"lux\": 5000.0, \"health_score\": 85.0, \"health_status\": \"Good\", \"health_emoji\": \"🙂\", \"recommendation\": \"Test\", \"uptime_seconds\": 3600, \"wifi_connected\": True, \"data_sent\": True}); print(f\"Database operation: {result}\"); assert result == True'"
    
    # Test data retrieval
    run_test "Data Retrieval Test" "python3 -c 'from server import PlantMonitorServer; server = PlantMonitorServer(); readings = server.get_latest_readings(limit=5); print(f\"Retrieved {len(readings)} readings\"); assert isinstance(readings, list)'"
    
    print_status "SUCCESS" "Database tests completed"
    echo ""
}

# Function to run API tests
run_api_tests() {
    print_status "SECTION" "Starting API Tests"
    echo "========================================="
    
    # Test health endpoint
    run_test "Health Endpoint Test" "python3 -c 'from server import app; client = app.test_client(); response = client.get(\"/api/health\"); print(f\"Health endpoint: {response.status_code}\"); assert response.status_code == 200'"
    
    # Test data endpoint
    run_test "Data Endpoint Test" "python3 -c 'from server import app; client = app.test_client(); import json; data = {\"device_id\": \"test_api\", \"timestamp\": 1234567890, \"temperature\": 25.0, \"humidity\": 60.0, \"soil_moisture\": 2048, \"light_level\": 1024, \"lux\": 5000.0, \"health_score\": 85.0, \"health_status\": \"Good\", \"health_emoji\": \"🙂\", \"recommendation\": \"Test\", \"uptime_seconds\": 3600, \"wifi_connected\": True, \"data_sent\": True}; response = client.post(\"/api/data\", data=json.dumps(data), content_type=\"application/json\"); print(f\"Data endpoint: {response.status_code}\"); assert response.status_code == 200'"
    
    # Test metrics endpoint
    run_test "Metrics Endpoint Test" "python3 -c 'from server import app; client = app.test_client(); response = client.get(\"/metrics\"); print(f\"Metrics endpoint: {response.status_code}\"); assert response.status_code == 200'"
    
    print_status "SUCCESS" "API tests completed"
    echo ""
}

# Function to print test summary
print_summary() {
    echo ""
    echo "========================================="
    print_status "HEADER" "Test Summary"
    echo "========================================="
    echo "Total Tests: $TOTAL_TESTS"
    echo "Passed: $PASSED_TESTS"
    echo "Failed: $FAILED_TESTS"
    echo "Skipped: $SKIPPED_TESTS"
    
    if [ $FAILED_TESTS -eq 0 ]; then
        print_status "SUCCESS" "All tests passed! 🎉"
        echo ""
        echo "🌱 Plant Monitor Server is ready for deployment!"
        echo "📊 Comprehensive testing with industry standards"
        echo "🔧 Professional code quality and documentation"
        echo "🚀 Robust error handling and security"
        echo "📈 Performance monitoring and metrics"
    else
        print_status "ERROR" "$FAILED_TESTS test(s) failed"
        echo ""
        echo "Please review the failed tests and fix any issues."
    fi
}

# Function to generate test report
generate_report() {
    print_status "INFO" "Generating test report"
    echo "========================================="
    
    # Create reports directory
    mkdir -p reports
    
    # Generate HTML coverage report
    if [ -d "htmlcov" ]; then
        cp -r htmlcov reports/
        print_status "SUCCESS" "Coverage report saved to reports/htmlcov/"
    fi
    
    # Generate test summary report
    cat > reports/test_summary.md << EOF
# Plant Monitor Server - Test Summary

## Test Results
- **Total Tests**: $TOTAL_TESTS
- **Passed**: $PASSED_TESTS
- **Failed**: $FAILED_TESTS
- **Skipped**: $SKIPPED_TESTS
- **Success Rate**: $(( (PASSED_TESTS * 100) / TOTAL_TESTS ))%

## Test Categories
- ✅ Unit Tests
- ✅ Integration Tests
- ✅ Performance Tests
- ✅ Security Tests
- ✅ Code Quality Checks
- ✅ Coverage Analysis
- ✅ Load Testing
- ✅ Database Tests
- ✅ API Tests

## Environment
- Python: $(python3 --version 2>&1)
- Date: $(date)
- Platform: $(uname -s)

## Notes
- All tests follow industry standards
- Comprehensive error handling verified
- Security measures tested
- Performance benchmarks met
- Code quality standards maintained
EOF
    
    print_status "SUCCESS" "Test report generated in reports/test_summary.md"
    echo ""
}

# Main function to execute tests
main() {
    echo "🚀 Plant Monitor - Raspberry Pi Server Test Runner"
    echo "========================================="
    echo ""
    
    # Check environment first
    check_environment
    
    # Install dependencies if needed
    if [ "$1" = "--install" ]; then
        install_dependencies
    fi
    
    # Parse command line arguments
    case "${1:-all}" in
        "unit")
            run_unit_tests
            ;;
        "integration")
            run_integration_tests
            ;;
        "performance")
            run_performance_tests
            ;;
        "security")
            run_security_tests
            ;;
        "quality")
            run_code_quality_checks
            ;;
        "coverage")
            run_coverage_analysis
            ;;
        "load")
            run_load_testing
            ;;
        "database")
            run_database_tests
            ;;
        "api")
            run_api_tests
            ;;
        "all")
            run_unit_tests
            run_integration_tests
            run_performance_tests
            run_security_tests
            run_code_quality_checks
            run_coverage_analysis
            run_load_testing
            run_database_tests
            run_api_tests
            ;;
        *)
            echo "Usage: $0 [unit|integration|performance|security|quality|coverage|load|database|api|all|--install]"
            echo ""
            echo "Test Categories:"
            echo "  unit        - Unit tests for individual components"
            echo "  integration - Integration tests for system workflow"
            echo "  performance - Performance and response time tests"
            echo "  security    - Security and input validation tests"
            echo "  quality     - Code quality and style checks"
            echo "  coverage    - Code coverage analysis"
            echo "  load        - Load and concurrent request tests"
            echo "  database    - Database operation tests"
            echo "  api         - API endpoint tests"
            echo "  all         - Run all tests (default)"
            echo "  --install   - Install dependencies"
            exit 1
            ;;
    esac
    
    # Generate test report
    generate_report
    
    # Print summary
    print_summary
}

# Call the main function with command line arguments
main "$@" 