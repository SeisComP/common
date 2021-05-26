SET(SCRIPTS_DATA sqlite3.sql mysql.sql postgres.sql)
SC_INSTALL_DATA(SCRIPTS db)

INSTALL(DIRECTORY migrations
	DESTINATION ${SC3_PACKAGE_SHARE_DIR}/db
)

INSTALL(PROGRAMS
            ${CMAKE_CURRENT_SOURCE_DIR}/mysql_setup.py
            ${CMAKE_CURRENT_SOURCE_DIR}/postgres_setup.py
            ${CMAKE_CURRENT_SOURCE_DIR}/sqlite3_setup.py
            ${CMAKE_CURRENT_SOURCE_DIR}/utils.py
            ${CMAKE_CURRENT_SOURCE_DIR}/pkexec_wrapper.sh
        DESTINATION ${SC3_PACKAGE_SHARE_DIR}/db
)
