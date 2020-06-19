SET(SEISMOLOGY_SOURCES
	chi2.cpp
	eigv.cpp
	locatorinterface.cpp
	mb.cpp
	Mjma.cpp
	ML.cpp
	Mwp.cpp
	ttt.cpp
)

SET(SEISMOLOGY_HEADERS
	locatorinterface.h
	magnitudes.h
	regions.h
	ttt.h
)

SC_ADD_SUBDIR_SOURCES(SEISMOLOGY locator)
SC_ADD_SUBDIR_SOURCES(SEISMOLOGY regions)
SC_ADD_SUBDIR_SOURCES(SEISMOLOGY ttt)
SC_SETUP_LIB_SUBDIR(SEISMOLOGY)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
