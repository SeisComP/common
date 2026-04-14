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


#include "filter.h"

#include <seiscomp/utils/leparser.h>
#include <seiscomp/datamodel/originquality.h>

#include <algorithm>
#include <stdexcept>


namespace Seiscomp {
namespace DataModel {


namespace {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Normalise a string value for case-insensitive, punctuation-tolerant matching:
// lowercase, spaces → hyphens, underscores → hyphens.
// Applied to both actual enum strings and user-supplied condition values so
// that e.g. "not existing", "not_existing" and "not-existing" all match.
std::string normalise(const std::string &s) {
	std::string v(s);
	std::transform(v.begin(), v.end(), v.begin(), ::tolower);
	std::replace(v.begin(), v.end(), ' ', '-');
	std::replace(v.begin(), v.end(), '_', '-');
	return v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class FilterAndExpression : public Utils::BinaryOperator<FilterExpression> {
	public:
		bool eval(const FilterContext &ctx) const override {
			return _lhs->eval(ctx) && _rhs->eval(ctx);
		}
};


class FilterOrExpression : public Utils::BinaryOperator<FilterExpression> {
	public:
		bool eval(const FilterContext &ctx) const override {
			return _lhs->eval(ctx) || _rhs->eval(ctx);
		}
};


class FilterNotExpression : public Utils::UnaryOperator<FilterExpression> {
	public:
		bool eval(const FilterContext &ctx) const override {
			return !_rhs->eval(ctx);
		}
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Leaf expression: evaluates key <op> value against a FilterContext.
// Supports ==, !=, <, >, <=, >= operators.
// Numeric comparison is attempted first; falls back to string for == and !=.
class FilterLeafExpression : public FilterExpression {
	public:
		enum Op { EQ, LT, GT, LE, GE };

		FilterLeafExpression(const std::string &key, Op op,
		                     const std::string &value)
		: _key(key), _op(op) {
			// Pre-parse numeric value if possible
			try {
				_numericValue = std::stod(value);
				_hasNumeric = true;
				_value = value;
			}
			catch ( ... ) {
				_hasNumeric = false;
				_numericValue = 0.0;
				// Normalise string values so that e.g. "not existing",
				// "not_existing" and "not-existing" all match the same enum string.
				_value = normalise(value);
			}
		}

		bool eval(const FilterContext &ctx) const override {
			// Try numeric comparison first
			if ( _hasNumeric ) {
				double actual;
				if ( ctx.getDouble(_key, actual) )
					return compareDouble(actual);
			}

			// Fall back to string comparison (== only; != is expressed as !(key==value))
			if ( _op == EQ ) {
				std::string actual;
				if ( ctx.getString(_key, actual) )
					return actual == _value;
			}

			return false;
		}

	private:
		bool compareDouble(double actual) const {
			switch ( _op ) {
				case EQ: return actual == _numericValue;
				case LT: return actual <  _numericValue;
				case GT: return actual >  _numericValue;
				case LE: return actual <= _numericValue;
				case GE: return actual >= _numericValue;
			}
			return false;
		}

		std::string _key;
		Op          _op;
		std::string _value;
		double      _numericValue;
		bool        _hasNumeric;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Parses a leaf token of the form "key==value", "key<value", "key>=value" etc.
// Note: != is NOT supported here because the LeTokenizer splits '!' as the
// logical NOT operator before this function is ever called. Use !(key==value)
// to express not-equal in condition strings.
// Returns nullptr and sets errorMsg on failure.
FilterExpression *parseLeaf(const std::string &token, std::string &errorMsg) {
	// Operators ordered longest-first to avoid partial matches
	static const struct { const char *sym; FilterLeafExpression::Op op; } ops[] = {
		{ "==", FilterLeafExpression::EQ },
		{ "<=", FilterLeafExpression::LE },
		{ ">=", FilterLeafExpression::GE },
		{ "<",  FilterLeafExpression::LT },
		{ ">",  FilterLeafExpression::GT },
	};

	for ( const auto &entry : ops ) {
		auto pos = token.find(entry.sym);
		if ( pos == std::string::npos ) continue;

		std::string key   = token.substr(0, pos);
		std::string value = token.substr(pos + std::strlen(entry.sym));

		// Trim whitespace
		auto trim = [](std::string &s) {
			auto a = s.find_first_not_of(" \t");
			auto b = s.find_last_not_of(" \t");
			s = (a == std::string::npos) ? "" : s.substr(a, b - a + 1);
		};
		trim(key);
		trim(value);

		if ( key.empty() ) {
			errorMsg = "Empty key in token: " + token;
			return nullptr;
		}

		return new FilterLeafExpression(key, entry.op, value);
	}

	errorMsg = "No operator found in token: " + token;
	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class FilterExpressionFactory
    : public Utils::ExpressionFactoryInterface<FilterExpression> {
	public:
		FilterExpressionFactory(std::string &errorMsg) : _errorMsg(errorMsg) {}

		FilterExpression *createAndExpression() override {
			return new FilterAndExpression;
		}
		FilterExpression *createOrExpression() override {
			return new FilterOrExpression;
		}
		FilterExpression *createNotExpression() override {
			return new FilterNotExpression;
		}
		FilterExpression *createExpression(const std::string &token) override {
			return parseLeaf(token, _errorMsg);
		}

	private:
		std::string &_errorMsg;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // anonymous namespace




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FilterExpression *parseFilterExpression(const std::string &condition,
                                        std::string &errorMsg) {
	errorMsg.clear();

	Utils::LeTokenizer tokenizer(condition);
	if ( !tokenizer.tokenize() || tokenizer.error() ) {
		errorMsg = tokenizer.what();
		return nullptr;
	}

	FilterExpressionFactory factory(errorMsg);
	Utils::LeParser<FilterExpression> parser(tokenizer.tokens(), &factory);

	FilterExpression *expr = parser.parse();
	if ( parser.error() ) {
		errorMsg = parser.what();
		delete expr;
		return nullptr;
	}

	if ( !errorMsg.empty() ) {
		delete expr;
		return nullptr;
	}

	return expr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<






// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventFilterContext::EventFilterContext(const Event *event,
                                       const Origin *origin,
                                       const Magnitude *magnitude)
: _event(event), _origin(origin), _magnitude(magnitude) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventFilterContext::getString(const std::string &key,
                                   std::string &value) const {
	if ( !_event ) return false;

	if ( key == "type" ) {
		try { value = normalise(_event->type().toString()); return true; }
		catch ( ... ) { return false; }
	}
	if ( key == "typecertainty" ) {
		try { value = normalise(_event->typeCertainty().toString()); return true; }
		catch ( ... ) { return false; }
	}

	if ( !_origin ) return false;

	if ( key == "evaluationstatus" ) {
		try { value = normalise(_origin->evaluationStatus().toString()); return true; }
		catch ( ... ) { return false; }
	}
	if ( key == "evaluationmode" ) {
		try { value = normalise(_origin->evaluationMode().toString()); return true; }
		catch ( ... ) { return false; }
	}
	if ( key == "agencyid" ) {
		try { value = _origin->creationInfo().agencyID(); return true; }
		catch ( ... ) { return false; }
	}
	if ( key == "author" ) {
		try { value = _origin->creationInfo().author(); return true; }
		catch ( ... ) { return false; }
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventFilterContext::getDouble(const std::string &key,
                                   double &value) const {
	if ( _origin ) {
		if ( key == "lat" || key == "latitude" ) {
			try { value = _origin->latitude().value(); return true; }
			catch ( ... ) { return false; }
		}
		if ( key == "lon" || key == "longitude" ) {
			try { value = _origin->longitude().value(); return true; }
			catch ( ... ) { return false; }
		}
		if ( key == "depth" ) {
			try { value = _origin->depth().value(); return true; }
			catch ( ... ) { return false; }
		}
		if ( key == "rms" ) {
			try { value = _origin->quality().standardError(); return true; }
			catch ( ... ) { return false; }
		}
		if ( key == "azimuthalgap" ) {
			try { value = _origin->quality().azimuthalGap(); return true; }
			catch ( ... ) { return false; }
		}
	}

	if ( _magnitude ) {
		if ( key == "magnitude" ) {
			try { value = _magnitude->magnitude().value(); return true; }
			catch ( ... ) { return false; }
		}
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // namespace DataModel
} // namespace Seiscomp
