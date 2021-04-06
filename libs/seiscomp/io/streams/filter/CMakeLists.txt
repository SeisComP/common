INCLUDE_DIRECTORIES(${THIRD_PARTY_DIRECTORY})

SET(FILTER_SOURCES
	lz4.cpp
	../../../../3rd-party/lz4/lz4.c
	../../../../3rd-party/lz4/lz4frame.c
	../../../../3rd-party/lz4/lz4hc.c
	../../../../3rd-party/lz4/xxhash.c

)

SET(FILTER_HEADERS
	lz4.h
)

SC_SETUP_LIB_SUBDIR(FILTER)
