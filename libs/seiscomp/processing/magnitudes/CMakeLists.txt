SET(
	MAGS_SOURCES
		mBc.cpp
		mb.cpp
		mB_BB.cpp
		Mjma.cpp
		ML.cpp
		MLc.cpp
		MLv.cpp
		Ms_20.cpp
		Ms_BB.cpp
		Mwp.cpp
		utils.cpp
)

SET(
	MAGS_HEADERS
		mBc.h
		mb.h
		mB_BB.h
		Mjma.h
		ML.h
		MLc.h
		MLv.h
		Ms_20.h
		Ms_BB.h
		Mwp.h
		utils.h
)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})

SC_SETUP_LIB_SUBDIR(MAGS)
