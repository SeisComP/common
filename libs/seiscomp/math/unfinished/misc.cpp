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

// namespace is Seiscomp::Filter

#include<math.h>

#include<vector>
#include<algorithm>
using namespace std;

template<typename TYPE>
static vector<TYPE> mytransform(vector<TYPE> const &f, double (*func)(double))
{
	vector<TYPE> y(f.size());
	transform( f.begin() , f.end() , y.begin() , func) ;
	return y;
}

template<typename TYPE>
vector<TYPE>   sin(vector<TYPE> const &f) { return mytransform(f, ::sin); }
template<typename TYPE>
vector<TYPE>   cos(vector<TYPE> const &f) { return mytransform(f, ::cos); }
template<typename TYPE>
vector<TYPE>   tan(vector<TYPE> const &f) { return mytransform(f, ::tan); }
template<typename TYPE>
vector<TYPE>  sqrt(vector<TYPE> const &f) { return mytransform(f, ::sqrt); }
template<typename TYPE>
vector<TYPE>   log(vector<TYPE> const &f) { return mytransform(f, ::log); }
template<typename TYPE>
vector<TYPE> log10(vector<TYPE> const &f) { return mytransform(f, ::log10); }
template<typename TYPE>
vector<TYPE>   exp(vector<TYPE> const &f) { return mytransform(f, ::exp); }

template<typename TYPE>
static vector<TYPE> arange(TYPE xmax)
{
	int size = int(xmax);
	size += int(ceil(xmax-size));
	vector<TYPE> y(size);
	TYPE *yy = &y[0];
	for(int i=0; i<size; i++)
		yy[i] = TYPE(i);
	return y;
}

#include<iostream>

int main()
{
	for (int count=0; count<5000; count++)
	{
//		vector<double> x(10000, 0.5), y;
//		y = sin(x);
//		cerr << x[4] << " " << y[4] << endl; break;
		vector<double> y;
		y = arange(10.1);
		for (int i=0; i<y.size(); i++)
		    cerr << i << " " << y[i]/3 << endl;
		break;
	}
}
