SUBDIRS(core datamodel io math processing utils)
IF (SC_GLOBAL_GUI)
	SUBDIRS(gui)
ENDIF ()
