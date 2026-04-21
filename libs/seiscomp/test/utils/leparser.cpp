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


#define SEISCOMP_TEST_MODULE SeisComP


#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

#include <seiscomp/unittest/unittests.h>

#include <seiscomp/core/strings.h>
#include <seiscomp/utils/leparser.h>


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Utils;


class TrueExpression : public LeExpression {
	public:
		bool eval(const void *context) override {
			return true;
		}
};

class FalseExpression : public LeExpression {
	public:
		bool eval(const void *context) override {
			return false;
		}
};


class SimpleFactory : public LeFactory {
	public:
		LeExpression *createExpression(string_view name) const override {
			string normalized;

			// Strip all whitespaces
			for ( auto c : name ) {
				if ( Core::WHITESPACE.find(c) == string::npos ) {
					normalized += c;
				}
			}

			if ( normalized == "true" ) {
				return new TrueExpression;
			}
			else if ( normalized == "false" ) {
				return new FalseExpression;
			}
			else if ( normalized == "a<5" ) {
				return new TrueExpression;
			}
			else if ( normalized == "a>=5" ) {
				return new FalseExpression;
			}
			else if ( normalized == "a!=5" ) {
				return new TrueExpression;
			}
			return nullptr;
		}
};


struct StaticKeyValueContext : public LeKeyValueContext {
	double getDouble(string_view key) const override {
		auto it = values.find(key);
		if ( it == values.end() ) {
			throw runtime_error(string(key) + " is not a valid key");
		}

		return it->second;
	}

	string getString(string_view) const override {
		throw runtime_error("no string values supported");
	}

	map<string, double, less<>> values;
};


struct DynamicKeyValueContext : public LeKeyValueContext {
	double getDouble(string_view key) const override {
		if ( key == "a" ) {
			return 0;
		}
		else if ( key == "b" ) {
			return 3;
		}

		throw runtime_error(string(key) + " is not a valid key");
	}

	string getString(string_view key) const override {
		if ( key == "type" ) {
			return "not existing";
		}

		throw runtime_error(string(key) + " is not a valid key");
	}

	map<string, double, less<>> values;
};


BOOST_AUTO_TEST_SUITE(seiscomp_utils_leparser)


BOOST_AUTO_TEST_CASE(parser) {
	SimpleFactory factory;
	LeParser parser(&factory);
	LeExpressionPtr expr;

	// And
	expr = parser.parse("true & false");
	BOOST_CHECK(expr);
	BOOST_CHECK(!expr->eval(nullptr));

	expr = parser.parse("true & true");
	BOOST_CHECK(expr);
	BOOST_CHECK(expr->eval(nullptr));

	expr = parser.parse("false & true");
	BOOST_CHECK(expr);
	BOOST_CHECK(!expr->eval(nullptr));

	expr = parser.parse("false & false");
	BOOST_CHECK(expr);
	BOOST_CHECK(!expr->eval(nullptr));

	// Or
	expr = parser.parse("false | false");
	BOOST_CHECK(expr);
	BOOST_CHECK(!expr->eval(nullptr));

	expr = parser.parse("false | true");
	BOOST_CHECK(expr);
	BOOST_CHECK(expr->eval(nullptr));

	expr = parser.parse("true | false");
	BOOST_CHECK(expr);
	BOOST_CHECK(expr->eval(nullptr));

	expr = parser.parse("true | true");
	BOOST_CHECK(expr);
	BOOST_CHECK(expr->eval(nullptr));

	// Complex
	expr = parser.parse("true & (false | true)");
	BOOST_CHECK(expr);
	BOOST_CHECK(expr->eval(nullptr));

	expr = parser.parse("(false | true) & true");
	BOOST_CHECK(expr);
	BOOST_CHECK(expr->eval(nullptr));

	expr = parser.parse("false & (false | true)");
	BOOST_CHECK(expr);
	BOOST_CHECK(!expr->eval(nullptr));

	expr = parser.parse("(false | true) & false");
	BOOST_CHECK(expr);
	BOOST_CHECK(!expr->eval(nullptr));

	expr = parser.parse("true & a < 5");
	BOOST_CHECK(expr);
	BOOST_CHECK(expr->eval(nullptr));

	expr = parser.parse("true & a >= 5");
	BOOST_CHECK(expr);
	BOOST_CHECK(!expr->eval(nullptr));

	expr = parser.parse("true & !false");
	BOOST_CHECK(expr);
	BOOST_CHECK(expr->eval(nullptr));

	expr = parser.parse("!true & !false");
	BOOST_CHECK(expr);
	BOOST_CHECK(!expr->eval(nullptr));

	expr = parser.parse("!!true & !!!false");
	BOOST_CHECK(expr);
	BOOST_CHECK(expr->eval(nullptr));

	expr = parser.parse("true & !(a >= 5)");
	BOOST_CHECK(expr);
	BOOST_CHECK(expr->eval(nullptr));

	expr = parser.parse("true & !(a < 5)");
	BOOST_CHECK(expr);
	BOOST_CHECK(!expr->eval(nullptr));
}


