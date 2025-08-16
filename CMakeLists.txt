cmake_minimum_required (VERSION 3.12)

project(ephembra VERSION 0.0.1)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(CheckLibraryExists)
check_library_exists(m sqrtf "" HAVE_LIB_M)
if (HAVE_LIB_M)
    set(EXTRA_LIBS ${EXTRA_LIBS} m)
endif (HAVE_LIB_M)

find_package(PkgConfig)
pkg_check_modules(GLFW3 glfw3)

# Find OpenGL library
include(FindOpenGL)

macro(add_compiler_flag)
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARGN}")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARGN}")
endmacro(add_compiler_flag)

macro(add_linker_flag)
  set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ${ARGN}")
endmacro(add_linker_flag)

# user configurable options
option(EXTERNAL_GLFW "Use external GLFW project" ON)
option(EXTERNAL_GLAD "Use external GLAD project" ON)
option(ENABLE_ASAN "Enable ASAN" OFF)
option(ENABLE_MSAN "Enable MSAN" OFF)
option(ENABLE_UBSAN "Enable UBSAN" OFF)

if(ENABLE_ASAN)
  add_compiler_flag(-fsanitize=address)
  add_linker_flag(-fsanitize=address)
endif()

if(ENABLE_MSAN)
  add_compiler_flag(-fsanitize=memory)
  add_linker_flag(-fsanitize=memory)
endif()

if(ENABLE_UBSAN)
  add_compiler_flag(-fsanitize=undefined)
  add_linker_flag(-fsanitize=undefined)
endif()

message(STATUS "Build external GLFW = ${EXTERNAL_GLFW}")
message(STATUS "Build external GLAD = ${EXTERNAL_GLAD}")

# Support for external glfw
if (EXTERNAL_GLFW)
    include(ExternalProject)
    ExternalProject_Add(
        GLFW
        GIT_REPOSITORY "https://github.com/glfw/glfw.git"
        GIT_TAG "3.3-stable"
        SOURCE_DIR "${CMAKE_BINARY_DIR}/third_party/glfw3"
        STEP_TARGETS build
        EXCLUDE_FROM_ALL TRUE
        CMAKE_ARGS -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF
                   -DGLFW_BUILD_DOCS=OFF -DGLFW_INSTALL=OFF
    )
    set(GLFW_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/third_party/glfw3/include")
    set(GLFW_LIBRARY_DIR "${CMAKE_BINARY_DIR}/GLFW-prefix/src/GLFW-build/src/")
    set(GLFW_LIBRARIES glfw3 pthread dl)
    include_directories(${GLFW_INCLUDE_DIRS})
    link_directories(${GLFW_LIBRARY_DIR})
    list(APPEND GLFW_LIBS_ALL ${GLFW_LIBRARIES} ${EXTRA_LIBS} X11)
else ()
    add_definitions(${GLFW3_CFLAGS})
    list(APPEND GLFW_LIBS_ALL ${GLFW3_LDFLAGS} ${EXTRA_LIBS})
endif ()

# Support for external glad
if (EXTERNAL_GLAD)
    include(ExternalProject)
    ExternalProject_Add(
        GLAD
        GIT_REPOSITORY "https://github.com/Dav1dde/glad.git"
        SOURCE_DIR "${CMAKE_BINARY_DIR}/third_party/glad"
        STEP_TARGETS build
        EXCLUDE_FROM_ALL TRUE
        CMAKE_ARGS -DGLAD_EXPORT=OFF
    )
    set(GLAD_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/GLAD-prefix/src/GLAD-build/include/")
    set(GLAD_LIBRARY_DIR "${CMAKE_BINARY_DIR}/GLAD-prefix/src/GLAD-build/")
    set(GLAD_LIBRARIES glad)
    include_directories(${GLAD_INCLUDE_DIRS})
    link_directories(${GLAD_LIBRARY_DIR})
    list(APPEND GLFW_LIBS_ALL ${GLAD_LIBRARIES})
else ()
    list(APPEND GLFW_LIBS_ALL ${OPENGL_opengl_LIBRARY})
endif ()

# Find FreeType2 library
find_package(PkgConfig)
pkg_check_modules(FT2 freetype2)
if (${FT2_FOUND})
  list(APPEND GLFW_LIBS_ALL ${FT2_LIBRARIES})
else()
  message(ERROR "can't find freetype2")
endif()

# macro to add opengl example
macro(gl_cprogram prog)
    target_link_libraries(${prog} ${GLFW_LIBS_ALL})
    if (EXTERNAL_GLAD)
        target_compile_definitions(${prog} PRIVATE -DHAVE_GLAD)
    endif ()
    if (EXTERNAL_GLFW)
        add_dependencies(${prog} GLFW-build)
    endif ()
    if (EXTERNAL_GLAD)
        add_dependencies(${prog} GLAD-build)
    endif ()
endmacro()

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

include_directories(third_party/nanovg/src)
add_library(nanovg STATIC third_party/nanovg/src/nanovg.c)
target_compile_definitions(nanovg PRIVATE FONS_USE_FREETYPE)

include_directories(third_party/stb)

include_directories(${FT2_INCLUDE_DIRS})

include_directories(third_party/imgui)
include_directories(third_party/imgui/backends)
set(IMGUI_SOURCES
  third_party/imgui/backends/imgui_impl_glfw.cpp
  third_party/imgui/backends/imgui_impl_opengl3.cpp
  third_party/imgui/imgui.cpp
  third_party/imgui/imgui_demo.cpp
  third_party/imgui/imgui_draw.cpp
  third_party/imgui/imgui_tables.cpp
  third_party/imgui/imgui_widgets.cpp)
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)
add_library(imgui STATIC ${IMGUI_SOURCES})

include_directories(include)
add_library(ephembra src/ephembra.c
    ${ephembra_data_file})

add_executable(demo src/demo.c)
target_link_libraries(demo PRIVATE ephembra)

add_executable(convert src/convert.c)
target_link_libraries(convert PRIVATE z matio ephembra)

list(APPEND GLFW_LIBS_ALL z matio ephembra nanovg imgui ${FT2_LIBRARIES})
add_executable(gldemo src/gldemo.cc)
gl_cprogram(gldemo)
