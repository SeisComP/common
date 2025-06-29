SET(TESTS
	configuration_files.cpp
	datetime_time.cpp
	datetime_timespan.cpp
	digits.cpp
	georegions.cpp
	geolib.cpp
	intrusive_list.cpp
	recordsequence.cpp
	refcounts.cpp
	strings.cpp
	timewindow.cpp
 	version.cpp
	xml.cpp
)

FOREACH(testSrc ${TESTS})
	GET_FILENAME_COMPONENT(testName ${testSrc} NAME_WE)
	SET(testName test_core_${testName})
	ADD_EXECUTABLE(${testName} ${testSrc})
	SC_LINK_LIBRARIES_INTERNAL(${testName} unittest core)

	TARGET_COMPILE_DEFINITIONS(${testName} PRIVATE
		SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
		BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}"
	)

	ADD_TEST(
		NAME ${testName}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMAND ${testName}
	)
ENDFOREACH(testSrc)
