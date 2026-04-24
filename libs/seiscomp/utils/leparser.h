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


#ifndef SEISCOMP_UTILS_LEPARSER_H
#define SEISCOMP_UTILS_LEPARSER_H


#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <set>
#include <sstream>
#include <stack>
#include <variant>

#include <seiscomp/core/baseobject.h>


namespace Seiscomp {
namespace Utils {


/** LeExpression can be used as baseclass for custom expression classes,
 * if a unified error handling mechanism is needed. Note: This is optional
 * as the LeClasses also work with classes not derived from LeExpression */
class SC_SYSTEM_CORE_API LeExpression {
	protected:
		LeExpression() : _error(false) {}
		virtual ~LeExpression() {}

	public:
		std::string what(bool clear = true) {
			if (clear) {
				std::string str = _ss.str();
				clearError();
				return str;
			}
			return _ss.str();
		}

		bool error() const {
			return _error;
		}

		void clearError() {
			_ss.str(std::string());
			_error = false;
		}

	protected:
		void append(const std::string& str) {
			if (_ss.str().size() > 0)
				_ss << "\n";
			_ss << str;
			if (!_error)
				_error = !_error;
		}

	private:
		std::ostringstream _ss;
		bool _error;
};




/** BinaryOperator implements AND and OR for a given Expression */
template <typename Expression>
class BinaryOperator : public Expression {
	public:
		BinaryOperator() : _lhs(nullptr), _rhs(nullptr) {}

		void setOperants(Expression* lhs, Expression* rhs) {
			_lhs = lhs;
			_rhs = rhs;
		}

		virtual ~BinaryOperator() {
			if (_lhs) delete _lhs;
			if (_rhs) delete _rhs;
		}

	private:
		Expression* _lhs;
		Expression* _rhs;
};




/** UnaryOperator implements NOT for a given Expression */
template <typename Expression>
class UnaryOperator : public Expression {
	public:
		UnaryOperator() : _rhs(nullptr) {}

		void setOperant(Expression* rhs) {
			_rhs = rhs;
		}

		virtual ~UnaryOperator() {
			if (_rhs) delete _rhs;
		}

	private:
		Expression* _rhs;
};




/** ExpressionFactoryInterface is a abstract class that needs to be
 * implemented by the factory passed to LeParser */
template <typename Expression>
class ExpressionFactoryInterface {

	public:
		virtual ~ExpressionFactoryInterface () {}

		virtual Expression* createAndExpression() = 0;
		virtual Expression* createOrExpression() = 0;
		virtual Expression* createNotExpression() = 0;
		virtual Expression* createExpression(const std::string& name) = 0;
};




/** Types used by the Le classes */
struct SC_SYSTEM_CORE_API LeParserTypes {

	// ----------------------------------------------------------------------
	// Nested types
	// ----------------------------------------------------------------------
	public:
		enum Operator { AND = 0x0, OR, NOT, POPEN, PCLOSE, OperatorCount };
		typedef std::vector<std::string> Tokens;

		LeParserTypes() {
			OperatorMap.insert(std::make_pair(AND, "&"));
			OperatorMap.insert(std::make_pair(OR, "|"));
			OperatorMap.insert(std::make_pair(NOT, "!"));
			OperatorMap.insert(std::make_pair(POPEN, "("));
			OperatorMap.insert(std::make_pair(PCLOSE, ")"));
		}

		static std::map<Operator, std::string> OperatorMap;
		static LeParserTypes __parserTypes__;
};




/** Tokenizes strings representing logical expressions of the form a & b & !(c | d) ...
 * The result is a vector containing the operands and terminals.
 */
class SC_SYSTEM_CORE_API LeTokenizer {

	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
	public:
		LeTokenizer(const std::string& expr);
		~LeTokenizer() {}


	// ----------------------------------------------------------------------
	// Public interface
	// ----------------------------------------------------------------------
	public:
		const LeParserTypes::Tokens& tokens() const;
		bool error() const;
		const std::string& what() const;
		bool tokenize(const std::string& expr);
		bool tokenize();

	// ----------------------------------------------------------------------
	// Private interface
	// ----------------------------------------------------------------------
	private:
		void init();
		void pushToken(std::string& token);

	// ----------------------------------------------------------------------
	// Private data member
	// ----------------------------------------------------------------------
	private:
		std::string   _expr;
		LeParserTypes::Tokens _tokens;

		bool _error;
		std::string _errorStr;
};




/** Parses the given tokens from LeTokenizer and creates a logical expression */
template <typename Expression>
class LeParser {


	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
	public:
		LeParser(
				const LeParserTypes::Tokens& tokens,
				ExpressionFactoryInterface<Expression>* factory);
		~LeParser();


	// ----------------------------------------------------------------------
	// Public interface
	// ----------------------------------------------------------------------
	public:
		Expression* parse();
		bool error() const;
		const char* what() const;
		Expression* expression() const;


	// ----------------------------------------------------------------------
	// Private interface
	// ----------------------------------------------------------------------
	private:
		void init();
		void applyBinaryOperator();
		void applyUnaryOperator();


	// ----------------------------------------------------------------------
	// Private data members
	// ----------------------------------------------------------------------
	private:
		bool                    _error;
		std::string             _errorStr;

		const LeParserTypes::Tokens& _tokens;
		LeParserTypes::Tokens::const_iterator _it;
		ExpressionFactoryInterface<Expression>* _factory;

