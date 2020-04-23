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
