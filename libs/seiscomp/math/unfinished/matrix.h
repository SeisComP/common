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

# ifndef _MATRIX_H_
# define _MATRIX_H_

# define ZZ 0
# define NN 1
# define EE 2
# define ZN 3
# define ZE 4
# define NE 5

class Matrix
{
public:
    Matrix(unsigned rows, unsigned cols);  // Default constructor
    ~Matrix();                             // Destructor
    Matrix(Matrix const &m);               // Copy constructor

    void    operator=  (Matrix const &m);  // Assignment operator
    double& operator() (unsigned row, unsigned col);
    double  operator() (unsigned row, unsigned col) const;

private:
    unsigned nrows, ncols;  // number of rows/columns
    double *data;       // data
};


int comp_cov_mat (int, float*, float*, float*, float*);
int comp_cov_mat_eig_val (float*, float*);

# endif
