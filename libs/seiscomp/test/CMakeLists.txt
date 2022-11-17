SUBDIRS(core datamodel io utils)
IF (SC_GLOBAL_GUI)
	SUBDIRS(gui)
ENDIF ()
