include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)
message(STATUS "Local .env.cmake: ${LOCAL_ENV}")

cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Run)

project(MyProject VERSION 1.0)

find_package(glfw3 REQUIRED)
find_package(TIFF REQUIRED)

# ===== GLAD ====================================================== #

add_library(GLAD STATIC ${GLAD_PATH}/src/gl.c)
target_include_directories(GLAD PUBLIC ${GLAD_PATH}/include)

# ===== ImGui (GLFW with OpenGL) + ImPlot ========================= #

add_library(ImGui_GLFW_OpenGL STATIC
  ${IMGUI_PATH}/imgui.cpp
  ${IMGUI_PATH}/imgui_draw.cpp
  ${IMGUI_PATH}/imgui_tables.cpp
  ${IMGUI_PATH}/imgui_widgets.cpp
  ${IMGUI_PATH}/backends/imgui_impl_glfw.cpp
  ${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp
  # ${IMPLOT_PATH}/implot.cpp
  # ${IMPLOT_PATH}/implot_items.cpp
)

target_include_directories(ImGui_GLFW_OpenGL PUBLIC
  ${IMGUI_PATH}
  # ${IMPLOT_PATH}
)

# ===== My utils ================================================== #

file(GLOB_RECURSE UTILS_SOURCES ${UTILS_PATH}/src/*.cpp)
add_library(Utils STATIC ${UTILS_SOURCES})

target_include_directories(Utils PUBLIC ${UTILS_PATH}/include)

# ===== Main ====================================================== #

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_options(${PROJECT_NAME} PRIVATE -Wall)
target_compile_options(${PROJECT_NAME} PRIVATE -isystem ${PROFILER_PATH}/include)

target_precompile_headers(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src/pch.hpp)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${STB_PATH}/include
  ${GLM_PATH}/include
  ${TOL_PATH}/include
  ${PROFILER_PATH}/include
  ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw TIFF::TIFF GLAD ImGui_GLFW_OpenGL Utils)

