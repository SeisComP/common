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


#define SEISCOMP_COMPONENT AmplitudeProcessor

#include <seiscomp/datamodel/pick.h>
#include <seiscomp/processing/amplitudeprocessor.h>
#include <seiscomp/processing/regions.h>
#include <seiscomp/geo/feature.h>
#include <seiscomp/math/mean.h>
#include <seiscomp/math/filter/iirdifferentiate.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/core/interfacefactory.ipp>
#include <seiscomp/system/environment.h>
#include <seiscomp/seismology/ttt.h>

#include <cmath>
#include <functional>
#include <fstream>
#include <limits>
#include <vector>
#include <mutex>


#include <boost/version.hpp>
#if BOOST_VERSION >= 103800
#include <boost/spirit/include/classic.hpp>
#include <boost/spirit/include/phoenix1_binders.hpp>
#include <boost/spirit/include/classic_error_handling.hpp>
namespace bs = boost::spirit::classic;
#else
#include <boost/spirit.hpp>
#include <boost/spirit/phoenix/binders.hpp>
#include <boost/spirit/error_handling.hpp>
namespace bs = boost::spirit;
#endif


using namespace std;


IMPLEMENT_INTERFACE_FACTORY(Seiscomp::Processing::AmplitudeProcessor, SC_SYSTEM_CLIENT_API);


namespace Seiscomp {
namespace Processing {


IMPLEMENT_SC_ABSTRACT_CLASS_DERIVED(AmplitudeProcessor, TimeWindowProcessor, "AmplitudeProcessor");


namespace {

typedef vector<AmplitudeProcessor::Locale> Regionalization;

DEFINE_SMARTPOINTER(TypeSpecificRegionalization);
class TypeSpecificRegionalization : public Core::BaseObject {
	public:
		const Regions   *regions;
		Regionalization  regionalization;
};

typedef map<string, TypeSpecificRegionalizationPtr> RegionalizationRegistry;
RegionalizationRegistry regionalizationRegistry;
mutex regionalizationRegistryMutex;


class StatusException : public std::exception {
	public:
		StatusException(WaveformProcessor::Status status, double value)
		: _status(status), _value(value) {}

	public:
		WaveformProcessor::Status status() const { return _status; }
		double value() const { return _value; }

	private:
		WaveformProcessor::Status _status;
		double                    _value;
};


namespace Expression {


class Context {
	public:
		Context(
			const AmplitudeProcessor *proc,
			bool leftSideOfTimeWindow
		)
		: _proc(proc), _leftSideOfTimeWindow(leftSideOfTimeWindow) {}

		~Context() {
			//
		}

		const AmplitudeProcessor *proc() const { return _proc; }

		double getTravelTime(const string &phase, int statusValueOffset) {
			auto env = _proc->environment();
			if ( !env.hypocenter ) {
				throw StatusException(WaveformProcessor::MissingHypocenter, statusValueOffset);
			}
			if ( !env.receiver ) {
				throw StatusException(WaveformProcessor::MissingReceiver, statusValueOffset);
			}

			if ( !_ttt ) {
				if ( _proc->config().ttInterface.empty() ) {
					_ttt = TravelTimeTableInterfaceFactory::Create("libtau");
				}
				else {
					_ttt = TravelTimeTableInterfaceFactory::Create(_proc->config().ttInterface);
				}

				if ( !_ttt ) {
					throw StatusException(WaveformProcessor::TravelTimeEstimateFailed, statusValueOffset);
				}

				if ( !_ttt->setModel(_proc->config().ttModel.empty() ? "iasp91" : _proc->config().ttModel) ) {
					throw StatusException(WaveformProcessor::TravelTimeEstimateFailed, statusValueOffset + 1);
				}
			}

			auto hypoLat = env.hypocenter->latitude().value();
			auto hypoLon = env.hypocenter->longitude().value();
			double hypoDepth;

			try {
				// All attributes are optional and throw an exception if not set
				hypoDepth = env.hypocenter->depth().value();
			}
			catch ( ... ) {
				throw StatusException(WaveformProcessor::IncompleteMetadata, statusValueOffset);
			}

			double recvLat, recvLon, recvElev;

			try {
				// Both attributes are optional and throw an exception if not set
				recvLat = env.receiver->latitude();
				recvLon = env.receiver->longitude();

				// Elevation is fully optional and set to zero if unset
				try {
					recvElev = env.receiver->elevation();
				}
				catch ( ... ) {
					recvElev = 0;
				}
			}
			catch ( ... ) {
				throw StatusException(WaveformProcessor::IncompleteMetadata, statusValueOffset + 1);
			}

			try {
				auto tt = Core::TimeSpan(
					_ttt->computeTime(
						phase.c_str(), hypoLat, hypoLon, hypoDepth,
						recvLat, recvLon, recvElev
					)
				);

				auto absoluteTime = env.hypocenter->time().value() + tt;
				return static_cast<double>(absoluteTime - _proc->trigger());
			}
			catch ( ... ) {
				throw StatusException(WaveformProcessor::TravelTimeEstimateFailed, statusValueOffset + 2);
			}
		}

		double getArrival(const string &phase, bool acceptAll, int statusValueOffset) {
			auto env = _proc->environment();
			if ( !env.hypocenter ) {
				throw StatusException(WaveformProcessor::MissingHypocenter, statusValueOffset);
			}

			for ( size_t i = 0; i < env.hypocenter->arrivalCount(); ++i ) {
				auto arr = env.hypocenter->arrival(i);
				if ( arr->phase().code() != phase ) {
					continue;
				}

				auto pick = DataModel::Pick::Find(arr->pickID());
				if ( !pick ) {
					continue;
				}

				if ( pick->waveformID().networkCode() != env.networkCode
				  || pick->waveformID().stationCode() != env.stationCode
				  || pick->waveformID().locationCode() != env.locationCode ) {
					continue;
				}

				bool acceptAllArrivals = acceptAll;
				if ( !acceptAllArrivals ) {
					try {
						// If the origins evaluation mode is manual then all
						// arrivals are accepted even automatic one since they
						// have been checked in the context of the origin
						acceptAllArrivals = env.hypocenter->evaluationMode() == DataModel::MANUAL;
					}
					catch ( ... ) {}
				}

				if ( !acceptAllArrivals ) {
					try {
						if ( pick->evaluationMode() != DataModel::MANUAL ) {
							// We do not accept automatic picks
							SEISCOMP_DEBUG("%s.%s.%s: arrival '%s' no accepted, origin evaluation  mode != manual",
							               env.networkCode.c_str(),
							               env.stationCode.c_str(),
							               env.locationCode.c_str(),
							               arr->phase().code().c_str());
							continue;
						}
					}
					catch ( ... ) {
						// An unset evaluation mode is not 'manual'
						continue;
					}
				}

				auto onset = pick->time().value();
				double scale = _leftSideOfTimeWindow ? -1.0 : 1.0;
				try {
					onset += scale * pick->time().lowerUncertainty();
				}
				catch ( ... ) {
					try {
						onset += scale * pick->time().uncertainty();
					}
					catch ( ... ) {}
				}

				return static_cast<double>(onset - _proc->trigger());;
			}

			throw StatusException(WaveformProcessor::ArrivalNotFound, statusValueOffset);
		}


	private:
		const AmplitudeProcessor    *_proc;
		TravelTimeTableInterfacePtr  _ttt;
		bool                         _leftSideOfTimeWindow;
};



DEFINE_SMARTPOINTER(Interface);
class Interface : public Core::BaseObject {
	public:
		static InterfacePtr parse(const std::string &, std::string *error_str);

