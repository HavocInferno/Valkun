# RTG 
# (C) 2019 Paul Preissner

project( Valkun CXX )
cmake_minimum_required (VERSION 3.0 )

file(GLOB_RECURSE RES_FILES *.frag *.vert *.glsl)
file(GLOB_RECURSE BAT_FILES *.bat)

add_executable( Valkun ${CMAKE_CURRENT_SOURCE_DIR}/src/Valkun.cpp ${RES_FILES} ${BAT_FILES})

####################################################################################

#add_definitions( /bigobj )

####################################################################################

include_directories( 
	${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
include_directories( C:/VulkanSDK/1.1.85.0/Include )
include_directories( C:/software/c++/glfw-3.2.1.bin.WIN64/include )

####################################################################################

#target_compile_features(Valkun PRIVATE cxx_std_11)

####################################################################################

set( GLFW_LIB C:/software/c++/glfw-3.2.1.bin.WIN64/lib-vc2015 )
target_link_libraries( Valkun ${GLFW_LIB}/glfw3.lib )

set( VULKAN_LIB C:/VulkanSDK/1.1.85.0/Lib )
target_link_libraries( Valkun ${VULKAN_LIB}/vulkan-1.lib )

####################################################################################

add_custom_command(TARGET ${PROJECT_NAME}
    PRE_BUILD
    COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/src/
    COMMAND call runCompiler.bat
	COMMAND ${CMAKE_COMMAND} -E copy vert.spv ${PROJECT_BINARY_DIR}/Resources/.
	COMMAND ${CMAKE_COMMAND} -E copy frag.spv ${PROJECT_BINARY_DIR}/Resources/.
	VERBATIM
)