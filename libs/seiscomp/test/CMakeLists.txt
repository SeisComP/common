SUBDIRS(core datamodel io processing utils seismology)
IF (SC_GLOBAL_GUI)
	SUBDIRS(gui)
ENDIF ()