		virtual double evaluate(Context &ctx) = 0;
		virtual std::string toString() const = 0;
};

class Fixed : public Interface {
	public:
		explicit Fixed(double value) : _value(value) {}

		double evaluate(Context &ctx) override { return _value; }
		std::string toString() const override { return Core::toString(_value); }

	private:
		double _value;
};


template <template<class> class F>
class OP1Expression : public Interface {
	public:
		OP1Expression(InterfacePtr op1) : _op(F<double>()), _op1(op1) {}

		double evaluate(Context &ctx) override {
			return _op(_op1->evaluate(ctx));
		}

	private:
		F<double>    _op;

	protected:
		InterfacePtr _op1;
};


class OP2BaseExpression : public Interface {
	public:
		OP2BaseExpression(InterfacePtr op1, InterfacePtr op2)
		: _op1(op1), _op2(op2) {}

	protected:
		InterfacePtr _op1;
		InterfacePtr _op2;
};


template <template<class> class F>
class OP2Expression : public OP2BaseExpression {
	public:
		OP2Expression(InterfacePtr op1, InterfacePtr op2)
		: OP2BaseExpression(op1, op2), _op(F<double>()) {}

		double evaluate(Context &ctx) override {
			return _op(_op1->evaluate(ctx), _op2->evaluate(ctx));
		}

	private:
		F<double> _op;
};


template<class T>
struct dabs {
	T operator()( const T& arg ) const { return std::abs(arg); }
};

template<class T>
struct dmod {
	T operator()( const T& arg1, const T& arg2 ) const { return std::fmod(arg1, arg2); }
};

template<class T>
struct dpow {
	T operator()( const T& arg1, const T& arg2 ) const { return std::pow(arg1, arg2); }
};


// Operators
class NegExpression : public OP1Expression<std::negate> {
	using OP1Expression<std::negate>::OP1Expression;

	std::string toString() const override {
		return "-" + _op1->toString();
	}
};

class AbsExpression : public OP1Expression<dabs> {
	using OP1Expression<dabs>::OP1Expression;

	std::string toString() const override {
		return "|" + _op1->toString() + "|";
	}
};

class MinExpression : public OP2BaseExpression {
	using OP2BaseExpression::OP2BaseExpression;

	double evaluate(Context &ctx) override {
		double e1, e2;

		try {
			e1 = _op1->evaluate(ctx);
		}
		catch ( ... ) {
			// Operand1 is not set, forward operand2 in any case,
			// even if it throws an exception. This case is only
			// valid if both operands are unset.
			return _op2->evaluate(ctx);
		}

		// Operand1 is set already
		try {
			e2 = _op2->evaluate(ctx);
		}
		catch ( ... ) {
			// Operand2 is not set, forward operand1
			return e1;
		}

		// Both operands are set, return minimum
		return e1 <= e2 ? e1 : e2;
	}

	std::string toString() const override {
		return "min(" + _op1->toString() + ", " + _op2->toString() + ")";
	}
};

class MaxExpression : public OP2BaseExpression {
	using OP2BaseExpression::OP2BaseExpression;

	double evaluate(Context &ctx) override {
		double e1, e2;

		try {
			e1 = _op1->evaluate(ctx);
		}
		catch ( ... ) {
			// Operand1 is not set, forward operand2 in any case,
			// even if it throws an exception. This case is only
			// valid if both operands are unset.
			return _op2->evaluate(ctx);
		}

		// Operand1 is set already
		try {
			e2 = _op2->evaluate(ctx);
		}
		catch ( ... ) {
			// Operand2 is not set, forward operand1
			return e1;
		}

		// Both operands are set, return maximum
		return e1 >= e2 ? e1 : e2;
	}

	std::string toString() const override {
		return "max(" + _op1->toString() + ", " + _op2->toString() + ")";
	}
};

class AltExpression : public OP2BaseExpression {
	using OP2BaseExpression::OP2BaseExpression;

	double evaluate(Context &ctx) override {
		try {
			return _op1->evaluate(ctx);
		}
		catch ( ... ) {
			return _op2->evaluate(ctx);
		}
	}

	std::string toString() const override {
		return _op1->toString() + " || " + _op2->toString();
	}
};

class AddExpression : public OP2Expression<std::plus> {
	using OP2Expression<std::plus>::OP2Expression;

	std::string toString() const override {
		return "(" + _op1->toString() + " + " + _op2->toString() + ")";
	}
};

class SubExpression : public OP2Expression<std::minus> {
	using OP2Expression<std::minus>::OP2Expression;

	std::string toString() const override {
		return "(" + _op1->toString() + " - " + _op2->toString() + ")";
	}
};

class MulExpression : public OP2Expression<std::multiplies> {
	using OP2Expression<std::multiplies>::OP2Expression;

	std::string toString() const override {
		return "(" + _op1->toString() + " * " + _op2->toString() + ")";
	}
};

class DivExpression : public OP2Expression<std::divides> {
	using OP2Expression<std::divides>::OP2Expression;

	std::string toString() const override {
		return "(" + _op1->toString() + " / " + _op2->toString() + ")";
	}
};

class ModExpression : public OP2Expression<dmod> {
	using OP2Expression<dmod>::OP2Expression;

	std::string toString() const override {
		return "(" + _op1->toString() + " % " + _op2->toString() + ")";
	}
};

class PowExpression : public OP2Expression<dpow> {
	using OP2Expression<dpow>::OP2Expression;

	std::string toString() const override {
		return "(" + _op1->toString() + " ^ " + _op2->toString() + ")";
	}
};


// Variables
class ParamEpiDistDeg : public Interface {
	public:
		static double Evaluate(Context &ctx) {
			auto env = ctx.proc()->environment();

			if ( !env.hypocenter ) {
				throw StatusException(WaveformProcessor::MissingHypocenter, 10);
			}

			auto hypoLon = env.hypocenter->longitude().value();
			auto hypoLat = env.hypocenter->latitude().value();

			double recvLat, recvLon;

			try {
				// Both attributes are optional and throw an exception if not set
				recvLat = env.receiver->latitude();
				recvLon = env.receiver->longitude();
			}
			catch ( ... ) {
				throw StatusException(WaveformProcessor::MissingReceiver, 10);
			}

			double dist, az, baz;
			Math::Geo::delazi_wgs84(hypoLat, hypoLon, recvLat, recvLon,
			                        &dist, &az, &baz);

			return dist;
		}

		double evaluate(Context &ctx) override {
			return Evaluate(ctx);
		}

		std::string toString() const override {
			return "D";
		}
};

class ParamEpiDistKM : public ParamEpiDistDeg {
	public:
		static double Evaluate(Context &ctx) {
			return Math::Geo::deg2km(ParamEpiDistDeg::Evaluate(ctx));
		}

		double evaluate(Context &ctx) override {
			return Evaluate(ctx);
		}

		std::string toString() const override {
			return "R";
		}
};

class ParamDepth : public Interface {
	public:
		static double Evaluate(Context &ctx) {
			auto env = ctx.proc()->environment();

			if ( !env.hypocenter ) {
				throw StatusException(WaveformProcessor::MissingHypocenter, 20);
			}

			try {
				return env.hypocenter->depth().value();
			}
			catch ( ... ) {
				throw StatusException(WaveformProcessor::MissingDepth, 20);
			}
		}

		double evaluate(Context &ctx) override {
			return Evaluate(ctx);
		}

		std::string toString() const override {
			return "Z";
		}
};


class ParamHypoDistKM : public Interface {
	public:
		static double Evaluate(Context &ctx) {
			auto R = ParamEpiDistKM::Evaluate(ctx);
			auto z = ParamDepth::Evaluate(ctx);
			return sqrt(R * R + z * z);
		}

