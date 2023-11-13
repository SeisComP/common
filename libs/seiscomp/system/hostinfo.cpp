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


#include <seiscomp/system/application.h>
#include <seiscomp/system/hostinfo.h>
#include <seiscomp/utils/files.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/core/platform/platform.h>
#include <seiscomp/utils/timer.h>

#if !defined(_MSC_VER)
#include <netdb.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#include <windows.h>
#include <psapi.h>
#endif

#ifdef MACOSX
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/param.h>
#define HOST_NAME_MAX MAXHOSTNAMELEN
#endif

#ifdef sun
#include <procfs.h>
#include <fcntl.h>
#include <stdio.h>
#define PROCFS "/proc"
#endif

#ifndef WIN32
#include <unistd.h>
#else
#include <winsock2.h>
#include <windows.h>
#endif

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif

#include <cstdint>
#include <fstream>
#include <string>
#include <mutex>
#include <thread>
#include <vector>


using namespace std;


namespace Seiscomp {
namespace System {

namespace {
#ifdef sun

string getProgramName() {
	psinfo_t currproc;
        int fd;
        char buf[30];
        snprintf(buf, sizeof(buf), "%s/%d/psinfo", PROCFS, getpid());
        if ((fd = open (buf, O_RDONLY)) >= 0) {
                if ( read(fd, &currproc, sizeof(psinfo_t)) == sizeof(psinfo_t) )
                        return currproc.pr_fname;
                close(fd);
        }
	return "";
}


int getPageShift() {
	int i = sysconf(_SC_PAGESIZE);
	int pageShift = 0;
	while ((i >>= 1) > 0)
		++pageShift;
	pageShift -= 10;
	return pageShift;
}


string staticProgramName = getProgramName();
int staticPageShift = getPageShift();


#endif
string getLineFromFile(const string &fileName, const string &tag) {
	string line;
	ifstream file(fileName.c_str());
	if ( !file.is_open() )
		return line;

	while ( getline(file, line) ) {
		vector<string> tokens;
		Seiscomp::Core::split(tokens, line.c_str(), ":");
		if ( tokens.size() > 1 )
			if ( tokens[0] == tag )
				break;
		line.clear();
	}

	return line;
}


struct HostInfoImpl {
	HostInfoImpl()
	: pid(-1) {}

	void init() {
		lock_guard<mutex> lk(resourceMutex);

		if ( pid != -1 )
			// Already initialized
			return;

		// Get PID
#if defined(_MSC_VER)
		pid = GetCurrentProcessId();
#else
		pid = (int)getpid();
#endif

		// Get hostname
#ifdef WIN32
		WSADATA wsaData;
		int wsaerr = WSAStartup(MAKEWORD(2, 0), &wsaData);
		if ( wsaerr != 0 )
			SEISCOMP_ERROR("WSAStartup failed with error: %d", wsaerr);

#endif
		char tmpHostname[HOST_NAME_MAX];
		if ( gethostname(tmpHostname, HOST_NAME_MAX) != 0 ) {
			const char* name = nullptr;
			name = getenv("HOSTNAME");
			hostname = name ? name : "";
		}
		else
			hostname = tmpHostname;

		// Login
#ifdef WIN32
		const char *name = getenv("USERNAME");
		login = name ? name : "";
#else
		char buf[100];
		if ( !getlogin_r(buf, 100) )
			login = buf;
		else {
			char *tmp;
			tmp = getenv("USER");
			if ( tmp == nullptr )
				login = "";
			else
				login = tmp;
		}
#endif

		// Program name
		programName.clear();

		auto app = Application::Instance();
		if ( app ) {
			programName = app->name();
		}
		else {
			#ifdef LINUX
			ifstream file("/proc/self/cmdline");
			if ( file.is_open() ) {
				getline(file, programName);
			}

			while ( true ) {
				size_t p;
				p = programName.find('\0');
				if ( p != string::npos ) {
					string baseName = Util::basename(programName.substr(0, p));
					if ( baseName.compare(0, 6, "python") == 0 ) {
						programName.erase(0, p+1);
					}
					else {
						programName.erase(p);
						break;
					}
				}
				else {
					break;
				}
			}

			programName = Util::basename(programName);
			#endif
			#ifdef MACOSX
			const char* programName  = getprogname();
			if ( programName ) {
				programName = programName;
			}
			#endif

			#if defined(_MSC_VER)
			char path[MAX_PATH];
			GetModuleFileName(nullptr, path, MAX_PATH);
			char *prog = strrchr(path, '\\');
			if ( !prog ) {
				prog = path;
			}
			else {
				++prog;
			}
			char *ending = strrchr(prog, '.');
			if ( ending ) *ending = '\0';
			programName = prog;
			#endif

			#ifdef sun
			programName = staticProgramName;
			#endif
		}

		// Total memory
		totalMemory = -1;
#ifdef LINUX
		string line = getLineFromFile("/proc/meminfo", "MemTotal");
		if ( line.empty() ) {
			return;
		}

		vector<string> tokens;
		Core::split(tokens, line.c_str(), ":");
		if ( tokens.size() < 2 ) {
			return;
		}

		line.assign(tokens[1]);
		tokens.clear();
		Core::split(tokens, line.c_str(), "kB");
		if ( tokens.size() < 1 ) {
			return;
		}

		Core::trim(tokens[0]);
		totalMemory = 0;
		Core::fromString(totalMemory, tokens[0]);
#endif

#ifdef MACOSX
		int mib[2];
		size_t size = sizeof(int);

		mib[0] = CTL_HW;
		mib[1] = HW_PHYSMEM;
		sysctl(mib, 2, &totalMemory, &size, nullptr, 0);
		totalMemory /= 1024;
#endif

#ifdef sun
		totalMemory = sysconf(_SC_PHYS_PAGES);
		if ( staticPageShift > 0 ) {
			totalMemory <<= staticPageShift;
		}
		else if ( staticPageShift < 0 ) {
			totalMemory >>= staticPageShift;
		}
#endif

#if _MSC_VER
		MEMORYSTATUS memstat;
		GlobalMemoryStatus(&memstat);
		totalMemory = memstat.dwTotalPhys / 1024;
#endif

#if !defined(_MSC_VER)
		clockTicks = sysconf(_SC_CLK_TCK);
#else
		clockTicks = 10000000U;
#endif
	}

