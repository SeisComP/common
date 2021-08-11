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


#ifndef SCARCHIVE_JSON_H
#define SCARCHIVE_JSON_H


#include <seiscomp/core/io.h>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>


namespace Seiscomp {
namespace IO {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** \brief An archive using JSON streams
 */
class SC_SYSTEM_CORE_API JSONArchive : public Core::Archive {
	// ----------------------------------------------------------------------
	//  Public data types
	// ----------------------------------------------------------------------
	public:
		typedef rapidjson::Document Document;
		typedef Document::ValueType Value;
		typedef Value::ConstMemberIterator ConstItr;
		typedef rapidjson::SizeType Size;


	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		//! Constructor
		JSONArchive();

		//! Constructor with predefined buffer and mode
		JSONArchive(std::streambuf* buf, bool isReading = true,
		            int forceWriteVersion = -1);

		//! Destructor
		~JSONArchive();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		bool open(const char *file);
		bool open(std::streambuf*);
		//! Reads an archive from a rapidjson document value
		bool from(const Value*);
		//! Reads the archive from a string
		bool from(const char *str);
		//! Reads the archive from a string and allows optional insitu parsing
		//! which replaces the contents of the string (destructive).
		bool from(char *str, bool insitu = true);

		bool create(const char *file, bool writeVersion = true);
		bool create(std::streambuf*, bool writeVersion = true);
		bool create(std::ostream*, bool writeVersion = true);

		void setFormattedOutput(bool);

		//! Sets if a root object is wrapped around the serialized objects.
		//! Default is true.
		void setRootObject(bool);

		//! Implements derived virtual method
		virtual void close();


	// ----------------------------------------------------------------------
	//  Read methods
	// ----------------------------------------------------------------------
	public:
		//! Reads an int8
		virtual void read(std::int8_t& value);
		//! Reads an int16
		virtual void read(std::int16_t& value);
		//! Reads an int32
		virtual void read(std::int32_t& value);
		//! Reads an int64
		virtual void read(std::int64_t& value);
		//! Reads a float
		virtual void read(float& value);
		//! Reads a double
		virtual void read(double& value);

		virtual void read(std::vector<char>& value);
		virtual void read(std::vector<int8_t>& value);
		virtual void read(std::vector<int16_t>& value);
		virtual void read(std::vector<int32_t>& value);
		virtual void read(std::vector<int64_t>& value);
		virtual void read(std::vector<float>& value);
		virtual void read(std::vector<double>& value);
		virtual void read(std::vector<std::string>& value);
		virtual void read(std::vector<Core::Time>& value);

		//! Reads a complex float
		virtual void read(std::complex<float>& value);
		//! Reads a complex double
		virtual void read(std::complex<double>& value);
		//! Reads a boolean
		virtual void read(bool& value);

		//! Reads a vector of complex doubles
		virtual void read(std::vector<std::complex<double> >& value);

		//! Reads a string
		virtual void read(std::string& value);

		//! Reads a time
		virtual void read(Seiscomp::Core::Time& value);


	// ----------------------------------------------------------------------
	//  Write methods
	// ----------------------------------------------------------------------
	public:
		//! Writes an int8
		virtual void write(std::int8_t value);
		//! Writes an int16
		virtual void write(std::int16_t value);
		//! Writes an int32
		virtual void write(std::int32_t value);
		//! Writes an int64
		virtual void write(std::int64_t value);
		//! Writes a float
		virtual void write(float value);
		//! Writes a double
		virtual void write(double value);

		virtual void write(std::vector<char>& value);
		virtual void write(std::vector<int8_t>& value);
		virtual void write(std::vector<int16_t>& value);
		virtual void write(std::vector<int32_t>& value);
		virtual void write(std::vector<int64_t>& value);
		virtual void write(std::vector<float>& value);
		virtual void write(std::vector<double>& value);
		virtual void write(std::vector<std::string>& value);
		virtual void write(std::vector<Core::Time>& value);

		//! Writes a complex float
		virtual void write(std::complex<float>& value);
		//! Writes a complex double
		virtual void write(std::complex<double>& value);
		//! Writes a boolean
		virtual void write(bool value);

		//! Writes a vector of complex doubles
		virtual void write(std::vector<std::complex<double> >& value);

		//! Writes a string
		virtual void write(std::string& value);

		//! Writes a time
		virtual void write(Seiscomp::Core::Time& value);


	// ----------------------------------------------------------------------
	//  Protected interface
	// ----------------------------------------------------------------------
	protected:
		//! Implements derived virtual method
		bool locateObjectByName(const char* name, const char* targetClass, bool nullable);
		bool locateNextObjectByName(const char* name, const char* targetClass);
		void locateNullObjectByName(const char* name, const char* targetClass, bool first);

		void readSequence();
		void writeSequence(int size);

		//! Implements derived virtual method
		std::string determineClassName();

		//! Implements derived virtual method
		virtual void setClassName(const char* className);

		//! Implements derived virtual method
		void serialize(RootType* object);

		//! Implements derived virtual method
		void serialize(SerializeDispatcher&);


	// ----------------------------------------------------------------------
	//  Private interface
	// ----------------------------------------------------------------------
	private:
		void parseVersion();
		void createDocument(bool writeVersion);

		template <typename T>
		void _serialize(T &target);

		void preAttrib();
		void postAttrib();


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		const Value *findAttrib(const Value *node, const char* name);
		const Value *findTag(const Value *node, Size &index,
		                     const char* name, const char* targetClass);
		const Value *findNextTag(const Value *node, Size &index,
		                         const char* name, const char* targetClass);

		template <typename T>
		void readIntVector(std::vector<T> &value);

		template <typename T>
		void writeVector(std::vector<T> &value);


	private:
		int             _forceWriteVersion;
		bool            _deleteBufOnClose;
		bool            _deleteStreamOnClose;
		bool            _rootObject;
		bool            _nullable;
		std::string     _tagname;
		int             _indent;
		int             _sequenceSize;
		bool            _isSequence;
		int             _attribIndex;
		bool            _isClass;
		bool            _formattedOutput;
		std::streambuf *_buf;
		std::ostream   *_os;
		Document       *_document;
		const Value    *_objectLocation;
		const Value    *_current;
		const Value    *_currentArray;
		Size            _currentIndex;


};


}
}



#endif
