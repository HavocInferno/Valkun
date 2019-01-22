# RTG 
# (C) 2019 Paul Preissner

project( Valkun CXX )
cmake_minimum_required (VERSION 3.0 )

add_executable( Valkun ${CMAKE_CURRENT_SOURCE_DIR}/src/Valkun.cpp )

####################################################################################

#add_definitions( /bigobj )

####################################################################################

include_directories( 
	${CMAKE_CURRENT_SOURCE_DIR}/include>
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
include_directories( C:/VulkanSDK/1.1.82.1/Include )
include_directories( T:/software/developer/c++/glfw-3.2.1.bin.WIN64/include )

####################################################################################

#target_compile_features(Valkun PRIVATE cxx_std_11)

####################################################################################

set( GLFW_LIB T:/software/developer/c++/glfw-3.2.1.bin.WIN64/lib-vc2015 )
target_link_libraries( Valkun ${GLFW_LIB}/glfw3.lib )

set( VULKAN_LIB C:/VulkanSDK/1.1.82.1/Lib )
target_link_libraries( Valkun ${VULKAN_LIB}/vulkan-1.lib )