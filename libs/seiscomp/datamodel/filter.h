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


#ifndef SEISCOMP_DATAMODEL_FILTER_H
#define SEISCOMP_DATAMODEL_FILTER_H


#include <seiscomp/core.h>
#include <seiscomp/datamodel/event.h>
#include <seiscomp/datamodel/origin.h>
#include <seiscomp/datamodel/magnitude.h>

#include <string>


namespace Seiscomp {
namespace DataModel {


/**
 * @brief Abstract context providing named value lookup for filter evaluation.
 *
 * Subclasses bind a specific data model object (e.g. Event + Origin) to
 * a flat key/value interface that FilterExpression nodes can query.
 *
 * Each key may resolve to a string value, a numeric (double) value, or
 * neither. Implementations should return false from the relevant getter
 * if the key is unknown or the value is not set on the underlying object.
 */
class SC_SYSTEM_CORE_API FilterContext {
	public:
		virtual ~FilterContext() {}

		virtual bool getString(const std::string &key,
		                       std::string &value) const = 0;
		virtual bool getDouble(const std::string &key,
		                       double &value) const = 0;
};


/**
 * @brief Abstract expression node in a filter expression tree.
 *
 * Each node evaluates to true or false given a FilterContext.
 * Trees are built by parseFilterExpression() using the LeParser
 * infrastructure and evaluated by calling eval() on the root.
 */
class SC_SYSTEM_CORE_API FilterExpression {
	public:
		virtual ~FilterExpression() {}
		virtual bool eval(const FilterContext &ctx) const = 0;
};


/**
 * @brief Concrete FilterContext for an Event and its preferred Origin.
 *
 * Supported string keys (all lowercase, case-sensitive):
 *   type               — event type (e.g. "earthquake", "not-existing")
 *   evaluationstatus   — origin evaluation status (e.g. "preliminary", "confirmed")
 *   evaluationmode     — origin evaluation mode ("automatic", "manual")
 *   typecertainty      — event type certainty ("known", "suspected", "damaging", "felt")
 *   agencyid           — agency ID from the preferred origin's creation info
 *   author             — author from the preferred origin's creation info
 *
 * Supported double keys (all lowercase, case-sensitive):
 *   lat / latitude     — preferred origin latitude in degrees
 *   lon / longitude    — preferred origin longitude in degrees
 *   depth              — preferred origin depth in km
 *   magnitude          — preferred magnitude value
 *   rms                — origin quality RMS residual
 *   azimuthalgap       — origin quality azimuthal gap in degrees
 *
 * String values on both sides of a comparison are normalised to lowercase
 * with spaces and underscores replaced by hyphens, so "not-existing",
 * "not existing" and "not_existing" all match the same enum string.
 * Keys are NOT normalised — they must be written in lowercase exactly as
 * listed above.
 */
class SC_SYSTEM_CORE_API EventFilterContext : public FilterContext {
	public:
		EventFilterContext(const Event *event, const Origin *origin,
		                   const Magnitude *magnitude = nullptr);

		bool getString(const std::string &key, std::string &value) const override;
		bool getDouble(const std::string &key, double &value) const override;

	private:
		const Event     *_event;
		const Origin    *_origin;
		const Magnitude *_magnitude;
};


/**
 * @brief Parse a condition string into a FilterExpression tree.
 *
 * Boolean operators: & (AND), | (OR), ! (NOT), parentheses for grouping.
 * Leaf format:  key==value  key<value  key>value  key<=value  key>=value
 *
 * Note: != is NOT a valid leaf operator because the LeTokenizer treats '!'
 * as the logical NOT operator and splits it from the rest of the token.
 * To express not-equal, use the NOT operator: !(key==value).
 *
 * String comparison uses == only (with the NOT wrapper for not-equal).
 * Numeric comparison supports == < > <= >=; the value is parsed as a double.
 * If the value cannot be parsed as a number, string == is used as fallback.
 *
 * Returns nullptr on parse error and sets errorMsg accordingly.
 * The caller owns the returned expression.
 */
SC_SYSTEM_CORE_API
FilterExpression *parseFilterExpression(const std::string &condition,
                                        std::string &errorMsg);


} // namespace DataModel
} // namespace Seiscomp


#endif
