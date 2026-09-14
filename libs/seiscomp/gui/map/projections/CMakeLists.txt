SET(GUI_MAP_PROJECTIONS_SOURCES
	equalearth.cpp
	mercator.cpp
	rectangular.cpp
)

SET(GUI_MAP_PROJECTIONS_HEADERS
	equalearth.h
	mercator.h
	rectangular.h
)

SC_SETUP_GUI_LIB_SUBDIR(GUI_MAP_PROJECTIONS)
