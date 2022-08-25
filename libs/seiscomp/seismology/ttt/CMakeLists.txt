SET(TTT_HEADERS libtau.h locsat.h)
SET(TTT_SOURCES libtau.cpp locsat.cpp homogeneous.cpp)

SC_SETUP_LIB_SUBDIR(TTT)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
