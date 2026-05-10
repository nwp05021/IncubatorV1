cmake_minimum_required(VERSION 3.16)

# Arduino 컴포넌트를 먼저 로드하기 위한 설정
# set(EXTRA_COMPONENT_DIRS "D:/_MyProjects/esp_shared_components/arduino")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(IncubatorV1)
