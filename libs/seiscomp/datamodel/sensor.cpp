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
#include <seiscomp/datamodel/sensor.h>
#include <seiscomp/datamodel/inventory.h>
#include <algorithm>
#include <seiscomp/datamodel/version.h>
#include <seiscomp/datamodel/metadata.h>
#include <seiscomp/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(Sensor, PublicObject, "Sensor");


Sensor::MetaObject::MetaObject(const Core::RTTI *rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("name", "string", false, false, true, false, false, false, nullptr, &Sensor::setName, &Sensor::name));
	addProperty(Core::simpleProperty("description", "string", false, false, false, false, false, false, nullptr, &Sensor::setDescription, &Sensor::description));
	addProperty(Core::simpleProperty("model", "string", false, false, false, false, false, false, nullptr, &Sensor::setModel, &Sensor::model));
	addProperty(Core::simpleProperty("manufacturer", "string", false, false, false, false, false, false, nullptr, &Sensor::setManufacturer, &Sensor::manufacturer));
	addProperty(Core::simpleProperty("type", "string", false, false, false, false, false, false, nullptr, &Sensor::setType, &Sensor::type));
	addProperty(Core::simpleProperty("unit", "string", false, false, false, false, false, false, nullptr, &Sensor::setUnit, &Sensor::unit));
	addProperty(Core::simpleProperty("lowFrequency", "float", false, false, false, false, true, false, nullptr, &Sensor::setLowFrequency, &Sensor::lowFrequency));
	addProperty(Core::simpleProperty("highFrequency", "float", false, false, false, false, true, false, nullptr, &Sensor::setHighFrequency, &Sensor::highFrequency));
	addProperty(Core::simpleProperty("response", "string", false, false, false, true, false, false, nullptr, &Sensor::setResponse, &Sensor::response));
	addProperty(objectProperty<Blob>("remark", "Blob", false, false, true, &Sensor::setRemark, &Sensor::remark));
	addProperty(arrayClassProperty<SensorCalibration>("calibration", "SensorCalibration", &Sensor::sensorCalibrationCount, &Sensor::sensorCalibration, static_cast<bool (Sensor::*)(SensorCalibration*)>(&Sensor::add), &Sensor::removeSensorCalibration, static_cast<bool (Sensor::*)(SensorCalibration*)>(&Sensor::remove)));
}


IMPLEMENT_METAOBJECT(Sensor)


