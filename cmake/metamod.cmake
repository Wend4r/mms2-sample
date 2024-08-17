# {project}
# Copyright (C) {year} {name of author}
# Licensed under the GPLv3 license. See LICENSE file in the project root for details.

if(NOT METAMOD_DIR)
	message(FATAL_ERROR "METAMOD_DIR is empty")
endif()

set(METAMOD_COMPILE_DEFINITIONS
	${METAMOD_COMPILE_DEFINITIONS}

	-DMETA_PLUGIN_AUTHOR="${PROJECT_AUTHOR}"
	-DMETA_PLUGIN_PREFIX="${PROJECT_NAME_SUBSTRING}"
	-DMETA_PLUGIN_PREFIX_LOWER="${PROJECT_NAME_LOWER}"
	-DMETA_PLUGIN_PREFIX_UPPER="${PROJECT_NAME_UPPER}"
	-DMETA_PLUGIN_NAME="${PROJECT_DESCRIPTION}"
	-DMETA_PLUGIN_DESCRIPTION="${PROJECT_DESCRIPTION_FULL}"
	-DMETA_PLUGIN_URL="${PROJECT_HOMEPAGE_URL}"
	-DMETA_PLUGIN_LICENSE="${PROJECT_LICENSE}"
	-DMETA_PLUGIN_VERSION="${PROJECT_VERSION}"
	-DMETA_PLUGIN_DATE="${PROJECT_BUILD_DATE} ${PROJECT_BUILD_TIME}"
	-DMETA_PLUGIN_LOG_TAG="${PROJECT_NAME_UPPER}"

	-DMETA_IS_SOURCE2
)

set(METAMOD_INCLUDE_DIR
	${METAMOD_INCLUDE_DIR}

	${METAMOD_DIR}/core/sourcehook
	${METAMOD_DIR}/core
)
