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


#define SEISCOMP_COMPONENT TEST_CORE
#define SEISCOMP_TEST_MODULE SeisComP

#include <seiscomp/config/config.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/system/application.h>
#include <seiscomp/system/environment.h>
#include <seiscomp/unittest/unittests.h>

#include <cstdlib>


using namespace Seiscomp::Config;
namespace bu = boost::unit_test;


BOOST_AUTO_TEST_SUITE(seiscomp_core_config)


class SimpleApp : public Seiscomp::System::Application {
	public:
		SimpleApp(int argc, char** argv)
		: Seiscomp::System::Application(argc, argv) {}

	public:
		bool run() override {
			return true;
		}
};


//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(read_chain) {
	Config config;

	std::string file = "./data/config.cfg";
	bool test = config.readConfig(file);
	BOOST_CHECK_EQUAL(test, true);

	std::string checkText = config.visitedFilesToString();
	BOOST_CHECK_EQUAL(checkText, "./data/config.cfg\n");

	bu::unit_test_log.set_threshold_level(bu::log_messages);

	/*************************include new file*******************************/

	file = "./data/config2.cfg";
	test = config.readConfig(file);
	BOOST_CHECK(test == true);

	checkText = config.visitedFilesToString();
	BOOST_CHECK_EQUAL(checkText, "./data/config.cfg\n./data/config2.cfg\n");

	/*************************include new file*******************************/

	file = "./data/config3.cfg";
	test = config.readConfig(file);
	BOOST_CHECK(test == true);

	checkText = config.visitedFilesToString();
	BOOST_CHECK_EQUAL(checkText, "./data/config.cfg\n./data/config2.cfg\n./data/config3.cfg\n");
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(setInt_getInt) {
	std::string file = "./data/config.cfg";
	Config config;
	config.readConfig(file);

	int getNum = config.getInt("logging.level");
	bool check = config.getInt(getNum, "logging.level");
	BOOST_CHECK(getNum == 2);
	BOOST_CHECK_EQUAL(check, true);
	check = config.setInt("logging.level", 4);
	BOOST_CHECK_EQUAL(check, true);

	getNum = config.getInt("logging.level");
	check = config.getInt(getNum, "logging.level");
	BOOST_CHECK(getNum == 4);
	BOOST_CHECK_EQUAL(check, true);
	check = config.setInt("logging.level", 3);
	BOOST_CHECK_EQUAL(check, true);
	std::vector<int> changeInts = config.getInts("logging.level");

	getNum = config.getInt("logging.file.rotator.timeSpan");
	check = config.getInt(getNum, "logging.file.rotator.timeSpan");
	BOOST_CHECK(getNum == 86400);
	BOOST_CHECK_EQUAL(check, true);
	check = config.setInt("logging.file.rotator.timeSpan", 86401);
	BOOST_CHECK_EQUAL(check, true);

	getNum = config.getInt("logging.file.rotator.archiveSize");
	check = config.getInt(getNum, "logging.file.rotator.archiveSize");
	BOOST_CHECK(getNum == 7);
	BOOST_CHECK_EQUAL(check, true);
	check = config.setInt("logging.file.rotator.archiveSize", 8);
	BOOST_CHECK_EQUAL(check, true);

	/*************************include new file*******************************/

	file = "./data/config3.cfg";
	config.readConfig(file);

	BOOST_CHECK_THROW(config.getInt("useConfiguredStreams"), std::exception);

	BOOST_CHECK_THROW(config.getInt("connection.subscriptions"),std::exception);

	std::vector<int> vec = config.getInts("plugins.default.archive.interval");
	BOOST_CHECK_EQUAL(vec[0], 3600);

	vec = config.getInts("plugins.default.buffer");
	BOOST_CHECK_EQUAL(vec[0], 4000);

	bool proof = config.setInts("plugins.default.buffer",changeInts);
	BOOST_CHECK(proof == true);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(getBool_setBool) {
	std::string file = "./data/config.cfg";
	Config config;
	config.readConfig(file);

	std::string element = "logging.file.rotator";
	bool takeIt = config.getBool(element);
	BOOST_CHECK(takeIt == true);
	bool proof = config.getBool(takeIt, element);
	BOOST_CHECK_EQUAL(proof, true);

	proof = config.setBool(element, false);
	BOOST_CHECK_EQUAL(proof, true);
	std::vector<bool> changeBool = config.getBools(element);

	element = "connection.encoding";
	BOOST_CHECK_THROW(config.getBool(element),std::exception);

	element = "recordstream.service";
	BOOST_CHECK_THROW(config.getBool(element), std::exception);

	/*************************include new file*******************************/

	file = "./data/config3.cfg";
	config.readConfig(file);
	element = "use3Components";
	std::vector<bool> vec = config.getBools(element);
	BOOST_CHECK_EQUAL(vec[0], false);

	element ="useConfiguredStreams";
	vec = config.getBools(element);
	BOOST_CHECK_EQUAL(vec[0], true);

	proof = config.setBools(element, changeBool);
	BOOST_CHECK(proof == true);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(getDouble_setDouble) {
	std::string file = "./data/config.cfg";
	Config config;
	config.readConfig(file);

	double getNum = config.getDouble("logging.level");
	BOOST_CHECK_EQUAL(2.0, getNum);
	bool check = config.getDouble(getNum, "logging.level");
	BOOST_CHECK_EQUAL(check, true);

	getNum = config.getDouble("logging.file.rotator.timeSpan");
	BOOST_CHECK_EQUAL(86400.0, getNum);
	check = config.getDouble(getNum, "logging.file.rotator.timeSpan");
	BOOST_CHECK_EQUAL(check, true);
	std::vector<double> changeDouble = config.getDoubles("logging.file.rotator.timeSpan");

	getNum = config.getDouble("logging.file.rotator.archiveSize");
	BOOST_CHECK_EQUAL(7.0, getNum);
	check = config.getDouble(getNum, "logging.file.rotator.archiveSize");
	BOOST_CHECK_EQUAL(check, true);

	check = config.setDouble("logging.level", 5.5);
	BOOST_CHECK_EQUAL(check, true);

	BOOST_CHECK_THROW(config.getDouble("connection.encoding"),std::exception);

	BOOST_CHECK_THROW(config.getDouble("logging.file.rotator"),std::exception);

	/*************************include new file*******************************/

	file = "./data/config3.cfg";
	config.readConfig(file);

	std::vector<double> vec = config.getDoubles("plugins.default.archive.buffer");
	BOOST_CHECK_EQUAL(vec[0], 3600.0);

	vec = config.getDoubles("plugins.default.report.buffer");
	BOOST_CHECK_EQUAL(vec[0], 600.0);

	bool proof = config.setDoubles("plugins.default.report.buffer", changeDouble);
	BOOST_CHECK(proof == true);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(getString_setString) {
	std::string file = "./data/config.cfg";
	Config config;
	config.readConfig(file);

	std::string getText = config.getString("connection.server");
	std::string checkText = "localhost";
	BOOST_CHECK_EQUAL(getText,checkText);

	bool proof = config.getString(getText, "connection.server");
	BOOST_CHECK(proof == true);
	proof = config.setString("connection.server","Localhost test");
	BOOST_CHECK(proof == true);

	getText = config.getString("connection.encoding");
	checkText = "binary";
	BOOST_CHECK_EQUAL(getText,checkText);

	proof = config.getString(getText, "connection.encoding");
	BOOST_CHECK(proof == true);
	proof = config.setString("connection.encoding","Binary");

	getText = config.getString("recordstream.service");
	checkText = "slink";
	BOOST_CHECK_EQUAL(getText,checkText);

	proof = config.getString(getText, "recordstream.service");
	BOOST_CHECK(proof == true);
	proof = config.setString("recordstream.service","Slink");

	getText = config.getString("connection.timeout");
	BOOST_CHECK_EQUAL("3", getText);
	proof = config.getString(getText,"connection.timeout");
	BOOST_CHECK(proof == true);

	BOOST_CHECK_NO_THROW(config.getString("logging.file.rotator.archiveSize"));

	std::vector<std::string> vec = config.getStrings("logging.file.rotator.archiveSize");
	BOOST_CHECK(vec.size() == 1);
	BOOST_CHECK(vec.front() == "7" && vec.back() == "7");

	vec = config.getStrings("connection.server");
	BOOST_CHECK_EQUAL(vec.size(), 1);
	BOOST_CHECK_EQUAL(vec.front(), "Localhost test");

	vec = config.getStrings("picker.filters");
	BOOST_CHECK_EQUAL(vec.size(), 10);
	BOOST_CHECK_EQUAL(vec.front(),"BP 0.1 - 1 Hz  3rd order;BW(3,0.1,1)");
	BOOST_CHECK_EQUAL(vec.back(),"BP 0.7 - 2 Hz + STA/LTA(1,50);RMHP(10)->ITAPER(30)->BW(3,0.7,2)->STALTA(1,50)");
	BOOST_CHECK_EQUAL(vec[3], "@BP 0.7 - 2 Hz  3rd order;BW(3,0.7,2)");

	/*************************include new file*******************************/

	file = "./data/config3.cfg";
	config.readConfig(file);

	bool error = false;
	std::vector<std::string> vec2 = config.getStrings("plugins",&error);
	BOOST_CHECK_EQUAL(vec.size(), 10);

	proof = config.setStrings("plugins", vec);
	BOOST_CHECK(proof == true);

	error = true;
	vec2 = config.getStrings("plugins.QcLatency.realTimeOnly", &error);
	BOOST_CHECK_EQUAL(vec2[0], "True");
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(tools) {
	std::string file = "./data/config3.cfg";
	Config config;
	config.readConfig(file);

	std::vector<std::string> vec = config.names();
	BOOST_CHECK_EQUAL(vec.size(), 26);
	for ( int i = 0; i < (int)vec.size(); i++ ) {
		BOOST_CHECK(vec[i].empty() == false);
	}

	std::string symbol = config.symbolsToString();
	BOOST_CHECK_EQUAL(symbol.size(), 3954);

	/*************************include new file*******************************/

	file = "./data/config.cfg";
	BOOST_CHECK_NO_THROW(config.readConfig(file));

	bool proof = config.eval("connection.encoding",vec);
	BOOST_CHECK_EQUAL(proof, true);

	proof = config.Eval("recordstream.source",vec);
	BOOST_CHECK_EQUAL(proof, true);

	std::stringstream ss;
	Symbol *sym = config.symbolTable()->get("connection.encoding");
	BOOST_CHECK_NO_THROW(config.writeValues(ss,sym));
	BOOST_CHECK_NO_THROW(config.trackVariables(true));
	BOOST_CHECK_NO_THROW(config.writeSymbol(ss,sym));

	size_t getNum = config.getInt("logging.level");
	proof = config.getBool("logging.file.rotator");
	getNum = config.getInt("logging.file.rotator.timeSpan");

	Variables var = config.getVariables();
	getNum = var.size();
	BOOST_CHECK_EQUAL(getNum,3);
	for ( size_t i = 0; i < getNum; i++ ) {
		BOOST_CHECK_EQUAL(var.empty(), false);
	}

	/*************************include new file*******************************/

	file = "./data/config2.cfg";
	BOOST_CHECK_EQUAL(config.readConfig(file), true);

	auto get = config.getString("holiday.Spain");
	BOOST_CHECK_EQUAL(get, "dance");
	auto vget = config.getStrings("holiday.Spain");
	BOOST_CHECK_EQUAL(vget[0], "dance");
	BOOST_CHECK_EQUAL(vget[1], "swim");
	proof = config.getBool("holiday.Spain.dance");
	BOOST_CHECK_EQUAL(proof, true);
	proof = config.getBool("holiday.Spain.swim.warm");
	BOOST_CHECK_EQUAL(proof, true);
	proof = config.getBool("holiday.Spain.swim.cold");
	BOOST_CHECK_EQUAL(proof, false);

	vget = config.getStrings("earth.continent");
	BOOST_CHECK_EQUAL(vget[0], "Asia");
	BOOST_CHECK_EQUAL(vget[1], "Europe");
	BOOST_CHECK_EQUAL(vget[2], "Africa");
	get = config.getString("earth.Asia.China");
	BOOST_CHECK_EQUAL(get, "Peking");
	get = config.getString("earth.Europe.Germany");
	BOOST_CHECK_EQUAL(get, "Berlin");
	get = config.getString("earth.Europe.Berlin.language");
	BOOST_CHECK_EQUAL(get, "German");
	get = config.getString("earth.Africa.Benin");
	BOOST_CHECK_EQUAL(get, "PortoNovo");
	vget = config.getStrings("earth.Europe.Madrid.language");
	BOOST_CHECK_EQUAL(vget[0], "Spanish");
	BOOST_CHECK_EQUAL(vget[1], "English");

	vget = config.getStrings("escaped");
	BOOST_CHECK_EQUAL(vget[0], "ABC\\D,EFGH");

	vget = config.getStrings("escaped2");
	BOOST_CHECK_EQUAL(vget[0], "ABC\"D");
	BOOST_CHECK_EQUAL(vget[1], "EFGH");
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(paths) {
	setenv("SEISCOMP_ROOT", "./data/seiscomp", 1);
	setenv("SEISCOMP_LOCAL_CONFIG", "./data/.seiscomp", 1);

	char *params[] = { strdup("testapp") };

	SimpleApp app(1, params);
	app.setLoggingToStdErr(true);
	app();

	auto env = Seiscomp::Environment::Instance();

	BOOST_CHECK_EQUAL(app.configGetString("path"), "@ROOTDIR@");
	BOOST_CHECK_EQUAL(app.configGetPath("path"), env->absolutePath(env->installDir()));

	auto paths = app.configGetStrings("paths");
	BOOST_CHECK_EQUAL(paths[0], "@ROOTDIR@");
	BOOST_CHECK_EQUAL(paths[1], "@DATADIR@");
	BOOST_CHECK_EQUAL(paths[2], "@LOGDIR@");
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(findSymbols) {
	std::string file = "./data/config3.cfg";
	Config cfg;
	BOOST_REQUIRE(cfg.readConfig(file));

	auto pluginNames = cfg.findSymbols("plugins.");
	BOOST_CHECK_EQUAL(pluginNames.size(), 5);
	BOOST_CHECK_EQUAL(pluginNames[0], "plugins.default");
	BOOST_CHECK_EQUAL(pluginNames[1], "plugins.QcLatency");
	BOOST_CHECK_EQUAL(pluginNames[2], "plugins.QcDelay");
	BOOST_CHECK_EQUAL(pluginNames[3], "plugins.QcAvailability");
	BOOST_CHECK_EQUAL(pluginNames[4], "plugins.QcOutage");

	pluginNames = cfg.findSymbols("plugins.", "realTimeOnly", true);
	BOOST_CHECK_EQUAL(pluginNames.size(), 4);
	BOOST_CHECK_EQUAL(pluginNames[0], "plugins.QcLatency");
	BOOST_CHECK_EQUAL(pluginNames[1], "plugins.QcDelay");
	BOOST_CHECK_EQUAL(pluginNames[2], "plugins.QcAvailability");
	BOOST_CHECK_EQUAL(pluginNames[3], "plugins.QcOutage");

	pluginNames = cfg.findSymbols("plugins.", "realTimeOnly", false);
	BOOST_CHECK_EQUAL(pluginNames.size(), 2);
	BOOST_CHECK_EQUAL(pluginNames[0], "plugins.QcLatency");
	BOOST_CHECK_EQUAL(pluginNames[1], "plugins.QcDelay");

	pluginNames = cfg.findSymbols("plugins.", "DOES_NOT_EXIST", true);
	BOOST_CHECK_EQUAL(pluginNames.size(), 5);
	BOOST_CHECK_EQUAL(pluginNames[0], "plugins.default");
	BOOST_CHECK_EQUAL(pluginNames[1], "plugins.QcLatency");
	BOOST_CHECK_EQUAL(pluginNames[2], "plugins.QcDelay");
	BOOST_CHECK_EQUAL(pluginNames[3], "plugins.QcAvailability");
	BOOST_CHECK_EQUAL(pluginNames[4], "plugins.QcOutage");

	pluginNames = cfg.findSymbols("plugins.", "DOES_NOT_EXIST", false);
	BOOST_CHECK_EQUAL(pluginNames.size(), 0);
}
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


BOOST_AUTO_TEST_SUITE_END()
