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


#ifndef SEISCOMP_DATAMODEL_QCLOG_H
#define SEISCOMP_DATAMODEL_QCLOG_H


#include <seiscomp/datamodel/waveformstreamid.h>
#include <string>
#include <seiscomp/core/datetime.h>
#include <seiscomp/datamodel/publicobject.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(QCLog);

class QualityControl;


class SC_SYSTEM_CORE_API QCLogIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		QCLogIndex();
		QCLogIndex(Seiscomp::Core::Time start,
		           const WaveformStreamID& waveformID);

		//! Copy constructor
		QCLogIndex(const QCLogIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const QCLogIndex&) const;
		bool operator!=(const QCLogIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		Seiscomp::Core::Time start;
		WaveformStreamID waveformID;
};


class SC_SYSTEM_CORE_API QCLog : public PublicObject {
	DECLARE_SC_CLASS(QCLog)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		QCLog();

	public:
		//! Copy constructor
		QCLog(const QCLog& other);

		//! Constructor with publicID
		QCLog(const std::string& publicID);

		//! Destructor
		~QCLog() override;
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static QCLog* Create();
		static QCLog* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static QCLog* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		QCLog& operator=(const QCLog& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const QCLog& other) const;
		bool operator!=(const QCLog& other) const;

		//! Wrapper that calls operator==
		bool equal(const QCLog& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setWaveformID(const WaveformStreamID& waveformID);
		WaveformStreamID& waveformID();
		const WaveformStreamID& waveformID() const;

		void setCreatorID(const std::string& creatorID);
		const std::string& creatorID() const;

		void setCreated(Seiscomp::Core::Time created);
		Seiscomp::Core::Time created() const;

		void setStart(Seiscomp::Core::Time start);
		Seiscomp::Core::Time start() const;

		void setEnd(Seiscomp::Core::Time end);
		Seiscomp::Core::Time end() const;

		void setMessage(const std::string& message);
		const std::string& message() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const QCLogIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const QCLog* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		QualityControl* qualityControl() const;

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
		// Index
		QCLogIndex _index;

		// Attributes
		std::string _creatorID;
		Seiscomp::Core::Time _created;
		Seiscomp::Core::Time _end;
		std::string _message;

	DECLARE_SC_CLASSFACTORY_FRIEND(QCLog);
};


}
}


#endif
