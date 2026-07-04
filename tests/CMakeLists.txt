# tests/CMakeLists.txt
# Unit and integration tests will be added in future sprints.
# Enable with: cmake -DBUILD_TESTS=ON ..

option(BUILD_TESTS "Build test suite" OFF)

if(BUILD_TESTS)
    enable_testing()
    # Future: add_subdirectory for test modules (e.g. GTest, Catch2)
endif()