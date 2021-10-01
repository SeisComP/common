SET(GUI_MAP_SOURCES
	annotations.cpp
	layer.cpp
	legend.cpp
	standardlegend.cpp
	mapsymbol.cpp
	maptree.cpp
	imagetree.cpp
	canvas.cpp
	mapwidget.cpp
	texturecache.cpp
	projection.cpp
)

SET(GUI_MAP_HEADERS
	annotations.h
	maptree.h
	imagetree.h
	mapsymbol.h
	decorator.h
	texturecache.h
	texturecache.ipp
	projection.h
)

SET(GUI_MAP_MOC_HEADERS
	legend.h
	layer.h
	imagetree.h
	canvas.h
	mapwidget.h
)

SET(GUI_MAP_RESOURCES
	mapicons.qrc
)

SC_ADD_GUI_SUBDIR_SOURCES(GUI_MAP projections)
SC_ADD_GUI_SUBDIR_SOURCES(GUI_MAP layers)
SC_SETUP_GUI_LIB_SUBDIR(GUI_MAP)
