SET(
	MAGS_SOURCES
		mBc.cpp
		mb.cpp
		mb_idc.cpp
		m_B.cpp
		Mjma.cpp
		ML.cpp
		ML_idc.cpp
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

SET(IDC_DATA data/qfvc.mb data/global.ml)
SC_INSTALL_DATA(IDC "magnitudes/IDC")

SC_SETUP_LIB_SUBDIR(MAGS)
