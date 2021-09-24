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

#include <seiscomp/geo/feature.h>
#include <iostream>


using namespace Seiscomp::Geo;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeature::GeoFeature(const Category* category, unsigned int rank)
: _category(category)
, _userData(nullptr)
, _rank(rank)
, _closedPolygon(false) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeature::GeoFeature(const std::string& name, const Category* category,
                       unsigned int rank)
: _name(name)
, _category(category)
, _userData(nullptr)
, _rank(rank)
, _closedPolygon(false) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeature::GeoFeature(const std::string &name, const Category* category,
                       unsigned int rank, const Attributes &attributes)
: _name(name)
, _category(category)
, _userData(nullptr)
, _rank(rank)
, _attributes(attributes)
, _closedPolygon(false) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeature::~GeoFeature() {
	_vertices.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeature::setRank(unsigned int rank) {
	_rank = rank;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeature::setAttribute(const std::string &name, const std::string &value) {
	_attributes[name] = value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeature::addVertex(const GeoCoordinate &v, bool newSubFeature) {
	// Mark this vertex as the beginning of a new sub feature. Note: The first
	// vertex is never a subfeature
	if ( newSubFeature && !_vertices.empty() )
		_subFeatures.push_back(_vertices.size());

	// Add the new vertex
	_vertices.push_back(v);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeature::updateBoundingBox() {
	size_t startIdx = 0, endIdx = 0;
	size_t nSubFeat = _subFeatures.size();

	_bbox = GeoBoundingBox();

	for ( size_t i = 0; i <= nSubFeat; ++i ) {
		endIdx = (i == nSubFeat ? _vertices.size() : _subFeatures[i]);
		GeoBoundingBox subFeatureBox;
		subFeatureBox.fromPolygon(endIdx-startIdx, &_vertices[startIdx], _closedPolygon);
		_bbox += subFeatureBox;
		startIdx = endIdx;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeature::invertOrder() {
	size_t startIdx = 0, endIdx = 0;
	size_t nSubFeat = _subFeatures.size();

	for ( size_t i = 0; i <= nSubFeat; ++i ) {
		endIdx = (i == nSubFeat ? _vertices.size() : _subFeatures[i]);
		size_t count = endIdx-startIdx;
		size_t halfCount = count/2;
		for ( size_t j = 0; j < halfCount; ++j )
			std::swap(_vertices[startIdx+j], _vertices[startIdx+count-1-j]);
		startIdx = endIdx;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

typedef std::pair<size_t, double> SubFeatureIndexWithArea;
bool cmpSubFeature(const SubFeatureIndexWithArea &sf1, const SubFeatureIndexWithArea &sf2) {
	return fabs(sf1.second) > fabs(sf2.second);
}

}

void GeoFeature::sort() {
	// Nothing to do without any subfeatures
	if ( _subFeatures.empty() || !_closedPolygon )
		return;

	std::vector<SubFeatureIndexWithArea> subFeatures;

	size_t startIdx = 0, endIdx;
	for ( size_t i = 0; i <= _subFeatures.size(); ++i, startIdx = endIdx ) {
		endIdx = (i == _subFeatures.size() ? _vertices.size() : _subFeatures[i]);
		double A = Seiscomp::Geo::area(&_vertices[startIdx], endIdx - startIdx);
		subFeatures.push_back(SubFeatureIndexWithArea(i, A));
	}

	std::sort(subFeatures.begin(), subFeatures.end(), cmpSubFeature);

	bool needResort = false;
	size_t i = 1;
	for ( ; i < subFeatures.size(); ++i ) {
		if ( subFeatures[i].first < subFeatures[i-1].first ) {
			needResort = true;
			break;
		}
	}

	if ( !needResort )
		return;

	GeoCoordinates tmpv(_vertices);
	std::vector<size_t> tmpsf(_subFeatures);
	size_t vi = 0;
	size_t sfi = 0;

	for ( size_t i = 0; i < subFeatures.size(); ++i ) {
		size_t sf = subFeatures[i].first;
		size_t startIdx = !sf ? 0 : tmpsf[sf-1];
		size_t endIdx = sf == tmpsf.size() ? tmpv.size() : tmpsf[sf];

		if ( sfi )
			_subFeatures[sfi-1] = vi;

		for ( size_t j = startIdx; j < endIdx; ++j )
			_vertices[vi++] = tmpv[j];

		++sfi;
	}

	assert(vi == _vertices.size());
	assert(sfi == _subFeatures.size()+1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeature::setUserData(void *d) {
	_userData = d;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoFeature::contains(const GeoCoordinate &v) const {
	if ( !closedPolygon() ) return false;

	size_t startIdx = 0, endIdx = 0;
	size_t nSubFeat = _subFeatures.size();
	bool isInside = false;

	for ( size_t i = 0; i <= nSubFeat; ++i ) {
		endIdx = (i == nSubFeat ? _vertices.size() : _subFeatures[i]);
		if ( Seiscomp::Geo::contains(v, &_vertices[startIdx], endIdx - startIdx) ) {
			isInside = !isInside;
		}

		startIdx = endIdx;
	}
	return isInside;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double GeoFeature::area() const {
	if ( !closedPolygon() ) return 0;

	size_t startIdx = 0, endIdx = 0;
	size_t nSubFeat = _subFeatures.size();
	double A = 0.0;

	for ( size_t i = 0; i <= nSubFeat; ++i ) {
		endIdx = (i == nSubFeat ? _vertices.size() : _subFeatures[i]);
		A += Seiscomp::Geo::area(&_vertices[startIdx], endIdx - startIdx);
		startIdx = endIdx;
	}

	return A;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double GeoFeature::area(const GeoCoordinate *polygon, size_t sides) {
	return Seiscomp::Geo::area(polygon, sides);
}
