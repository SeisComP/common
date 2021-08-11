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

#ifndef SCARCHIVE_BSON_H
#define SCARCHIVE_BSON_H

#include <seiscomp/core/io.h>
#include <seiscomp/core.h>


namespace Seiscomp {
namespace IO {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** \brief An archive using BSON streams
 */
class SC_SYSTEM_CORE_API BSONArchive : public Seiscomp::Core::Archive {
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		//! Constructor
		BSONArchive();

		//! Constructor with predefined buffer and mode
		BSONArchive(std::streambuf* buf, bool isReading = true,
			   int forceWriteVersion = -1);

		//! Destructor
		~BSONArchive();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		//! Opens an archive reading from a streambuf
		bool open(std::streambuf*);

		//! Implements derived virtual method
		virtual bool open(const char* filename);

		//! Creates an archive writing to a streambuf
		bool create(std::streambuf* buf, bool writeVersion = true);

		//! Implements derived virtual method
		virtual bool create(const char* filename, bool writeVersion = true);

		//! Implements derived virtual method
		virtual void close();

		/**
		 * Enables/Disables zip compression
		 * @param enable The state of this flag
		 */
		void setCompression(bool enable);

		/**
		 * Enables/Disables JSON format
		 * @param enable The state of this flag
		 */
		void setJSON(bool enable);


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

		virtual void read(char& value);

		//! Reads a vector of native types
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

		virtual void write(char& value);

		//! Writes a vector of native types
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
		void setValidity(bool v);

		//! Implements derived virtual method
		bool locateObjectByName(const char* name, const char* targetClass, bool nullable);
		bool locateNextObjectByName(const char* name, const char* targetClass);

		//! Implements derived virtual method
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
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		bool open();
		bool create(bool writeVersion);

		template<typename T>
		void readInt(T& value);

		template<typename T>
		void readVector(std::vector<T>& value);

		template<typename T>
		void readComplex(std::complex<T>& value);

		template<typename T>
		void writeVector(std::vector<T>& value);

		template<typename T>
		void writeComplex(std::complex<T>& value);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		DEFINE_SMARTPOINTER(BSONImpl);

		BSONImplPtr            _impl;

		std::string            _className;
		std::string            _attribName;
		int                    _siblingCount;
		bool                   _startSequence;
		bool                   _validObject;

		std::streambuf        *_buf;
		bool                   _deleteOnClose;

		bool                   _compression;
		bool                   _json;
		int                    _forceWriteVersion;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}

#endif
