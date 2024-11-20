INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(../../../system/libs/swig)

IF (CMAKE_COMPILER_IS_GNUCC)
	# Important for O2 compilation
	ADD_DEFINITIONS("-fno-strict-aliasing")
ENDIF (CMAKE_COMPILER_IS_GNUCC)

IF (MSEED_FOUND)
	ADD_DEFINITIONS(-DHAVE_MSEED)
	INCLUDE_DIRECTORIES(${LIBMSEED_INCLUDE_DIR})
ENDIF (MSEED_FOUND)

IF (SC_GLOBAL_PYTHON_WRAPPER_NUMPY)
	MESSAGE(STATUS "Adding support for Python NumPy")
	ADD_DEFINITIONS(-DHAVE_NUMPY)
	INCLUDE_DIRECTORIES(${PYTHON_NUMPY_INCLUDE_DIR})
ENDIF (SC_GLOBAL_PYTHON_WRAPPER_NUMPY)

SET(
	WRAPPER_MODULES
		core
		logging
		system
		io
		utils
		datamodel
		math
		seismology
		client
		geo
)


SET(WRAPPER_MODULE_core_EXTRA_SC_LIBS core)
SET(WRAPPER_MODULE_logging_EXTRA_SC_LIBS core)
SET(WRAPPER_MODULE_system_EXTRA_SC_LIBS core)
SET(WRAPPER_MODULE_io_EXTRA_SC_LIBS core)
SET(WRAPPER_MODULE_datamodel_EXTRA_SC_LIBS core)
SET(WRAPPER_MODULE_math_EXTRA_SC_LIBS core)
SET(WRAPPER_MODULE_client_EXTRA_SC_LIBS client)
SET(WRAPPER_MODULE_utils_EXTRA_SC_LIBS core)
SET(WRAPPER_MODULE_seismology_EXTRA_SC_LIBS core)
SET(WRAPPER_MODULE_geo_EXTRA_SC_LIBS core)

SET(build_lib_python_seiscomp_dir ${CMAKE_BINARY_DIR}/lib/python/seiscomp)
FILE(MAKE_DIRECTORY ${build_lib_python_seiscomp_dir})

FOREACH (wrapper_module ${WRAPPER_MODULES})
	SET(
		${wrapper_module}_SOURCES
			${wrapper_module}_python_wrap.cxx
			${WRAPPER_MODULE_${wrapper_module}_EXTRA_FILES}
	)
	ADD_LIBRARY(_${wrapper_module} MODULE ${${wrapper_module}_SOURCES})
	SET_TARGET_PROPERTIES(_${wrapper_module} PROPERTIES PREFIX "")
	TARGET_LINK_LIBRARIES(_${wrapper_module} ${PYTHON_LIBRARIES})
	TARGET_LINK_LIBRARIES(_${wrapper_module} ${WRAPPER_MODULE_${wrapper_module}_EXTRA_LIBS})
	SC_LINK_LIBRARIES_INTERNAL(_${wrapper_module} ${WRAPPER_MODULE_${wrapper_module}_EXTRA_SC_LIBS})

	ADD_CUSTOM_TARGET(link_build_lib_python_seiscomp_${wrapper_module} ALL
		COMMAND ${CMAKE_COMMAND} -E create_symlink
			${CMAKE_CURRENT_SOURCE_DIR}/${wrapper_module}.py
			${build_lib_python_seiscomp_dir}/${wrapper_module}.py
		COMMAND ${CMAKE_COMMAND} -E create_symlink
			${CMAKE_BINARY_DIR}/lib/_${wrapper_module}.so
			${build_lib_python_seiscomp_dir}/_${wrapper_module}.so
	)
ENDFOREACH (wrapper_module)

SC_SWIG_GET_MODULE_PATH(_module_path)
SET(_module_path ${SC3_PACKAGE_PYTHON_LIB_DIR}/seiscomp)

FOREACH (wrapper_module ${WRAPPER_MODULES})
	SET(_local_module_path ${_module_path})
	SET(_local_module_file ${wrapper_module})

	IF ("${wrapper_module}" STREQUAL "datamodel")
		SET(_local_module_path ${_local_module_path}/datamodel)
		SET(_local_module_file __init__)
	ENDIF()

	INSTALL(
		TARGETS _${wrapper_module}
		LIBRARY DESTINATION ${_local_module_path}
	)

	INSTALL(
		FILES ${CMAKE_CURRENT_SOURCE_DIR}/${wrapper_module}.py
		RENAME ${_local_module_file}.py
		DESTINATION ${_local_module_path}
	)

	INSTALL(
		FILES ${CMAKE_CURRENT_BINARY_DIR}/${wrapper_module}.pyc
		RENAME ${_local_module_file}.pyc
		DESTINATION ${_local_module_path}
		OPTIONAL
	)

	INSTALL(
		FILES ${CMAKE_CURRENT_BINARY_DIR}/${wrapper_module}.pyo
		RENAME ${_local_module_file}.pyo
		DESTINATION ${_local_module_path}
		OPTIONAL
	)

	ADD_CUSTOM_COMMAND(
		TARGET _${wrapper_module}
		POST_BUILD
		COMMAND ${CMAKE_SOURCE_DIR}/py-compile ARGS --python \"${PYTHON_EXECUTABLE}\" --destdir ${CMAKE_CURRENT_BINARY_DIR} ${wrapper_module}.py
	)
ENDFOREACH (wrapper_module)

