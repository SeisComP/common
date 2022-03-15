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

import subprocess
import sys

#------------------------------------------------------------------------------
# Python version depended string conversion
if sys.version_info[0] < 3:
    py3bstr = str
    py3ustr = str

else:
    py3bstr = lambda s: s.encode('utf-8')
    py3ustr = lambda s: s.decode('utf-8', 'replace')

class ProcResult:
    def __init__(self):
        self.error = ""
        self.data = ""

def execute(cmd):
    proc = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        shell=True)
    out = proc.communicate()

    res = ProcResult()
    res.data = py3ustr(out[0].strip())
    if proc.returncode != 0:
        res.error = py3ustr(out[1].strip())
        if not res.error:
            res.error = "Error: Process returned exit code: {}".format(proc.returncode)
    else:
        # Ignore any stderr output in case of the exit code is 0
        res.error = None

    return res

def write(msg):
    print(msg)
    sys.stdout.flush()
