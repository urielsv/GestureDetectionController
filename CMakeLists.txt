cmake_minimum_required(VERSION 3.10)
project(GestureDetectionController VERSION 0.1.0)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(NATNET_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/NatNetSDK3") # Setup on Motive Lab PC
set(NATNET_INCLUDE_DIR "${NATNET_SDK_DIR}/include")
set(NATNET_LIB_DIR "${NATNET_SDK_DIR}/lib/x64")

# Include directories
include_directories(${NATNET_INCLUDE_DIR})
include_directories(include)

# Add executable
add_executable(GestureDetectionController src/main.cpp)

# Link with NatNet libraries
if(WIN32)
    target_link_libraries(GestureDetectionController
        ${NATNET_LIB_DIR}/NatNetLib.lib
        ws2_32.lib # Windows socket library
    )
    
    # Copy DLL to output directory
    add_custom_command(TARGET GestureDetectionController POST_BUILD
     COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${NATNET_LIB_DIR}/NatNetLib.dll"
       $<TARGET_FILE_DIR:GestureDetectionController>
    )
endif()

# Install
install(TARGETS GestureDetectionController DESTINATION bin)
