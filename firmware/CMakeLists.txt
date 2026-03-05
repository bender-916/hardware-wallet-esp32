# Hardware Wallet ESP32 - Main CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(hardware-wallet)

# Include components
set(EXTRA_COMPONENT_DIRS "components")
