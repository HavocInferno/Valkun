# RTG 
# (C) 2019 Paul Preissner

project( Valkun CXX )
cmake_minimum_required (VERSION 3.0 )

file(GLOB_RECURSE RES_FILES *.frag *.vert *.glsl)
file(GLOB_RECURSE BAT_FILES *.bat)
file(GLOB_RECURSE CXX_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE CXX_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

add_executable( Valkun ${CXX_FILES} ${CXX_HEADERS} ${RES_FILES} ${BAT_FILES})

####################################################################################

#add_definitions( /bigobj )

####################################################################################

set( GLFW_DIR C:/software/c++/glfw-3.2.1.bin.WIN64 )
set( VULKAN_DIR C:/VulkanSDK/1.1.85.0 )
set( STB_DIR C:/software/c++/stb )

include_directories( 
	${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
include_directories( ${VULKAN_DIR}/Include )
include_directories( ${GLFW_DIR}/include )
include_directories( ${VULKAN_DIR}/Third-Party/Include )
include_directories( ${STB_DIR} )


####################################################################################

#target_compile_features(Valkun PRIVATE cxx_std_11)

####################################################################################

set( GLFW_LIB ${GLFW_DIR}/lib-vc2015 )
target_link_libraries( Valkun ${GLFW_LIB}/glfw3.lib )

set( VULKAN_LIB ${VULKAN_DIR}/Lib )
target_link_libraries( Valkun ${VULKAN_LIB}/vulkan-1.lib )

####################################################################################

add_custom_command(TARGET ${PROJECT_NAME}
    PRE_BUILD
    COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/src/
    COMMAND call runCompiler.bat
	COMMAND ${CMAKE_COMMAND} -E copy vert.spv ${PROJECT_BINARY_DIR}/Resources/.
	COMMAND ${CMAKE_COMMAND} -E copy frag.spv ${PROJECT_BINARY_DIR}/Resources/.
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Resources ${PROJECT_BINARY_DIR}/Resources
	VERBATIM
)