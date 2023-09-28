SUBDIRS(core datamodel io processing utils)
IF (SC_GLOBAL_GUI)
	SUBDIRS(gui)
ENDIF ()
