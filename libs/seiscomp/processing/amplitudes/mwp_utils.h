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


#ifndef SEISMOLOGY_MWP_H
#define SEISMOLOGY_MWP_H


/*
 * i0 is the index at which the onset is expected.
 * i < i0 is noise, i >= i0 is P-wave signal
 */
void   Mwp_double_integration(int n, double *f, int i0, double fsamp);

double Mwp_SNR(int n, double *f, int i0);

double Mwp_amplitude(int n, double *f, int i0, int *pos);

void   Mwp_demean(int n, double *f, int i0);
void   Mwp_taper(int n, double *f, int i0);


#endif
