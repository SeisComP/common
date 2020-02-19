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



#ifndef SEISCOMP_DATAMODEL_MESSAGES_H__
#define SEISCOMP_DATAMODEL_MESSAGES_H__


#include <seiscomp/core/genericmessage.h>
#include <seiscomp/datamodel/eventparameters.h>
#include <seiscomp/datamodel/origin.h>
#include <seiscomp/datamodel/creationinfo.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(ConfigSyncMessage);

class SC_SYSTEM_CORE_API ConfigSyncMessage : public Seiscomp::Core::Message {
	DECLARE_SC_CLASS(ConfigSyncMessage);
	DECLARE_SERIALIZATION;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		ConfigSyncMessage();
		ConfigSyncMessage(bool finished);


	// ------------------------------------------------------------------
	//  Message interface
	// ------------------------------------------------------------------
	public:
		bool empty() const;


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		void setCreationInfo(const OPT(CreationInfo)& creationInfo);
		CreationInfo &creationInfo();
		const CreationInfo &creationInfo() const;


	// ------------------------------------------------------------------
	//  Public members
	// ------------------------------------------------------------------
	public:
		bool isFinished;

	private:
		OPT(CreationInfo) _creationInfo;
};


DEFINE_SMARTPOINTER(InventorySyncMessage);

class SC_SYSTEM_CORE_API InventorySyncMessage : public Seiscomp::Core::Message {
	DECLARE_SC_CLASS(InventorySyncMessage);
	DECLARE_SERIALIZATION;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		InventorySyncMessage();
		InventorySyncMessage(bool finished);


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		void setCreationInfo(const OPT(CreationInfo)& creationInfo);
		CreationInfo &creationInfo();
		const CreationInfo &creationInfo() const;


	// ------------------------------------------------------------------
	//  Message interface
	// ------------------------------------------------------------------
	public:
		bool empty() const;


	// ------------------------------------------------------------------
	//  Public members
	// ------------------------------------------------------------------
	public:
		bool isFinished;

	private:
		OPT(CreationInfo) _creationInfo;
};


DEFINE_SMARTPOINTER(ArtificialOriginMessage);

class SC_SYSTEM_CORE_API ArtificialOriginMessage : public Seiscomp::Core::Message {
	DECLARE_SC_CLASS(ArtificialOriginMessage);
	DECLARE_SERIALIZATION;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		ArtificialOriginMessage();

	public:
		ArtificialOriginMessage(DataModel::Origin *origin);

		
	// ------------------------------------------------------------------
	//  Message interface
	// ------------------------------------------------------------------
	public:
		DataModel::Origin *origin() const;
		void setOrigin(DataModel::Origin *origin);
		
		virtual bool empty() const;

	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		DataModel::OriginPtr _origin;
		
		DECLARE_SC_CLASSFACTORY_FRIEND(ArtificialOriginMessage);
};


DEFINE_SMARTPOINTER(ArtificialEventParametersMessage);

class SC_SYSTEM_CORE_API ArtificialEventParametersMessage : public Seiscomp::Core::Message {
	DECLARE_SC_CLASS(ArtificialEventParametersMessage);
	DECLARE_SERIALIZATION;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		ArtificialEventParametersMessage();

	public:
		ArtificialEventParametersMessage(DataModel::EventParameters *eventParameters);


	// ------------------------------------------------------------------
	//  Message interface
	// ------------------------------------------------------------------
	public:
		DataModel::EventParameters *eventParameters() const;
		void setEventParameters(DataModel::EventParameters *eventParameters);

		virtual bool empty() const;

	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		DataModel::EventParametersPtr _eventParameters;

		DECLARE_SC_CLASSFACTORY_FRIEND(ArtificialEventParametersMessage);
};


}
}


#endif
