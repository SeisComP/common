SET(SEISMOLOGY_SOURCES
	firstmotion.cpp
	locatorinterface.cpp
	mb.cpp
	ML.cpp
	Mjma.cpp
	Mwp.cpp
	ttt.cpp
)

SET(SEISMOLOGY_HEADERS
	firstmotion.h
	ttt.h
	regions.h
	magnitudes.h
	locatorinterface.h
)

SC_ADD_SUBDIR_SOURCES(SEISMOLOGY locator)
SC_ADD_SUBDIR_SOURCES(SEISMOLOGY regions)
SC_ADD_SUBDIR_SOURCES(SEISMOLOGY ttt)
SC_SETUP_LIB_SUBDIR(SEISMOLOGY)
