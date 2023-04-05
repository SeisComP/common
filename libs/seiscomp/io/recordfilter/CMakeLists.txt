SET(RECORDFILTER_SOURCES
	iirfilter.cpp
	resample.cpp
	demux.cpp
	spectralizer.cpp
	crop.cpp
)

SET(RECORDFILTER_HEADERS
	iirfilter.h
	resample.h
	demux.h
	spectralizer.h
	crop.h
)

SC_SETUP_LIB_SUBDIR(RECORDFILTER)
