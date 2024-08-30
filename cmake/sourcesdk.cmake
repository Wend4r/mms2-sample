# {project}
# Copyright (C) {year} {name of author}
# Licensed under the GPLv3 license. See LICENSE file in the project root for details.

if(NOT SOURCESDK_DIR)
	message(FATAL_ERROR "SOURCESDK_DIR is empty")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8) # 64-bit
	set(SOURCESDK_COMPILE_DEFINTIONS
		${SOURCESDK_COMPILE_DEFINTIONS}

		-DPLATFORM_64BITS -DX64BITS
	)
else()
	set(SIZEOF_BITS ${CMAKE_SIZEOF_VOID_P})
	math(EXPR SIZEOF_BITS "${SIZEOF_BITS}*8")
	message(FATAL_ERROR "${SIZEOF_BITS}-bit platform is not supported")
endif()

if(LINUX)
	set(SOURCESDK_COMPILE_DEFINTIONS
		${SOURCESDK_COMPILE_DEFINTIONS}

		-DPOSIX
		-D_LINUX -DLINUX

		-Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_strnicmp=strncasecmp
		-Dstrnicmp=strncasecmp -D_snprintf=snprintf
		-D_vsnprintf=vsnprintf -D_alloca=alloca -Dstrcmpi=strcasecmp
	)
endif()

if(WINDOWS)
	set(SOURCESDK_COMPILE_DEFINTIONS
		${SOURCESDK_COMPILE_DEFINTIONS}

		-D_WIN32 -DWIN32
	)
endif()

if(MSVC)
	set(SOURCESDK_COMPILE_DEFINTIONS
		${SOURCESDK_COMPILE_DEFINTIONS}

		-DCOMPILER_MSVC -DCOMPILER_MSVC64
	)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(PLATFORM_COMPILE_DEFINITIONS
		${PLATFORM_COMPILE_DEFINITIONS}

		-D_DEBUG -DDEBUG
	)
endif()

set(SOURCESDK_INCLUDE_DIR
	${SOURCESDK_INCLUDE_DIR}

	${SOURCESDK_DIR}/common
	${SOURCESDK_DIR}/game/shared
	${SOURCESDK_DIR}/game/server
	${SOURCESDK_DIR}/public/engine
	${SOURCESDK_DIR}/public/entity2
	${SOURCESDK_DIR}/public/game/server
	${SOURCESDK_DIR}/public/mathlib
	${SOURCESDK_DIR}/public/tier0
	${SOURCESDK_DIR}/public/tier1
	${SOURCESDK_DIR}/public
	${SOURCESDK_DIR}/thirdparty/protobuf-3.21.8/src
	${SOURCESDK_DIR}
)

set(SOURCESDK_LIB_DIR ${SOURCESDK_DIR}/lib)
set(SOURCESDK_BINARY_DIR "sourcesdk")

add_subdirectory(${SOURCESDK_DIR} ${SOURCESDK_BINARY_DIR})
