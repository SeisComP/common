SET(PROC_SOURCES
	response.cpp
	sensor.cpp
	stream.cpp
	processor.cpp
	waveformprocessor.cpp
	timewindowprocessor.cpp
	waveformoperator.cpp
	application.cpp
	streambuffer.cpp
	regions.cpp

	detector.cpp
	fx.cpp
	picker.cpp
	secondarypicker.cpp
	amplitudeprocessor.cpp
	magnitudeprocessor.cpp
)

SET(PROC_HEADERS
	response.h
	sensor.h
	stream.h
	processor.h
	waveformprocessor.h
	timewindowprocessor.h
	waveformoperator.h
	application.h
	streambuffer.h
	regions.h

	detector.h
	picker.h
	secondarypicker.h
	amplitudeprocessor.h
	magnitudeprocessor.h
)

SC_ADD_SUBDIR_SOURCES(PROC fx)
SC_ADD_SUBDIR_SOURCES(PROC picker)
SC_ADD_SUBDIR_SOURCES(PROC secondarypicker)
SC_ADD_SUBDIR_SOURCES(PROC amplitudes)
SC_ADD_SUBDIR_SOURCES(PROC magnitudes)
SC_ADD_SUBDIR_SOURCES(PROC operator)
SC_SETUP_LIB_SUBDIR(PROC)
