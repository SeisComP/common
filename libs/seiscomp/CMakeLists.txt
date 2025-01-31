# ------------------------------------------------------
# ------ CORE LIBRARY ----------------------------------
# ------------------------------------------------------

SET(SYSTEM_CORE_SOURCES "")
SET(SYSTEM_CORE_HEADERS "")

INCLUDE_DIRECTORIES(${LIBMSEED_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${Boost_filesystem_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Boost_regex_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Boost_thread_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${THIRD_PARTY_DIRECTORY}/libbson/src)
INCLUDE_DIRECTORIES(${SC3_PACKAGE_BINARY_DIR}/libs/3rd-party/libbson/src)
INCLUDE_DIRECTORIES(${SC3_PACKAGE_SOURCE_DIR}/apps/messaging/scmaster)

ADD_DEFINITIONS(-DWITH_GIT_REVISION -DWITH_BUILD_INFOS -DFMT_HEADER_ONLY)

ADD_CUSTOM_TARGET(
	build_and_git_infos ALL
	COMMENT "Generate build infos and gather Git revision"
	COMMAND ${CMAKE_COMMAND} -DSYSTEM=${CMAKE_SYSTEM_NAME} -DSRC=${CMAKE_CURRENT_SOURCE_DIR} -DROOT=${CMAKE_CURRENT_SOURCE_DIR} -DCOMPILER=${CMAKE_CXX_COMPILER} -P ${CMAKE_CURRENT_SOURCE_DIR}/core/build_version.cmake
)


# ------------------------------------------------------------------------------
# SYSTEM CHECK
# ------------------------------------------------------------------------------

INCLUDE(CheckFunctionExists)
INCLUDE(CheckIncludeFile)

# eventfd check
CHECK_INCLUDE_FILE(sys/eventfd.h HAS_SYS_EVENTFD_H)

IF(HAS_SYS_EVENTFD_H)
	MESSAGE(STATUS "Found sys/eventfd.h")
	CHECK_FUNCTION_EXISTS(eventfd SC_HAS_EVENTFD)
	IF(SC_HAS_EVENTFD)
		MESSAGE(STATUS "Found function eventfd")
	ENDIF()
ENDIF()

# epoll
CHECK_INCLUDE_FILE(sys/epoll.h HAS_SYS_EPOLL_H)

IF(HAS_SYS_EPOLL_H)
	MESSAGE(STATUS "Found sys/epoll.h")
	CHECK_FUNCTION_EXISTS(epoll_ctl SC_HAS_EPOLL)
	IF(SC_HAS_EPOLL)
		MESSAGE(STATUS "Found function epoll_ctl")
	ENDIF()
ELSE()
	CHECK_INCLUDE_FILE(sys/event.h HAS_SYS_EVENT_H)
	IF(HAS_SYS_EVENT_H)
		CHECK_FUNCTION_EXISTS(kqueue SC_HAS_KQUEUE)
		IF(SC_HAS_KQUEUE)
			MESSAGE(STATUS "Found function kqueue")
		ENDIF()
	ELSE()
		MESSAGE(FATAL_ERROR "Require support for either epoll or kqueue")
	ENDIF()
ENDIF()

# timer
IF(LINUX)
	SET(SC_HAS_TIMER_CREATE 1)
ELSE()
	SET(SC_HAS_TIMER_CREATE 0)
ENDIF()


CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/core/platform.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/core/platform/platform.h
               @ONLY)


# ------------------------------------------------------------------------------
# LIBRARY VERSION
# ------------------------------------------------------------------------------

SET(SEISCOMP_VERSION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/core/version.h)

# Force CMake re-run if the version file has been modified.
CONFIGURE_FILE(${SEISCOMP_VERSION_FILE} ${CMAKE_CURRENT_BINARY_DIR}/core/version.h)

FILE(READ ${SEISCOMP_VERSION_FILE} FILE_BUFFER)
STRING(REPLACE "\n" ";" LINES ${FILE_BUFFER})

SET(SEARCH_STRING "#define SC_API_VERSION ")
STRING(LENGTH ${SEARCH_STRING} SEARCH_STRING_LEN)

FOREACH(LINE ${LINES})
	IF(${LINE} MATCHES "^${SEARCH_STRING}")
		STRING(LENGTH ${LINE} LINE_LEN)

		# We have to calculate the line length of the sub string
		# because -1 is not supported by older CMake versions.
		MATH(EXPR LEN "${LINE_LEN}-${SEARCH_STRING_LEN}")
		STRING(SUBSTRING ${LINE} ${SEARCH_STRING_LEN} ${LEN} SC_COMMON_VERSION)
	ENDIF()
ENDFOREACH()

# Convert hexadecimal number to decimal number
# cmake supports hexadecimal representation from version 3.13 but since
# support for version 2 is still required we have to roll our own conversion
# macro.
# Macro source: https://stackoverflow.com/questions/26182289/convert-from-decimal-to-hexadecimal-in-cmake

MACRO(HEXCHAR2DEC VAR VAL)
	IF(${VAL} MATCHES "[0-9]")
		SET(${VAR} ${VAL})
	ELSEIF(${VAL} MATCHES "[aA]")
		SET(${VAR} 10)
	ELSEIF(${VAL} MATCHES "[bB]")
		SET(${VAR} 11)
	ELSEIF(${VAL} MATCHES "[cC]")
		SET(${VAR} 12)
	ELSEIF(${VAL} MATCHES "[dD]")
		SET(${VAR} 13)
	ELSEIF(${VAL} MATCHES "[eE]")
		SET(${VAR} 14)
	ELSEIF(${VAL} MATCHES "[fF]")
		SET(${VAR} 15)
	ELSE()
		MESSAGE(FATAL_ERROR "Invalid format for hexidecimal character")
	ENDIF()
ENDMACRO(HEXCHAR2DEC)

MACRO(HEX2DEC VAR VAL)
	IF (${VAL} EQUAL 0)
		SET(${VAR} 0)
	ELSE()
		SET(CURINDEX 0)
		STRING(LENGTH "${VAL}" CURLENGTH)

		SET(${VAR} 0)

		STRING(SUBSTRING "${VAL}" ${CURINDEX} 2 PREFIX)
		IF(${VAL} MATCHES "0[xX]")
			SET(CURINDEX 2)

			WHILE(CURINDEX LESS  CURLENGTH)
				STRING(SUBSTRING "${VAL}" ${CURINDEX} 1 CHAR)

				HEXCHAR2DEC(CHAR ${CHAR})

				MATH(EXPR POWAH "(1<<((${CURLENGTH}-${CURINDEX}-1)*4))")
				MATH(EXPR CHAR "(${CHAR}*${POWAH})")
				MATH(EXPR ${VAR} "${${VAR}}+${CHAR}")
				MATH(EXPR CURINDEX "${CURINDEX}+1")
			ENDWHILE()
		ELSE()
			MESSAGE(FATAL_ERROR "Not a hexadecimal number")
		ENDIF()
	ENDIF()
ENDMACRO(HEX2DEC)

# Convert hexadecimal version number to decimal number
HEX2DEC(SC_COMMON_VERSION ${SC_COMMON_VERSION})

MATH(EXPR SC_COMMON_VERSION_MAJOR "${SC_COMMON_VERSION} >> 16")
MATH(EXPR SC_COMMON_VERSION_MINOR "${SC_COMMON_VERSION} >> 8")
MATH(EXPR SC_COMMON_VERSION_MINOR "${SC_COMMON_VERSION_MINOR} & 255")
MATH(EXPR SC_COMMON_VERSION_PATCH "${SC_COMMON_VERSION} & 255")
SET(SC_COMMON_VERSION "${SC_COMMON_VERSION_MAJOR}.${SC_COMMON_VERSION_MINOR}.${SC_COMMON_VERSION_PATCH}")

MESSAGE(STATUS "SeisComP common API version: ${SC_COMMON_VERSION}")


# ------------------------------------------------------------------------------
# CORE LIBRARY
# ------------------------------------------------------------------------------

SC_ADD_SUBDIR_SOURCES(SYSTEM_CORE logging)
SC_ADD_SUBDIR_SOURCES(SYSTEM_CORE core)
SC_ADD_SUBDIR_SOURCES(SYSTEM_CORE system)
SC_ADD_SUBDIR_SOURCES(SYSTEM_CORE io)
SC_ADD_SUBDIR_SOURCES(SYSTEM_CORE utils)
SC_ADD_SUBDIR_SOURCES(SYSTEM_CORE datamodel)
SC_ADD_SUBDIR_SOURCES(SYSTEM_CORE math)
SC_ADD_SUBDIR_SOURCES(SYSTEM_CORE geo)
SC_ADD_SUBDIR_SOURCES(SYSTEM_CORE seismology)
SC_ADD_SUBDIR_SOURCES(SYSTEM_CORE wired)

SET(LIBLOCSAT_PATH ${THIRD_PARTY_DIRECTORY}/locsat/lib)

SET(HAVE_LOCSAT 1)

INCLUDE_DIRECTORIES(${THIRD_PARTY_DIRECTORY}/tau)
INCLUDE_DIRECTORIES(${LIBLOCSAT_PATH}/include)
INCLUDE_DIRECTORIES(${LIBLOCSAT_PATH}/include/db3)
INCLUDE_DIRECTORIES(${LIBLOCSAT_PATH}/include/css)

# Add libtau and LOCSAT sources
AUX_SOURCE_DIRECTORY(${LIBLOCSAT_PATH}/libgeog LIBGEOG_SOURCES)
AUX_SOURCE_DIRECTORY(${LIBLOCSAT_PATH}/libinterp LIBINTERP_SOURCES)
AUX_SOURCE_DIRECTORY(${LIBLOCSAT_PATH}/libloc LIBLOC_SOURCES)
AUX_SOURCE_DIRECTORY(${LIBLOCSAT_PATH}/librdwrt LIBRDWRT_SOURCES)
AUX_SOURCE_DIRECTORY(${LIBLOCSAT_PATH}/libtime LIBTIME_SOURCES)
AUX_SOURCE_DIRECTORY(${LIBLOCSAT_PATH}/libf2c LIBF2C_SOURCES)

SET(SYSTEM_CORE_SOURCES ${SYSTEM_CORE_SOURCES}
	${LIBGEOG_SOURCES}
	${LIBINTERP_SOURCES}
	${LIBLOC_SOURCES}
	${LIBRDWRT_SOURCES}
	${LIBTIME_SOURCES}
	${LIBXPRT2_SOURCES}
	${LIBF2C_SOURCES}
)


SET(PROJECT_API_H core.h)
SET(SYSTEM_CORE_HEADERS "")
SC_ADD_LIBRARY(SYSTEM_CORE core)

SC_LIB_INSTALL_HEADERS(SYSTEM_CORE)

SC_LIB_LINK_LIBRARIES(
	core
		${Boost_thread_LIBRARY}
		${Boost_iostreams_LIBRARY}
		${Boost_filesystem_LIBRARY}
		${Boost_regex_LIBRARY}
		${Boost_program_options_LIBRARY}
		${Boost_LIBRARIES}
		pthread
		tau_internal
		${LIBXML2_LIBRARIES}
		${OPENSSL_LIBRARIES}
)
SC_LIB_VERSION(core ${SC_COMMON_VERSION} ${SC_COMMON_VERSION_MAJOR})
SC_LIB_LINK_LIBRARIES_INTERNAL(core config)
SC_LIB_LINK_LIBRARIES(core mongo::bson_shared)

IF(WIN32)
	SC_LIB_LINK_LIBRARIES(core ws2_32)
ENDIF(WIN32)

IF(WIN32)
	SC_LIB_LINK_LIBRARIES(core zlib)
ENDIF(WIN32)

IF ( MACOSX )
	SC_LIB_LINK_LIBRARIES(core iconv)
ENDIF ( MACOSX )

IF (CMAKE_SYSTEM_NAME STREQUAL SunOS)
	SC_LIB_LINK_LIBRARIES(core iconv)
ENDIF (CMAKE_SYSTEM_NAME STREQUAL SunOS)

IF (MSEED_FOUND)
	SC_LIB_LINK_LIBRARIES(core ${LIBMSEED_LIBRARY})
ENDIF (MSEED_FOUND)


# ------------------------------------------------------------------------------
# CLIENT LIBRARY
# ------------------------------------------------------------------------------

SET(SYSTEM_CLIENT_SOURCES "")
SET(SYSTEM_CLIENT_HEADERS "")

SC_ADD_SUBDIR_SOURCES(SYSTEM_CLIENT client)
SC_ADD_SUBDIR_SOURCES(SYSTEM_CLIENT messaging)
SC_ADD_SUBDIR_SOURCES(SYSTEM_CLIENT processing)
SC_ADD_SUBDIR_SOURCES(SYSTEM_CLIENT qc)

SET(PROJECT_API_H client.h)
SET(SYSTEM_CLIENT_HEADERS "")

SC_ADD_LIBRARY(SYSTEM_CLIENT client)

target_sources(seiscomp_client
  PRIVATE
    io/recordfilter/mseedencoder.h
	io/recordfilter/mseedencoder.cpp
)
SC_LIB_INSTALL_HEADERS(SYSTEM_CLIENT)

SET(SYSTEM_CLIENT_DATA client/data/cities.xml)
SC_INSTALL_DATA(SYSTEM_CLIENT "")

SC_LIB_LINK_LIBRARIES_INTERNAL(client core)
SC_LIB_VERSION(client ${SC_COMMON_VERSION} ${SC_COMMON_VERSION_MAJOR})

IF(WIN32)
	SC_LIB_LINK_LIBRARIES(client zlib psapi)
ENDIF(WIN32)

ADD_DEPENDENCIES(seiscomp_core build_and_git_infos)

# Add gui directory
IF (SC_GLOBAL_GUI)
	SUBDIRS(gui)
ENDIF ()

# Add test directory
IF (SC_GLOBAL_UNITTESTS)
	SUBDIRS(unittest test)
ENDIF (SC_GLOBAL_UNITTESTS)
