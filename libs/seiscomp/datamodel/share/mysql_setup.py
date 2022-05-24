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

def createMYSQLDB(
        db,
        rwuser,
        rwpwd,
        rouser,
        ropwd,
        rwhost,
        rootpwd,
        drop,
        schemapath):
    cmd = "mysql -u root -h " + rwhost
    if rootpwd:
        cmd += " -p" + rootpwd

    write("+ Create MYSQL database")

    res = execute(cmd + " -s --skip-column-names -e \"select version()\"")
    if res.error:
        print("  \n"
              "  Could not determine MYSQL server version. Is the root password correct\n"
              "  and MYSQL is running and the client installed on "
              "this machine?",
              file=sys.stderr)
        return False

    write("  + Found MYSQL server version {}".format(res.data))
    if drop:
        q = "DROP DATABASE IF EXISTS \`{}\`;".format(db)
        print("  + Drop database {}".format(db))
        res = execute(cmd + " -e \"{}\"".format(q))
        if res.error:
            print("  + {}".format(res.error))
            return False

    write("  + Create database  {}".format(db))

    q = "CREATE DATABASE \`{}\` CHARACTER SET utf8 COLLATE utf8_bin;".format(db)
    res = execute(cmd + " -e \"{}\"".format(q))
    if res.error:
        print("  + {}".format(res.error))
        return False

    write("  + Setup user roles")

    # MySQL 8 requires explicit "CREATE USER", but this fails
    # if the user already exists.
    # "CREATE USER IF NOT EXISTS" is not supported by MySQL<5.7.
    # Drop possibly existing users, ignoring errors.

    res = execute(cmd + " -e \"SELECT 1 FROM mysql.user "
                  "WHERE user = '{}'\"".format(rwuser))
    if res.error:
        print("  + {}".format(res.error))
        return False

    q = ""
    exists = "1" in res.data
    if not exists:
        q += "CREATE USER '{}'@'localhost' IDENTIFIED BY '{}';".format(rwuser, rwpwd)
        q += "CREATE USER '{}'@'%' IDENTIFIED BY '{}';".format(rwuser, rwpwd)

    q += "GRANT ALL ON \`{}\`.* TO '{}'@'localhost';".format(db, rwuser)
    q += "GRANT ALL ON \`{}\`.* TO '{}'@'%';".format(db, rwuser)

    res = execute(cmd + " -e \"{}\"".format(q))
    if res.error:
        print("  + {}".format(res.error))
        return False

    if rwuser != rouser:
        res = execute(cmd + " -e \"SELECT 1 FROM mysql.user "
                      "WHERE user = '{}'\"".format(rouser))
        if res.error:
            print("  + {}".format(res.error))
            return False

        q = ""
        exists = "1" in res.data
        if not exists:
            q += "CREATE USER '{}'@'localhost' IDENTIFIED BY '{}';".format(rouser, ropwd)
            q += "CREATE USER '{}'@'%' IDENTIFIED BY '{}';".format(rouser, ropwd)

        q += "GRANT SELECT ON \`{}\`.* TO '{}'@'localhost';".format(db, rouser)
        q += "GRANT SELECT ON \`{}\`.* TO '{}'@'%';".format(db, rouser)

        res = execute(cmd + " -e \"{}\"".format(q))
        if res.error:
            print("  + {}".format(res.error))
            return False

    write("  + Create tables")
    q = "USE \`{}\`; source {};".format(db, os.path.join(schemapath, "mysql.sql"))
    res = execute(cmd + " -e \"{}\"".format(q))
    if res.error:
        print("  + {}".format(res.error))
        return False

    return True

def main():
    if len(sys.argv) != 10:
        print("Usage: mysql_setup.py <db> <rwuser> <rwpwd> <rouser> <ropwd> "
              "<rwhost> <mysql rootpwd> <drop> <schema path>\n\n"
              "For example: mysql_setup.py seiscomp sysop sysop sysop sysop "
              "localhost <password> false ~/seiscomp/share/db/")
        return 1

    db = sys.argv[1]
    rwuser = sys.argv[2]
    rwpwd = sys.argv[3]
    rouser = sys.argv[4]
    ropwd = sys.argv[5]
    rwhost = sys.argv[6]
    rootpwd = sys.argv[7]
    schemapath = sys.argv[9]

    drop = sys.argv[8].lower() == 'true'

    os.chdir(tempfile.gettempdir())
    if not createMYSQLDB(db, rwuser, rwpwd, rouser, ropwd, rwhost, rootpwd, drop, schemapath):
        return 1

    return 0

if __name__ == "__main__":
    sys.exit(main())
