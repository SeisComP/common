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


#define SEISCOMP_COMPONENT DataModel
#include <seiscomp/datamodel/integerquantity.h>
#include <seiscomp/datamodel/version.h>
#include <seiscomp/datamodel/metadata.h>
#include <seiscomp/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS(IntegerQuantity, "IntegerQuantity");


IntegerQuantity::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("value", "int", false, false, false, false, false, false, nullptr, &IntegerQuantity::setValue, &IntegerQuantity::value));
	addProperty(Core::simpleProperty("uncertainty", "int", false, false, false, false, true, false, nullptr, &IntegerQuantity::setUncertainty, &IntegerQuantity::uncertainty));
	addProperty(Core::simpleProperty("lowerUncertainty", "int", false, false, false, false, true, false, nullptr, &IntegerQuantity::setLowerUncertainty, &IntegerQuantity::lowerUncertainty));
	addProperty(Core::simpleProperty("upperUncertainty", "int", false, false, false, false, true, false, nullptr, &IntegerQuantity::setUpperUncertainty, &IntegerQuantity::upperUncertainty));
	addProperty(Core::simpleProperty("confidenceLevel", "float", false, false, false, false, true, false, nullptr, &IntegerQuantity::setConfidenceLevel, &IntegerQuantity::confidenceLevel));
}


IMPLEMENT_METAOBJECT(IntegerQuantity)


IntegerQuantity::IntegerQuantity() {
	_value = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IntegerQuantity::IntegerQuantity(const IntegerQuantity& other)
: Core::BaseObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IntegerQuantity::IntegerQuantity(int value,
                                 const OPT(int)& uncertainty,
                                 const OPT(int)& lowerUncertainty,
                                 const OPT(int)& upperUncertainty,
                                 const OPT(double)& confidenceLevel)
: _value(value)
, _uncertainty(uncertainty)
, _lowerUncertainty(lowerUncertainty)
, _upperUncertainty(upperUncertainty)
, _confidenceLevel(confidenceLevel)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IntegerQuantity::~IntegerQuantity() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IntegerQuantity::operator int&() {
	return _value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IntegerQuantity::operator int() const {
	return _value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool IntegerQuantity::operator==(const IntegerQuantity& rhs) const {
	if ( !(_value == rhs._value) )
		return false;
	if ( !(_uncertainty == rhs._uncertainty) )
		return false;
	if ( !(_lowerUncertainty == rhs._lowerUncertainty) )
		return false;
	if ( !(_upperUncertainty == rhs._upperUncertainty) )
		return false;
	if ( !(_confidenceLevel == rhs._confidenceLevel) )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool IntegerQuantity::operator!=(const IntegerQuantity& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool IntegerQuantity::equal(const IntegerQuantity& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void IntegerQuantity::setValue(int value) {
	_value = value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int IntegerQuantity::value() const {
	return _value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void IntegerQuantity::setUncertainty(const OPT(int)& uncertainty) {
	_uncertainty = uncertainty;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int IntegerQuantity::uncertainty() const {
	if ( _uncertainty )
		return *_uncertainty;
	throw Seiscomp::Core::ValueException("IntegerQuantity.uncertainty is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void IntegerQuantity::setLowerUncertainty(const OPT(int)& lowerUncertainty) {
	_lowerUncertainty = lowerUncertainty;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int IntegerQuantity::lowerUncertainty() const {
	if ( _lowerUncertainty )
		return *_lowerUncertainty;
	throw Seiscomp::Core::ValueException("IntegerQuantity.lowerUncertainty is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void IntegerQuantity::setUpperUncertainty(const OPT(int)& upperUncertainty) {
	_upperUncertainty = upperUncertainty;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int IntegerQuantity::upperUncertainty() const {
	if ( _upperUncertainty )
		return *_upperUncertainty;
	throw Seiscomp::Core::ValueException("IntegerQuantity.upperUncertainty is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void IntegerQuantity::setConfidenceLevel(const OPT(double)& confidenceLevel) {
	_confidenceLevel = confidenceLevel;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double IntegerQuantity::confidenceLevel() const {
	if ( _confidenceLevel )
		return *_confidenceLevel;
	throw Seiscomp::Core::ValueException("IntegerQuantity.confidenceLevel is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IntegerQuantity& IntegerQuantity::operator=(const IntegerQuantity& other) {
	_value = other._value;
	_uncertainty = other._uncertainty;
	_lowerUncertainty = other._lowerUncertainty;
	_upperUncertainty = other._upperUncertainty;
	_confidenceLevel = other._confidenceLevel;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void IntegerQuantity::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<Version::Major,Version::Minor>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: IntegerQuantity skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	ar & NAMED_OBJECT_HINT("value", _value, Archive::XML_ELEMENT | Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("uncertainty", _uncertainty, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("lowerUncertainty", _lowerUncertainty, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("upperUncertainty", _upperUncertainty, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("confidenceLevel", _confidenceLevel, Archive::XML_ELEMENT);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
