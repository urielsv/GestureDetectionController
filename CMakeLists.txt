#cmake -S . -B build -A x64 -DNATNET_SDK_DIR="c:\Users\itba\Documents\NatNetSDK"
#cmake --build build --config Release
cmake_minimum_required(VERSION 3.16)
project(NatNetCapture CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Configurá la ruta al NatNet SDK (pasala por -DNATNET_SDK_DIR=...) ----
if(NOT DEFINED NATNET_SDK_DIR)
    message(FATAL_ERROR "Debes definir NATNET_SDK_DIR (ruta a la carpeta del NatNet SDK).")
endif()

# Headers y libs del SDK
set(NATNET_INCLUDE_DIR "${NATNET_SDK_DIR}/include")
# Elegimos 64-bit
set(NATNET_LIB_DIR     "${NATNET_SDK_DIR}/lib/x64")

# Ejecutable
add_executable(NatNetCapture
    src/cpp/main.cpp
)

target_include_directories(NatNetCapture PRIVATE "${NATNET_INCLUDE_DIR}")
target_link_directories(NatNetCapture PRIVATE "${NATNET_LIB_DIR}")

# NatNetLib + dependencias WinSock/IP Helper
if(WIN32)
    target_link_libraries(NatNetCapture PRIVATE NatNetLib Ws2_32 Iphlpapi)
else()
    # En Linux/Mac, si usás el C API, podés necesitar -lpthread, etc.
    target_link_libraries(NatNetCapture PRIVATE NatNetLib pthread)
endif()


# Copiar la DLL al lado del .exe (solo Windows)
if(WIN32)
    add_custom_command(TARGET NatNetCapture POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${NATNET_LIB_DIR}/NatNetLib.dll"
                $<TARGET_FILE_DIR:NatNetCapture>)
endif()
