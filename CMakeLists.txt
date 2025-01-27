cmake_minimum_required(VERSION 3.0)
project(ClothSim)

option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(Template/Vendor/glfw)

option(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
option(ASSIMP_BUILD_SAMPLES OFF)
option(ASSIMP_BUILD_TESTS OFF)
add_subdirectory(Template/Vendor/assimp)

option(BUILD_BULLET2_DEMOS OFF)
option(BUILD_CPU_DEMOS OFF)
option(BUILD_EXTRAS OFF)
option(BUILD_OPENGL3_DEMOS OFF)
option(BUILD_UNIT_TESTS OFF)
add_subdirectory(Template/Vendor/bullet)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	set (CMAKE_CXX_STANDARD 17)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++11")
    if(NOT WIN32)
        set(GLAD_LIBRARIES dl)
    endif()
endif()

include_directories(Template/Headers/
                    Template/Vendor/assimp/include/
                    Template/Vendor/bullet/src/
                    Template/Vendor/glad/include/
                    Template/Vendor/glfw/include/
                    Template/Vendor/glm/
                    Template/Vendor/stb/
					Template/Sources/OpenCL/include
					Template/imgui/)
					
file(GLOB IMGUI Template/imgui/*.h
				Template/imgui/backends/*.h
                Template/imgui/*.cpp
				Template/imgui/backends/*.cpp)
file(GLOB VENDORS_SOURCES Template/Vendor/glad/src/glad.c)
file(GLOB PROJECT_HEADERS Template/Headers/*.hpp)
file(GLOB PROJECT_SOURCES Template/Sources/*.cpp)
file(GLOB PROJECT_SHADERS Template/Shaders/*.comp
                          Template/Shaders/*.frag
                          Template/Shaders/*.geom
                          Template/Shaders/*.vert
						  Template/Shaders/*.vs
						  Template/Shaders/*.fs
						  Template/Sources/gpu_src/*.cl)
file(GLOB PROJECT_CONFIGS CMakeLists.txt
                          Readme.md
                         .gitattributes
                         .gitignore
                         .gitmodules)

source_group("Headers" FILES ${PROJECT_HEADERS})
source_group("Shaders" FILES ${PROJECT_SHADERS})
source_group("Sources" FILES ${PROJECT_SOURCES})
source_group("Vendors" FILES ${VENDORS_SOURCES})
source_group("Imgui" FILES ${IMGUI})

link_directories(Template/Sources/OpenCL/lib)

add_definitions(-DGLFW_INCLUDE_NONE
                -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS}
                               ${PROJECT_SHADERS} ${PROJECT_CONFIGS} ${IMGUI}
                               ${VENDORS_SOURCES})
target_link_libraries(${PROJECT_NAME} assimp glfw
                      ${GLFW_LIBRARIES} ${GLAD_LIBRARIES}
                      BulletDynamics BulletCollision LinearMath opencl)
target_link_libraries(${PROJECT_NAME} opengl32)
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME})

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Template/Shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>
    DEPENDS ${PROJECT_SHADERS})