		double evaluate(Context &ctx) override {
			return Evaluate(ctx);
		}

		std::string toString() const override {
			return "h";
		}
};


class ParamHypoDistDeg : public ParamHypoDistKM {
	public:
		static double Evaluate(Context &ctx) {
			return Math::Geo::km2deg(ParamHypoDistKM::Evaluate(ctx));
		}

		double evaluate(Context &ctx) override {
			return Evaluate(ctx);
		}

		std::string toString() const override {
			return "H";
		}
};


class ParamTT : public Interface {
	public:
		ParamTT(const std::string &phase) : _phase(phase) {}

	public:
		double evaluate(Context &ctx) override {
			return ctx.getTravelTime(_phase, 30);
		}

		std::string toString() const override {
			return "tt(" + _phase + ")";
		}

	private:
		std::string _phase;
};


class ParamArrival : public Interface {
	public:
		ParamArrival(const std::string &phase, bool acceptAll)
		: _phase(phase), _acceptAll(acceptAll) {}

	public:
		double evaluate(Context &ctx) override {
			return ctx.getArrival(_phase, _acceptAll, 40);
		}

		std::string toString() const override {
			return "arr(" + _phase + ", " + (_acceptAll ? "true" : "false") + ")";
		}

	private:
		std::string _phase;
		bool        _acceptAll;
};


}


// -----------------------------------------------------------------------------
// Expression parser
// -----------------------------------------------------------------------------
struct LiteralClosure : bs::closure<LiteralClosure, double> {
	member1 value;
};

template <typename T>
struct ValueClosure : bs::closure<ValueClosure<T>, T> {
	typename bs::closure<ValueClosure<T>, T>::member1 value;
};

template <typename T>
struct ParamClosure : bs::closure<ParamClosure<T>, T, string> {
	typename bs::closure<ParamClosure<T>, T, string>::member1 value;
	typename bs::closure<ParamClosure<T>, T, string>::member2 name;
};

template <typename T>
struct Param2Closure : bs::closure<Param2Closure<T>, T, string, string> {
	typename bs::closure<Param2Closure<T>, T, string, string>::member1 value;
	typename bs::closure<Param2Closure<T>, T, string, string>::member2 name;
	typename bs::closure<Param2Closure<T>, T, string, string>::member3 flag;
};

struct StringClosure : bs::closure<StringClosure, string> {
	member1 name;
};

template <typename T> struct Parser;

template <typename ParserT>
struct Generator {
	typedef typename ParserT::component_type component_type;
	typedef typename ParserT::value_type value_type;

	Generator(ParserT &p) : parser(p) {}

	value_type constant(float f) const {
		return new Expression::Fixed(f);
	}

	value_type abs(value_type f) const {
		return new Expression::AbsExpression(f);
	}

	value_type min(value_type a, value_type b) const {
		return new Expression::MinExpression(a, b);
	}

	value_type max(value_type a, value_type b) const {
		return new Expression::MaxExpression(a, b);
	}

	value_type alt(value_type a, value_type b) const {
		return new Expression::AltExpression(a, b);
	}

	value_type add(value_type a, value_type b) const {
		return new Expression::AddExpression(a, b);
	}

	value_type sub(value_type a, value_type b) const {
		return new Expression::SubExpression(a, b);
	}

	value_type mul(value_type a, value_type b) const {
		return new Expression::MulExpression(a, b);
	}

	value_type div(value_type a, value_type b) const {
		return new Expression::DivExpression(a, b);
	}

	value_type mod(value_type a, value_type b) const {
		return new Expression::ModExpression(a, b);
	}

	value_type pow(value_type a, value_type b) const {
		return new Expression::PowExpression(a, b);
	}

	value_type parameter(const string &name) const {
		value_type f = nullptr;

		if ( name == "R" || name == "d" ) {
			f = new Expression::ParamEpiDistKM;
		}
		else if ( name == "D" ) {
			f = new Expression::ParamEpiDistDeg;
		}
		else if ( name == "Z" ) {
			f = new Expression::ParamDepth;
		}
		else if ( name == "h" ) {
			f = new Expression::ParamHypoDistKM;
		}
		else if ( name == "H" ) {
			f = new Expression::ParamHypoDistDeg;
		}

		if ( !f )
			parser.error_message = "unknown parameter '" + name + "'";

		return f;
	}

	value_type travelTime(const string &name) const {
		return new Expression::ParamTT(name);
	}

	value_type arrival(const string &name, const string &flag) const {
		if ( flag == "true" ) {
			return new Expression::ParamArrival(name, true);
		}
		else if ( flag.empty() || (flag == "false") ) {
			return new Expression::ParamArrival(name, false);
		}

		parser.error_message = "invalid manual flag '" + flag + "'";
		return nullptr;
	}

	ParserT &parser;
};


template <typename T>
struct Parser : bs::grammar< Parser<T> > {
	typedef T component_type;
	typedef Expression::InterfacePtr value_type;
	typedef Generator< Parser<T> > generator_type;

	// The parser object is copied a lot, so instead of keeping its own table
	// of variables, it keeps track of a reference to a common table.
	Parser(value_type &res, string &err)
	 : result(res), error_message(err), generator(*this) {
		result = nullptr;
	}

	struct Handler {
		template <typename ScannerT, typename ErrorT>
		bs::error_status<>
		operator()(ScannerT const& /*scan*/, ErrorT const& /*error*/) const {
			return bs::error_status<> (bs::error_status<>::fail);
		}
	};

	template <typename ParserT>
	struct ErrorCheck {
		ErrorCheck(ParserT const &p) : parser(p) {}

		template <typename Iterator>
		void operator()(Iterator first, Iterator last) const {
			if ( !parser.error_message.empty() )
				bs::throw_(first, (int)-1);
		}

		ParserT const &parser;
	};

