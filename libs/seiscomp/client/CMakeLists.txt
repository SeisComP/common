SET(CLIENT_SOURCES
	application.cpp
	streamapplication.cpp
	inventory.cpp
	configdb.cpp
	queue.cpp
	monitor.cpp
)

SET(CLIENT_HEADERS
	application.h
	streamapplication.h
	queue.h
	queue.ipp
	inventory.h
	configdb.h
	monitor.h
)

SC_SETUP_LIB_SUBDIR(CLIENT)
