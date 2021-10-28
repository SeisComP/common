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


#ifndef SEISCOMP_DATAMODEL_FOCALMECHANISM_H
#define SEISCOMP_DATAMODEL_FOCALMECHANISM_H


#include <string>
#include <seiscomp/datamodel/nodalplanes.h>
#include <seiscomp/datamodel/principalaxes.h>
#include <seiscomp/datamodel/types.h>
#include <seiscomp/datamodel/creationinfo.h>
#include <vector>
#include <seiscomp/datamodel/comment.h>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/publicobject.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(FocalMechanism);
DEFINE_SMARTPOINTER(Comment);
DEFINE_SMARTPOINTER(MomentTensor);

class EventParameters;


/**
 * \brief This class describes the focal mechanism of an event. It
 * \brief includes different
 * \brief descriptions like nodal planes, principal axes, and a
 * \brief moment tensor.
 * \brief The moment tensor description is provided by objects of the
 * \brief class
 * \brief MomentTensor which can be specified as child elements of
 * \brief FocalMechanism.
 */
class SC_SYSTEM_CORE_API FocalMechanism : public PublicObject {
	DECLARE_SC_CLASS(FocalMechanism)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		FocalMechanism();

	public:
		//! Copy constructor
		FocalMechanism(const FocalMechanism& other);

		//! Constructor with publicID
		FocalMechanism(const std::string& publicID);

		//! Destructor
		~FocalMechanism() override;
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static FocalMechanism* Create();
		static FocalMechanism* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static FocalMechanism* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		FocalMechanism& operator=(const FocalMechanism& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const FocalMechanism& other) const;
		bool operator!=(const FocalMechanism& other) const;

		//! Wrapper that calls operator==
		bool equal(const FocalMechanism& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Refers to the publicID of the triggering origin.
		void setTriggeringOriginID(const std::string& triggeringOriginID);
		const std::string& triggeringOriginID() const;

		//! Nodal planes of the focal mechanism.
		void setNodalPlanes(const OPT(NodalPlanes)& nodalPlanes);
		NodalPlanes& nodalPlanes();
		const NodalPlanes& nodalPlanes() const;

		//! Principal axes of the focal mechanism.
		void setPrincipalAxes(const OPT(PrincipalAxes)& principalAxes);
		PrincipalAxes& principalAxes();
		const PrincipalAxes& principalAxes() const;

		//! Largest azimuthal gap in distribution of stations used for
		//! determination
		//! of focal mechanism in degrees.
		void setAzimuthalGap(const OPT(double)& azimuthalGap);
		double azimuthalGap() const;

		//! Number of station polarities used for determination of
		//! focal mechanism.
		void setStationPolarityCount(const OPT(int)& stationPolarityCount);
		int stationPolarityCount() const;

		//! Fraction of misfit polarities in a first-motion focal
		//! mechanism determination.
		//! Decimal fraction between 0 and 1.
		void setMisfit(const OPT(double)& misfit);
		double misfit() const;

		//! Station distribution ratio (STDR) parameter. Indicates how
		//! the stations
		//! are distributed about the focal sphere (Reasenberg and
		//! Oppenheimer 1985).
		//! Decimal fraction between 0 and 1.
		void setStationDistributionRatio(const OPT(double)& stationDistributionRatio);
		double stationDistributionRatio() const;

		//! Resource identifier of the method used for determination of
		//! the focal mechanism.
		void setMethodID(const std::string& methodID);
		const std::string& methodID() const;

		//! Evaluation mode of FocalMechanism.
		void setEvaluationMode(const OPT(EvaluationMode)& evaluationMode);
		EvaluationMode evaluationMode() const;

		//! Evaluation status of FocalMechanism.
		void setEvaluationStatus(const OPT(EvaluationStatus)& evaluationStatus);
		EvaluationStatus evaluationStatus() const;

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
		bool add(MomentTensor* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(Comment* obj);
		bool remove(MomentTensor* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeComment(size_t i);
		bool removeComment(const CommentIndex& i);
		bool removeMomentTensor(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t commentCount() const;
		size_t momentTensorCount() const;

		//! Index access
		//! @return The object at index i
		Comment* comment(size_t i) const;
		Comment* comment(const CommentIndex& i) const;
		MomentTensor* momentTensor(size_t i) const;

		//! Find an object by its unique attribute(s)
		MomentTensor* findMomentTensor(const std::string& publicID) const;

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
		std::string _triggeringOriginID;
		OPT(NodalPlanes) _nodalPlanes;
		OPT(PrincipalAxes) _principalAxes;
		OPT(double) _azimuthalGap;
		OPT(int) _stationPolarityCount;
		OPT(double) _misfit;
		OPT(double) _stationDistributionRatio;
		std::string _methodID;
		OPT(EvaluationMode) _evaluationMode;
		OPT(EvaluationStatus) _evaluationStatus;
		OPT(CreationInfo) _creationInfo;

		// Aggregations
		std::vector<CommentPtr> _comments;
		std::vector<MomentTensorPtr> _momentTensors;

	DECLARE_SC_CLASSFACTORY_FRIEND(FocalMechanism);
};


}
}


#endif