BOOST_AUTO_TEST_CASE(specials) {
	SimpleFactory factory;
	auto symbols = LeParser::DefaultSymbols();
	symbols.specials.insert("!=");
	LeParser parser(&factory, &symbols);
	LeExpressionPtr expr;

	expr = parser.parse("true & a != 5");
	BOOST_CHECK(expr);
	BOOST_CHECK(expr->eval(nullptr));

	expr = parser.parse("true & !(a != 5)");
	BOOST_CHECK(expr);
	BOOST_CHECK(!expr->eval(nullptr));
}


BOOST_AUTO_TEST_CASE(custom_syntax) {
	SimpleFactory factory;
	LeParser::Symbols symbols = { {"&&", "||", "!", "{", "}"} };
	LeParser parser(&factory, &symbols);
	LeExpressionPtr expr;

	expr = parser.parse("true && false");
	BOOST_CHECK(expr);
	BOOST_CHECK(!expr->eval(nullptr));

	expr = parser.parse("false && {false || true}");
	BOOST_CHECK(expr);
	BOOST_CHECK(!expr->eval(nullptr));
}


BOOST_AUTO_TEST_CASE(custom_syntax2) {
	SimpleFactory factory;
	LeParser::Symbols symbols = { {"and", "or", "not", "BEGIN", "END"} };
	LeParser parser(&factory, &symbols);
	LeExpressionPtr expr;

	expr = parser.parse("true and false");
	BOOST_CHECK(expr);
	BOOST_CHECK(!expr->eval(nullptr));

	expr = parser.parse("false and BEGIN false or true END");
	BOOST_CHECK(expr);
	BOOST_CHECK(!expr->eval(nullptr));
}


BOOST_AUTO_TEST_CASE(keyvalue) {
	LeKeyValueFactory kv;
	StaticKeyValueContext ctx;
	LeParser::Symbols symbols = LeParser::DefaultSymbols();
	symbols.specials = LeKeyValueFactory::Specials();
	LeParser parser(&kv, &symbols);
	LeExpressionPtr expr;

	ctx.values["a"] = 0;
	ctx.values["b"] = 3;

	expr = parser.parse("a != 5 & b == 3");
	BOOST_CHECK(expr);
	BOOST_CHECK(expr->eval(&ctx));

	expr = parser.parse("a == 5 & b != 3");
	BOOST_CHECK(expr);
	BOOST_CHECK(!expr->eval(&ctx));

	expr = parser.parse("a == 5");
	BOOST_CHECK(expr);
	BOOST_CHECK(!expr->eval(&ctx));

	expr = parser.parse("a == 0");
	BOOST_CHECK(expr);
	BOOST_CHECK(expr->eval(&ctx));

	expr = parser.parse("a < 5");
	BOOST_CHECK(expr);
	BOOST_CHECK(expr->eval(&ctx));

	expr = parser.parse("a <= 5");
	BOOST_CHECK(expr);
	BOOST_CHECK(expr->eval(&ctx));

	expr = parser.parse("a > 5");
	BOOST_CHECK(expr);
	BOOST_CHECK(!expr->eval(&ctx));

	expr = parser.parse("a >= 5");
	BOOST_CHECK(expr);
	BOOST_CHECK(!expr->eval(&ctx));

	expr = parser.parse("(a < 5) | (b > 1)");
	BOOST_CHECK(expr);
	BOOST_CHECK(expr->eval(&ctx));

	expr = parser.parse("(a >= 5) | (b > 1)");
	BOOST_CHECK(expr);
	BOOST_CHECK(expr->eval(&ctx));

	expr = parser.parse("(a < 5) | (b <= 1)");
	BOOST_CHECK(expr);
	BOOST_CHECK(expr->eval(&ctx));
}


BOOST_AUTO_TEST_CASE(keyvalue2) {
	LeKeyValueFactory kv;
	DynamicKeyValueContext ctx;
	LeParser::Symbols symbols = LeParser::DefaultSymbols();
	symbols.specials = LeKeyValueFactory::Specials();
	LeParser parser(&kv, &symbols);
	LeExpressionPtr expr;

	expr = parser.parse("a != 5 & b == 3");
	BOOST_CHECK(expr);
	BOOST_CHECK(expr->eval(&ctx));

	expr = parser.parse("a == 5 & b != 3");
	BOOST_CHECK(expr);
	BOOST_CHECK(!expr->eval(&ctx));

	expr = parser.parse("type == not existing");
	BOOST_CHECK(expr);
	BOOST_CHECK(expr->eval(&ctx));

	expr = parser.parse("type != earthquake");
	BOOST_CHECK(expr);
	BOOST_CHECK(expr->eval(&ctx));

	expr = parser.parse("type == who knows what");
	BOOST_CHECK(expr);
	BOOST_CHECK(!expr->eval(&ctx));
}


BOOST_AUTO_TEST_SUITE_END()
