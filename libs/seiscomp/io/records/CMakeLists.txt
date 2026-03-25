SET(RECORDS_SOURCES
	binaryrecord.cpp
	mseedrecord.cpp
	sacrecord.cpp
	shrecord.cpp
	mseed/decoder/cdsn.cpp
	mseed/decoder/dwwssn.cpp
	mseed/decoder/geoscope.cpp
	mseed/decoder/format.cpp
	mseed/decoder/sro.cpp
	mseed/decoder/steim1.cpp
	mseed/decoder/steim2.cpp
	mseed/encoder/format.cpp
)

SET(RECORDS_HEADERS
	binaryrecord.h
	mseedrecord.h
	sacrecord.h
	shrecord.h
)

SC_SETUP_LIB_SUBDIR(RECORDS)
