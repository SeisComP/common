SET(TESTS
	datetime_time.cpp
	datetime_timespan.cpp
	georegions.cpp
	units.cpp
	xml.cpp
	geolib.cpp
	configure_files.cpp
	utils_misc.cpp
	strings.cpp
)

FOREACH(testSrc ${TESTS})
	GET_FILENAME_COMPONENT(testName ${testSrc} NAME_WE)
	SET(testName test_core_${testName})
	ADD_EXECUTABLE(${testName} ${testSrc})
	SC_LINK_LIBRARIES_INTERNAL(${testName} unittest core)
	SC_LINK_LIBRARIES(${testName})

	ADD_TEST(
		NAME ${testName}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMAND ${testName}
	)
ENDFOREACH(testSrc)
