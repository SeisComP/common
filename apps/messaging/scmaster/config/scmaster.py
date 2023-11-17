from __future__ import print_function

import os
import shutil
import shlex
import sys
import subprocess
import tempfile
from seiscomp import config, kernel, system

# Python version depended string conversion
if sys.version_info[0] < 3:
    py3bstr = str
    py3ustr = str

else:
    py3bstr = lambda s: s.encode('utf-8')
    py3ustr = lambda s: s.decode('utf-8', 'replace')


class DBParams:
    def __init__(self):
        self.db = None
        self.rwuser = None
        self.rwpwd = None
        self.rouser = None
        self.ropwd = None
        self.rohost = None
        self.rwhost = None
        self.drop = False
        self.create = False


def check_output(cmd):
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE, shell=True)
    out = proc.communicate()
    return [py3ustr(out[0]), py3ustr(out[1]), proc.returncode]


def addEntry(cfg, param, item):
    # Adds an item to a parameter list
    try:
        items = cfg.getStrings(param)
    except ValueError:
        items = config.VectorStr()

    if item not in items:
        items.push_back(item)
    cfg.setStrings(param, items)


def removeEntry(cfg, param, item):
    # Removes an items from a parameter list
    try:
        items = cfg.getStrings(param)
        for i in range(items.size()):
            if items[i] == item:
                items.erase(items.begin() + i)
                cfg.setStrings(param, items)
                break
    except ValueError:
        # No parameter set, nothing to do
        pass


