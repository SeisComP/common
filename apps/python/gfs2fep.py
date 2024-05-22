#!/usr/bin/env seiscomp-python
# -*- coding: utf-8 -*-

############################################################################
# Copyright (C) gempa GmbH                                                 #
# All rights reserved.                                                     #
# Contact: gempa GmbH (seiscomp-dev@gempa.de)                              #
#                                                                          #
# GNU Affero General Public License Usage                                  #
# This file may be used under the terms of the GNU Affero                  #
# Public License version 3.0 as published by the Free Software Foundation  #
# and appearing in the file LICENSE included in the packaging of this      #
# file. Please review the following information to ensure the GNU Affero   #
# Public License version 3.0 requirements will be met:                     #
# https://www.gnu.org/licenses/agpl-3.0.html.                              #
#                                                                          #
# Other Usage                                                              #
# Alternatively, this file may be used in accordance with the terms and    #
# conditions contained in a signed written agreement between you and       #
# gempa GmbH.                                                              #
############################################################################


import datetime
import getopt
import sys

from typing import TextIO

from seiscomp import geo


# -----------------------------------------------------------------------------
def printHelp():
    msg = """
gfs2fep - converts a SeisComP GeoFeatureSet file (GeoJSON or BNA) to FEP format

usage: {} [OPTIONS]

    -h, --help
        print this help message

    -i, --input
        input file (default: -)

    -o, --output
        output fep file (default: -)

    -a, --append
        append fep data to output file instead of overriding it

    -p, --precision (default: unrestricted)
        number of decimal places of coordintes"""
    print(msg.format(sys.argv[0]), file=sys.stderr)
    sys.exit(0)


# -----------------------------------------------------------------------------
def error(code, msg):
    print(f"error ({code}): {msg}", file=sys.stderr)
    sys.exit(code)


# -----------------------------------------------------------------------------
def run():
    if len(sys.argv) == 1:
        printHelp()

    inFile = "-"
    outFile = None
    append = False
    precision = None
    opts, _ = getopt.getopt(
        sys.argv[1:], "hi:o:ap:", ["help", "input=", "output=", "append", "precision"]
    )
    for o, a in opts:
        if o in ("-h", "--help"):
            printHelp()

        if o in ("-i", "--input"):
            inFile = a

        if o in ("-o", "--output"):
            outFile = a

        if o in ("-a", "--append"):
            append = True

        if o in ("-p", "--precision"):
            precision = max(int(a), 0)

    gfs = geo.GeoFeatureSet()
    if not gfs.readFile(inFile, None):
        error(1, f"Could not read from file '{inFile}'")

    # combine features sharing the same name
    featureDict = {}
    for f in gfs.features():
        if not f.closedPolygon():
            print(
                f"warning: feature not a closed polygon: {f.name()}",
                file=sys.stderr,
            )
        if f.name() in featureDict:
            featureDict[f.name()].append(f)
        else:
            featureDict[f.name()] = [f]

    # output is set to stdout or a file name if specified
    if outFile and outFile != "-":
        try:
            with open(outFile, "a" if append else "w", encoding="utf8") as fp:
                writeFEPFile(featureDict, inFile, fp, precision)
        except Exception as e:
            error(2, e)

    else:
        writeFEPFile(featureDict, inFile, sys.stdout, precision)
        sys.stdout.flush()


# -----------------------------------------------------------------------------
def writeFEPFile(featureDict: dict, inFile: str, fp: TextIO, precision: int = None):
    def _print(data: str):
        print(data, file=fp)

    if precision:

        def _printVertex(v):
            print(
                f"{v.longitude():.{precision}f} {v.latitude():.{precision}f}", file=fp
            )

    else:

        def _printVertex(v):
            print(f"{v.longitude()} {v.latitude()}", file=fp)

    _print(f"# created from file: {inFile}")
    _print(
        f"# created on {str(datetime.datetime.now())} by gfs2fep.py - (C) gempa GmbH"
    )
    _print("# LON LAT")

    # write fep
    for name, features in featureDict.items():
        # print("{}: {}".format(len(features), name))
        vCount = 0
        fStart = features[0].vertices()[0]
        v = fStart

        # iterate over features sharing name
        for f in features:
            # vertex array contains vertices of main land and sub features
            vertices = f.vertices()

            # sub feature array holds indices of starting points
            endIndices = list(f.subFeatures()) + [len(vertices)]

            # iterate of main land and sub features
            i = 0
            for iEnd in endIndices:
                vStart = vertices[i]
                while i < iEnd:
                    v = vertices[i]
                    _printVertex(v)
                    vCount += 1
                    i += 1

                # end sub feature on sub feature start
                v = vStart
                _printVertex(v)
                vCount += 1

                # go back to start of main land
                if v != vertices[0]:
                    v = vertices[0]
                    _printVertex(v)
                    vCount += 1

            # go back to start of first feature
            if v != fStart:
                v = fStart
                _printVertex(v)
                vCount += 1

        # end fep region
        _print(f"99.0 99.0 {vCount}")
        _print(f"L  {name}")


# -----------------------------------------------------------------------------
if __name__ == "__main__":
    run()
