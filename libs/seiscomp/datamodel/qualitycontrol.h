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


#ifndef SEISCOMP_DATAMODEL_QUALITYCONTROL_H
#define SEISCOMP_DATAMODEL_QUALITYCONTROL_H


#include <vector>
#include <seiscomp/datamodel/qclog.h>
#include <seiscomp/datamodel/waveformquality.h>
#include <seiscomp/datamodel/outage.h>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/publicobject.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(QualityControl);
DEFINE_SMARTPOINTER(QCLog);
DEFINE_SMARTPOINTER(WaveformQuality);
DEFINE_SMARTPOINTER(Outage);


class SC_SYSTEM_CORE_API QualityControl : public PublicObject {
	DECLARE_SC_CLASS(QualityControl)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		QualityControl();

		//! Copy constructor
		QualityControl(const QualityControl& other);

		//! Destructor
		~QualityControl() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		QualityControl& operator=(const QualityControl& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const QualityControl& other) const;
		bool operator!=(const QualityControl& other) const;

		//! Wrapper that calls operator==
		bool equal(const QualityControl& other) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		/**
		 * Add an object.
		 * @param obj The object pointer
		 * @return true The object has been added
		 * @return false The object has not been added
		 *               because it already exists in the list
		 *               or it already has another parent
		 */
		bool add(QCLog* obj);
		bool add(WaveformQuality* obj);
		bool add(Outage* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(QCLog* obj);
		bool remove(WaveformQuality* obj);
		bool remove(Outage* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeQCLog(size_t i);
		bool removeQCLog(const QCLogIndex& i);
		bool removeWaveformQuality(size_t i);
		bool removeWaveformQuality(const WaveformQualityIndex& i);
		bool removeOutage(size_t i);
		bool removeOutage(const OutageIndex& i);

		//! Retrieve the number of objects of a particular class
		size_t qCLogCount() const;
		size_t waveformQualityCount() const;
		size_t outageCount() const;

		//! Index access
		//! @return The object at index i
		QCLog* qCLog(size_t i) const;
		QCLog* qCLog(const QCLogIndex& i) const;

		WaveformQuality* waveformQuality(size_t i) const;
		WaveformQuality* waveformQuality(const WaveformQualityIndex& i) const;

		Outage* outage(size_t i) const;
		Outage* outage(const OutageIndex& i) const;

		//! Find an object by its unique attribute(s)
		QCLog* findQCLog(const std::string& publicID) const;

		//! Implement Object interface
		bool assign(Object *other) override;
		bool attachTo(PublicObject *parent) override;
		bool detachFrom(PublicObject *parent) override;
		bool detach() override;

		//! Creates a clone
		Object *clone() const override;

		//! Implement PublicObject interface
		bool updateChild(Object *child) override;

		void accept(Visitor *visitor) override;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Aggregations
		std::vector<QCLogPtr> _qCLogs;
		std::vector<WaveformQualityPtr> _waveformQualitys;
		std::vector<OutagePtr> _outages;

	DECLARE_SC_CLASSFACTORY_FRIEND(QualityControl);
};


}
}


#endif
