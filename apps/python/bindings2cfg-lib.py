############################################################################
# Copyright (C) gempa GmbH                                                 #
# All rights reserved.                                                     #
# Contact: gempa GmbH (seiscomp-dev@gempa.de)                              #
#                                                                          #
# GNU Affero General Public License Usage                                  #
# This file may be used under the terms of the GNU Affero                  #
# Public License version 3.0 as published by the Free Software Foundation  #
# and appearing in the file LICENSE included in the packaging of this      #
# file. Please review the following information to ensure the GNU Affero   #
# Public License version 3.0 requirements will be met:                     #
# https://www.gnu.org/licenses/agpl-3.0.html.                              #
#                                                                          #
# Other Usage                                                              #
# Alternatively, this file may be used in accordance with the terms and    #
# conditions contained in a signed written agreement between you and       #
# gempa GmbH.                                                              #
############################################################################

import os, time, sys
import seiscomp.core, seiscomp.client, seiscomp.datamodel
import seiscomp.io, seiscomp.system


def collectParams(container):
    params = {}
    for i in range(container.groupCount()):
        params.update(collectParams(container.group(i)))
    for i in range(container.structureCount()):
        params.update(collectParams(container.structure(i)))
    for i in range(container.parameterCount()):
        p = container.parameter(i)
        if p.symbol.stage == seiscomp.system.Environment.CS_UNDEFINED:
            continue
        params[p.variableName] = ",".join(p.symbol.values)

    return params


def collect(idset, paramSetID):
    paramSet = seiscomp.datamodel.ParameterSet.Find(paramSetID)
    if not paramSet:
        return
    idset[paramSet.publicID()] = 1
    if not paramSet.baseID():
        return
    collect(idset, paramSet.baseID())


def sync(paramSet, params):
    obsoleteParams = []
    seenParams = {}
    i = 0
    while i < paramSet.parameterCount():
        p = paramSet.parameter(i)
        if p.name() in params:
            if p.name() in seenParams:
                # Multiple parameter definitions with same name
                sys.stderr.write(
                    f"- {p.publicID()}:{p.name()} / duplicate parameter name\n"
                )
                p.detach()
                continue
            seenParams[p.name()] = 1
            val = params[p.name()]
            if val != p.value():
                p.setValue(val)
                p.update()
        else:
            obsoleteParams.append(p)
        i = i + 1

    for p in obsoleteParams:
        p.detach()

    for key, val in list(params.items()):
        if key in seenParams:
            continue
        p = seiscomp.datamodel.Parameter.Create()
        p.setName(key)
        p.setValue(val)
        paramSet.add(p)


