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

def createSQLite3DB(filename, schemapath, override):
    cmd = "sqlite3 {} < {}".format(filename, os.path.join(schemapath, "sqlite3.sql"))
    write("+ Create SQLite3 database at '{}'".format(filename))

    if os.path.exists(filename) and not override:
        print("+ Database file '{}' exists".format(filename))
        return False

    path = os.path.dirname(filename)
    if not os.path.exists(path):
        try:
            os.makedirs(path)
        except OSError as err:
            print("+ Could not create directory '{}': {} ".format(path, err))
            return False

    res = execute(cmd)
    if res.error:
        print(" + {}".format(res.error))
        return False

    return True

def main():
    if len(sys.argv) != 4:
        print("Usage: sqlite3_setup.py <filename> ~/seiscomp/share/db/ <override>")
        return 1

    filename = sys.argv[1]
    schemapath = sys.argv[2]
    override = sys.argv[3].lower() == "true"

    os.chdir(tempfile.gettempdir())
    if not createSQLite3DB(filename, schemapath, override):
        return 1

    return 0

if __name__ == "__main__":
    sys.exit(main())
