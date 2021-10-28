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


#ifndef SEISCOMP_DATAMODEL_PICK_H
#define SEISCOMP_DATAMODEL_PICK_H


#include <seiscomp/datamodel/timequantity.h>
#include <seiscomp/datamodel/waveformstreamid.h>
#include <string>
#include <seiscomp/datamodel/realquantity.h>
#include <seiscomp/datamodel/types.h>
#include <seiscomp/datamodel/phase.h>
#include <seiscomp/datamodel/creationinfo.h>
#include <vector>
#include <seiscomp/datamodel/comment.h>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/publicobject.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Pick);
DEFINE_SMARTPOINTER(Comment);

class EventParameters;


/**
 * \brief A pick is the observation of an amplitude anomaly in a
 * \brief seismogram at a
 * \brief specific point in time. It is not necessarily related to a
 * \brief seismic event.
 */
class SC_SYSTEM_CORE_API Pick : public PublicObject {
	DECLARE_SC_CLASS(Pick)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		Pick();

	public:
		//! Copy constructor
		Pick(const Pick& other);

		//! Constructor with publicID
		Pick(const std::string& publicID);

		//! Destructor
		~Pick() override;
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static Pick* Create();
		static Pick* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static Pick* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		Pick& operator=(const Pick& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const Pick& other) const;
		bool operator!=(const Pick& other) const;

		//! Wrapper that calls operator==
		bool equal(const Pick& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Observed onset time of signal ("pick time").
		void setTime(const TimeQuantity& time);
		TimeQuantity& time();
		const TimeQuantity& time() const;

		//! Identifes the waveform stream.
		void setWaveformID(const WaveformStreamID& waveformID);
		WaveformStreamID& waveformID();
		const WaveformStreamID& waveformID() const;

		//! Identifies the filter or filter setup used for filtering
		//! the waveform
		//! stream referenced by waveformID.
		void setFilterID(const std::string& filterID);
		const std::string& filterID() const;

		//! Identifies the picker that produced the pick. This can be
		//! either a
		//! detection software program or a person.
		void setMethodID(const std::string& methodID);
		const std::string& methodID() const;

		//! Observed horizontal slowness of the signal. Most relevant
		//! in array measurements
		//! in s/deg.
		void setHorizontalSlowness(const OPT(RealQuantity)& horizontalSlowness);
		RealQuantity& horizontalSlowness();
		const RealQuantity& horizontalSlowness() const;

		//! Observed backazimuth of the signal. Most relevant in array
		//! measurements
		//! in degrees.
		void setBackazimuth(const OPT(RealQuantity)& backazimuth);
		RealQuantity& backazimuth();
		const RealQuantity& backazimuth() const;

		//! Identifies the method that was used to determine the
		//! slowness.
		void setSlownessMethodID(const std::string& slownessMethodID);
		const std::string& slownessMethodID() const;

		//! Flag that roughly categorizes the sharpness of the onset.
		void setOnset(const OPT(PickOnset)& onset);
		PickOnset onset() const;

		//! Tentative phase identification as specified by the picker.
		void setPhaseHint(const OPT(Phase)& phaseHint);
		Phase& phaseHint();
		const Phase& phaseHint() const;

		//! Indicates the polarity of first motion, usually from
		//! impulsive onsets.
		void setPolarity(const OPT(PickPolarity)& polarity);
		PickPolarity polarity() const;

		//! Evaluation mode of Pick.
		void setEvaluationMode(const OPT(EvaluationMode)& evaluationMode);
		EvaluationMode evaluationMode() const;

		//! Evaluation status of Pick.
		void setEvaluationStatus(const OPT(EvaluationStatus)& evaluationStatus);
		EvaluationStatus evaluationStatus() const;

		//! CreationInfo for the Pick object.
		void setCreationInfo(const OPT(CreationInfo)& creationInfo);
		CreationInfo& creationInfo();
		const CreationInfo& creationInfo() const;

	
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
		bool add(Comment* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(Comment* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeComment(size_t i);
		bool removeComment(const CommentIndex& i);

		//! Retrieve the number of objects of a particular class
		size_t commentCount() const;

		//! Index access
		//! @return The object at index i
		Comment* comment(size_t i) const;
		Comment* comment(const CommentIndex& i) const;

		//! Find an object by its unique attribute(s)

		EventParameters* eventParameters() const;

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
		// Attributes
		TimeQuantity _time;
		WaveformStreamID _waveformID;
		std::string _filterID;
		std::string _methodID;
		OPT(RealQuantity) _horizontalSlowness;
		OPT(RealQuantity) _backazimuth;
		std::string _slownessMethodID;
		OPT(PickOnset) _onset;
		OPT(Phase) _phaseHint;
		OPT(PickPolarity) _polarity;
		OPT(EvaluationMode) _evaluationMode;
		OPT(EvaluationStatus) _evaluationStatus;
		OPT(CreationInfo) _creationInfo;

		// Aggregations
		std::vector<CommentPtr> _comments;

	DECLARE_SC_CLASSFACTORY_FRIEND(Pick);
};


}
}


#endif
