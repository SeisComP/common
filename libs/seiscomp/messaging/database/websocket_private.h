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


#ifndef SEISCOMP_IO_DATABASE_WEBSOCKET_H
#define SEISCOMP_IO_DATABASE_WEBSOCKET_H


#include <seiscomp/io/database.h>
#include <seiscomp/wired/devices/socket.h>


namespace {


class WebsocketProxy : public Seiscomp::IO::DatabaseInterface,
                       virtual public Seiscomp::Core::InterruptibleObject {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		WebsocketProxy();

	// ----------------------------------------------------------------------
	//  DatabaseInterface interface
	// ----------------------------------------------------------------------
	public:
		bool connect(const char* connection) override;
		void disconnect() override;
		bool isConnected() const override;
		void start() override;
		void commit() override;
		void rollback() override;
		bool execute(const char* command) override;
		bool beginQuery(const char* query) override;
		void endQuery() override;
		OID lastInsertId(const char* table) override;
		uint64_t numberOfAffectedRows() override;
		bool fetchRow() override;
		int findColumn(const char* name) override;
		int getRowFieldCount() const override;
		const char *getRowFieldName(int index) override;
		const void *getRowField(int index) override;
		size_t getRowFieldSize(int index) override;

	protected:
		bool handleURIParameter(const std::string &name,
		                        const std::string &value) override;
		bool open() override;

	protected:
		void handleInterrupt(int) override;

	private:
		bool establishConnection() const;
		void teardownConnection();

	// ----------------------------------------------------------------------
	//  Private interface
	// ----------------------------------------------------------------------
	protected:
		bool validConnection() const {
			return _socket && _socket->isValid();
		}

		virtual void setDefaults();
		virtual Seiscomp::Wired::Socket *createSocket() const;

		enum Command {
			WS_START          = 1,
			WS_COMMIT         = 2,
			WS_ROLLBACK       = 3,
			WS_EXECUTE        = 4,
			WS_QUERY          = 5,
			WS_QUERY_END      = 6,
			WS_LAST_ID        = 7,
			WS_AFFECTED_ROWS  = 8,
			WS_FETCH          = 9
		};

		bool send(Command, const char *body = nullptr, int length = 0);
		bool sendAndReceiveStatus(Command, const char *body = nullptr, int length = 0);
		bool readFrame(Seiscomp::Wired::Websocket::Frame &frame);

	private:
		struct Field {
			std::string  name;
			char        *content; // This just stores a pointer into the
			                      // _row buffer.
			int          length;  // Length in bytes of the field content.
		};

		std::string                        _dbURL;
		bool                               _debug{false};

		mutable Seiscomp::Wired::SocketPtr _socket;
		mutable bool                       _disconnected{true};

		std::string                        _row;
		std::vector<Field>                 _fields;
};


class WebsocketProxySecure : public WebsocketProxy {
	protected:
		void setDefaults() override;
		Seiscomp::Wired::Socket *createSocket() const override;
};


}


#endif
