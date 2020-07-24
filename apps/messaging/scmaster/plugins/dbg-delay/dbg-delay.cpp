/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * Author: Jan Becker                                                      *
 * Email: jabe@gempa.de                                                    *
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


#define SEISCOMP_COMPONENT dbg-delay
#include <seiscomp/logging/log.h>
#include <seiscomp/core/plugin.h>
#include <seiscomp/core/system.h>
#include <seiscomp/system/settings.h>

#include <seiscomp/broker/messageprocessor.h>

#include <cstdlib>
#include <unistd.h>


using namespace Seiscomp;
using namespace Seiscomp::System;


namespace {


class DebugDelay : public Messaging::Broker::MessageProcessor {
	public:
		DebugDelay() {
			setMode(Messages);
		}


	public:
		virtual bool init(const Config::Config &cfg, const std::string &configPrefix) override {
			ConfigSettingsLinker linker;
			linker.configPrefix = configPrefix;
			linker.proc().get(cfg);
			_settings.accept(linker);
			return linker;
		}


		virtual bool acceptConnection(Messaging::Broker::Client *,
		                              const KeyCStrValues, int,
		                              KeyValues &) override {
			return true;
		}


		virtual void dropConnection(Messaging::Broker::Client *) override {}


		virtual bool process(Messaging::Broker::Message *tmsg) override {
			int delay = _settings.minDelay;
			if ( _settings.randDelay ) {
				delay += int(int64_t(rand()) * _settings.randDelay / RAND_MAX);
			}

			SEISCOMP_DEBUG("Delay message processing by %d ms", delay);
			usleep(delay * 1000);
			return true;
		}


		virtual bool close() override {
			return true;
		}


		virtual void getInfo(const Core::Time &, std::ostream &) override {}


	private:
		struct Settings {
			Settings() : minDelay(10), randDelay(10) {}

			int minDelay;  // Minimum delay in milliseconds
			int randDelay; // Random delay in milliseconds

			void accept(ConfigSettingsLinker &linker) {
				linker
				& ConfigSettingsLinker::cfg(minDelay, "minDelay")
				& ConfigSettingsLinker::cfg(randDelay, "randDelay");
			}
		};

		Settings _settings;
};


}


ADD_SC_PLUGIN("scmaster message delay debugging plugin", "gempa GmbH <seiscomp-dev@gempa.de>", 0, 1, 0)
REGISTER_BROKER_MESSAGE_PROCESSOR(DebugDelay, "dbg-delay");
