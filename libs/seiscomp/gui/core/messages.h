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



#ifndef SEISCOMP_GUI_MESSAGE_H
#define SEISCOMP_GUI_MESSAGE_H

#ifndef Q_MOC_RUN
#include <seiscomp/core/message.h>
#include <seiscomp/core/enumeration.h>
#endif
#include <seiscomp/gui/qt.h>


namespace Seiscomp {
namespace Gui {


MAKEENUM(
	Command,
	EVALUES(
		CM_UNDEFINED,
		CM_SHOW_ORIGIN,
		CM_SHOW_STREAMS,
		CM_SHOW_MAGNITUDE,
		CM_OBSERVE_LOCATION
	),
	ENAMES(
		"undefined",
		"show origin",
		"show streams",
		"show magnitude",
		"observe location"
	)
);


DEFINE_SMARTPOINTER(CommandMessage);

class SC_GUI_API CommandMessage : public Seiscomp::Core::Message {
	DECLARE_SC_CLASS(CommandMessage);
	DECLARE_SERIALIZATION;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	private:
		CommandMessage();
		CommandMessage(const std::string client, Command command);

	// ------------------------------------------------------------------
	//  Message interface
	// ------------------------------------------------------------------
	public:
		void setParameter(const std::string&);
		void setObject(Core::BaseObject*);

		const std::string& client() const;
		Command command() const;

		const std::string& parameter() const;
		Core::BaseObject* object() const;

		bool empty() const;

	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		std::string _client;
		Command _command;
		std::string _parameter;
		Core::BaseObjectPtr _object;

	DECLARE_SC_CLASSFACTORY_FRIEND(CommandMessage);

	friend class Application;
};


}
}


#endif
