SUBDIRS(core datamodel io math processing utils seismology)
IF (SC_GLOBAL_GUI)
	SUBDIRS(gui)
ENDIF ()
