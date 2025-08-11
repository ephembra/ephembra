cmake_minimum_required (VERSION 3.12)

project(ephembra VERSION 0.0.1)

set(CMAKE_C_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#
# ephembra
#

string(CONCAT ephembra_data_url
    "https://media.githubusercontent.com/media/michaeljclark/"
    "ephembra-data/refs/heads/trunk/data/DE440Coeff.bin")
set(ephembra_data_file ${PROJECT_BINARY_DIR}/data/DE440Coeff.bin)
add_custom_command(OUTPUT ${ephembra_data_file}
    COMMAND ${PROJECT_SOURCE_DIR}/scripts/fetch.py
    ARGS ${ephembra_data_url} ${ephembra_data_file})

add_subdirectory(third_party/matio)
include_directories(third_party/matio/src)

include_directories(include)
add_library(ephembra src/ephembra.c ${ephembra_data_file})

add_executable(demo src/demo.c)
target_link_libraries(demo PRIVATE ephembra)

add_executable(convert src/convert.c)
target_link_libraries(convert PRIVATE z matio ephembra)
