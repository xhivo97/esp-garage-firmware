cmake_minimum_required(VERSION 3.23)

set(EXTRA_COMPONENT_DIRS src esp-rainmaker/components/esp-insights/components esp-rainmaker/components esp-rainmaker/examples/common)

set(PROJECT_VER "0.2.0")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(garage-opener)