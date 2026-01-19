SET(SYSTEM_SOURCES
	application.cpp
	commandline.cpp
	environment.cpp
	hostinfo.cpp
	model.cpp
	pluginregistry.cpp
	schema.cpp
	settings.cpp
)

SET(SYSTEM_HEADERS
	application.h
	commandline.h
	commandline.ipp
	environment.h
	hostinfo.h
	model.h
	pluginregistry.h
	schema.h
	settings.h
	settings.ipp
)

SET(SYSTEM_DEFINITIONS -DSEISCOMP_ROOT="${SC3_PACKAGE_INSTALL_PREFIX}")
SET(SYSTEM_DEFINITIONS ${SYSTEM_DEFINITIONS} -DSEISCOMP_SHARE_DIR="${PROJECT_SHARE_DIR}")
SET(SYSTEM_DEFINITIONS ${SYSTEM_DEFINITIONS} -DSEISCOMP_CONFIG_DIR="${PROJECT_CONFIG_DIR}/defaults")
SET(SYSTEM_DEFINITIONS ${SYSTEM_DEFINITIONS} -DSEISCOMP_SOURCE_DIR="${SC3_PACKAGE_ROOT_SOURCE_DIR}")

SC_SETUP_LIB_SUBDIR(SYSTEM)
