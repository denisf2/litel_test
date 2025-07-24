# CMakeList.txt : CMake project for demo, include source and define
# project specific logic here.
#

set(gtest_force_shared_crt on)

# Add source to this project's executable.
add_executable (demo "demo.cpp" "demo.h" "S1apDb.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET demo PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
# Add your test executable

# Download and unpack googletest at configure time
include(FetchContent)
FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG 58d77fa8070e8cec2dc1ed015d66b454c8d78850 # release-1.12.1
	OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Create a test executable
add_executable(Demo_Tests
  "S1apDb.cpp"
  "S1apDb.h"
  "tests/test_1.cpp"  # Your test files
  # Add more test files here
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
	set_property(TARGET Demo_Tests PROPERTY CXX_STANDARD 20)
endif()

# Link with GTest
target_link_libraries(Demo_Tests PRIVATE
  gtest_main  # Links GTest and provides a main() function
  # Add other dependencies if needed (e.g., your library)
)

# Register tests for CTest (so they appear in Test Explorer)
include(GoogleTest)
gtest_discover_tests(Demo_Tests)