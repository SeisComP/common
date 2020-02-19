SET(MSG_SOURCES
	connection.cpp
	packet.cpp
	protocol.cpp
	status.cpp
	protocols/scmp/socket.cpp
	protocols/scmp/websocket.cpp
)

SET(MSG_HEADERS
	connection.h
	packet.h
	protocol.h
	status.h
)

SC_ADD_SUBDIR_SOURCES(MSG messages)
SC_SETUP_LIB_SUBDIR(MSG)
