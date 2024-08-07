import seiscomp.kernel


class Module(seiscomp.kernel.Module):
    def __init__(self, env):
        seiscomp.kernel.Module.__init__(self, env, env.moduleName(__file__))

    # do not allow to start the module as daemon
    def start(self):
        self.env.log("%s cannot be started by design" % self.name)
        return None

    # do not allow to stop the module as daemon
    def stop(self):
        return None

    def enable(self):
        self.env.log("%s cannot be enabled by design" % self.name)
        return None

    # module should not run and not be listed upon seiscomp status or in scconfig system panel
    def status(self, _shouldRun):
        return 1

    # do nothing
    def updateConfig(self):
        return 0

    def supportsAliases(self):
        # The default handler does not support aliases
        return True