	// Following is the grammar definition.
	template <typename ScannerT>
	struct definition {
		definition(Parser const& self) {
			using namespace bs;
			using namespace phoenix;

			identifier
			= lexeme_d
			[
				alpha_p
				>> *alnum_p
			][identifier.name = construct_<string>(arg1, arg2)];

			// The longest_d directive is built-in to tell the parser to make
			// the longest match it can. Thus "1.23" matches real_p rather than
			// int_p followed by ".23".
			literal
			= longest_d
			[
				int_p[literal.value = arg1]
				| real_p[literal.value = arg1]
			];

			group
			= '(' >> expression[group.value = arg1]	>> ')';

			abs
			=  '|'
			>> expression[abs.value = phoenix::bind(&generator_type::abs)(self.generator, arg1)]
			>> '|';

			min
			=  "min("
			>> expression[min.value = arg1]
			>> ','
			>> expression[min.value = phoenix::bind(&generator_type::min)(self.generator, min.value, arg1)]
			>> ')';

			max
			=  "max("
			>> expression[max.value = arg1]
			>> ','
			>> expression[max.value = phoenix::bind(&generator_type::max)(self.generator, max.value, arg1)]
			>> ')';

			constant
			= literal[constant.value = phoenix::bind(&generator_type::constant)(self.generator, arg1)];

			param
			= (+identifier[param.name = arg1])
			[
				param.value = phoenix::bind(&generator_type::parameter)(self.generator, param.name)][ErrorCheck<Parser>(self)
			];

			tt
			= ("tt(" >> (+identifier[tt.name = arg1]) >> ')')
			[
				tt.value = phoenix::bind(&generator_type::travelTime)(self.generator, tt.name)][ErrorCheck<Parser>(self)
			];

			arrival
			= ("arr(" >> (+identifier[arrival.name = arg1][arrival.flag = string()]) >> * ( "," >> (+identifier[arrival.flag = arg1])) >> ')')
			[
				arrival.value = phoenix::bind(&generator_type::arrival)(self.generator, arrival.name, arrival.flag)][ErrorCheck<Parser>(self)
			];

			// A statement can end at the end of the line, or with a semicolon.
			statement
			= guard<int>()
			(
				expression[phoenix::bind(&Parser::setResult)(self, arg1)] >> end_p
			)[Handler()];

			value
			= constant[value.value = arg1]
				| tt[value.value = arg1]
				| arrival[value.value = arg1]
				| min[value.value = arg1]
				| max[value.value = arg1]
				| param[value.value = arg1]
				| group[value.value = arg1]
				| abs[value.value = arg1];

			alternative
			= value[alternative.value = arg1]
			>> * (
				"||" >> value[alternative.value = phoenix::bind(&generator_type::alt)(self.generator, alternative.value, arg1)]
			);

			power
			= alternative[power.value = arg1]
			>> * (
				'^' >> alternative[power.value = phoenix::bind(&generator_type::pow)(self.generator, power.value, arg1)]
			);

			modulus
			= power[modulus.value = arg1]
			>> * ('%' >> power[modulus.value = phoenix::bind(&generator_type::mod)(self.generator, modulus.value, arg1)]);

			multiplication
			= modulus[multiplication.value = arg1]
			>> * ( ( '*' >> modulus[multiplication.value = phoenix::bind(&generator_type::mul)(self.generator, multiplication.value, arg1)])
			     | ( '/' >> modulus[multiplication.value = phoenix::bind(&generator_type::div)(self.generator, multiplication.value, arg1)])
			);

			addition
			= multiplication[addition.value = arg1]
			>> * ( ( '+' >> multiplication[addition.value = phoenix::bind(&generator_type::add)(self.generator, addition.value, arg1)])
			     | ( '-' >> multiplication[addition.value = phoenix::bind(&generator_type::sub)(self.generator, addition.value, arg1)])
			);

			expression = addition[expression.value = arg1];
		}

		bs::rule<ScannerT> const&
		start() const { return statement; }

		// Each rule must be declared, optionally with an associated closure.
		bs::rule<ScannerT> statement;
		bs::rule<ScannerT, StringClosure::context_t> identifier;
		bs::rule<ScannerT, LiteralClosure::context_t> literal;
		bs::rule<ScannerT, typename ParamClosure<value_type>::context_t> param, tt;
		bs::rule<ScannerT, typename Param2Closure<value_type>::context_t> arrival;
		bs::rule<ScannerT, typename ValueClosure<value_type>::context_t> expression, value,
		constant, group, abs, min, max,
		multiplication, addition, modulus, power, alternative;
	};

	void setResult(value_type x) const {
		result = x;
	}

