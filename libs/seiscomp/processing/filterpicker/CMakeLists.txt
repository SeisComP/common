# CMakeLists.txt for FilterPicker plugin
# Based on the Lomax et al. (2012) FilterPicker algorithm

SET(FILTERPICKER_SOURCES filterpicker.cpp)
SET(FILTERPICKER_HEADERS filterpicker.h)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml") INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})

SC_SETUP_LIB_SUBDIR(FILTERPICKER)