SensorIndex::SensorIndex() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SensorIndex::SensorIndex(const std::string& name_) {
	name = name_;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SensorIndex::SensorIndex(const SensorIndex &idx) {
	name = idx.name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SensorIndex::operator==(const SensorIndex &idx) const {
	return name == idx.name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SensorIndex::operator!=(const SensorIndex &idx) const {
	return !operator==(idx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Sensor::Sensor() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Sensor::Sensor(const Sensor &other)
: PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Sensor::Sensor(const std::string& publicID)
: PublicObject(publicID) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Sensor::~Sensor() {
	for ( auto &sensorCalibration : _sensorCalibrations ) {
		sensorCalibration->setParent(nullptr);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Sensor *Sensor::Create() {
	Sensor *object = new Sensor();
	return static_cast<Sensor*>(GenerateId(object));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Sensor *Sensor::Create(const std::string& publicID) {
	if ( PublicObject::IsRegistrationEnabled() && Find(publicID) != nullptr ) {
		SEISCOMP_ERROR(
			"There exists already a PublicObject with Id '%s'",
			publicID.c_str()
		);
		return nullptr;
	}

	return new Sensor(publicID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Sensor *Sensor::Find(const std::string& publicID) {
	return Sensor::Cast(PublicObject::Find(publicID));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sensor::operator==(const Sensor &rhs) const {
	if ( _index != rhs._index ) return false;
	if ( _description != rhs._description ) return false;
	if ( _model != rhs._model ) return false;
	if ( _manufacturer != rhs._manufacturer ) return false;
	if ( _type != rhs._type ) return false;
	if ( _unit != rhs._unit ) return false;
	if ( _lowFrequency != rhs._lowFrequency ) return false;
	if ( _highFrequency != rhs._highFrequency ) return false;
	if ( _response != rhs._response ) return false;
	if ( _remark != rhs._remark ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sensor::operator!=(const Sensor &rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sensor::equal(const Sensor &other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Sensor::setName(const std::string& name) {
	_index.name = name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Sensor::name() const {
	return _index.name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Sensor::setDescription(const std::string& description) {
	_description = description;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Sensor::description() const {
	return _description;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Sensor::setModel(const std::string& model) {
	_model = model;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Sensor::model() const {
	return _model;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Sensor::setManufacturer(const std::string& manufacturer) {
	_manufacturer = manufacturer;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Sensor::manufacturer() const {
	return _manufacturer;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Sensor::setType(const std::string& type) {
	_type = type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Sensor::type() const {
	return _type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Sensor::setUnit(const std::string& unit) {
	_unit = unit;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Sensor::unit() const {
	return _unit;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Sensor::setLowFrequency(const OPT(double)& lowFrequency) {
	_lowFrequency = lowFrequency;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Sensor::lowFrequency() const {
	if ( _lowFrequency )
		return *_lowFrequency;
	throw Seiscomp::Core::ValueException("Sensor.lowFrequency is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Sensor::setHighFrequency(const OPT(double)& highFrequency) {
	_highFrequency = highFrequency;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Sensor::highFrequency() const {
	if ( _highFrequency )
		return *_highFrequency;
	throw Seiscomp::Core::ValueException("Sensor.highFrequency is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Sensor::setResponse(const std::string& response) {
	_response = response;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Sensor::response() const {
	return _response;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Sensor::setRemark(const OPT(Blob)& remark) {
	_remark = remark;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Blob& Sensor::remark() {
	if ( _remark )
		return *_remark;
	throw Seiscomp::Core::ValueException("Sensor.remark is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Blob& Sensor::remark() const {
	if ( _remark )
		return *_remark;
	throw Seiscomp::Core::ValueException("Sensor.remark is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const SensorIndex &Sensor::index() const {
	return _index;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sensor::equalIndex(const Sensor *lhs) const {
	if ( !lhs ) {
		return false;
	}

	return lhs->index() == index();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Inventory *Sensor::inventory() const {
	return static_cast<Inventory*>(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Sensor &Sensor::operator=(const Sensor &other) {
	PublicObject::operator=(other);
	_index = other._index;
	_description = other._description;
	_model = other._model;
	_manufacturer = other._manufacturer;
	_type = other._type;
	_unit = other._unit;
	_lowFrequency = other._lowFrequency;
	_highFrequency = other._highFrequency;
	_response = other._response;
	_remark = other._remark;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sensor::assign(Object *other) {
	Sensor *otherSensor = Sensor::Cast(other);
	if ( !other ) {
		return false;
	}

	*this = *otherSensor;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sensor::attachTo(PublicObject *parent) {
	if ( !parent ) {
		return false;
	}

	// check all possible parents
	Inventory *inventory = Inventory::Cast(parent);
	if ( inventory != nullptr )
		return inventory->add(this);

	SEISCOMP_ERROR("Sensor::attachTo(%s) -> wrong class type", parent->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sensor::detachFrom(PublicObject *object) {
	if ( !object ) {
		return false;
	}

	// check all possible parents
	Inventory *inventory = Inventory::Cast(object);
	if ( inventory != nullptr ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return inventory->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			Sensor *child = inventory->findSensor(publicID());
			if ( child != nullptr )
				return inventory->remove(child);
			else {
				SEISCOMP_DEBUG("Sensor::detachFrom(Inventory): sensor has not been found");
				return false;
			}
		}
	}

	SEISCOMP_ERROR("Sensor::detachFrom(%s) -> wrong class type", object->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sensor::detach() {
	if ( !parent() ) {
		return false;
	}

	return detachFrom(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object *Sensor::clone() const {
	Sensor *clonee = new Sensor();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sensor::updateChild(Object *child) {
	SensorCalibration *sensorCalibrationChild = SensorCalibration::Cast(child);
	if ( sensorCalibrationChild != nullptr ) {
		SensorCalibration *sensorCalibrationElement = sensorCalibration(sensorCalibrationChild->index());
		if ( sensorCalibrationElement != nullptr ) {
			*sensorCalibrationElement = *sensorCalibrationChild;
			sensorCalibrationElement->update();
			return true;
		}
		return false;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Sensor::accept(Visitor *visitor) {
	if ( visitor->traversal() == Visitor::TM_TOPDOWN )
		if ( !visitor->visit(this) )
			return;

	for ( auto &&elem : _sensorCalibrations )
		elem->accept(visitor);

	if ( visitor->traversal() == Visitor::TM_BOTTOMUP )
		visitor->visit(this);
	else
		visitor->finished();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Sensor::sensorCalibrationCount() const {
	return _sensorCalibrations.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SensorCalibration *Sensor::sensorCalibration(size_t i) const {
	return _sensorCalibrations[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SensorCalibration *Sensor::sensorCalibration(const SensorCalibrationIndex &i) const {
	for ( const auto &elem : _sensorCalibrations ) {
		if ( i == elem->index() ) {
			return elem.get();
		}
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sensor::add(SensorCalibration *sensorCalibration) {
	if ( !sensorCalibration ) {
		return false;
	}

	// Element has already a parent
	if ( sensorCalibration->parent() != nullptr ) {
		SEISCOMP_ERROR("Sensor::add(SensorCalibration*) -> element has already a parent");
		return false;
	}

	// Duplicate index check
	for ( auto &&elem : _sensorCalibrations ) {
		if ( elem->index() == sensorCalibration->index() ) {
			SEISCOMP_ERROR("Sensor::add(SensorCalibration*) -> an element with the same index has been added already");
			return false;
		}
	}

	// Add the element
	_sensorCalibrations.push_back(sensorCalibration);
	sensorCalibration->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		sensorCalibration->accept(&nc);
	}

	// Notify registered observers
	childAdded(sensorCalibration);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sensor::remove(SensorCalibration *sensorCalibration) {
	if ( !sensorCalibration ) {
		return false;
	}

	if ( sensorCalibration->parent() != this ) {
		SEISCOMP_ERROR("Sensor::remove(SensorCalibration*) -> element has another parent");
		return false;
	}

	auto it = std::find(_sensorCalibrations.begin(), _sensorCalibrations.end(), sensorCalibration);
	// Element has not been found
	if ( it == _sensorCalibrations.end() ) {
		SEISCOMP_ERROR("Sensor::remove(SensorCalibration*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		Notifier::Create(this, OP_REMOVE, it->get());
	}

	(*it)->setParent(nullptr);
	childRemoved((*it).get());

	_sensorCalibrations.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sensor::removeSensorCalibration(size_t i) {
	// index out of bounds
	if ( i >= _sensorCalibrations.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		Notifier::Create(this, OP_REMOVE, _sensorCalibrations[i].get());
	}

	_sensorCalibrations[i]->setParent(nullptr);
	childRemoved(_sensorCalibrations[i].get());

	_sensorCalibrations.erase(_sensorCalibrations.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sensor::removeSensorCalibration(const SensorCalibrationIndex &i) {
	SensorCalibration *object = sensorCalibration(i);
	if ( !object ) {
		return false;
	}

	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Sensor::serialize(Archive &ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<Version::Major,Version::Minor>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: Sensor skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	PublicObject::serialize(ar);
	if ( !ar.success() ) return;

	ar & NAMED_OBJECT_HINT("name", _index.name, Archive::XML_MANDATORY | Archive::INDEX_ATTRIBUTE);
	ar & NAMED_OBJECT_HINT("description", _description, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("model", _model, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("manufacturer", _manufacturer, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("type", _type, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("unit", _unit, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("lowFrequency", _lowFrequency, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("highFrequency", _highFrequency, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT("response", _response);
	ar & NAMED_OBJECT_HINT("remark", _remark, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	if ( ar.hint() & Archive::IGNORE_CHILDS ) return;
	ar & NAMED_OBJECT_HINT("calibration",
		Seiscomp::Core::Generic::containerMember(
			_sensorCalibrations,
			[this](const SensorCalibrationPtr &sensorCalibration) {
				return add(sensorCalibration.get());
			}
		),
		Archive::STATIC_TYPE
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
