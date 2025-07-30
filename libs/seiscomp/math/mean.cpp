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


#include<algorithm>
#include<vector>
#include<stdexcept>
using namespace std;

#include<seiscomp/math/math.h>
#include<seiscomp/math/mean.h>

namespace Seiscomp
{
namespace Math
{
namespace Statistics
{


double median(const DoubleArray &v)
{
	return median(v.size(), (const double*)v.data());
}

double median(const std::vector<double> &v)
{
	return median(v.size(), v.data());
}

double median(int n, const double *f)
{
	if (n==0)
		throw std::out_of_range("attempted computation of median for zero-length array");
	vector<double> v(f, &f[n]);
	sort(v.begin(), v.end());
	int mid = n/2;

	return (n%2) ? v[mid] : (v[mid-1]+v[mid])/2;
}

double fractile(const DoubleArray &v, double x)
{
	return fractile(v.size(), (const double*)v.data(), x);
}

double fractile(const std::vector<double> &v, double x)
{
	return fractile(v.size(), v.data(), x);
}

double fractile(int n, const double *f, double x)
{
	vector<double> v(f, f + n);
	sort(v.begin(), v.end());

	double i = double(v.size()-1)*x;
	double diff = double(int(i))-i;

	if (diff==0)
		return v[int(i)];

	return (v[int(i)]-v[int(i)+1])*diff + v[int(i)];
}


double mean(const DoubleArray &v)
{
	return mean(v.size(), (const double*)v.data());
}

double mean(const std::vector<double> &v)
{
	return mean(v.size(), v.data());
}

double mean(int n, const double *f)
{
	double m=0;

	for (int i=0; i<n; i++)
		m += f[i];

	return m/n;
}


namespace {

struct WeightedDouble {
	double value;
	double weight;
	size_t index;

	WeightedDouble() {}
	WeightedDouble(double v) : value(v), weight(1.) {}

	WeightedDouble& operator=(double v) {
		value = v;
		weight = 1.;
		return *this;
	}

	bool operator<(const WeightedDouble &other) {
		return value < other.value;
	}

