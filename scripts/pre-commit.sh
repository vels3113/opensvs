#!/bin/bash

set -e

echo "=== Pre-commit Hook ==="
echo ""

# Get list of all staged files
STAGED_FILES=$(git diff --cached --name-only --diff-filter=ACM)

# Get list of staged C++ files
STAGED_CPP_FILES=$(echo "$STAGED_FILES" | grep -E '\.(cpp|cc|cxx|h|hpp)$' || true)

# Check if staged files include any build-relevant files
STAGED_RELEVANT_FILES=$(echo "$STAGED_FILES" | grep -E '^(resources/fixtures|src/|tests/|CMakeLists\.txt|CMakePresets\.json|configure\.sh)' || true)

# ============================================================================
# 1. Run clang-format on staged files
# ============================================================================
if [ -n "$STAGED_CPP_FILES" ]; then
    echo "Running clang-format on staged files..."

    # Run clang-format with automatic fixes on staged files
    for file in $STAGED_CPP_FILES; do
        if [ -f "$file" ]; then
            echo "  Formatting: $file"
            # Format the file
            clang-format -i "$file"
            # Only add changes in the file that were already staged
            git add -p "$file" 2>/dev/null || git add "$file"
        fi
    done

    echo "clang-format checks passed"
    echo ""
else
    echo "No staged C++ files to format with clang-format"
    echo ""
fi

# Check if we should skip build and tests
if [ -z "$STAGED_RELEVANT_FILES" ]; then
    echo "No relevant source files staged (src/, tests/, resources/fixtures/, CMakeLists.txt, CMakePresets.json, configure.sh)"
    echo "Skipping build and test steps"
    echo ""
    echo "=== All pre-commit checks passed! ==="
    echo ""
    exit 0
fi

# ============================================================================
# 2. Build and test Release version
# ============================================================================
echo "Building and testing Release version..."

./configure.sh --preset release

cmake --build build/release -j$(nproc)

# Run smoke test if available
if [ -f "tests/smoke_offscreen.sh" ]; then
    echo "Running smoke test..."
    ./tests/smoke_offscreen.sh
fi

# Run QtTest
echo "Running Release tests..."
ctest --test-dir build/release --output-on-failure

echo "Release build and tests passed"
echo ""

# ============================================================================
# 3. Build and test sanitizers (asan and ubsan)
# ============================================================================
for preset in asan ubsan; do
    echo "Building and testing $preset..."

    cmake --preset $preset --fresh

    cmake --build --preset $preset --target tests -j$(nproc)

    echo "Running $preset tests..."
    ctest --preset $preset --output-on-failure

    echo "$preset build and tests passed"
    echo ""
done

# ============================================================================
# Success
# ============================================================================
echo "=== All pre-commit checks passed! ==="
echo ""