# The kernel module which starts scmaster if enabled
class Module(kernel.CoreModule):
    def __init__(self, env):
        kernel.CoreModule.__init__(
            self, env, env.moduleName(__file__))
        # High priority
        self.order = -1

        # Default values
        self.messaging = True
        self.messagingBind = None

        try:
            self.messaging = self.env.getBool("messaging.enable")
        except ValueError:
            pass
        try:
            self.messagingBind = self.env.getString("messaging.bind")
        except ValueError:
            pass

    # Add master port
    def _get_start_params(self):
        if self.messagingBind:
            return kernel.Module._get_start_params(self) + \
                   " --bind %s" % self.messagingBind

        return kernel.Module._get_start_params(self)

    def start(self):
        if not self.messaging:
            print("[kernel] {} is disabled by config".format(self.name),
                  file=sys.stderr)
            return 1

        appConfig = system.Environment.Instance().appConfigFileName(self.name)
        localConfig = system.Environment.Instance().configFileName(self.name)
        lockFile = os.path.join(self.env.SEISCOMP_ROOT, self.env.lockFile(self.name))
        try:
            needRestart = False
            started = os.path.getmtime(lockFile)
            try:
                needRestart = started < os.path.getmtime(appConfig)
            except Exception:
                pass
            try:
                needRestart = started < os.path.getmtime(localConfig)
            except Exception:
                pass

            if needRestart:
                self.stop()
        except Exception:
            pass

        return kernel.CoreModule.start(self)

    def check(self):
        if not self.messaging:
            print("[kernel] {} is disabled by config".format(self.name),
                  file=sys.stderr)
            return 0

        return kernel.CoreModule.check(self)

    def status(self, shouldRun):
        if not self.messaging:
            shouldRun = False
        return kernel.CoreModule.status(self, shouldRun)

    def readDBParams(self, params, setup_config):
        try:
            params.db = setup_config.getString(self.name
                                               + ".database.enable.backend.db")
        except ValueError as err:
            print(err)
            print("  - database name not set, ignoring setup",
                  file=sys.stderr)
            return False

        try:
            params.rwhost = setup_config.getString(
                self.name + ".database.enable.backend.rwhost")
        except ValueError:
            print("  - database host (rw) not set, ignoring setup",
                  file=sys.stderr)
            return False

        try:
            params.rwuser = setup_config.getString(
                self.name + ".database.enable.backend.rwuser")
        except ValueError:
            print("  - database user (rw) not set, ignoring setup",
                  file=sys.stderr)
            return False

        try:
            params.rwpwd = setup_config.getString(
                self.name + ".database.enable.backend.rwpwd")
        except ValueError:
            print("  - database password (rw) not set, ignoring setup",
                  file=sys.stderr)
            return False

        try:
            params.rohost = setup_config.getString(
                self.name + ".database.enable.backend.rohost")
        except ValueError:
            print("  - database host (ro) not set, ignoring setup",
                  file=sys.stderr)
            return False

        try:
            params.rouser = setup_config.getString(
                self.name + ".database.enable.backend.rouser")
        except ValueError:
            print("  - database user (ro) not set, ignoring setup",
                  file=sys.stderr)
            return False

        try:
            params.ropwd = setup_config.getString(
                self.name + ".database.enable.backend.ropwd")
        except ValueError:
            print("  - database password (ro) not set, ignoring setup",
                  file=sys.stderr)
            return False

        try:
            params.create = setup_config.getBool(
                self.name + ".database.enable.backend.create")
        except ValueError:
            params.create = False

        try:
            params.drop = setup_config.getBool(
                self.name + ".database.enable.backend.create.drop")
        except ValueError:
            params.drop = False

        return True

    def setup(self, setup_config):
        schemapath = os.path.join(self.env.SEISCOMP_ROOT, "share", "db")

        cfg = config.Config()
        system.Environment.Instance().initConfig(cfg, self.name)

        try:
            dbenable = setup_config.getBool(self.name + ".database.enable")
        except ValueError:
            print("  - database.enable not set, ignoring setup",
                  file=sys.stderr)
            return 0

        dbBackend = None

        if not dbenable:
            removeEntry(cfg, "queues.production.plugins", "dbstore")
            removeEntry(
                cfg, "queues.production.processors.messages", "dbstore")
            cfg.remove("queues.production.processors.messages.dbstore.driver")
            cfg.remove("queues.production.processors.messages.dbstore.read")
            cfg.remove("queues.production.processors.messages.dbstore.write")
        else:
            try:
                dbBackend = setup_config.getString(
                    self.name + ".database.enable.backend")
            except ValueError:
                print("  - database backend not set, ignoring setup",
                      file=sys.stderr)
                return 1

            if dbBackend == "mysql/mariadb":
                dbBackend = "mysql"
                try:
                    rootpwd = setup_config.getString(
                        self.name + ".database.enable.backend.create.rootpw")
                except ValueError:
                    rootpwd = ""

                try:
                    runAsSuperUser = setup_config.getBool(
                        self.name + ".database.enable.backend.create.runAsSuperUser")
                except ValueError:
                    runAsSuperUser = False

                params = DBParams()
                if not self.readDBParams(params, setup_config):
                    return 1

                cfg.setString("queues.production.processors.messages.dbstore.read",
                              "{}:{}@{}/{}"
                              .format(params.rouser, params.ropwd, params.rohost, params.db))
                cfg.setString("queues.production.processors.messages.dbstore.write",
                              "{}:{}@{}/{}"
                              .format(params.rwuser, params.rwpwd, params.rwhost, params.db))

                if params.create:
                    dbScript = os.path.join(schemapath, "mysql_setup.py")
                    options = [
                        params.db,
                        params.rwuser,
                        params.rwpwd,
                        params.rouser,
                        params.ropwd,
                        params.rwhost,
                        rootpwd,
                        str(params.drop),
                        schemapath
                    ]

                    binary = os.path.join(schemapath, "pkexec_wrapper.sh")
                    print("+ Running MySQL database setup script {}"
                          .format(dbScript), file=sys.stderr)
                    if runAsSuperUser:
                        cmd = "{} seiscomp-python {} {}".format(
                            binary, dbScript, " ".join(shlex.quote(o) for o in options)
                        )
                    else:
                        cmd = "{} {}".format(dbScript, " ".join(shlex.quote(o) for o in options))

                    p = subprocess.Popen(cmd, shell=True)
                    ret = p.wait()
                    if ret != 0:
                        print("  - Failed to setup database", file=sys.stderr)
                        return 1

            elif dbBackend == "postgresql":
                dbBackend = "postgresql"

                params = DBParams()
                if not self.readDBParams(params, setup_config):
                    return 1

                cfg.setString("queues.production.processors.messages.dbstore.read",
                              "{}:{}@{}/{}"
                              .format(params.rouser, params.ropwd,
                                      params.rohost, params.db))
                cfg.setString("queues.production.processors.messages.dbstore.write",
                              "{}:{}@{}/{}"
                              .format(params.rwuser, params.rwpwd,
                                      params.rwhost, params.db))

                if params.create:
                    try:
                        tmpPath = tempfile.mkdtemp()
                        os.chmod(tmpPath, 0o755)
                        tmpPath = os.path.join(tmpPath, "setup")
                        try:
                            shutil.copytree(schemapath, tmpPath)
                            filename = os.path.join(self.env.SEISCOMP_ROOT,
                                                    "bin", "seiscomp-python")
                            shutil.copy(filename, tmpPath)
                        except Exception as err:
                            print(err)
                            return 1

                        dbScript = os.path.join(tmpPath, "postgres_setup.py")
                        options = [
                            params.db,
                            params.rwuser,
                            params.rwpwd,
                            params.rouser,
                            params.ropwd,
                            params.rwhost,
                            str(params.drop),
                            tmpPath
                        ]

                        binary = os.path.join(schemapath, "pkexec_wrapper.sh")
                        print("+ Running PostgreSQL database setup script {}"
                              .format(dbScript), file=sys.stderr)
                        cmd = '{} su postgres -c "{}/seiscomp-python {} {}"'.format(
                              binary, tmpPath, dbScript, " ".join(shlex.quote(o) for o in options)
                        )

                        p = subprocess.Popen(cmd, shell=True)
                        ret = p.wait()
                        if ret != 0:
                            print("  - Failed to setup database",
                                  file=sys.stderr)
                            return 1
                    finally:
                        try:
                            shutil.rmtree(tmpPath)
                        except OSError:
                            pass

            elif dbBackend == "sqlite3":
                dbBackend = "sqlite3"
                dbScript = os.path.join(schemapath, "sqlite3_setup.py")

                try:
                    create = setup_config.getBool(
                        self.name + ".database.enable.backend.create")
                except BaseException:
                    create = False

                try:
                    filename = setup_config.getString(
                        self.name + ".database.enable.backend.filename")
                    filename = system.Environment.Instance().absolutePath(filename)
                except BaseException:
                    filename = os.path.join(self.env.SEISCOMP_ROOT, "var",
                                            "lib", "seiscomp.db")

                if not filename:
                    print("  - location not set, ignoring setup",
                          file=sys.stderr)
                    return 1

                try:
                    override = setup_config.getBool(
                        self.name + ".database.enable.backend.create.override")
                except BaseException:
                    override = False

                options = [
                    filename,
                    schemapath
                ]

                if create:
                    print("+ Running SQLite3 database setup script {}"
                          .format(dbScript), file=sys.stderr)
                    cmd = "seiscomp-python {} {} {}".format(dbScript, " ".join(shlex.quote(o) for o in options), override)
                    p = subprocess.Popen(cmd, shell=True)
                    ret = p.wait()
                    if ret != 0:
                        print("  - Failed to setup database", file=sys.stderr)
                        return 1

                cfg.setString("queues.production.processors.messages.dbstore.read",
                              filename)
                cfg.setString("queues.production.processors.messages.dbstore.write",
                              filename)

            # Configure db backend for scmaster
            cfg.setString("core.plugins", "db" + dbBackend)
            cfg.setString(
                "queues.production.processors.messages.dbstore.driver",
                dbBackend)

            addEntry(cfg, "queues.production.plugins", "dbstore")
            addEntry(cfg, "queues.production.processors.messages", "dbstore")

        cfg.writeConfig(
            system.Environment.Instance().configFileLocation(
                self.name, system.Environment.CS_CONFIG_APP))

        # Now we need to insert the corresponding plugin to etc/global.cfg
        # that all connected local clients can handle the database backend
        if dbBackend:
            cfgfile = os.path.join(self.env.SEISCOMP_ROOT, "etc", "global.cfg")
            cfg = config.Config()
            cfg.readConfig(cfgfile)
            cfg.setString("core.plugins", "db" + dbBackend)
            cfg.writeConfig()

        return 0

    def updateConfig(self):
        cfg = config.Config()
        system.Environment.Instance().initConfig(cfg, self.name)

        try:
            queues = cfg.getStrings("queues")
        except ValueError:
            queues = []

        # iterate through all queues and check DB schema version if message
        # processor dbstore is present
        for queue in queues:
            print("INFO: Checking queue '{}'".format(queue), file=sys.stderr)
            try:
                msgProcs = cfg.getStrings("queues.{}.processors.messages"
                                          .format(queue))
                if "dbstore" in msgProcs and not self.checkDBStore(cfg, queue):
                    return 1
            except ValueError:
                print("  * ignoring - no database backend configured",
                      file=sys.stderr)

        return 0

    def checkDBStore(self, cfg, queue):
        prefix = "queues.{}.processors.messages.dbstore".format(queue)

        print("  * checking DB schema version", file=sys.stderr)

        try:
            backend = cfg.getString("{}.driver".format(prefix))
        except ValueError:
            print("WARNING: dbstore message processor activated but no "
                  "database backend configured", file=sys.stderr)
            return True

        if backend not in ("mysql", "postgresql"):
            print("WARNING: Only MySQL and PostgreSQL migrations are "
                  "supported right now. Please check and upgrade the "
                  "database schema version yourselves.", file=sys.stderr)
            return True

        print("  * check database write access ... ", end='', file=sys.stderr)

        # 1. Parse connection
        try:
            params = cfg.getString("{}.write".format(prefix))
        except ValueError:
            print("failed", file=sys.stderr)
            print("WARNING: dbstore message processor activated but no "
                  "write connection configured", file=sys.stderr)
            return True

        user = 'sysop'
        pwd = 'sysop'
        host = 'localhost'
        db = 'seiscomp'
        port = None

        tmp = params.split('@')
        if len(tmp) > 1:
            params = tmp[1]

            tmp = tmp[0].split(':')
            if len(tmp) == 1:
                user = tmp[0]
                pwd = None
            elif len(tmp) == 2:
                user = tmp[0]
                pwd = tmp[1]
            else:
                print("failed", file=sys.stderr)
                print("WARNING: Invalid scmaster.cfg:{}.write, cannot check "
                      "schema version".format(prefix), file=sys.stderr)
                return True

        tmp = params.split('/')
        if len(tmp) > 1:
            tmpHost = tmp[0]
            db = tmp[1]
        else:
            tmpHost = tmp[0]

        # get host name and port
        tmp = tmpHost.split(':')
        host = tmp[0]
        if len(tmp) == 2:
            try:
                port = int(tmp[1])
            except ValueError:
                print("ERROR: Invalid port number {}".format(tmp[1]),
                      file=sys.stderr)
                return True

        db = db.split('?')[0]

        # 2. Try to login
        if backend == "mysql":
            cmd = "mysql -u \"%s\" -h \"%s\" -D\"%s\" --skip-column-names" % (
                user, host, db)
            if port:
                cmd += " -P %d" % (port)
            if pwd:
                cmd += " -p\"%s\"" % pwd.replace('$', '\\$')
            cmd += " -e \"SELECT value from Meta where name='Schema-Version'\""
        else:
            if pwd:
                os.environ['PGPASSWORD'] = pwd
            cmd = "psql -U \"%s\" -h \"%s\" -t \"%s\"" % (user, host, db)
            if port:
                cmd += " -p %d" % (port)
            cmd += " -c \"SELECT value from Meta where name='Schema-Version'\""

        out = check_output(cmd)
        if out[2] != 0:
            print("failed", file=sys.stderr)
            print("WARNING: {} returned with error:".format(backend),
                  file=sys.stderr)
            print(out[1].strip(), file=sys.stderr)
            return False

        print("passed", file=sys.stderr)

        version = out[0].strip()
        print("  * database schema version is {}".format(version),
              file=sys.stderr)

        try:
            vmaj, vmin = [int(t) for t in version.split('.')]
        except ValueError:
            print("WARNING: wrong version format: expected MAJOR.MINOR",
                  file=sys.stderr)
            return True

        strictVersionMatch = True
        try:
            strictVersionMatch = cfg.getBool("{}.strictVersionMatch"
                                             .format(prefix))
        except ValueError:
            pass

        if not strictVersionMatch:
            print("  * database version check is disabled", file=sys.stderr)
            return True

        migrations = os.path.join(self.env.SEISCOMP_ROOT, "share", "db",
                                  "migrations", backend)
        migration_paths = {}

        vcurrmaj = 0
        vcurrmin = 0

        for f in os.listdir(migrations):
            if os.path.isfile(os.path.join(migrations, f)):
                name, ext = os.path.splitext(f)
                if ext != '.sql':
                    continue
                try:
                    vfrom, vto = name.split('_to_')
                except ValueError:
                    continue

                try:
                    vfrommaj, vfrommin = [int(t) for t in vfrom.split('_')]
                except ValueError:
                    continue

                try:
                    vtomaj, vtomin = [int(t) for t in vto.split('_')]
                except ValueError:
                    continue

                migration_paths[(vfrommaj, vfrommin)] = (vtomaj, vtomin)

                if (vtomaj > vcurrmaj) or ((vtomaj == vcurrmaj) and (vtomin > vcurrmin)):
                    vcurrmaj = vtomaj
                    vcurrmin = vtomin

        print("  * last migration version is %d.%d" % (vcurrmaj, vcurrmin),
              file=sys.stderr)

        if vcurrmaj == vmaj and vcurrmin == vmin:
            print("  * schema up-to-date", file=sys.stderr)
            return True

        if (vmaj, vmin) not in migration_paths:
            print("  * no migrations found", file=sys.stderr)
            return True

        print("  * migration to the current version is required. Apply the "
              "following", file=sys.stderr)
        print("    database migration scripts in exactly the given order:",
              file=sys.stderr)
        while (vmaj, vmin) in migration_paths:
            (vtomaj, vtomin) = migration_paths[(vmaj, vmin)]
            fname = "%d_%d_to_%d_%d.sql" % (vmaj, vmin, vtomaj, vtomin)
            if backend == "mysql":
                print("    * mysql -u {} -h {} -p {} < {}"
                      .format(user, host, db, os.path.join(migrations, fname)),
                      file=sys.stderr)
            elif backend == "postgresql":
                print("    * psql -U {} -h {} -d {} -W -f {}"
                      .format(user, host, db, os.path.join(migrations, fname)),
                      file=sys.stderr)
            else:
                print("    * {}".format(os.path.join(migrations, fname)),
                      file=sys.stderr)
            (vmaj, vmin) = (vtomaj, vtomin)

        return False
