SET(LOG_SOURCES
	log.cpp
	channel.cpp
	node.cpp
	publisher.cpp
	output.cpp
)

SET(LOG_HEADERS
	common.h
	defs.h
	log.h
	channel.h
	node.h
	publisher.h
	publishloc.h
	output.h
)

SC_ADD_SUBDIR_SOURCES(LOG output)

SC_SETUP_LIB_SUBDIR(LOG)
