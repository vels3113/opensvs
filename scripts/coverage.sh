#!/bin/bash

set -e

echo "=== Code Coverage Generation ==="
echo ""

# Configuration
BUILD_DIR="build/coverage"
COVERAGE_DIR=".coverage-report"
SOURCE_DIR="$(pwd)"

# Clean previous coverage data
echo "Cleaning previous coverage data..."
find . -name "*.gcda" -delete
rm -rf "$COVERAGE_DIR"

# Configure with coverage preset
echo "Configuring with coverage preset..."
cmake --preset coverage

# Build tests
echo "Building tests..."
cmake --build --preset coverage --target tests

# Run tests to generate coverage data
echo "Running tests..."
ctest --preset coverage --output-on-failure

# Check if lcov is installed
if ! command -v lcov &> /dev/null; then
    echo "Error: lcov is not installed. Please install it with:"
    echo "  sudo apt-get install lcov"
    exit 1
fi

# Check if genhtml is installed
if ! command -v genhtml &> /dev/null; then
    echo "Error: genhtml is not installed. Please install it with:"
    echo "  sudo apt-get install lcov"
    exit 1
fi

# Capture coverage data
echo "Capturing coverage data..."
lcov --capture \
     --directory "$BUILD_DIR" \
     --output-file .coverage.info \
     --rc branch_coverage=1

# Filter out system headers and test files
echo "Filtering coverage data..."
lcov --remove .coverage.info \
     '/usr/*' \
     '*/tests/*' \
     '*/build/*' \
     --output-file .coverage_filtered.info \
     --rc branch_coverage=1 \
     --ignore-errors unused

# Generate HTML report
echo "Generating HTML report..."
genhtml .coverage_filtered.info \
        --output-directory "$COVERAGE_DIR" \
        --title "OpenSVS Code Coverage" \
        --legend \
        --show-details \
        --rc branch_coverage=1

# Display summary
echo ""
echo "=== Coverage Summary ==="
lcov --summary .coverage_filtered.info --rc branch_coverage=1

echo ""
echo "HTML coverage report generated in: $COVERAGE_DIR/index.html"
echo "To view the report, open: file://$(pwd)/$COVERAGE_DIR/index.html"
echo ""
