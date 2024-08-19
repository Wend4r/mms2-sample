# {project}
# Copyright (C) {year} {name of author}
# Licensed under the GPLv3 license. See LICENSE file in the project root for details.

cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if(POLICY CMP0092)
	cmake_policy(SET CMP0092 NEW) # Don't add -W3 warning level by default.
endif()

project(mms2-sample
	VERSION 1.0.0
	DESCRIPTION "Sample"
	HOMEPAGE_URL "https://github.com/Wend4r/mms2-sample"
	LANGUAGES C CXX
)

string(REGEX REPLACE "^[^-]*-" "" PROJECT_NAME_SUBSTRING "${PROJECT_NAME}")
string(TOLOWER "${PROJECT_NAME_SUBSTRING}" PROJECT_NAME_LOWER)
string(TOUPPER "${PROJECT_NAME_SUBSTRING}" PROJECT_NAME_UPPER)

set(PROJECT_OUTPUT_NAME "sample")

set(PROJECT_AUTHOR "Sample author")
set(PROJECT_DESCRIPTION_FULL "Metamod:Source sample plugin")
set(PROJECT_LICENSE "GPLv3")
string(TIMESTAMP PROJECT_BUILD_DATE "%Y-%m-%d")
string(TIMESTAMP PROJECT_BUILD_TIME "%H:%M:%S")

set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

function(set_or_external_dir VAR_NAME DEFAULT_DIR)
	if(${VAR_NAME})
		file(TO_CMAKE_PATH "${${VAR_NAME}}" ${VAR_NAME})
	else()
		set(${VAR_NAME} "${EXTERNAL_DIR}/${DEFAULT_DIR}")
	endif()

	set(${VAR_NAME} "${${VAR_NAME}}" PARENT_SCOPE)
endfunction()

set_or_external_dir(SOURCESDK_DIR "hl2sdk")
set_or_external_dir(METAMOD_DIR "metamod-source")

set(SOURCESDK_PROTO_FILES
	common/netmessages
	common/network_connection
	common/networkbasetypes
)

include("cmake/platform/shared.cmake")

if(LINUX)
	include("cmake/platform/linux.cmake")
elseif(WINDOWS)
	include("cmake/platform/windows.cmake")
endif()

include("cmake/sourcesdk.cmake")
include("cmake/sourcesdk/proto.cmake")
include("cmake/metamod.cmake")

set(SOURCE_FILES
	${SOURCE_DIR}/globals.cpp
	${SOURCE_DIR}/sample_plugin.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${SOURCESDK_SOURCE_FILES} ${SOURCESDK_PROTO_SOURCE_FILES})

set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY C_EXTENSIONS OFF)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_EXTENSIONS OFF)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_OUTPUT_NAME})

target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILER_OPTIONS} ${PLATFORM_COMPILER_OPTIONS})
target_link_options(${PROJECT_NAME} PRIVATE ${LINKER_OPTIONS} ${PLATFORM_LINKER_OPTIONS})

target_compile_definitions(${PROJECT_NAME} PRIVATE ${COMPILE_DEFINITIONS} ${PLATFORM_COMPILE_DEFINITIONS} ${SOURCESDK_COMPILE_DEFINTIONS} ${METAMOD_COMPILE_DEFINITIONS})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR} ${SOURCESDK_INCLUDE_DIR} ${METAMOD_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE ${LINK_LIBRARIES} ${SOURCESDK_LINK_LIBRARIES} ${SOURCESDK_PROTOBUF_STATIC_LIBRARY})
