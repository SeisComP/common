SET(RECORDFILTER_SOURCES
	pipe.cpp
	iirfilter.cpp
	resample.cpp
	demux.cpp
	spectralizer.cpp
	crop.cpp
)

SET(RECORDFILTER_HEADERS
	pipe.h
	iirfilter.h
	resample.h
	demux.h
	spectralizer.h
	crop.h
)

SC_SETUP_LIB_SUBDIR(RECORDFILTER)
