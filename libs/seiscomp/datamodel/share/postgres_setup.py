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
        q = "DROP DATABASE IF EXISTS {};".format(db)
        write("  + Drop database {}".format(db))

        res = execute("{} -c \"{}\"".format(cmd, q))
        if res.error:
            print("  + {}".format(res.error))
            return False

    write("  + Create database {}".format(db))

    q = "CREATE DATABASE {} ENCODING 'UTF8'".format(db)
    res = execute("{} -c \"{}\"".format(cmd, q))
    if res.error:
        print("  + {}".format(res.error))
        return False

    q = "ALTER DATABASE {} SET bytea_output TO 'escape'".format(db)
    res = execute("{} -c \"{}\"".format(cmd, q))
    if res.error:
        print("  + {}".format(res.error))
        return False

    write("  + Create SeisComP tables")

    q = "\\i {};".format(os.path.join(schemapath, "postgres.sql"))
    res = execute("{} -d {} -c \"{}\"".format(cmd, db, q))
    if res.error:
        print("  + {}".format(res.error))
        return False

    write("  + Setup user roles")

    q = "SELECT 1 FROM pg_roles WHERE rolname='{}'".format(rwuser)
    res = execute(cmd + " -c \"{}\" -d {}".format(q, db))
    if res.error:
        print("  + {}".format(res.error))
        return False

    q = ""
    exits = "1" in res.data
    if not exits:
        q += "CREATE USER {} WITH ENCRYPTED PASSWORD '{}';".format(rwuser, rwpwd)

    q += "GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO {};".format(rwuser)
    q += "GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO {};".format(rwuser)

    if rwuser != rouser:
        q = "SELECT 1 FROM pg_roles WHERE rolname='{}'".format(rouser)
        res = execute(cmd + " -c \"{}\" -d {}".format(q, db))
        if res.error:
            print("  + {}".format(res.error))
            return False

        q = ""
        exits = "1" in res.data
        if not exits:
            q += "CREATE USER {} WITH ENCRYPTED PASSWORD '{}';".format(rouser, ropwd)

        q += "GRANT SELECT ON ALL TABLES IN SCHEMA public TO {};".format(rouser)


    res = execute("{} -c \"{}\" -d {}".format(cmd, q, db))
    if res.error:
        print("  + {}".format(res.error))
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
