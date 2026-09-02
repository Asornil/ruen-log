cmake_minimum_required(VERSION 3.15)
project(r_log LANGUAGES C CXX) 

# 默认 Debug 构建，保证可断点调试；可用 -DCMAKE_BUILD_TYPE=Release 覆盖
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "构建类型" FORCE)
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    src
    drivers
)

set(SOURCES
    drivers/vendor_windows.c
    drivers/vendor_linux.c
    drivers/vendor_stm32.c
    drivers/vendor_esp32.c

    src/Rlog.c
    
    example.c
)

add_executable(r_log_exe ${SOURCES})