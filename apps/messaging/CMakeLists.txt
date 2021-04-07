OPTION(SC_TRUNK_MASTER "Compile the messaging master" ON)

IF (SC_TRUNK_MASTER)
	SUBDIRS(scmaster)
ENDIF (SC_TRUNK_MASTER)

