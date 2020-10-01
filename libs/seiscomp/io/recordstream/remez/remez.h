/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * GNU Affero General Public License Usage                                 *
 * This file may be used under the terms of the GNU Affero                 *
 * Public License version 3.0 as published by the Free Software Foundation *
 * and appearing in the file LICENSE included in the packaging of this     *
 * file. Please review the following information to ensure the GNU Affero  *
 * Public License version 3.0 requirements will be met:                    *
 * https://www.gnu.org/licenses/agpl-3.0.html.                             *
 *                                                                         *
 * Other Usage                                                             *
 * Alternatively, this file may be used in accordance with the terms and   *
 * conditions contained in a signed written agreement between you and      *
 * gempa GmbH.                                                             *
 ***************************************************************************/
#ifndef REMEZ_H
#define REMEZ_H

#define BANDPASS       1
#define DIFFERENTIATOR 2
#define HILBERT        3

#define NEGATIVE       0
#define POSITIVE       1

#define Pi             3.1415926535897932
#define Pi2            6.2831853071795865

#define GRIDDENSITY    16
#define MAXITERATIONS  40

/* Function prototype for remez() - the only function that should need be
 * called from external code
 */
#ifdef __cplusplus
extern "C" {
#endif
int remez(double h[], int numtaps,
          int numband, double bands[], double des[], double weight[],
          int type);
#ifdef __cplusplus
}
#endif

#endif /* __REMEZ_H__ */

