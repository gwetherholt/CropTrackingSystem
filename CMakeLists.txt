# CMakeLists.txt  (top-level)
cmake_minimum_required(VERSION 3.10)
project(crop_tracking)
set(CMAKE_CXX_STANDARD 17)

# add *both* header folders
include_directories(
    ${CMAKE_SOURCE_DIR}/include      # json.hpp  tracker.hpp
    ${CMAKE_SOURCE_DIR}/src          # for any in-source headers
)

find_package(OpenCV REQUIRED)
add_executable(crop_tracking
    src/main.cpp
    src/tracker.cpp
)

# --- NEW: make “cmake --install” copy the exe -----------------
install(
    TARGETS crop_tracking
    RUNTIME DESTINATION bin   # goes to  /stage/bin/crop_tracking
)

target_link_libraries(crop_tracking ${OpenCV_LIBS})