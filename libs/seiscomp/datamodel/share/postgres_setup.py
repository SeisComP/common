#!/usr/bin/env seiscomp-python
# -*- coding: utf-8 -*-

############################################################################
# Copyright (C) 2016 by gempa GmbH                                         #
#                                                                          #
# All Rights Reserved.                                                     #
#                                                                          #
# NOTICE: All information contained herein is, and remains                 #
# the property of gempa GmbH and its suppliers, if any. The intellectual   #
# and technical concepts contained herein are proprietary to gempa GmbH    #
# and its suppliers.                                                       #
# Dissemination of this information or reproduction of this material       #
# is strictly forbidden unless prior written permission is obtained        #
# from gempa GmbH.                                                         #
############################################################################


from __future__ import (
    absolute_import,
    print_function)

import os
import sys
import tempfile

from utils import write, execute

def createPostgresSQLDB(
        db,
        rwuser,
        rwpwd,
        rouser,
        ropwd,
        rwhost,
        drop,
        schemapath):
    #cmd = "psql --host {}".format(rwhost)
    # We have to disable notice messages with --client-min-messages=warning
    # because PostgreSQL outputs notice messages to stderr when e.g. tables should
    # be removed which do not exist even the if exist check is inplace.
    cmd = "PGOPTIONS='--client-min-messages=warning' psql"

    write("+ Create PostgresSQL database")

    if drop:
        q = f"DROP DATABASE IF EXISTS {db};"
        write(f"  + Drop database {db}")

        res = execute(f"{cmd} -c \"{q}\"")
        if res.error:
            print(f"  + {res.error}")
            return False

    write(f"  + Create database {db}")

    q = f"CREATE DATABASE {db} ENCODING 'UTF8'"
    res = execute(f"{cmd} -c \"{q}\"")
    if res.error:
        print(f"  + {res.error}")
        return False

    q = f"ALTER DATABASE {db} SET bytea_output TO 'escape'"
    res = execute(f"{cmd} -c \"{q}\"")
    if res.error:
        print(f"  + {res.error}")
        return False

    write("  + Create SeisComP tables")

    q = f"\\i {os.path.join(schemapath, 'postgres.sql')};"
    res = execute(f"{cmd} -d {db} -c \"{q}\"")
    if res.error:
        print(f"  + {res.error}")
        return False

    write("  + Setup user roles")

    q = f"SELECT 1 FROM pg_roles WHERE rolname='{rwuser}'"
    res = execute(cmd + f" -c \"{q}\" -d {db}")
    if res.error:
        print(f"  + {res.error}")
        return False

    q = ""
    exits = "1" in res.data
    if not exits:
        q += f"CREATE USER {rwuser} WITH ENCRYPTED PASSWORD '{rwpwd}';"

    q += f"GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO {rwuser};"
    q += f"GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO {rwuser};"

    if rwuser != rouser:
        q = f"SELECT 1 FROM pg_roles WHERE rolname='{rouser}'"
        res = execute(cmd + f" -c \"{q}\" -d {db}")
        if res.error:
            print(f"  + {res.error}")
            return False

        q = ""
        exits = "1" in res.data
        if not exits:
            q += f"CREATE USER {rouser} WITH ENCRYPTED PASSWORD '{ropwd}';"

        q += f"GRANT SELECT ON ALL TABLES IN SCHEMA public TO {rouser};"


    res = execute(f"{cmd} -c \"{q}\" -d {db}")
    if res.error:
        print(f"  + {res.error}")
        return False

    return True

def main():
    if len(sys.argv) != 9:
        print("Usage: postgres_setup.py <db> <rwuser> <rwpwd> "
              "<rouser> <ropwd> <rwhost> <drop> <schema path>\n\n"
              "For example: su postgres -c postgres_setup.py seiscomp sysop sysop "
              "sysop sysop localhost false ~/seiscomp/share/db/")
        return 1

    db = sys.argv[1]
    rwuser = sys.argv[2]
    rwpwd = sys.argv[3]
    rouser = sys.argv[4]
    ropwd = sys.argv[5]
    rwhost = sys.argv[6]
    schemapath = sys.argv[8]

    drop = sys.argv[7].lower() == 'true'

    os.chdir(tempfile.gettempdir())
    if not createPostgresSQLDB(db, rwuser, rwpwd, rouser, ropwd, rwhost, drop, schemapath):
        return 1

    return 0

if __name__ == "__main__":
    sys.exit(main())
