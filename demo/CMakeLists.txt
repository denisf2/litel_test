# CMakeList.txt : CMake project for demo, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (demo "demo.cpp" "demo.h" "S1apDb.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET demo PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
