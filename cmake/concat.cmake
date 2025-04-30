# {project}
# Copyright (C) {year} {name of author}
# Licensed under the GPLv3 license. See LICENSE file in the project root for details.

if(NOT CONCAT_DIR)
	message(FATAL_ERROR "CONCAT_DIR is empty")
endif()

set(CONCAT_BINARY_DIR "s2u-concat")

set(CONCAT_INCLUDE_DIRS
	${CONCAT_INCLUDE_DIRS}

	${CONCAT_DIR}/include
)

add_subdirectory(${CONCAT_DIR} ${CONCAT_BINARY_DIR})
