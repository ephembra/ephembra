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

# Find GLFW3 library
find_package(glfw3)

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
    list(APPEND GLFW_LIBS_ALL ${GLFW_LIBRARIES} ${EXTRA_LIBS})
    if(UNIX AND NOT APPLE)
        find_package(X11 REQUIRED)
        list(APPEND GLFW_LIBS_ALL X11::X11)
        include_directories(${X11_INCLUDE_DIR})
    endif()
else ()
    list(APPEND GLFW_LIBS_ALL glfw ${EXTRA_LIBS})
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
    add_definitions(-DHAVE_GLAD)
    include_directories(${GLAD_INCLUDE_DIRS})
    link_directories(${GLAD_LIBRARY_DIR})
    list(APPEND GLFW_LIBS_ALL ${GLAD_LIBRARIES})
else ()
    list(APPEND GLFW_LIBS_ALL ${OPENGL_opengl_LIBRARY})
endif ()

# Find FreeType2 library
find_package(Freetype REQUIRED)
list(APPEND GLFW_LIBS_ALL Freetype::Freetype)
include_directories(${FREETYPE_INCLUDE_DIRS})

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

find_package(Python COMPONENTS Interpreter REQUIRED)

string(CONCAT ephembra_data_url
    "https://media.githubusercontent.com/media/ephembra/"
    "ephembra-data/refs/heads/trunk/data/DE440Coeff.bin")
set(ephembra_data_file ${CMAKE_BINARY_DIR}/resources/data/DE440Coeff.bin)
add_custom_command(OUTPUT ${ephembra_data_file}
    COMMAND ${Python_EXECUTABLE} ${PROJECT_SOURCE_DIR}/scripts/fetch.py
    ARGS ${ephembra_data_url} ${ephembra_data_file})

set(MATIO_SHARED OFF CACHE BOOL "")
set(MATIO_MAT73 OFF CACHE BOOL "Enable MATLAB 7.3 files" FORCE)
set(MATIO_WITH_HDF5 OFF CACHE BOOL "Enable HDF5 library" FORCE)
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
add_library(ephembra
  lib/color.c
  lib/data.c
  lib/date.c
  lib/ephem.c
  lib/iau2006.c
  lib/osutil.c
  lib/screen.c
  ${ephembra_data_file})

add_executable(demo util/demo.c)
target_link_libraries(demo PRIVATE ephembra)

add_executable(convert util/convert.c)
target_link_libraries(convert PRIVATE z matio ephembra)

add_executable(search util/search.c)
target_link_libraries(search PRIVATE ephembra ${EXTRA_LIBS})

set(gldemo_fonts
  resources/fonts/fontawesome-webfont.ttf
  resources/fonts/DejaVuSans.ttf
  resources/fonts/DejaVuSansMono.ttf)
set(gldemo_images
  resources/images/Saturn.png
  resources/images/Uranus.png
  resources/images/Sun.png
  resources/images/Moon.png
  resources/images/Mars.png
  resources/images/Earth.png
  resources/images/Jupiter.png
  resources/images/Neptune.png
  resources/images/Mercury.png
  resources/images/Pluto.png
  resources/images/Venus.png)

foreach(rsrc ${gldemo_fonts})
  file(COPY ${CMAKE_SOURCE_DIR}/${rsrc}
       DESTINATION ${CMAKE_BINARY_DIR}/resources/fonts/)
endforeach()
foreach(rsrc ${gldemo_images})
  file(COPY ${CMAKE_SOURCE_DIR}/${rsrc}
       DESTINATION ${CMAKE_BINARY_DIR}/resources/images/)
endforeach()

list(PREPEND GLFW_LIBS_ALL z matio ephembra nanovg imgui)
add_executable(gldemo
    src/glui.cc
    src/gldemo.c
    src/stb_image.c
    src/nanovg_gles3.c)
gl_cprogram(gldemo)
