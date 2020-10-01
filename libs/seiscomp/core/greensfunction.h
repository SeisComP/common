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



#ifndef SEISCOMP_CORE_GREENSFUNCTION_H
#define SEISCOMP_CORE_GREENSFUNCTION_H


#include <seiscomp/core/array.h>
#include <seiscomp/core/enumeration.h>
#include <seiscomp/core/exceptions.h>
#include <seiscomp/core.h>


namespace Seiscomp {
namespace Core {


MAKEENUM(
	GreensFunctionComponent,
	EVALUES(
		ZSS,
		ZDS,
		ZDD,
		RSS,
		RDS,
		RDD,
		TSS,
		TDS,
		ZEP,
		REP
	),
	ENAMES(
		"ZSS",
		"ZDS",
		"ZDD",
		"RSS",
		"RDS",
		"RDD",
		"TSS",
		"TDS",
		"ZEP",
		"REP"
	)
);


DEFINE_SMARTPOINTER(GreensFunction);

class SC_SYSTEM_CORE_API GreensFunction : public Core::BaseObject {
	DECLARE_SC_CLASS(GreensFunction);
	DECLARE_SERIALIZATION;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		GreensFunction();
		GreensFunction(const std::string &model, double distance,
		               double depth, double fsamp, double timeOffset);

		virtual ~GreensFunction();

	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		void setId(const std::string &id);
		const std::string &id() const;

		void setModel(const std::string &model);
		const std::string &model() const;

		void setDistance(double);
		double distance() const;

		void setDepth(double);
		double depth() const;

		void setSamplingFrequency(double);
		double samplingFrequency() const;

		void setTimeOffset(double);
		double timeOffset() const;

		//! Returns the length in seconds
		double length(GreensFunctionComponent) const;

		void setData(GreensFunctionComponent, Array *);
		Array *data(GreensFunctionComponent) const;

		void setData(int, Array *);
		Array *data(int) const;


	// ------------------------------------------------------------------
	//  Private members
	// ------------------------------------------------------------------
	private:
		std::string _id;
		std::string _model;
		double      _distance;
		double      _depth;
		double      _samplingFrequency;
		double      _timeOffset;
		ArrayPtr    _components[GreensFunctionComponent::Quantity];
};


}
}


#endif
