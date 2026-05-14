cmake_minimum_required(VERSION 3.16)

# 1. 파일 이름을 리스트에 등록
set(project_bin_sources "https_server.crt")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(IncubatorV1)

# 2. 바이너리 임베딩 명령 (확인)
target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "${project_bin_sources}" TEXT)