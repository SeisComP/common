SUBDIRS(utils)

SET(BROKER_HEADERS
	client.h
	group.h
	hashset.h
	message.h
	messagedispatcher.h
	messageprocessor.h
	processor.h
	protocol.h
	queue.h
	statistics.h
)

SET(BROKER_SOURCES
	client.cpp
	group.cpp
	queue.cpp
	message.cpp
	messageprocessor.cpp
	processor.cpp
	statistics.cpp
	utils/utils.cpp
)

SC_LIB_INSTALL_HEADERS(BROKER seiscomp/broker)
SC_ADD_LIBRARY(BROKER broker)
SC_LIB_LINK_LIBRARIES_INTERNAL(broker core)