class ConfigDBUpdater(seiscomp.client.Application):
    def __init__(self, argc, argv):
        seiscomp.client.Application.__init__(self, argc, argv)
        self.setLoggingToStdErr(True)
        self.setMessagingEnabled(True)
        self.setDatabaseEnabled(True, True)
        self.setAutoApplyNotifierEnabled(False)
        self.setInterpretNotifierEnabled(False)
        self.setMessagingUsername("_sccfgupd_")
        self.setLoadConfigModuleEnabled(True)
        # Load all configuration modules
        self.setConfigModuleName("")
        self.setPrimaryMessagingGroup(seiscomp.client.Protocol.LISTENER_GROUP)

        self._outputFile = None
        self._createNotifier = False
        self._keyDir = None

    def createCommandLineDescription(self):
        self.commandline().addGroup("Input")
        self.commandline().addStringOption(
            "Input",
            "key-dir",
            "Overrides the location of the default key directory ($SEISCOMP_ROOT/etc/key)",
        )
        self.commandline().addGroup("Output")
        self.commandline().addStringOption(
            "Output", "output,o", "If given, an output XML file is generated"
        )
        self.commandline().addOption(
            "Output", "create-notifier", "If given then a notifier message containing all notifiers "
            "will be written to the output XML. This option only applies "
            "if an output file is given. Notifier creation either requires "
            "and input database and an input config XML as reference."
        )

    def validateParameters(self):
        if not seiscomp.client.Application.validateParameters(self):
            return False

        try:
            self._outputFile = self.commandline().optionString("output")
            self._createNotifier = self.commandline().hasOption("create-notifier")
            # Switch to offline mode
            self.setMessagingEnabled(False)
            self.setDatabaseEnabled(False, False)
            if self._createNotifier:
                if self.isConfigDatabaseEnabled() == True:
                    self.setDatabaseEnabled(True, False);
            else:
                self.setLoadConfigModuleEnabled(False)
        except:
            pass

        try:
            self._keyDir = self.commandline().optionString("key-dir")
        except:
            pass

        return True

    def init(self):
        if not seiscomp.client.Application.init(self):
            return False

        # Initialize the basic directories
        filebase = seiscomp.system.Environment.Instance().installDir()
        descdir = os.path.join(filebase, "etc", "descriptions")

        # Load definitions of the configuration schema
        defs = seiscomp.system.SchemaDefinitions()
        if not defs.load(descdir):
            print("Error: could not read descriptions", file=sys.stderr)
            return False

        if defs.moduleCount() == 0:
            print("Warning: no modules defined, nothing to do", file=sys.stderr)
            return False

        # Create a model from the schema and read its configuration including
        # all bindings.
        model = seiscomp.system.Model()
        if self._keyDir:
            model.keyDirOverride = self._keyDir
        model.create(defs)
        model.readConfig()

        # Find all binding mods for trunk. Bindings of modules where standalone
        # is set to true are ignored. They are supposed to handle their bindings
        # on their own.
        self.bindingMods = []
        for i in range(defs.moduleCount()):
            mod = defs.module(i)
            # Ignore stand alone modules (eg seedlink, slarchive, ...) as they
            # are not using the trunk libraries and don't need database
            # configurations
            if mod.isStandalone():
                continue

            self.bindingMods.append(mod.name)

        if len(self.bindingMods) == 0:
            print("Warning: no usable modules found, nothing to do", file=sys.stderr)
            return False

        self.stationSetups = {}

        # Read bindings
        for m in self.bindingMods:
            mod = model.module(m)
            if not mod:
                print(f"Warning: module {m} not assigned", file=sys.stderr)
                continue
            if len(mod.bindings) == 0:
                continue

            if len(m) > 20:
                print(
                    f"Error: rejecting module {m} - name is longer than 20 characters",
                    file=sys.stderr,
                )
                return False

            # Rename global to default for being compatible with older
            # releases
            if m == "global":
                m = "default"

            print(f"+ {m}", file=sys.stderr)

            for staid in list(mod.bindings.keys()):
                binding = mod.getBinding(staid)
                if not binding:
                    continue
                # sys.stderr.write("  + %s.%s\n" % (staid.networkCode, staid.stationCode))
                params = {}
                for i in range(binding.sectionCount()):
                    params.update(collectParams(binding.section(i)))
                key = (staid.networkCode, staid.stationCode)
                if not key in self.stationSetups:
                    self.stationSetups[key] = {}
                self.stationSetups[key][m] = params
            print(
                f"  + read {len(list(mod.bindings.keys()))} stations", file=sys.stderr
            )

        return True

    def printUsage(self):
        print(
            """Usage:
  bindings2cfg [options]

Dump global and module bindings configurations"""
        )

        seiscomp.client.Application.printUsage(self)

        print(
            """Examples:
Write bindings configuration from key directory to a configuration XML file:
  bindings2cfg --key-dir ./etc/key -o config.xml

Synchronize bindings configuration from key directory to a processing system
  bindings2cfg --key-dir ./etc/key -H proc
"""
        )

        return True

    def send(self, *args):
        """
        A simple wrapper that sends a message and tries to resend it in case of
        an error.
        """
        while not self.connection().send(*args):
            print("Warning: sending failed, retrying", file=sys.stderr)
            time.sleep(1)

    def run(self):
        """
        Reimplements the main loop of the application. This methods collects
        all bindings and updates the database. It searches for already existing
        objects and updates them or creates new objects. Objects that is didn't
        touched are removed. This tool is the only one that should writes the
        configuration into the database and thus manages the content.
        """
        config = seiscomp.client.ConfigDB.Instance().config()
        if config is None:
            config = seiscomp.datamodel.Config()

        configMod = None
        obsoleteConfigMods = []

        if self._outputFile is None or self._createNotifier == True:
            moduleName = self.name()
            seiscomp.datamodel.Notifier.Enable()
        else:
            moduleName = "trunk"

        configID = f"Config/{moduleName}"

        for i in range(config.configModuleCount()):
            if config.configModule(i).publicID() != configID:
                obsoleteConfigMods.append(config.configModule(i))
            else:
                configMod = config.configModule(i)

        # Remove obsolete config modules
        for cm in obsoleteConfigMods:
            print(f"- {cm.name()} / obsolete module configuration", file=sys.stderr)
            ps = seiscomp.datamodel.ParameterSet.Find(cm.parameterSetID())
            if not ps is None:
                ps.detach()
            cm.detach()
        del obsoleteConfigMods

        if not configMod:
            configMod = seiscomp.datamodel.ConfigModule.Find(configID)
            if configMod is None:
                configMod = seiscomp.datamodel.ConfigModule.Create(configID)
                config.add(configMod)
            else:
                if configMod.name() != moduleName:
                    configMod.update()
                if not configMod.enabled():
                    configMod.update()

            configMod.setName(moduleName)
            configMod.setEnabled(True)
        else:
            if configMod.name() != moduleName:
                configMod.setName(moduleName)
                configMod.update()
            paramSet = seiscomp.datamodel.ParameterSet.Find(configMod.parameterSetID())
            if configMod.parameterSetID():
                configMod.setParameterSetID("")
                configMod.update()

            if not paramSet is None:
                paramSet.detach()

        stationConfigs = {}
        obsoleteStationConfigs = []

        for i in range(configMod.configStationCount()):
            cs = configMod.configStation(i)
            if (cs.networkCode(), cs.stationCode()) in self.stationSetups:
                stationConfigs[(cs.networkCode(), cs.stationCode())] = cs
            else:
                obsoleteStationConfigs.append(cs)

        for cs in obsoleteStationConfigs:
            print(
                f"- {configMod.name()}/{cs.networkCode()}/{cs.stationCode()} / obsolete "
                "station configuration",
                file=sys.stderr,
            )
            cs.detach()
        del obsoleteStationConfigs

        for staid, setups in list(self.stationSetups.items()):
            try:
                cs = stationConfigs[staid]
            except:
                cs = seiscomp.datamodel.ConfigStation.Find(
                    f"Config/{configMod.name()}/{staid[0]}/{staid[1]}"
                )
                if not cs:
                    cs = seiscomp.datamodel.ConfigStation.Create(
                        f"Config/{configMod.name()}/{staid[0]}/{staid[1]}"
                    )
                    configMod.add(cs)
                cs.setNetworkCode(staid[0])
                cs.setStationCode(staid[1])
                cs.setEnabled(True)

                ci = seiscomp.datamodel.CreationInfo()
                ci.setCreationTime(seiscomp.core.Time.GMT())
                ci.setAgencyID(self.agencyID())
                ci.setAuthor(self.name())
                cs.setCreationInfo(ci)

            stationSetups = {}
            obsoleteSetups = []
            for i in range(cs.setupCount()):
                setup = cs.setup(i)
                if setup.name() in setups:
                    stationSetups[setup.name()] = setup
                else:
                    obsoleteSetups.append(setup)

            for s in obsoleteSetups:
                print(
                    f"- {configMod.name()}/{cs.networkCode()}/{cs.stationCode()}/{setup.name()} "
                    "/ obsolete station setup",
                    file=sys.stderr,
                )
                ps = seiscomp.datamodel.ParameterSet.Find(s.parameterSetID())
                if ps:
                    ps.detach()
                s.detach()
            del obsoleteSetups

            newParamSets = {}
            globalSet = ""
            for mod, params in list(setups.items()):
                try:
                    setup = stationSetups[mod]
                except:
                    setup = seiscomp.datamodel.Setup()
                    setup.setName(mod)
                    setup.setEnabled(True)
                    cs.add(setup)

                paramSet = seiscomp.datamodel.ParameterSet.Find(setup.parameterSetID())
                if not paramSet:
                    paramSet = seiscomp.datamodel.ParameterSet.Find(
                        "ParameterSet/%s/Station/%s/%s/%s"
                        % (
                            configMod.name(),
                            cs.networkCode(),
                            cs.stationCode(),
                            setup.name(),
                        )
                    )
                    if not paramSet:
                        paramSet = seiscomp.datamodel.ParameterSet.Create(
                            "ParameterSet/%s/Station/%s/%s/%s"
                            % (
                                configMod.name(),
                                cs.networkCode(),
                                cs.stationCode(),
                                setup.name(),
                            )
                        )
                        config.add(paramSet)
                    paramSet.setModuleID(configMod.publicID())
                    paramSet.setCreated(seiscomp.core.Time.GMT())
                    newParamSets[paramSet.publicID()] = 1
                    setup.setParameterSetID(paramSet.publicID())
                    if mod in stationSetups:
                        setup.update()
                elif paramSet.moduleID() != configMod.publicID():
                    paramSet.setModuleID(configMod.publicID())
                    paramSet.update()

                # Synchronize existing parameterset with the new parameters
                sync(paramSet, params)

                if setup.name() == "default":
                    globalSet = paramSet.publicID()

            for i in range(cs.setupCount()):
                setup = cs.setup(i)
                paramSet = seiscomp.datamodel.ParameterSet.Find(setup.parameterSetID())
                if not paramSet:
                    continue

                if paramSet.publicID() != globalSet and paramSet.baseID() != globalSet:
                    paramSet.setBaseID(globalSet)
                    if not paramSet.publicID() in newParamSets:
                        paramSet.update()

        # Collect unused ParameterSets
        usedSets = {}
        for i in range(config.configModuleCount()):
            configMod = config.configModule(i)
            for j in range(configMod.configStationCount()):
                cs = configMod.configStation(j)
                for k in range(cs.setupCount()):
                    setup = cs.setup(k)
                    collect(usedSets, setup.parameterSetID())

        # Delete unused ParameterSets
        i = 0
        while i < config.parameterSetCount():
            paramSet = config.parameterSet(i)
            if not paramSet.publicID() in usedSets:
                print(
                    f"- {paramSet.publicID()} / obsolete parameter set", file=sys.stderr
                )
                paramSet.detach()
            else:
                i = i + 1

        # Generate output file and exit if configured
        if self._outputFile is not None:
            ar = seiscomp.io.XMLArchive()
            if not ar.create(self._outputFile):
                print(
                    f"Failed to created output file: {self._outputFile}",
                    file=sys.stderr,
                )
                return False

            ar.setFormattedOutput(True)
            if self._createNotifier:
                nmsg = seiscomp.datamodel.Notifier.GetMessage(True)
                ar.writeObject(nmsg)
            else:
                ar.writeObject(config)
            ar.close()
            return True

        ncount = seiscomp.datamodel.Notifier.Size()
        if ncount > 0:
            print(f"+ synchronize {ncount} change(s)", file=sys.stderr)
        else:
            print("- database is already up-to-date", file=sys.stderr)
            return True

        cfgmsg = seiscomp.datamodel.ConfigSyncMessage(False)
        cfgmsg.setCreationInfo(seiscomp.datamodel.CreationInfo())
        cfgmsg.creationInfo().setCreationTime(seiscomp.core.Time.GMT())
        cfgmsg.creationInfo().setAuthor(self.author())
        cfgmsg.creationInfo().setAgencyID(self.agencyID())
        self.send(seiscomp.client.Protocol.STATUS_GROUP, cfgmsg)

        # Send messages in a batch of 100 notifiers to not exceed the
        # maximum allowed message size of ~300kb.
        msg = seiscomp.datamodel.NotifierMessage()
        nmsg = seiscomp.datamodel.Notifier.GetMessage(False)
        count = 0
        sys.stderr.write("\r  + sending notifiers: %d%%" % (count * 100 / ncount))
        sys.stderr.flush()
        while nmsg:
            for o in nmsg:
                n = seiscomp.datamodel.Notifier.Cast(o)
                if n:
                    msg.attach(n)

            if msg.size() >= 100:
                count += msg.size()
                self.send("CONFIG", msg)
                msg.clear()
                sys.stderr.write(
                    "\r  + sending notifiers: %d%%" % (count * 100 / ncount)
                )
                sys.stderr.flush()

            nmsg = seiscomp.datamodel.Notifier.GetMessage(False)

        if msg.size() > 0:
            count += msg.size()
            self.send("CONFIG", msg)
            msg.clear()
            sys.stderr.write("\r  + sending notifiers: %d%%" % (count * 100 / ncount))
            sys.stderr.flush()

        sys.stderr.write("\n")

        # Notify about end of synchronization
        cfgmsg.creationInfo().setCreationTime(seiscomp.core.Time.GMT())
        cfgmsg.isFinished = True
        self.send(seiscomp.client.Protocol.STATUS_GROUP, cfgmsg)

        return True


def main():
    app = ConfigDBUpdater(len(sys.argv), sys.argv)
    return app()
