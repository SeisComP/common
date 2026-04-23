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

#include "leparser.h"

#include <seiscomp/core/strings.h>


using namespace std;


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp {
namespace Utils {
namespace {


class LeBinaryOperator : public LeExpression {
	public:
		LeBinaryOperator(LeExpression *lhs, LeExpression *rhs)
		: _lhs(lhs), _rhs(rhs) {}

	protected:
		LeExpressionPtr _lhs;
		LeExpressionPtr _rhs;
};


class LeAndOperator : public LeBinaryOperator {
	public:
		using LeBinaryOperator::LeBinaryOperator;

	public:
		bool eval(const void *context) override {
			// Shortcut evaluation
			if ( !_lhs->eval(context) ) {
				return false;
			}
			return _rhs->eval(context);
		}
};


class LeOrOperator : public LeBinaryOperator {
	public:
		using LeBinaryOperator::LeBinaryOperator;

	public:
		bool eval(const void *context) override {
			// Shortcut evaluation
			if ( _lhs->eval(context) ) {
				return true;
			}
			return _rhs->eval(context);
		}
};


class LeNotOperator : public LeExpression {
	public:
		LeNotOperator(LeExpression *rhs) : _rhs(rhs) {}

	public:
		bool eval(const void *context) override {
			return !_rhs->eval(context);
		}

	private:
		LeExpressionPtr _rhs;
};


void pushToken(LeParser::Tokens &tokens, string &token) {
	Core::trim(token);
	if ( !token.empty() ) {
		tokens.push_back(token);
		token.clear();
	}
}


template <typename T>
constexpr size_t idx(T enumeration) {
	return static_cast<size_t>(enumeration);
}


LeParser::Symbols DefaultParserSymbols = {
	{ "&", "|", "!", "(", ")" },
	{}
};


std::string comparisonOperators[idx(LeKeyValueFactory::ComparisonOperator::Quantity)] = {
	"==", "!=", "<=", "<", ">=", ">"
};


class LeKeyValueExpression : public LeExpression {
	public:
		using ComparisonOperator = LeKeyValueFactory::ComparisonOperator;
		LeKeyValueExpression(string_view key, ComparisonOperator op, string_view value)
		: _key(key), _op(op), _value(value) {
			double v;
			if ( Core::fromString(v, _value) ) {
				_numericValue = v;
			}
		}

	public:
		bool eval(const void *context) override {
			auto ctx = reinterpret_cast<const LeKeyValueContext*>(context);
			if ( _numericValue ) {
				switch ( _op ) {
					case ComparisonOperator::Equal:
						return ctx->getDouble(_key) == *_numericValue;
					case ComparisonOperator::NotEqual:
						return ctx->getDouble(_key) != *_numericValue;
					case ComparisonOperator::LessThan:
						return ctx->getDouble(_key)< *_numericValue;
					case ComparisonOperator::LessOrEqual:
						return ctx->getDouble(_key) <= *_numericValue;
					case ComparisonOperator::GreaterThan:
						return ctx->getDouble(_key) > *_numericValue;
					case ComparisonOperator::GreaterOrEqual:
						return ctx->getDouble(_key) >= *_numericValue;
					default:
						throw runtime_error("invalid comparison operator");
				}
			}
			else {
				switch ( _op ) {
					case ComparisonOperator::Equal:
						return ctx->getString(_key) == _value;
					case ComparisonOperator::NotEqual:
						return ctx->getString(_key) != _value;
					case ComparisonOperator::LessThan:
						return ctx->getString(_key) < _value;
					case ComparisonOperator::LessOrEqual:
						return ctx->getString(_key) <= _value;
					case ComparisonOperator::GreaterThan:
						return ctx->getString(_key) > _value;
					case ComparisonOperator::GreaterOrEqual:
						return ctx->getString(_key) >= _value;
					default:
						throw runtime_error("invalid comparison operator");
				}
			}
		}

	private:
		string             _key;
		ComparisonOperator _op;
		string             _value;
		OPT(double)        _numericValue;
};


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::set<std::string> &LeKeyValueFactory::Specials() {
	static std::set<std::string> specials = { "!=" };
	return specials;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LeExpression *LeKeyValueFactory::createExpression(string_view condition) const {
	for ( size_t i = 0; i < idx(ComparisonOperator::Quantity); ++i ) {
		auto p = condition.find(comparisonOperators[i]);
		if ( p != string::npos ) {
			auto key = condition.substr(0, p);
			if ( !isValid(key) ) {
				throw runtime_error("invalid key name '" + string(key) + "'");
			}

			auto value = condition.substr(p + comparisonOperators[i].size());
			return create(Core::trim(key), ComparisonOperator(i), Core::trim(value));
		}
	}

	throw runtime_error("comparison operator required");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LeExpression *LeKeyValueFactory::create(std::string_view key,
                                        ComparisonOperator op,
                                        std::string_view value) const {
	return new LeKeyValueExpression(key, op, value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool LeKeyValueFactory::isValid(std::string_view key) const {
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LeParser::LeParser(LeFactory *factory, const Symbols *symbols)
: _factory(factory) {
	_symbols = symbols ? symbols : &DefaultParserSymbols;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LeParser::~LeParser() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LeParser::tokenize(Tokens &tokens, string_view expr) {
	int parenthesesCounter = 0;

	string token;

	for ( size_t current = 0; current < expr.size(); ) {
		if ( !_symbols->specials.empty() ) {
			bool skipSpecial = false;

			for ( auto &special : _symbols->specials ) {
				if ( !expr.compare(current, special.size(), special) ) {
					token += special;
					current += special.size();
					skipSpecial = true;
					break;
				}
			}

			if ( skipSpecial ) {
				continue;
			}
		}

		if ( !expr.compare(current, _symbols->operators[idx(Operator::And)].size(), _symbols->operators[idx(Operator::And)]) ) {
			pushToken(tokens, token);
			current += _symbols->operators[idx(Operator::And)].size();
			tokens.push_back(_symbols->operators[idx(Operator::And)]);
		}
		else if ( !expr.compare(current, _symbols->operators[idx(Operator::Or)].size(), _symbols->operators[idx(Operator::Or)]) ) {
			pushToken(tokens, token);
			current += _symbols->operators[idx(Operator::Or)].size();
			tokens.push_back(_symbols->operators[idx(Operator::Or)]);
		}
		else if ( !expr.compare(current, _symbols->operators[idx(Operator::Not)].size(), _symbols->operators[idx(Operator::Not)]) ) {
			pushToken(tokens, token);
			current += _symbols->operators[idx(Operator::Not)].size();
			tokens.push_back(_symbols->operators[idx(Operator::Not)]);
		}
		else if ( !expr.compare(current, _symbols->operators[idx(Operator::POpen)].size(), _symbols->operators[idx(Operator::POpen)]) ) {
			++parenthesesCounter;

			pushToken(tokens, token);
			current += _symbols->operators[idx(Operator::POpen)].size();
			tokens.push_back(_symbols->operators[idx(Operator::POpen)]);
		}
		else if ( !expr.compare(current, _symbols->operators[idx(Operator::PClose)].size(), _symbols->operators[idx(Operator::PClose)]) ) {
			--parenthesesCounter;

			pushToken(tokens, token);
			current += _symbols->operators[idx(Operator::PClose)].size();
			tokens.push_back(_symbols->operators[idx(Operator::PClose)]);
		}
		/*
		else if ( Core::WHITESPACE.find(expr[current]) != string::npos ) {
			// Just ignore whitespaces
			++current;
		}
		*/
		else {
			token.push_back(expr[current]);
			++current;
		}
	}

	pushToken(tokens, token);

	if ( parenthesesCounter < 0 ) {
		throw runtime_error("too many closing parentheses");
	}
	else if ( parenthesesCounter > 0 ) {
		throw runtime_error("too many opening parentheses");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LeParser::Symbols LeParser::DefaultSymbols() {
	return DefaultParserSymbols;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LeExpression *LeParser::parse(string_view expr) {
	Tokens tokens;
	tokenize(tokens, expr);

	Tokens::const_iterator it = tokens.begin();
	return parse(it, tokens, false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LeExpression *LeParser::parse(Tokens::const_iterator &it,
                              const Tokens &tokens,
                              bool single) {
	LeExpression *lastExpression = nullptr;
	OPT(Operator) op;

	while ( it != tokens.end() ) {
		if ( *it == _symbols->operators[idx(Operator::And)] ) {
			if ( op ) {
				if ( lastExpression ) {
					delete lastExpression;
				}
				throw runtime_error("operator must not follow another operator");
			}
			op = Operator::And;
		}
		else if ( *it == _symbols->operators[idx(Operator::Or)] ) {
			if ( op ) {
				if ( lastExpression ) {
					delete lastExpression;
				}
				throw runtime_error("operator must not follow another operator");
			}
			op = Operator::Or;
		}
		else {
			LeExpression *currentExpression = nullptr;

			if ( *it == _symbols->operators[idx(Operator::Not)] ) {
				++it;
				currentExpression = parse(it, tokens, true);
				if ( !currentExpression ) {
					if ( lastExpression ) {
						delete lastExpression;
					}
					throw runtime_error("missing negate right-hand operator");
				}

				currentExpression = new LeNotOperator(currentExpression);
			}
			else if ( *it == _symbols->operators[idx(Operator::POpen)] ) {
				++it;
				try {
					currentExpression = parse(it, tokens, false);
					if ( !currentExpression ) {
						if ( lastExpression ) {
							delete lastExpression;
						}
						throw runtime_error("empty parenthesis");
					}
				}
				catch ( exception &e ) {
					if ( lastExpression ) {
						delete lastExpression;
					}
					throw e;
				}
			}
			else if ( *it == _symbols->operators[idx(Operator::PClose)] ) {
				return lastExpression;
			}
			else {
				try {
					currentExpression = _factory->createExpression(*it);
					if ( !currentExpression ) {
						if ( lastExpression ) {
							delete lastExpression;
						}
						throw runtime_error("could not create expression: " + *it);
					}
				}
				catch ( ... ) {
					if ( lastExpression ) {
						delete lastExpression;
					}
					throw;
				}
			}

			if ( currentExpression ) {
				if ( !op ) {
					if ( lastExpression ) {
						delete currentExpression;
						delete lastExpression;
						throw runtime_error("two expressions without operator are not allowed");
					}

					lastExpression = currentExpression;
				}
				else {
					// Instantiate operand
					if ( *op == Operator::And ) {
						if ( !lastExpression ) {
							delete currentExpression;
							throw runtime_error(
								Core::stringify(
									"%s operator requires left-hand operand at token %d",
									_symbols->operators[idx(Operator::And)], int(it - tokens.begin())
								)
							);
						}

						lastExpression = new LeAndOperator(lastExpression, currentExpression);
					}
					else if ( *op == Operator::Or ) {
						if ( !lastExpression ) {
							delete currentExpression;
							throw runtime_error(
								Core::stringify(
									"%s operator requires left-hand operand at token %d",
									_symbols->operators[idx(Operator::Or)], int(it - tokens.begin())
								)
							);
						}

						lastExpression = new LeOrOperator(lastExpression, currentExpression);
					}
					op = Core::None;
				}
			}
		}

		if ( single ) {
			break;
		}

		++it;
	}

	if ( op ) {
		// Still an operation active but without an operand?
		if ( lastExpression ) {
			delete lastExpression;
		}
		throw runtime_error("operator requires a right-hand operand");
	}

	return lastExpression;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace Utils
} // namespace Seiscomp
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