	value_type &result;
	string &error_message;
	generator_type generator;
};


Expression::InterfacePtr Expression::Interface::parse(const std::string &text, std::string *error_str) {
	string error;
	Parser<double>::value_type result;
	Parser<double> parser(result, error);
	bs::parse_info<string::const_iterator> info;

	info = bs::parse(text.begin(), text.end(), parser, bs::space_p);

	if ( error_str ) {
		*error_str = error;
	}

	if ( !info.full ) {
		if ( error_str && error_str->empty() ) {
			*error_str = "not fully parsed";
		}
		result = nullptr;
	}

	return result;
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::SignalTime::SignalTime(int v) {
	*this = static_cast<double>(v);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::SignalTime::SignalTime(double v) {
	*this = v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::SignalTime::SignalTime(const char *expr)
: SignalTime(std::string(expr)) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::SignalTime::SignalTime(const std::string &expr) {
	string err;
	if ( !set(expr, &err) ) {
		throw invalid_argument(err);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::SignalTime::SignalTime(const SignalTime &time) {
	*this = time;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::SignalTime &AmplitudeProcessor::SignalTime::operator=(double v) {
	_exp = new Expression::Fixed(v);
	_value = v;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::SignalTime &AmplitudeProcessor::SignalTime::operator=(const SignalTime &time) {
	_exp = time._exp;
	_value = time._value;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::SignalTime &AmplitudeProcessor::SignalTime::operator+=(double v) {
	if ( !_exp ) {
		throw std::runtime_error("No expression to add to");
	}

	_exp = new Expression::AddExpression(
		static_cast<Expression::Interface*>(_exp.get()),
		new Expression::Fixed(v)
	);

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::SignalTime &AmplitudeProcessor::SignalTime::operator-=(double v) {
	if ( !_exp ) {
		throw std::runtime_error("No expression to substract from");
	}

	_exp = new Expression::SubExpression(
		static_cast<Expression::Interface*>(_exp.get()),
		new Expression::Fixed(v)
	);

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::SignalTime::operator double() const {
	if ( _value ) {
		return *_value;
	}

	throw Core::ValueError();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor::SignalTime::set(const string &text, string *error) {
	Expression::InterfacePtr tmp = Expression::Interface::parse(text, error);
	if ( !tmp ) {
		return false;
	}

	_exp = tmp;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string AmplitudeProcessor::SignalTime::toString() const {
	return _exp ? static_cast<Expression::Interface*>(_exp.get())->toString() : string();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::SignalTime::evaluate(const AmplitudeProcessor *proc, bool left) {
	_value = Core::None;
	if ( _exp ) {
		Expression::Context ctx(proc, left);
		_value = static_cast<Expression::Interface*>(_exp.get())->evaluate(ctx);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::AmplitudeProcessor() {
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::AmplitudeProcessor(const std::string& type)
: _type(type) {
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::init() {
	_enableUpdates = false;
	_enableResponses = false;
	_responseApplied = false;

	_config.noiseBegin = -35;
	_config.noiseEnd = -5;
	_config.signalBegin = -5;
	_config.signalEnd = 30;

	_config.snrMin = 3;

	_config.minimumDistance = 0;
	_config.maximumDistance = 180;
	_config.minimumDepth = -1E6;
	_config.maximumDepth = 1E6;

	_config.respTaper = 5.0;
	_config.respMinFreq = 0.00833333; // 120 secs
	_config.respMaxFreq = 0;

	_config.iaspeiAmplitudes = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::~AmplitudeProcessor() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::setEnvironment(const DataModel::Origin *hypocenter,
                                        const DataModel::SensorLocation *receiver,
                                        const DataModel::Pick *pick) {
	_environment.hypocenter = hypocenter;
	_environment.receiver = receiver;
	_environment.locale = nullptr;
	setPick(pick);

	// Check if regionalization is desired
	lock_guard<mutex> l(regionalizationRegistryMutex);
	auto it = regionalizationRegistry.find(type());
	if ( it == regionalizationRegistry.end() ) {
		// No type specific regionalization, check given distance and depth
		checkEnvironmentalLimits();
		return;
	}

	TypeSpecificRegionalization *tsr = it->second.get();
	if ( !tsr or tsr->regionalization.empty() ) {
		// No type specific regionalization, check given distance and depth
		checkEnvironmentalLimits();
		return;
	}

	// There are region profiles so meta data are required
	if ( !hypocenter ) {
		setStatus(MissingHypocenter, 0);
		return;
	}

	if ( !receiver ) {
		setStatus(MissingReceiver, 0);
		return;
	}

	auto hypoLat = environment().hypocenter->latitude().value();
	auto hypoLon = environment().hypocenter->longitude().value();
	double hypoDepth;

	try {
		// Depth is optional and throws an exception if not set
		hypoDepth = environment().hypocenter->depth().value();
	}
	catch ( ... ) {
		setStatus(MissingHypocenter, 1);
		return;
	}

	double recvLat, recvLon;

	try {
		// Both attributes are optional and throw an exception if not set
		recvLat = environment().receiver->latitude();
		recvLon = environment().receiver->longitude();
	}
	catch ( ... ) {
		setStatus(MissingReceiver, 1);
		return;
	}

	Status notFoundStatus = Error;

	for ( const Locale &profile : tsr->regionalization ) {
		if ( profile.feature ) {
			switch ( profile.check ) {
				case Locale::Source:
					if ( !profile.feature->contains(Geo::GeoCoordinate(hypoLat, hypoLon)) ) {
						notFoundStatus = EpicenterOutOfRegions;
						continue;
					}

					break;

				case Locale::SourceReceiver:
					if ( !profile.feature->contains(Geo::GeoCoordinate(hypoLat, hypoLon)) ) {
						notFoundStatus = EpicenterOutOfRegions;
						continue;
					}
					if ( !profile.feature->contains(Geo::GeoCoordinate(recvLat, recvLon)) ) {
						notFoundStatus = ReceiverOutOfRegions;
						continue;
					}
					break;

				case Locale::SourceReceiverPath:
					if ( !Regions::contains(profile.feature, hypoLat, hypoLon, recvLat, recvLon) ) {
						notFoundStatus = RayPathOutOfRegions;
						continue;
					}
					break;
			}
		}

		// Found region
		_environment.locale = &profile;

		// Copy the profile to the current configuration, actually only
		// distance and depth ranges. Others can be easily added in future.
		_config.minimumDistance = profile.minimumDistance;
		_config.maximumDistance = profile.maximumDistance;
		_config.minimumDepth = profile.minimumDepth;
		_config.maximumDepth = profile.maximumDepth;
		break;
	}

	if ( !_environment.locale ) {
		setStatus(notFoundStatus, 0);
		return;
	}

	double dist, az, baz;
	Math::Geo::delazi_wgs84(hypoLat, hypoLon, recvLat, recvLon,
	                        &dist, &az, &baz);

	if ( dist < _config.minimumDistance || dist > _config.maximumDistance ) {
		setStatus(DistanceOutOfRange, dist);
		return;
	}

	if ( hypoDepth < _config.minimumDepth || hypoDepth > _config.maximumDepth ) {
		setStatus(DepthOutOfRange, hypoDepth);
		return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::checkEnvironmentalLimits() {
	if ( _environment.hypocenter ) {
		try {
			auto hypoDepth = _environment.hypocenter->depth().value();
			if ( hypoDepth < _config.minimumDepth || hypoDepth > _config.maximumDepth ) {
				setStatus(DepthOutOfRange, hypoDepth);
				return;
			}
		}
		catch ( ... ) {}

		if ( _environment.receiver ) {
			auto hypoLat = _environment.hypocenter->latitude().value();
			auto hypoLon = _environment.hypocenter->longitude().value();
			double recvLat, recvLon;

			try {
				// Both attributes are optional and throw an exception if not set
				recvLat = environment().receiver->latitude();
				recvLon = environment().receiver->longitude();
			}
			catch ( ... ) {
				return;
			}

			double dist, az, baz;
			Math::Geo::delazi_wgs84(hypoLat, hypoLon, recvLat, recvLon,
			                        &dist, &az, &baz);

			if ( dist < _config.minimumDistance || dist > _config.maximumDistance ) {
				setStatus(DistanceOutOfRange, dist);
				return;
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::setPick(const DataModel::Pick *pick) {
	_environment.pick = pick;
	if ( pick )
		setReferencingPickID(pick->publicID());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::setUpdateEnabled(bool e) {
	_enableUpdates = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor::isUpdateEnabled() const {
	return _enableUpdates;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::setReferencingPickID(const std::string& pickID) {
	_pickID = pickID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& AmplitudeProcessor::referencingPickID() const {
	return _pickID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OPT(double) AmplitudeProcessor::noiseOffset() const {
	return _noiseOffset;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OPT(double) AmplitudeProcessor::noiseAmplitude() const {
	return _noiseAmplitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& AmplitudeProcessor::type() const {
	return _type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& AmplitudeProcessor::unit() const {
	return _unit;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::computeTimeWindow() {
	if ( !(bool)_trigger ) {
		setTimeWindow(Core::TimeWindow());
		return;
	}

	// Evaluate noise and signal time windows
	try {
		_config.noiseBegin.evaluate(this, true);
		_config.noiseEnd.evaluate(this, false);
		_config.signalBegin.evaluate(this, true);
		_config.signalEnd.evaluate(this, false);
	}
	catch ( StatusException &e ) {
		setStatus(e.status(), e.value());
		setTimeWindow(Core::TimeWindow());
		return;
	}

	Core::Time startTime = _trigger + Core::TimeSpan(_config.noiseBegin);
	Core::Time   endTime = _trigger + Core::TimeSpan(_config.signalEnd);

	// Add the taper length to the requested time window otherwise
	// amplitudes are damped
	if ( _enableResponses ) {
		endTime += Core::TimeSpan(std::max(0.0, _config.respTaper));
		if ( (double)margin() < _config.respTaper )
			startTime -= Core::TimeSpan(_config.respTaper) - margin();
	}

	setTimeWindow(Core::TimeWindow(startTime, endTime));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int AmplitudeProcessor::capabilities() const {
	return NoCapability;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor::supports(Capability c) const {
	return (capabilities() & c) > 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::IDList
AmplitudeProcessor::capabilityParameters(Capability) const {
	return IDList();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor::setParameter(Capability , const std::string &) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::setUnit(const std::string &unit) {
	_unit = unit;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::reprocess(OPT(double) searchBegin,
                                   OPT(double) searchEnd) {
	if ( _stream.lastRecord ) {
		_searchBegin = searchBegin;
		_searchEnd = searchEnd;

		// Force recomputation of noise amplitude and noise offset
		_noiseAmplitude = Core::None;
		process(_stream.lastRecord.get());

		// Reset search window again
		_searchBegin = _searchEnd = Core::None;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::reset() {
	TimeWindowProcessor::reset();
	_noiseAmplitude = Core::None;
	_noiseOffset = Core::None;
	_responseApplied = false;
	_trigger = Core::Time();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::process(const Record *record, const DoubleArray &) {
	process(record);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::process(const Record *record) {
	// Sampling frequency has not been set yet
	if ( _stream.fsamp == 0.0 )
		return;

	int n = static_cast<int>(_data.size());

	// signal and noise window relative to _continuous->startTime()
	double dt0  = _trigger - dataTimeWindow().startTime();
	double dt1  = dataTimeWindow().endTime() - dataTimeWindow().startTime();
	double dtw1  = timeWindow().endTime() - dataTimeWindow().startTime();
	double dtn1 = dt0 + _config.noiseBegin;
	double dtn2 = dt0 + _config.noiseEnd;
	double dts1 = dt0 + _config.signalBegin;
	double dts2 = dt0 + _config.signalEnd;

	// Noise indicies
	int ni1 = static_cast<int>(dtn1*_stream.fsamp + 0.5);
	int ni2 = static_cast<int>(dtn2*_stream.fsamp + 0.5);

	if ( ni1 < 0 || ni2 < 0 ) {
		SEISCOMP_DEBUG("Noise data not available -> abort");
		setStatus(Error, 1);
		return;
	}

	if ( n < ni2 ) {
		// the noise window is not complete
		return;
	}


	// **** compute signal amplitude ********************************

	// these are the offsets of the beginning and end
	// of the signal window relative to the start of
	// the continuous record in samples
	int i1 = static_cast<int>(dts1 * _stream.fsamp + 0.5);
	int i2 = static_cast<int>(dts2 * _stream.fsamp + 0.5);

	//int progress = int(100.*(n-i1)/(i2-i1));
	//int progress = int(100.*(dt1-dts1)/(dts2-dts1));
	int progress = static_cast<int>(100. * (dt1 - dts1) / (std::max(dtw1, dts2) - dts1));

	if ( progress > 100 ) {
		progress = 100;
	}

	setStatus(InProgress, progress);

	if ( i1 < 0 ) i1 = 0;
	if ( i2 > n ) i2 = n;

	bool unlockCalculation = ((_enableUpdates && !_enableResponses) && progress > 0) || progress >= 100;

	if ( unlockCalculation ) {
		if ( _streamConfig[_usedComponent].gain == 0.0 ) {
			setStatus(MissingGain, 0);
			return;
		}

		// **** prepare the data to compute the noise
		prepareData(_data);
		if ( isFinished() )
			return;

		// **** compute noise amplitude *********************************
		// if the noise hasn't been measured yet...
		if ( !_noiseAmplitude ) {
			// compute pre-arrival data offset and noise amplitude

			double off = 0., amp = 0.;

			if ( !computeNoise(_data, ni1, ni2, &off, &amp) ) {
				SEISCOMP_DEBUG("Noise computation failed -> abort");
				setStatus(Error, 2);
				return;
			}

			_noiseOffset = off;
			_noiseAmplitude = amp;
		}

		AmplitudeIndex index;
		Result res;
		res.component = _usedComponent;
		res.record = record;
		res.period = -1;
		res.snr = -1;

		res.amplitude.value = -1;
		res.amplitude.lowerUncertainty = Core::None;
		res.amplitude.upperUncertainty = Core::None;

		index.index = -1;
		index.begin = 0;
		index.end = 0;

		double dtsw1, dtsw2;

		if ( _searchBegin ) {
			dtsw1 = dt0 + *_searchBegin;
			if ( dtsw1 < dts1 ) dtsw1 = dts1;
			if ( dtsw1 > dts2 ) dtsw1 = dts2;
		}
		else
			dtsw1 = dts1;

		if ( _searchEnd ) {
			dtsw2 = dt0 + *_searchEnd;
			if ( dtsw2 < dts1 ) dtsw2 = dts1;
			if ( dtsw2 > dts2 ) dtsw2 = dts2;
		}
		else
			dtsw2 = dts2;

		int si1 = int(dtsw1*_stream.fsamp+0.5);
		int si2 = int(dtsw2*_stream.fsamp+0.5);

		si1 = std::max(si1, i1);
		si2 = std::min(si2, i2);

		if ( !computeAmplitude(_data, i1, i2, si1, si2, *_noiseOffset,
		                       &index, &res.amplitude, &res.period, &res.snr) ) {
			if ( progress >= 100 ) {
				if ( status() == LowSNR )
					SEISCOMP_DEBUG("Amplitude %s computation for stream %s failed because of low SNR (%.2f < %.2f)",
					              _type.c_str(), record->streamID().c_str(), res.snr, _config.snrMin);
				else if ( status() < Terminated ) {
					SEISCOMP_DEBUG("Amplitude %s computation for stream %s failed -> abort",
					              _type.c_str(), record->streamID().c_str());
					setStatus(Error, 3);
				}

				_lastAmplitude = Core::None;
			}

			return;
		}

		if ( _lastAmplitude ) {
			if ( res.amplitude.value <= *_lastAmplitude ) {
				if ( progress >= 100 ) {
					setStatus(Finished, 100.);
					_lastAmplitude = Core::None;
				}

				return;
			}
		}

		_lastAmplitude = res.amplitude.value;

		double dt = index.index / _stream.fsamp;
		res.period /= _stream.fsamp;

		if ( index.begin > index.end ) std::swap(index.begin, index.end);

		// Update status information
		res.time.reference = dataTimeWindow().startTime() + Core::TimeSpan(dt);
		//res.time.begin = index.begin / _stream.fsamp;
		//res.time.end = index.end / _stream.fsamp;
		res.time.begin = (si1 - index.index) / _stream.fsamp;
		res.time.end = (si2 - index.index) / _stream.fsamp;

		if ( progress >= 100 ) {
			setStatus(Finished, 100.);
			_lastAmplitude = Core::None;
		}

		emitAmplitude(res);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor::handleGap(Filter *, const Core::TimeSpan &span,
                                   double, double, size_t) {
	if ( _stream.dataTimeWindow.endTime()+span < timeWindow().startTime() ) {
		// Save trigger, because reset will unset it
		Core::Time t = _trigger;
		reset();
		_trigger = t;
		return true;
	}

	//TODO: Handle gaps
	setStatus(QCError, 1);
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::prepareData(DoubleArray &data) {
	Sensor *sensor = _streamConfig[_usedComponent].sensor();

	// When using full responses then all information needs to be set up
	// correctly otherwise an error is set
	if ( _enableResponses ) {
		if ( !sensor ) {
			setStatus(MissingResponse, 1);
			return;
		}

		if ( !sensor->response() ) {
			setStatus(MissingResponse, 2);
			return;
		}

		// If the unit cannot be converted into the internal
		// enum (what basically means "unknown") then the deconvolution
		// cannot be correctly. We do not want to assume a unit here
		// to prevent computation errors in case of bad configuration.
		SignalUnit unit;
		if ( !unit.fromString(_streamConfig[_usedComponent].gainUnit.c_str()) ) {
			// Invalid unit string
			setStatus(IncompatibleUnit, 2);
			return;
		}

		int intSteps = 0;
		switch ( unit ) {
			case Meter:
				intSteps = -1;
				break;
			case MeterPerSecond:
				break;
			case MeterPerSecondSquared:
				intSteps = 1;
				break;
			default:
				setStatus(IncompatibleUnit, 1);
				return;
		}

		if ( _responseApplied ) return;

		_responseApplied = true;

		if ( !deconvolveData(sensor->response(), _data, intSteps) ) {
			setStatus(DeconvolutionFailed, 0);
			return;
		}
	}
	else {
		// If the sensor is known then check the unit and skip
		// non velocity streams. Otherwise simply use the data
		// to be compatible to the old version. This will be
		// changed in the future and checked more strictly.
		if ( sensor ) {
			SignalUnit unit;
			if ( !unit.fromString(_streamConfig[_usedComponent].gainUnit.c_str()) ) {
				// Invalid unit string
				setStatus(IncompatibleUnit, 4);
				return;
			}

			switch ( unit ) {
				case Meter:
					if ( _enableUpdates ) {
						// Updates with differentiation are not yet supported.
						setStatus(IncompatibleUnit, 5);
						return;
					}

					// Derive to m/s
					{
						Math::Filtering::IIRDifferentiate<double> diff;
						diff.setSamplingFrequency(_stream.fsamp);
						diff.apply(data.size(), data.typedData());
					}
					break;

				case MeterPerSecond:
					break;

				default:
					setStatus(IncompatibleUnit, 3);
					return;
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor::deconvolveData(Response *resp, DoubleArray &data,
                                        int numberOfIntegrations) {
	// Remove linear trend
	double m,n;
	Math::Statistics::computeLinearTrend(data.size(), data.typedData(), m, n);
	Math::Statistics::detrend(data.size(), data.typedData(), m, n);

	bool ret = resp->deconvolveFFT(data, _stream.fsamp, _config.respTaper,
	                               _config.respMinFreq, _config.respMaxFreq,
	                               numberOfIntegrations < 0 ? 0 : numberOfIntegrations);

	if ( !ret )
		return false;

	// If number of integrations are negative, derive data
	while ( numberOfIntegrations < 0 ) {
		Math::Filtering::IIRDifferentiate<double> diff;
		diff.setSamplingFrequency(_stream.fsamp);
		diff.apply(data.size(), data.typedData());
		++numberOfIntegrations;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor::computeNoise(const DoubleArray &data, int i1, int i2, double *offset, double *amplitude) {
	// compute offset and rms within the time window
	if ( i1 < 0 ) {
		i1 = 0;
	}

	if ( i2 < 0 ) {
		return false;
	}

	if ( i2 > static_cast<int>(data.size()) ) {
		i2 = static_cast<int>(data.size());
	}

	// If noise window is zero return an amplitude and offset of zero as well.
	if ( i2-i1 == 0 ) {
		*amplitude = 0;
		*offset = 0;
		return true;
	}

	DoubleArrayPtr d = static_cast<DoubleArray*>(data.slice(i1, i2));
	if ( !d ) {
		return false;
	}

	double ofs, amp;

	// compute pre-arrival offset
	ofs = d->median();
	// compute rms after removing offset
	amp = 2 * d->rms(ofs);

	if ( offset ) *offset = ofs;
	if ( amplitude ) *amplitude = amp;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor::setup(const Settings &settings) {
	SEISCOMP_DEBUG("%s.%s.%s.%s - %s amplitude configuration:",
	               settings.networkCode.c_str(),
	               settings.stationCode.c_str(),
	               settings.locationCode.c_str(),
	               settings.channelCode.c_str(),
	               _type.c_str());

	_environment.networkCode = settings.networkCode;
	_environment.stationCode = settings.stationCode;
	_environment.locationCode = settings.locationCode;
	_environment.channelCode = settings.channelCode;

	if ( !TimeWindowProcessor::setup(settings) ) {
		return false;
	}

	// initalize regionalized settings
	if ( !initRegionalization(settings) ) {
		return false;
	}

	// Reset Wood-Anderson response to default
	_config.woodAndersonResponse = Math::SeismometerResponse::WoodAnderson::Config();

	auto cfg = settings.localConfiguration;
	try { _config.woodAndersonResponse.gain = cfg->getDouble("amplitudes.WoodAnderson.gain"); }
	catch ( ... ) {}
	try { _config.woodAndersonResponse.T0 = cfg->getDouble("amplitudes.WoodAnderson.T0"); }
	catch ( ... ) {}
	try { _config.woodAndersonResponse.h = cfg->getDouble("amplitudes.WoodAnderson.h"); }
	catch ( ... ) {}

	settings.getValue(_config.woodAndersonResponse.gain, "amplitudes.WoodAnderson.gain");
	settings.getValue(_config.woodAndersonResponse.T0, "amplitudes.WoodAnderson.T0");
	settings.getValue(_config.woodAndersonResponse.h, "amplitudes.WoodAnderson.h");

	if ( !parseSaturationThreshold(settings, "amplitudes.saturationThreshold") ) {
		return false;
	}

	if ( !parseSaturationThreshold(settings, "amplitudes." + _type + ".saturationThreshold") ) {
		return false;
	}

	try {
		if ( settings.getBool("amplitudes." + _type + ".enable") == false ) {
			return false;
		}
	}
	catch ( ... ) {
		// In case the amplitude specific enable flag is not set,
		// check the global flag
		try {
			if ( settings.getBool("amplitudes.enable") == false ) {
				return false;
			}
		}
		catch ( ... ) {}
	}

	if ( !settings.getValue(_enableResponses, "amplitudes." + _type + ".enableResponses") ) {
		settings.getValue(_enableResponses, "amplitudes.enableResponses");
	}

	string expr, error;

	try {
		expr = settings.getString("amplitudes." + _type + ".noiseBegin");
		if ( !_config.noiseBegin.set(expr, &error) ) {
			SEISCOMP_ERROR("%s.%s.%s.%s - %s noise begin '%s': %s",
			               settings.networkCode.c_str(),
			               settings.stationCode.c_str(),
			               settings.locationCode.c_str(),
			               settings.channelCode.c_str(),
			               _type.c_str(), expr.c_str(), error.c_str());
			return false;
		}
	}
	catch ( ... ) {}

	try {
		expr = settings.getString("amplitudes." + _type + ".noiseEnd");
		if ( !_config.noiseEnd.set(expr, &error) ) {
			SEISCOMP_ERROR("%s.%s.%s.%s - %s noise end '%s': %s",
			               settings.networkCode.c_str(),
			               settings.stationCode.c_str(),
			               settings.locationCode.c_str(),
			               settings.channelCode.c_str(),
			               _type.c_str(), expr.c_str(), error.c_str());
			return false;
		}
	}
	catch ( ... ) {}

	try {
		expr = settings.getString("amplitudes." + _type + ".signalBegin");
		if ( !_config.signalBegin.set(expr, &error) ) {
			SEISCOMP_ERROR("%s.%s.%s.%s - %s signal begin '%s': %s",
			               settings.networkCode.c_str(),
			               settings.stationCode.c_str(),
			               settings.locationCode.c_str(),
			               settings.channelCode.c_str(),
			               _type.c_str(), expr.c_str(), error.c_str());
			return false;
		}
	}
	catch ( ... ) {}

	try {
		expr = settings.getString("amplitudes." + _type + ".signalEnd");
		if ( !_config.signalEnd.set(expr, &error) ) {
			SEISCOMP_ERROR("%s.%s.%s.%s - %s signal end '%s': %s",
			               settings.networkCode.c_str(),
			               settings.stationCode.c_str(),
			               settings.locationCode.c_str(),
			               settings.channelCode.c_str(),
			               _type.c_str(), expr.c_str(), error.c_str());
			return false;
		}
	}
	catch ( ... ) {}

	try { _config.ttInterface = cfg->getString("amplitudes.ttt.interface"); }
	catch ( ... ) {}
	try { _config.ttModel = cfg->getString("amplitudes.ttt.model"); }
	catch ( ... ) {}

	settings.getValue(_config.ttInterface, "amplitudes.ttt.interface");
	settings.getValue(_config.ttModel, "amplitudes.ttt.model");

	settings.getValue(_config.snrMin, "amplitudes." + _type + ".minSNR");
	settings.getValue(_config.minimumDistance, "amplitudes." + _type + ".minDist");
	settings.getValue(_config.maximumDistance, "amplitudes." + _type + ".maxDist");
	settings.getValue(_config.minimumDepth, "amplitudes." + _type + ".minDepth");
	settings.getValue(_config.maximumDepth, "amplitudes." + _type + ".maxDepth");

	SEISCOMP_DEBUG("  + WA.gain = %f", _config.woodAndersonResponse.gain);
	SEISCOMP_DEBUG("  + WA.T0 = %f", _config.woodAndersonResponse.T0);
	SEISCOMP_DEBUG("  + WA.h = %f", _config.woodAndersonResponse.h);
	SEISCOMP_DEBUG("  + minimum distance = %.5f deg", _config.minimumDistance);
	SEISCOMP_DEBUG("  + maximum distance = %.5f deg", _config.maximumDistance);
	SEISCOMP_DEBUG("  + minimum depth = %.3f km", _config.minimumDepth);
	SEISCOMP_DEBUG("  + maximum depth = %.3f km", _config.maximumDepth);
	SEISCOMP_DEBUG("  + noise begin = %s", _config.noiseBegin.toString().c_str());
	SEISCOMP_DEBUG("  + noise end = %s", _config.noiseEnd.toString().c_str());
	SEISCOMP_DEBUG("  + signal begin = %s", _config.signalBegin.toString().c_str());
	SEISCOMP_DEBUG("  + signal end = %s", _config.signalEnd.toString().c_str());
	SEISCOMP_DEBUG("  + minimum SNR = %.3f", _config.snrMin);
	SEISCOMP_DEBUG("  + response correction = %i", _enableResponses);

	if ( !settings.getValue(_config.respTaper, "amplitudes." + _type + ".resp.taper") ) {
		settings.getValue(_config.respTaper, "amplitudes.resp.taper");
		if ( _enableResponses ) {
			SEISCOMP_DEBUG("  + response taper = %.3f", _config.respTaper);
		}
	}

	if ( !settings.getValue(_config.respMinFreq, "amplitudes." + _type + ".resp.minFreq") ) {
		settings.getValue(_config.respMinFreq, "amplitudes.resp.minFreq");
		if ( _enableResponses && _config.respMinFreq != 0.0 ) {
			SEISCOMP_DEBUG("  + response minimum frequency = %.3f", _config.respMinFreq);
		}
	}

	if ( !settings.getValue(_config.respMaxFreq, "amplitudes." + _type + ".resp.maxFreq") ) {
		settings.getValue(_config.respMaxFreq, "amplitudes.resp.maxFreq");
		if ( _enableResponses && _config.respMaxFreq != 0.0 ) {
			SEISCOMP_DEBUG("  + response maximum frequency = %.3f", _config.respMaxFreq);
		}
	}

	try { _config.iaspeiAmplitudes = cfg->getBool("amplitudes.iaspei"); }
	catch ( ... ) {}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor::initLocale(Locale *, const Settings &) {
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor::readLocale(Locale *locale,
                                    const Settings &settings,
                                    const std::string &cfgPrefix) {
	OPT(double) minDist, maxDist, minDepth, maxDepth;
	const Seiscomp::Config::Config *cfg = settings.localConfiguration;

	try { minDist  = cfg->getDouble(cfgPrefix + "minDist"); } catch ( ... ) {}
	try { maxDist  = cfg->getDouble(cfgPrefix + "maxDist"); } catch ( ... ) {}
	try { minDepth = cfg->getDouble(cfgPrefix + "minDepth"); } catch ( ... ) {}
	try { maxDepth = cfg->getDouble(cfgPrefix + "maxDepth"); } catch ( ... ) {}

	locale->check = Locale::Source;
	locale->minimumDistance = minDist ? *minDist : _config.minimumDistance;
	locale->maximumDistance = maxDist ? *maxDist : _config.maximumDistance;
	locale->minimumDepth = minDepth ? *minDepth : _config.minimumDepth;
	locale->maximumDepth = maxDepth ? *maxDepth : _config.maximumDepth;

	try {
		string check = cfg->getString(cfgPrefix + "check");
		if ( check == "source" ) {
			locale->check = Locale::Source;
		}
		else if ( check == "source-receiver" ) {
			locale->check = Locale::SourceReceiver;
		}
		else if ( check == "raypath" ) {
			locale->check = Locale::SourceReceiverPath;
		}
		else {
			SEISCOMP_ERROR("%scheck: invalid region check: %s",
			               cfgPrefix.c_str(), check.c_str());
			return false;
		}
	}
	catch ( ... ) {}

	if ( !initLocale(locale, settings) ) {
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor::initRegionalization(const Settings &settings) {
	lock_guard<mutex> l(regionalizationRegistryMutex);

	TypeSpecificRegionalizationPtr regionalizedSettings;
	auto it = regionalizationRegistry.find(type());
	if ( it == regionalizationRegistry.end() ) {
		regionalizedSettings = new TypeSpecificRegionalization();
		regionalizationRegistry[type()] = nullptr;

		const Seiscomp::Config::Config *cfg = settings.localConfiguration;
		if ( cfg ) {
			bool regionalize = true;
			try {
				regionalize = cfg->getBool("amplitudes." + _type + ".regionalize");
			}
			catch ( ... ) {}

			if ( regionalize ) {
				try {
					string filename = cfg->getString("magnitudes." + type() + ".regionFile");
					filename = Seiscomp::Environment::Instance()->absolutePath(filename);
					regionalizedSettings->regions = Regions::load(filename);

					if ( !regionalizedSettings->regions ) {
						SEISCOMP_ERROR("Failed to read/parse %s regions file: %s",
						               type().c_str(), filename.c_str());
						return false;
					}

					for ( Geo::GeoFeature *feature : regionalizedSettings->regions->featureSet.features()) {
						if ( feature->name().empty() ) continue;
						if ( feature->name() == "world" ) {
							SEISCOMP_ERROR("Region name 'world' is not allowed as it is "
							               "reserved");
							return false;
						}

						string cfgPrefix = "magnitudes." + type() + ".region." + feature->name() + ".";
						try {
							if ( !cfg->getBool(cfgPrefix + "enable") ) {
								SEISCOMP_DEBUG("%s: - region %s (disabled)",
								               _type.c_str(), feature->name().c_str());
								continue;
							}
						}
						catch ( ... ) {
							SEISCOMP_DEBUG("%s: - region %s (disabled)",
							               _type.c_str(), feature->name().c_str());
							continue;
						}

						Locale config;
						config.name = feature->name();
						config.feature = feature;

						if ( !readLocale(&config, settings, cfgPrefix) )
							return false;

						regionalizedSettings->regionalization.push_back(config);
					}
				}
				catch ( ... ) {}

				try {
					if ( cfg->getBool("magnitudes." + type() + ".region.world.enable") ) {
						string cfgPrefix = "magnitudes." + type() + ".region.world.";
						Locale config;
						config.name = "world";
						config.feature = nullptr;

						if ( !readLocale(&config, settings, cfgPrefix) )
							return false;

						regionalizedSettings->regionalization.push_back(config);
					}
				}
				catch ( ... ) {}
			}
		}

		regionalizationRegistry[type()] = regionalizedSettings;
	}
	else
		regionalizedSettings = it->second;

	if ( regionalizedSettings ) {
		return true;
	}

	setStatus(ConfigurationError, 0);
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::setTrigger(const Core::Time& trigger) {
	if ( _trigger )
		throw Core::ValueException("The trigger has been set already");

	_trigger = trigger;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::Time AmplitudeProcessor::trigger() const {
	return _trigger;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::setPublishFunction(const PublishFunc& func) {
	_func = func;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::emitAmplitude(const Result &res) {
	if ( isEnabled() && _func )
		_func(this, res);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::finalizeAmplitude(DataModel::Amplitude *) const {
	// Do nothing
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const AmplitudeProcessor *
AmplitudeProcessor::componentProcessor(Component comp) const {
	if ( comp < VerticalComponent || comp > SecondHorizontalComponent )
		return nullptr;

	if ( comp != (Component)_usedComponent ) return nullptr;

	return this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DoubleArray *AmplitudeProcessor::processedData(Component comp) const {
	if ( comp < VerticalComponent || comp > SecondHorizontalComponent )
		return nullptr;

	if ( comp != (Component)_usedComponent ) return nullptr;

	return &continuousData();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor::writeData() const {
	if ( !_stream.lastRecord ) return;

	const DoubleArray *data = processedData((Component)_usedComponent);
	if ( data == nullptr ) return;

 	std::ofstream of((_stream.lastRecord->streamID() + "-" + type() + ".data").c_str());

	of << "#sampleRate: " << _stream.lastRecord->samplingFrequency() << std::endl;

	for ( int i = 0; i < data->size(); ++i )
		of << i << "\t" << (*data)[i] << std::endl;
	of.close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}

}
