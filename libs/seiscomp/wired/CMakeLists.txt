INCLUDE(CheckFunctionExists)
INCLUDE(CheckIncludeFile)
CHECK_INCLUDE_FILE(sys/eventfd.h HAVE_SYS_EVENTFD_H)
CHECK_FUNCTION_EXISTS(eventfd HAVE_EVENTFD)

IF(HAVE_SYS_EVENTFD_H)
	MESSAGE(STATUS "Found sys/eventfd.h")
	SET(WIRE_DEFINITIONS ${WIRE_DEFINITIONS} -DHAVE_SYS_EVENTFD_H)
ENDIF(HAVE_SYS_EVENTFD_H)

IF(HAVE_EVENTFD)
	MESSAGE(STATUS "Found function eventfd")
	SET(WIRE_DEFINITIONS ${WIRE_DEFINITIONS} -DHAVE_EVENTFD)
ENDIF(HAVE_EVENTFD)

SET (
	WIRE_SOURCES
		buffer.cpp
		clientsession.cpp
		device.cpp
		endpoint.cpp
		ipacl.cpp
		mime.cpp
		reactor.cpp
		server.cpp
		session.cpp
)

SET(
	WIRE_HEADERS
		buffer.h
		clientsession.h
		device.h
		endpoint.h
		ipacl.h
		mime.h
		reactor.h
		server.h
		session.h
)

SC_ADD_SUBDIR_SOURCES(WIRE buffers)
SC_ADD_SUBDIR_SOURCES(WIRE devices)
SC_ADD_SUBDIR_SOURCES(WIRE protocols)
SC_SETUP_LIB_SUBDIR(WIRE)
