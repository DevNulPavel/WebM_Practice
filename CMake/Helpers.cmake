
# Информация о текущих настройках
function(print_current_cmake_config)
	# https://cmake.org/Wiki/CMake_Useful_Variables
	message("******** Project compilation settings ***********")
	message("System = ${CMAKE_SYSTEM}")
	message("System name = ${CMAKE_SYSTEM_NAME}")
	message("System version = ${CMAKE_SYSTEM_VERSION}")
	message("CMake Generator = ${CMAKE_GENERATOR}")
	message("CMake build type = ${CMAKE_BUILD_TYPE}")
	message("CC compiler = ${CMAKE_C_COMPILER}")
	message("CXX compiler = ${CMAKE_CXX_COMPILER}")
	message("CC Flags = ${CMAKE_C_FLAGS}")
	message("CXX Flags = ${CMAKE_CXX_FLAGS}")
endfunction()