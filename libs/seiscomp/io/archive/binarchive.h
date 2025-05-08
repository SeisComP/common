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


#ifndef SCARCHIVE_BIN_H
#define SCARCHIVE_BIN_H

#include <seiscomp/core/io.h>
#include <seiscomp/core.h>
#include <streambuf>

namespace Seiscomp {
namespace IO {



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** \brief An archive using binary streams
 */
class SC_SYSTEM_CORE_API BinaryArchive : public Seiscomp::Core::Archive {
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		//! Constructor
		BinaryArchive();

		//! Constructor with predefined buffer and mode
		BinaryArchive(std::streambuf* buf, bool isReading = true);

		//! Destructor
		~BinaryArchive();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		bool open(const char* file) override;
		bool open(std::streambuf*);

		bool create(const char* file) override;
		bool create(std::streambuf*);

		//! Implements derived virtual method
		virtual void close() override;


	// ----------------------------------------------------------------------
	//  Read methods
	// ----------------------------------------------------------------------
	public:
		//! Reads an integer
		virtual void read(std::int8_t& value) override;
		virtual void read(std::int16_t& value) override;
		virtual void read(std::int32_t& value) override;
		virtual void read(std::int64_t& value) override;
		//! Reads a float
		virtual void read(float& value) override;
		//! Reads a double
		virtual void read(double& value) override;

		virtual void read(std::vector<char>& value) override;
		virtual void read(std::vector<int8_t>& value) override;
		virtual void read(std::vector<int16_t>& value) override;
		virtual void read(std::vector<int32_t>& value) override;
		virtual void read(std::vector<int64_t>& value) override;
		virtual void read(std::vector<float>& value) override;
		virtual void read(std::vector<double>& value) override;
		virtual void read(std::vector<std::string>& value) override;
		virtual void read(std::vector<Core::Time>& value) override;

		//! Reads a complex float
		virtual void read(std::complex<float>& value) override;
		//! Reads a complex double
		virtual void read(std::complex<double>& value) override;
		//! Reads a boolean
		virtual void read(bool& value) override;

		//! Reads a vector of complex doubles
		virtual void read(std::vector<std::complex<double> >& value) override;

		//! Reads a string
		virtual void read(std::string& value) override;

		//! Reads a time
		virtual void read(Seiscomp::Core::Time& value) override;


	// ----------------------------------------------------------------------
	//  Write methods
	// ----------------------------------------------------------------------
	public:
		//! Writes an integer
		virtual void write(std::int8_t value) override;
		virtual void write(std::int16_t value) override;
		virtual void write(std::int32_t value) override;
		virtual void write(std::int64_t value) override;
		//! Writes a float
		virtual void write(float value) override;
		//! Writes a double
		virtual void write(double value) override;

		virtual void write(std::vector<char>& value) override;
		virtual void write(std::vector<int8_t>& value) override;
		virtual void write(std::vector<int16_t>& value) override;
		virtual void write(std::vector<int32_t>& value) override;
		virtual void write(std::vector<int64_t>& value) override;
		virtual void write(std::vector<float>& value) override;
		virtual void write(std::vector<double>& value) override;
		virtual void write(std::vector<std::string>& value) override;
		virtual void write(std::vector<Core::Time>& value) override;

		//! Writes a complex float
		virtual void write(std::complex<float>& value) override;
		//! Writes a complex double
		virtual void write(std::complex<double>& value) override;
		//! Writes a boolean
		virtual void write(bool value) override;

		//! Writes a vector of complex doubles
		virtual void write(std::vector<std::complex<double> >& value) override;

		//! Writes a string
		virtual void write(std::string& value) override;

		//! Writes a time
		virtual void write(Seiscomp::Core::Time& value) override;


	// ----------------------------------------------------------------------
	//  Protected interface
	// ----------------------------------------------------------------------
	protected:
		//! Implements derived virtual method
		bool locateObjectByName(const char* name, const char* targetClass, bool nullable) override;
		bool locateNextObjectByName(const char* name, const char* targetClass) override;
		void locateNullObjectByName(const char* name, const char* targetClass, bool first) override;

		void readSequence() override;
		void writeSequence(int size) override;

		//! Implements derived virtual method
		std::string determineClassName() override;

		//! Implements derived virtual method
		virtual void setClassName(const char* className) override;

		//! Implements derived virtual method
		void serialize(RootType* object) override;

		//! Implements derived virtual method
		void serialize(SerializeDispatcher&) override;

		int writeBytes(const void*, int);


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		int classId(const std::string& classname);

		template <typename T>
		void readInt(T &value);

		template <typename T>
		void readIntVector(std::vector<T> &value);

		template <typename T>
		void writeVector(std::vector<T> &value);


	protected:
		std::streambuf* _buf;

	private:
		bool _deleteOnClose;
		bool _nullable;
		bool _usedObject;
		std::string _classname;

		int _sequenceSize;

		typedef std::vector<std::string> ClassList;
		ClassList _classes;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class SC_SYSTEM_CORE_API VBinaryArchive : public BinaryArchive {
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		//! Constructor
		VBinaryArchive(int forceWriteVersion = -1);

		//! Constructor with predefined buffer and mode
		VBinaryArchive(std::streambuf* buf, bool isReading = true,
		               int forceWriteVersion = -1);


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		void setWriteVersion(int version);

		bool open(const char* file) override;
		bool open(std::streambuf*);

		bool create(const char* file) override;
		bool create(std::streambuf*);

		void close() override;

		const char *errorMsg() const;


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		void writeHeader();
		bool readHeader();


	private:
		int         _forceWriteVersion;
		std::string _error;
};


}
}

#endif
