cmake_minimum_required (VERSION 2.8)

set(PROJECT_NAME libvpx)
project(${PROJECT_NAME})

# Search setup
set(FILE_SEARCH_FOLDERS "${CMAKE_CURRENT_SOURCE_DIR}"
						# vpx_scale
						"${CMAKE_CURRENT_SOURCE_DIR}/vpx_scale"
						"${CMAKE_CURRENT_SOURCE_DIR}/vpx_scale/generic"
						# vpx_mem
						"${CMAKE_CURRENT_SOURCE_DIR}/vpx_mem"
						"${CMAKE_CURRENT_SOURCE_DIR}/vpx_mem/include"
						# vpx_ports
						"${CMAKE_CURRENT_SOURCE_DIR}/vpx_ports"
						# vpx_util
						"${CMAKE_CURRENT_SOURCE_DIR}/vpx_util"
						# vpx_dsp
						"${CMAKE_CURRENT_SOURCE_DIR}/vpx_dsp"
						# vpx
						"${CMAKE_CURRENT_SOURCE_DIR}/vpx"
						"${CMAKE_CURRENT_SOURCE_DIR}/vpx/internal"
						"${CMAKE_CURRENT_SOURCE_DIR}/vpx/src"
						# vp8
						"${CMAKE_CURRENT_SOURCE_DIR}/vp8"
						"${CMAKE_CURRENT_SOURCE_DIR}/vp8/common"
						"${CMAKE_CURRENT_SOURCE_DIR}/vp8/common/generic"
						"${CMAKE_CURRENT_SOURCE_DIR}/vp8/decoder"
						"${CMAKE_CURRENT_SOURCE_DIR}/vp8/encoder"
						# vp9
						"${CMAKE_CURRENT_SOURCE_DIR}/vp9"
						"${CMAKE_CURRENT_SOURCE_DIR}/vp9/common"
						"${CMAKE_CURRENT_SOURCE_DIR}/vp9/decoder"
						"${CMAKE_CURRENT_SOURCE_DIR}/vp9/encoder"
						# third_party
						"${CMAKE_CURRENT_SOURCE_DIR}/third_party/libyuv"
						"${CMAKE_CURRENT_SOURCE_DIR}/third_party/libyuv/include"
						"${CMAKE_CURRENT_SOURCE_DIR}/third_party/libyuv/source")

# Ignored files
set(IGNORE_FILES "mfqe.c"
				 "vp9_mfqe.h"
				 "vp9_mfqe.c"
				 "error_concealment.c"
				 "mr_dissim.c"
				 "vp9_denoiser.c")

# Sources
set(SOURCES "")
foreach(SOURCE_SERACH_FOLDER ${FILE_SEARCH_FOLDERS})
	# search files
	file(GLOB FILES_IN_FOLDER "${SOURCE_SERACH_FOLDER}/*.h"
							  "${SOURCE_SERACH_FOLDER}/*.c"
							  "${SOURCE_SERACH_FOLDER}/*.cpp")
	# concatenate in one
	set(SOURCES ${SOURCES} ${FILES_IN_FOLDER})

	# ignore files
	foreach(IGNORE_FILE ${IGNORE_FILES})
		list(REMOVE_ITEM SOURCES "${SOURCE_SERACH_FOLDER}/${IGNORE_FILE}")
	endforeach()
endforeach()


# Configuration files
set(CONFIGURATION_FILES "vpx_version.h" 
						"vpx_config.h" 
						"vp8_rtcd.h" 
						"vp9_rtcd.h" 
						"vpx_scale_rtcd.h"
						"vpx_dsp_rtcd.h")
# Add config files
foreach(CONFIG_FILE ${CONFIGURATION_FILES})
	# copy
	file(COPY "${PROJECT_SOURCE_DIR}/../SourcesConfigureFiles/${CONFIG_FILE}" DESTINATION "${PROJECT_BINARY_DIR}")
	# add to sources
	set(SOURCES ${SOURCES} "${PROJECT_BINARY_DIR}/${CONFIG_FILE}")
endforeach()


if(MSYS OR WIN32 OR MSVC)
	add_definitions(-D_WIN32)
else()
	add_definitions(-DHAVE_PTHREAD_H=1)
	add_definitions(-DHAVE_UNISTD_H=1)
endif()


# configured file include + project includes
include_directories(${PROJECT_BINARY_DIR} 
                    ${PROJECT_SOURCE_DIR}
                    ${FILE_SEARCH_FOLDERS})


# make library
add_library(${PROJECT_NAME} STATIC ${SOURCES})

