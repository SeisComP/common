
template <typename T, typename... Args>
std::string DatabaseInterface::Query(
	const DatabaseInterface *db, std::string_view s, T param1, Args... args
) {
	using namespace std;

	size_t tail = 0;
	string sql = Parse(db, s.substr(tail), &tail);

	if ( tail == string::npos ) {
		// No parameters found but given in the signature.
		throw runtime_error("parameter overflow");
	}

	if constexpr ( Seiscomp::Core::TypeTraits::IsStringLike<T>() ) {
		sql += '\'';
		string converted;
		db->escape(converted, param1);
		sql += converted;
		sql += '\'';
	}
	else {
		// Only numeric types are safe to emit unquoted and unescaped. Any other
		// type (Core::Time, Enumeration, a user type with operator<<, ...) would
		// be rendered via toString() without quoting or escaping, which is either
		// invalid SQL or an injection vector. Reject those at compile time so the
		// caller must pass a string type (escaped + quoted) or a number.
		static_assert(Seiscomp::Core::TypeTraits::IsSQLNumber<T>(),
		              "Query() parameter must be a string type (escaped and quoted) "
		              "or a numeric type. Convert other types to a string explicitly.");
		sql += Seiscomp::Core::toString(param1);
	}

	if ( tail < s.length() ) {
		sql += Query(db, s.substr(tail), args...);
	}
	else {
		if constexpr ( sizeof...(Args) > 0 ) {
			// There are still parameters but no input left.
			throw runtime_error("parameter underflow");
		}
	}

	return sql;
}
