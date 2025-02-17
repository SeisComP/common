SET(CORE_SOURCES
	enumeration.cpp
	rtti.cpp
	metaobject.cpp
	datetime.cpp
	timewindow.cpp
	optional.cpp
	strings.cpp
	arrayfactory.cpp
	typedarray.cpp
	bitset.cpp
	record.cpp
	array.cpp
	genericrecord.cpp
	greensfunction.cpp
	recordsequence.cpp
	interruptible.cpp
	message.cpp
	genericmessage.cpp
	datamessage.cpp
	exceptions.cpp
	plugin.cpp
	system.cpp
	baseobject.cpp
	version.cpp
)

SET(CORE_HEADERS
	archive.h
	archive.inl
	archive.ipp
	baseobject.h
	baseobject.inl
	defs.h
	factory.h
	factory.inl
	factory.ipp
	interfacefactory.h
	interfacefactory.ipp
	io.h
	rtti.h
	metaobject.h
	metaproperty.h
	serialization.h
	datetime.h
	datetime.ipp
	timewindow.h
	optional.h
	optional.inl
	enumeration.h
	enumeration.inl
	strings.h
	strings.ipp
	arrayfactory.h
	array.h
	typedarray.h
	bitset.h
	bitset.ipp
	record.h
	genericrecord.h
	greensfunction.h
	exceptions.h
	recordsequence.h
	interruptible.h
	message.h
	genericmessage.h
	genericmessage.ipp
	datamessage.h
	plugin.h
	system.h
	version.h
)

IF(WIN32)
	SET(CORE_SOURCES ${CORE_SOURCES} strptime.c)
	SET(CORE_HEADERS ${CORE_HEADERS} strptime.h)
ENDIF(WIN32)

IF(CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION LESS 11)
	MESSAGE(STATUS "Using backported from_chars for floating point values")
	SET(CORE_SOURCES
		${CORE_SOURCES}
		backports/charconv/floating_from_chars.cpp
	)
ENDIF()

SC_ADD_SUBDIR_SOURCES(CORE platform)
SC_SETUP_LIB_SUBDIR(CORE)