	operator double() const { return value; }
};

}

bool
computeTrimmedMean(int n, const double *f, double percent,
                   double &value, double &stdev, double *weights)
// From Rosenberger, J.L., and Gasko, M. (1983). "Comparing Location
// Estimators: Trimmed Means, Medians, and Trimean", in Understanding
// Robust and Exploratory Data Analysis, ed. Hoaglin, D.C., Mosteller,
// F., and Tukey, J.W., p. 297-338, John Wiley, NY
{
	double xl = percent*0.005, cumd=0, cumw=0., cumv=0;
	int k = int(n*xl+1e-5);

	vector<WeightedDouble> v(n);

	// Fill mapped indices
	for ( size_t i = 0; i < v.size(); ++i ) {
		v[i].value = f[i];
		v[i].index = i;
	}

	sort(v.begin(), v.end());

	value = stdev = 0.;

	// set weights and compute (weighted) mean
	for (int i=0; i<n; i++) {

		if (i >= k+1 && i< n-k-1) {
			v[i].weight = 1;
		}
		else if (i==k || i==n-k-1) {
			v[i].weight = k+1-n*xl;
		}
		else {
			v[i].weight = 0;
		}

		cumv += v[i].weight*v[i].value;
		cumw += v[i].weight;
	}

	value = cumv/cumw;

	// compute (weighted) standard deviation
	for (int i=0; i<n; i++) {
		double dv = v[i].value-value;
		cumd += v[i].weight*dv*dv;
	}

	if ( weights ) {
		for (int i=0; i<n; i++)
			weights[v[i].index] = v[i].weight;
	}

	if ( cumw > 1 )
		stdev = sqrt(cumd/(cumw-1));

	return true;
}

bool
computeTrimmedMean(int n, const double *f, double &value, double &stdev, double *weights)
{
	return computeTrimmedMean(n, f, 0., value, stdev, weights);
}

bool
computeTrimmedMean(const std::vector<double> &v, double percent,
                   double &value, double &stdev, std::vector<double> *weights)
{
	if ( weights ) {
		weights->resize(v.size());
		return computeTrimmedMean(v.size(), v.data(), percent, value, stdev, weights->data());
	}

	return computeTrimmedMean(v.size(), v.data(), percent, value, stdev, nullptr);
}

bool
computeMean(const std::vector<double> &v, double &value, double &stdev)
{
	return computeTrimmedMean(v.size(), v.data(), 0., value, stdev);
}

bool
computeMedianTrimmedMean(int n, const double *f, double distance, double &value, double &stdev, double *weights)
{
	double m = median(n, f);
	double cum = 0;
	int count = 0;

	for ( int i = 0; i < n; ++i ) {
		if ( abs(f[i] - m) <= distance ) {
			cum += f[i];
			++count;
			if ( weights )
				weights[i] = 1;
		}
		else if ( weights )
			weights[i] = 0;
	}

	value = cum / count;

	cum = 0;

	for ( int i = 0; i < n; ++i ) {
		if ( abs(f[i] - m) <= distance ) {
			double dv = f[i] - value;
			cum += dv * dv;
		}
	}

	if ( count > 1 )
		stdev = sqrt(cum/(count-1));
	else
		stdev = 0;

	return true;
}

bool
computeMedianTrimmedMean(const std::vector<double> &v, double distance, double &value, double &stdev, std::vector<double> *weights)
{
	if ( weights ) {
		weights->resize(v.size());
		return computeMedianTrimmedMean(v.size(), v.data(), distance, value, stdev, weights->data());
	}

	return computeMedianTrimmedMean(v.size(), v.data(), distance, value, stdev, nullptr);
}


bool
average(int n, const double *values, const double *weights, double &value, double &stdev) {
	double cumd = 0, cumw = 0., cumv = 0;

	// set weights and compute (weighted) mean
	for ( int i = 0; i < n; ++i ) {
		cumv += weights[i] * values[i];
		cumw += weights[i];
	}

	value = cumv / cumw;

	// compute (weighted) standard deviation
	for ( int i = 0; i < n; ++i ) {
		double dv = values[i] - value;
		cumd += weights[i] * dv * dv;
	}

	if ( cumw > 1 )
		stdev = sqrt(cumd/(cumw-1));
	else
		stdev = -1;

	return true;
}

bool
average(const std::vector<double> &values, const std::vector<double> &weights, double &value, double &stdev) {
	if ( values.size() != weights.size() )
		return false;

	return average((int)values.size(), values.data(), weights.data(), value, stdev);
}

double
trimmedMean(int n, const double *f, double percent)
{
// XXX deprecated XXX
	double value=0, stdev=0;
	computeTrimmedMean(n, f, percent, value, stdev);

	return value;
}

double
trimmedMean(const std::vector<double> &v, double percent)
{
// XXX deprecated XXX
	return trimmedMean(v.size(), v.data(), percent);
}

double
trimmedMean(const DoubleArray &v, double percent)
{
// XXX deprecated XXX
	return trimmedMean(v.size(), (const double*)v.data(), percent);
}


namespace {

template <typename T>
void _computeLinearTrend(int cnt, const T *data, double &m, double &n) {
	if ( cnt <= 1 ) {
		m = 0; n = 0;
		return;
	}

	T xm = T(cnt-1)*(T)0.5;
	T ym = 0;

	for ( int i = 0; i < cnt; ++i )
		ym += data[i];

	ym /= (T)cnt;

	T covar = 0, varx = 0;

	for ( int i = 0; i < cnt; ++i ) {
		T x = (T)i-xm;
		covar += x*(data[i]-ym);
		varx += x*x;
	}

	m = covar / varx;
	n = ym - m*xm;
}


template <typename T>
void _detrend(int cnt, T *data, double m, double n) {
	for ( int i = 0; i < cnt; ++i )
		data[i] = data[i] - (n + m*(T)i);
}


}


void detrend(int cnt, float *data, double m, double n) {
	_detrend(cnt, data, m, n);
}


void detrend(int cnt, double *data, double m, double n) {
	_detrend(cnt, data, m, n);
}


void detrend(std::vector<float> &data, double m, double n) {
	detrend((int)data.size(), data.data(), m, n);
}


void detrend(std::vector<double> &data, double m, double n) {
	detrend((int)data.size(), data.data(), m, n);
}

void computeLinearTrend(const std::vector<float> &data, double &m, double &n) {
	_computeLinearTrend((int)data.size(), data.data(), m, n);
}


void computeLinearTrend(const std::vector<double> &data, double &m, double &n) {
	_computeLinearTrend((int)data.size(), data.data(), m, n);
}


void computeLinearTrend(int cnt, const float *data, double &m, double &n) {
	_computeLinearTrend(cnt, data, m, n);
}


void computeLinearTrend(int cnt, const double *data, double &m, double &n) {
	_computeLinearTrend(cnt, data, m, n);
}


} // namespace Statistics
} // namespace Math
} // namespace Seiscomp