	int           pid;
	string        hostname;
	string        login;
	string        programName;
	int64_t       totalMemory;
	long          clockTicks;
	mutable mutex resourceMutex;
};


HostInfoImpl g_impl;


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class HostInfo::Impl {
	public:
		double getCurrentCpuUsage(bool forceReset) {
			g_impl.init();

			lock_guard<mutex> lk(_mutex);

#if !defined(_MSC_VER)
			struct tms current;

			times(&current);

			if ( forceReset || _first ) {
				_timer.restart();
				_last = current;
				_first = false;
				return -1;
			}

			double elapsed = _timer.elapsed();
			_timer.restart();

			double usage = ((current.tms_utime - _last.tms_utime) +
			                (current.tms_stime - _last.tms_stime) +
			                (current.tms_cutime - _last.tms_cutime) +
			                (current.tms_cstime - _last.tms_cstime)) / double(elapsed*g_impl.clockTicks);
			_last = current;
			return usage;
#else
			HANDLE hProcess;

			hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,
			                       FALSE, _pid);
			if ( hProcess != nullptr ) {
				FILETIME creationtime, exittime, kerneltime, usertime;
				if ( GetProcessTimes(hProcess, &creationtime, &exittime, &kerneltime, &usertime) ) {
					ULARGE_INTEGER current;
					current.LowPart = usertime.dwLowDateTime;
					current.HighPart = usertime.dwHighDateTime;

					if ( forceReset || _first ) {
						_timer.restart();
						_last.QuadPart = current.QuadPart;
						_first = false;
						return -1;
					}

					double elapsed = _timer.elapsed();
					_timer.restart();

					double usage = (current.QuadPart - _last.QuadPart) / double(elapsed*g_impl.clockTicks);
					_last = current;
					return usage;
				}
			}
			else {
				return -1;
			}
#endif
		}

	private:
		mutex           _mutex;
		bool            _first = true;
		Util::StopWatch _timer;
#if !defined(_MSC_VER)
		struct tms      _last;
#else
		ULARGE_INTEGER  _last;
#endif
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
HostInfo::HostInfo()
: _impl(nullptr) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
HostInfo::~HostInfo() {
	if ( _impl ) delete _impl;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int HostInfo::pid() const {
	g_impl.init();
	return g_impl.pid;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string HostInfo::name() const {
	g_impl.init();
	return g_impl.hostname;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string HostInfo::login() const {
	g_impl.init();
	return g_impl.login;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string HostInfo::programName() const {
	g_impl.init();
	return g_impl.programName;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int64_t HostInfo::totalMemory() const {
	g_impl.init();
	return g_impl.totalMemory;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int HostInfo::getCurrentMemoryUsage() const {
	int usedMemory = 0;
#ifdef LINUX
	std::string line = getLineFromFile("/proc/self/status", "VmRSS");
	if ( line.empty() ) return -1;

	std::vector<std::string> tokens;
	if ( Core::split(tokens, line.c_str(), ":") != 2 )
		return 0;

	line = tokens[1];
	tokens.clear();
	if ( Core::split(tokens, line.c_str(), "kB") != 2 )
		return 0;

	Core::trim(tokens[0]);
	Core::fromString(usedMemory, tokens[0]);
#endif

#if defined(MACOSX) or defined(BSD)
	FILE *pipe;
	char cmd[256];
	pid_t PID = getpid();
	long rsize = 0;
	sprintf(cmd,"ps -o rss -p %d | grep -v RSS", PID);
	pipe = popen(cmd, "r");
	if ( pipe ) {
		fscanf( pipe, "%ld", &rsize);
		pclose(pipe);
	}
	usedMemory = rsize;
#endif

#ifdef sun
	psinfo_t currproc;
	int fd;
	char buf[30];
	usedMemory = 0;
	snprintf(buf, sizeof(buf), "%s/%lu/psinfo", PROCFS, _pid);
	if ( (fd = open (buf, O_RDONLY)) >= 0 ) {
		if ( read(fd, &currproc, sizeof(psinfo_t)) == sizeof(psinfo_t) ) {
			usedMemory = currproc.pr_rssize;
		}
		close(fd);
	}
#endif

#if _MSC_VER
	HANDLE hProcess;
	PROCESS_MEMORY_COUNTERS pmc;

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
	                       PROCESS_VM_READ,
	                       FALSE, _pid);
	if ( hProcess != nullptr ) {
		if ( GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)) )
			// map to kbyte
			usedMemory = pmc.WorkingSetSize / 1024;
	}
#endif

	return usedMemory;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double HostInfo::getCurrentCpuUsage(bool forceReset) {
	if ( !_impl ) _impl = new Impl;
	return _impl->getCurrentCpuUsage(forceReset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
