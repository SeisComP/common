SET(
	MAGS_SOURCES
		mBc.cpp
		mb.cpp
		m_B.cpp
		Mjma.cpp
		ML.cpp
		MLc.cpp
		MLv.cpp
		Ms20.cpp
		msbb.cpp
		Mwp.cpp
		utils.cpp
)

SET(
	MAGS_HEADERS
		mBc.h
		mb.h
		m_B.h
		Mjma.h
		ML.h
		MLc.h
		MLv.h
		Ms20.h
		msbb.h
		Mwp.h
		utils.h
)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})

SC_SETUP_LIB_SUBDIR(MAGS)
