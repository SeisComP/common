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


#ifndef SCARCHIVE_XML_H
#define SCARCHIVE_XML_H

#include <seiscomp/core/io.h>
#include <seiscomp/core.h>

namespace Seiscomp {
namespace IO {



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** \brief An archive using XML streams
 */
class SC_SYSTEM_CORE_API XMLArchive : public Seiscomp::Core::Archive {
	public:
		enum CompressionMethod {
			ZIP,
			GZIP
		};

	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		//! Constructor
		XMLArchive();

		//! Constructor with predefined buffer and mode
		XMLArchive(std::streambuf* buf, bool isReading = true,
		           int forceWriteVersion = -1);

		//! Destructor
		~XMLArchive();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		//! Opens an archive reading from a streambuf
		bool open(std::streambuf*);

		//! Implements derived virtual method
		virtual bool open(const char* filename);

		//! Creates an archive writing to a streambuf
		bool create(std::streambuf* buf, bool writeVersion = true, bool headerNode = true);

		//! Implements derived virtual method
		virtual bool create(const char* filename, bool writeVersion = true, bool headerNode = true);

		//! Implements derived virtual method
		virtual void close();

		//! Sets the root tagname to define the document entry.
		//! The default tagname is "seiscomp"
		void setRootName(const std::string &name);

		/**
		 * Enables/Disabled formatted XML output meaning inserting
		 * formatting spaces.
		 * @param enable The state of this flag
		 */
		void setFormattedOutput(bool enable);

		/**
		 * Enables/Disabled zip compression of XML output
		 * @param enable The state of this flag
		 */
		void setCompression(bool enable);

		/**
		 * Sets the compression method if compression is enabled
		 * @param method The method to be used
		 */
		void setCompressionMethod(CompressionMethod method);

		//! Returns the used namesspace. If no namespace has been used,
		//! an empty string will be returned
		const std::string& rootNamespace() const;

		//! Returns the used namesspace uri. If no namespace uri has been used,
		//! an empty string will be returned
		const std::string& rootNamespaceUri() const;

		//! Sets the root namespace used when creating new documents
		void setRootNamespace(const std::string& name, const std::string& uri);
		

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
		bool create(bool writeVersion, bool headerNode);

		void addChild(const char* name, const char* type) const;
		void* addRootNode(const char* name) const;
		void writeAttrib(const std::string& value);


	protected:
		mutable void* _document;
		mutable void* _current;
		mutable void* _objectLocation;
		mutable std::string _property;
		mutable std::string _attribName;

		int         _forceWriteVersion;
		std::string _rootTag;

		std::streambuf* _buf;
		bool _deleteOnClose;

		bool _formattedOutput;
		bool _compression;
		CompressionMethod _compressionMethod;

		std::pair<std::string, std::string> _namespace;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}

#endif
