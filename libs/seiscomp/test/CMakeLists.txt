SUBDIRS(core datamodel io utils math)
IF (SC_GLOBAL_GUI)
	SUBDIRS(gui)
ENDIF ()
