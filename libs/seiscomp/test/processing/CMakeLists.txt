SET(TESTS
	amplitudes.cpp
)

FOREACH(testSrc ${TESTS})
	GET_FILENAME_COMPONENT(testName ${testSrc} NAME_WE)
	SET(testName test_core_processing_${testName})
	ADD_EXECUTABLE(${testName} ${testSrc})
	SC_LINK_LIBRARIES_INTERNAL(${testName} unittest client)
	SC_LINK_LIBRARIES(${testName})

	ADD_TEST(
		NAME ${testName}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMAND ${testName}
	)
ENDFOREACH(testSrc)
