SET(PYTHON_PACKAGE_PATH ${SC3_PACKAGE_PYTHON_LIB_DIR}/seiscomp3)

INSTALL(FILES
	# Python files to be installed
	Client.py
	Core.py
	DataModel.py
	Geo.py
	IO.py
	Logging.py
	Math.py
	Seismology.py
	System.py
	Util.py
	DESTINATION ${PYTHON_PACKAGE_PATH})