		std::stack<Expression*>  _stack;
		std::stack<LeParserTypes::Operator> _modeStack;
};

#include "leparser.ipp"


/**
 * Namespace for version 2 of the logical expression parser.
 */
namespace V2 {


DEFINE_SMARTPOINTER(LeExpression);
/**
 * @brief LeExpression can be used as baseclass for custom expression classes,
 *        if a unified error handling mechanism is needed.
 */
class SC_SYSTEM_CORE_API LeExpression : public Core::BaseObject {
	// ----------------------------------------------------------------------
	// X'truction
	// ----------------------------------------------------------------------
	protected:
		//! Protected c'tor
		LeExpression() {}

	public:
		//! D'tor
		virtual ~LeExpression() {}


	// ----------------------------------------------------------------------
	// Public interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Evaluates an expression.
		 * This method may throw an exception if the evaluation fails due to
		 * an error like an undefined variable or parsing errors or anything
		 * else.
		 * @param context The context of the evaluation
		 * @return Evaluation success.
		 */
		virtual bool eval(const void *context) = 0;
};


/**
 * @brief LeFactory is a abstract class that needs to be implemented to
 *        create concrete expressions which will be combined with logical
 *        operations.
 */
class SC_SYSTEM_CORE_API LeFactory {
	public:
		virtual ~LeFactory () {}

	public:
		virtual LeExpression *createExpression(std::string_view token) const = 0;
};


class SC_SYSTEM_CORE_API LeKeyValueFactory : public LeFactory {
	public:
		enum class ComparisonOperator {
			Equal = 0,
			NotEqual,
			LessOrEqual,
			LessThan,
			GreaterOrEqual,
			GreaterThan,
			Quantity
		};

	public:
		/**
		 * @brief Creates an expression from a comparison condition.
		 * The syntax is: [key] [==|!=|<|<=|>=|>] '[string]'|null|[double].
		 * Null is a special value which will be used for comparison if the context
		 * throws a Seiscomp::Core::ValueException when retrieving a value.
		 * @param condition The string condition.
		 * @return An expression.
		 */
		LeExpression *createExpression(std::string_view condition) const override;

	public:
		/**
		 * @brief Returns a set of special keywords required by this factory.
		 * @return
		 */
		static const std::set<std::string> &Reserved();

	protected:
		using Variant = std::variant<double,std::string_view>;
		using RHS = OPT(Variant);

		/**
		 * @brief Returns whether the key is valid or not.
		 * If the key is not valid then the expression won't be created.
		 * The default application always returns true.
		 * @param key The name of the key.
		 * @return Validation flag.
		 */
		virtual bool isValid(std::string_view key) const;
		virtual LeExpression *create(std::string_view key, ComparisonOperator op, RHS rhs) const;
};


class SC_SYSTEM_CORE_API LeKeyValueContext {
	public:
		/**
		 * @brief Returns a double value for a key.
		 * If the key cannot be found or does not store a double value then
		 * std::runtime_error is thrown. If the value is not set but the key is
		 * valid then Seiscomp::Core::ValueException is thrown.
		 * @param key The key.
		 * @return The associated double value.
		 */
		virtual double getDouble(std::string_view key) const = 0;

		/**
		 * @brief Returns a string value for a key.
		 * If the key cannot be found then std::runtime_error is thrown. If the
		 * value is not set but the key is valid then Seiscomp::Core::ValueException
		 * is thrown.
		 * @param key The key.
		 * @return The associated string value.
		 */
		virtual std::string getString(std::string_view key) const = 0;
};


/**
 * @brief Parses a string containing a logical expression in the form
 *        a & b & !(c | d) ...
 */
class SC_SYSTEM_CORE_API LeParser {
	// ----------------------------------------------------------------------
	// Public types
	// ----------------------------------------------------------------------
	public:
		enum class Operator {
			And = 0,
			Or,
			Not,
			POpen,
			PClose,
			Quote,
			Quantity
		};

		struct Symbols {
			void set(Operator, const std::string &);

			std::string           operators[static_cast<size_t>(Operator::Quantity)];
			std::set<std::string> reserved;
		};

		using Tokens = std::vector<std::string>;


	// ----------------------------------------------------------------------
	// X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		LeParser(LeFactory *factory, const Symbols *symbols = nullptr);
		//! D'tor
		~LeParser();


	// ----------------------------------------------------------------------
	// Public interface
	// ----------------------------------------------------------------------
	public:
		static Symbols DefaultSymbols();

		/**
		 * @brief Parses a string and creates an expression to be evaluated.
		 * The parse method may throw an exception in case of parser errors.
		 * @param expr The expression string to be parsed.
		 * @return An LeExpression instance which must be managed by the caller.
		 */
		LeExpression *parse(std::string_view expr);


	// ----------------------------------------------------------------------
	// Private interface
	// ----------------------------------------------------------------------
	private:
		void tokenize(Tokens &tokens, std::string_view expr);
		LeExpression *parse(Tokens::const_iterator &it,
		                    const Tokens &tokens, bool single);


	// ----------------------------------------------------------------------
	// Private data members
	// ----------------------------------------------------------------------
	private:
		LeFactory     *_factory;
		const Symbols *_symbols;
};


} // namespace V2


} // namespace Utils
} // namespace Seiscomp


#endif
