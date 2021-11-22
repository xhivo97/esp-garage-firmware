# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
set(PROJECT_VER "0.1.1")
set(RMAKER_PATH ${PROJECT_PATH})
set(EXTRA_COMPONENT_DIRS ./esp-rainmaker/components ./esp-rainmaker/examples/common ./esp-rainmaker/components/esp-insights/components)
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(main)