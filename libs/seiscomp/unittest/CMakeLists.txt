SET(CORE_UNITTEST_SOURCES
	output.cpp
)

SC_ADD_LIBRARY(CORE_UNITTEST unittest)
SC_LIB_LINK_LIBRARIES(unittest ${Boost_unit_test_framework_LIBRARY})
