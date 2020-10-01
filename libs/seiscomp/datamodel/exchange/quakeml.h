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


#ifndef SEISCOMP_QML_XML_H
#define SEISCOMP_QML_XML_H

#include <string>

#include <seiscomp/datamodel/types.h>

#include <seiscomp/io/xml/exporter.h>
#include <seiscomp/io/xml/handler.h>


namespace Seiscomp {
namespace QML {

class TypeMapper {
	public:
		/**
		 * @brief Maps an QML event type string to the EventType enumeration.
		 *
		 * If the conversion fails, an ValueException is thrown.
		 *
		 * @param str The input string
		 * @return The mapped event type or an exception
		 */
		static DataModel::EventType EventTypeFromString(const std::string &str);

		/**
		 * @brief Maps an EventType to an QML event type string.
		 * @param type The EventType
		 * @return The QML event type string
		 */
		static std::string EventTypeToString(DataModel::EventType type);
};

class Exporter : public IO::XML::Exporter {
	public:
		Exporter();

	protected:
		virtual void collectNamespaces(Core::BaseObject *);
};

class RTExporter : public IO::XML::Exporter {
	public:
		RTExporter();

	protected:
		virtual void collectNamespaces(Core::BaseObject *);
};

struct Formatter {
	virtual void to(std::string& v) {}
	virtual void from(std::string& v) {}
};

template <typename T>
struct TypedClassHandler : IO::XML::TypedClassHandler<T> {
	void add(const char *property, const char *name, Formatter *format = nullptr,
	         IO::XML::ClassHandler::Type t = IO::XML::ClassHandler::Optional,
	         IO::XML::ClassHandler::Location l = IO::XML::ClassHandler::Element);
	void add(const char *property, Formatter *format = nullptr,
	         IO::XML::ClassHandler::Type t = IO::XML::ClassHandler::Optional,
	         IO::XML::ClassHandler::Location l = IO::XML::ClassHandler::Element);
	void addList(const char *properties,
	             IO::XML::ClassHandler::Type t = IO::XML::ClassHandler::Optional,
	             IO::XML::ClassHandler::Location l = IO::XML::ClassHandler::Element);
	void addPID();
	void addEmptyPID();
};

}
}


#endif
