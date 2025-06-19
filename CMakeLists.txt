cmake_minimum_required(VERSION 3.10)
project(crop_tracking)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/include)
find_package(OpenCV REQUIRED)

add_executable(crop_tracking
    src/main.cpp
    src/tracker.cpp        # <-- add this
)

target_link_libraries(crop_tracking ${OpenCV_LIBS})

# ---------- NEW ----------  make "cmake --install" put the EXE in <prefix>/bin
install(TARGETS crop_tracking RUNTIME DESTINATION bin)

