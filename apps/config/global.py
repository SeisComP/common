import os
import glob
import time
import sys
import seiscomp.config
import seiscomp.kernel
import seiscomp.system
import seiscomp.bindings2cfg


def parseBindPort(bind):
    bindToks = bind.split(':')
    if len(bindToks) == 1:
        return int(bindToks[0])
    elif len(bindToks) == 2:
        return int(bindToks[1])
    else:
        return -1


class Module(seiscomp.kernel.Module):
    def __init__(self, env):
        seiscomp.kernel.Module.__init__(self, env, env.moduleName(__file__))
        # This is a config module which synchronizes bindings with the database
        self.isConfigModule = True

    def updateConfig(self):
        messaging = True
        messagingPort = 18180
        messagingProtocol = 'scmp'

        try:
            messaging = self.env.getBool("messaging.enable")
        except:
            pass

        # If messaging is disabled in kernel.cfg, do not do anything
        if not messaging:
            sys.stdout.write("- messaging disabled, nothing to do\n")
            return 0

        # Load scmaster configuration and figure the bind ports of scmaster out
        cfg = seiscomp.config.Config()
        seiscomp.system.Environment.Instance().initConfig(cfg, "scmaster")

        # First check the unencrypted port and prefer that
        p = parseBindPort(cfg.getString("interface.bind"))
        if p > 0:
            messagingPort = p

            try:
                bind = self.env.getString("messaging.bind")
                bindToks = bind.split(':')
                if len(bindToks) == 1:
                    messagingPort = int(bindToks[0])
                elif len(bindToks) == 2:
                    messagingPort = int(bindToks[1])
                else:
                    sys.stdout.write(
                        "E invalid messaging bind parameter: %s\n" % bind)
                    sys.stdout.write("  expected either 'port' or 'ip:port'\n")
                    return 1
            except:
                pass

        # Otherwise check if ssl is enabled
        else:
            p = parseBindPort(cfg.getString("interface.ssl.bind"))
            if p > 0:
                messagingPort = p
                messagingProtocol = 'scmps'

        # Synchronize database configuration
        params = [self.name, '--console', '1', '-H',
                  '%s://localhost:%d/production' % (messagingProtocol, messagingPort)]
        # Create the database update app and run it
        # This app implements a seiscomp.client.Application and connects
        # to localhost regardless of connections specified in global.cfg to
        # prevent updating a remote installation by accident.
        app = seiscomp.bindings2cfg.ConfigDBUpdater(len(params), params)
        app.setConnectionRetries(3)
        return app()

    def setup(self, setup_config):
        cfgfile = os.path.join(self.env.SEISCOMP_ROOT, "etc", "global.cfg")

        cfg = seiscomp.config.Config()
        cfg.readConfig(cfgfile)
        try:
            cfg.setString("datacenterID", setup_config.getString(
                "global.meta.datacenterID"))
        except:
            cfg.remove("datacenterID")

        try:
            cfg.setString("agencyID", setup_config.getString(
                "global.meta.agencyID"))
        except:
            cfg.remove("agencyID")

        try:
            cfg.setString("organization", setup_config.getString(
                "global.meta.organization"))
        except:
            cfg.remove("organization")

        cfg.writeConfig()

        return 0
