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


#define SEISCOMP_COMPONENT SDS

#include <sys/stat.h>

#include <boost/version.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/regex.hpp>

#include <seiscomp/logging/log.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/core/system.h>
#include <seiscomp/system/environment.h>
#include <seiscomp/io/records/mseedrecord.h>
#include <seiscomp/utils/files.h>

#include "sdsarchive.h"


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::RecordStream;


namespace fs = boost::filesystem;


REGISTER_RECORDSTREAM(SDSArchive, "sdsarchive");


namespace {


#if (BOOST_VERSION <= 103301)
#define FS_LEAF(it) it->leaf()
#else
#define FS_LEAF(it) it->path().leaf()
#endif


fs::directory_iterator fsDirEnd;

bool isWildcard(const string &s) {
	return s.find('?') != string::npos || s.find('*') != string::npos;
}


Time getStartTime(const string &file) {
	Time startTime;

	ifstream ifs(file.c_str());
	if ( !ifs ) {
		return startTime;
	}

	while ( ifs ) {
		IO::MSeedRecord rec;
		rec.setHint(Record::META_ONLY);
		try {
			rec.read(ifs);
			if ( rec.sampleCount() > 0 ) {
				return rec.startTime();
			}
		}
		catch ( ... ) {}
	}


	return startTime;
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SDSArchive::Index::Index() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SDSArchive::Index::Index(const string& n, const string& s,
                  const string& l, const string& c)
: net(n), sta(s), loc(l), cha(c) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SDSArchive::Index::Index(const string& n, const string& s,
                         const string& l, const string& c,
                         const OPT(Time) &st, const OPT(Time) &et)
: net(n), sta(s), loc(l), cha(c), stime(st), etime(et) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SDSArchive::Index &SDSArchive::Index::operator=(const SDSArchive::Index &other) {
	if ( this != &other ) {
		net = other.net;
		sta = other.sta;
		loc = other.loc;
		cha = other.cha;
		stime = other.stime;
		etime = other.etime;
	}
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SDSArchive::Index::operator<(const Index &other) const {
	if ( net < other.net ) return true;
	if ( net > other.net ) return false;

	if ( sta < other.sta ) return true;
	if ( sta > other.sta ) return false;

	if ( loc < other.loc ) return true;
	if ( loc > other.loc ) return false;

	if ( cha < other.cha ) return true;
	if ( cha > other.cha ) return false;

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SDSArchive::Index::operator!=(const Index &other) const {
	if ( net != other.net ) return true;
	if ( sta != other.sta ) return true;
	if ( loc != other.loc ) return true;
	if ( cha != other.cha ) return true;
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SDSArchive::Index::operator==(const Index &other) const {
	if ( net != other.net ) return false;
	if ( sta != other.sta ) return false;
	if ( loc != other.loc ) return false;
	if ( cha != other.cha ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SDSArchive::SDSArchive() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SDSArchive::SDSArchive(const string arcroot) {
	SDSArchive::setSource(arcroot);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SDSArchive::~SDSArchive() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SDSArchive::setSource(const string &src) {
	if ( src.empty() ) {
		_arcroots.push_back(Environment::Instance()->installDir() + "/var/lib/archive");
	}
	else {
		Core::split(_arcroots, src.c_str(), ",");
	}

	for ( string &root : _arcroots ) {
		root = Environment::Instance()->absolutePath(root);
		SEISCOMP_DEBUG("+ Archive root: %s", root.c_str());
	}

	_closeRequested = false;
	SEISCOMP_DEBUG("Total of %ld archive roots are in use.", _arcroots.size());
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SDSArchive::addStream(const string &net, const string &sta,
                    const string &loc, const string &cha) {
	pair<IndexSet::iterator, bool> result;

	try {
		result = _streamSet.insert(Index(net,sta,loc,cha));
		if ( result.second ) _orderedRequests.push_back(*result.first);
	}
	catch(...) {
		return false;
	}

	return result.second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SDSArchive::addStream(const string &net, const string &sta,
                    const string &loc, const string &cha,
                    const OPT(Time) &stime, const OPT(Time) &etime) {
	pair<IndexSet::iterator, bool> result;

	try {
		result = _streamSet.insert(Index(net, sta, loc, cha, stime, etime));
		if ( result.second ) {
			_orderedRequests.push_back(*result.first);
		}
	}
	catch(...) {
		return false;
	}

	return result.second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SDSArchive::setStartTime(const OPT(Time) &stime) {
	_stime = stime;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SDSArchive::setEndTime(const OPT(Time) &etime) {
	_etime = etime;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SDSArchive::setTimeWindow(const TimeWindow &w) {
	return setStartTime(w.startTime()) && setEndTime(w.endTime());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SDSArchive::setTimeout(int seconds) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SDSArchive::close() {
	lock_guard<mutex> l(_mutex);
	_readFiles.clear();
	_fnames = FileQueue();
	_streamSet.clear();
	_orderedRequests.clear();
	_curiter = _orderedRequests.begin();
	_stime = _etime = None;
	_curidx = nullptr;
	_closeRequested = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SDSArchive::getDoy(const Time &time) {
	int year;

	time.get(&year);
	if ( (year%4==0 && year%100!=0) || year%400==0 ) {
		return (366 - ((int)(Time(year, 12, 31, 23, 59, 59) - time).seconds() / 86400));
	}

	return (365 - ((int)(Time(year, 12, 31, 23, 59, 59) - time).seconds() / 86400));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SDSArchive::resolveNet(string &pathStr,
                            const string &net, const string &sta,
                            const string &loc, const string &cha,
                            const Time &requestStartTime,
                            int doy, int year, bool first) {
	if ( !isWildcard(net) ) {
		pathStr += net + "/";
		return resolveSta(pathStr, net, sta, loc, cha, requestStartTime, doy, year, first);
	}

	try {
		SC_FS_DECLARE_PATH(path, pathStr)
		fs::directory_iterator it(path);
		for ( ; it != fsDirEnd; ++it ) {
			if ( !fs::is_directory(*it) ) continue;

			string leaf = SC_FS_FILE_NAME(SC_FS_DE_PATH(it));
			if ( !wildcmp(net, leaf) ) continue;

			string netPath = pathStr + leaf + "/";

			// Got a network here
			resolveSta(netPath, leaf, sta, loc, cha, requestStartTime, doy, year, first);
		}
	}
	catch ( ... ) {
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SDSArchive::resolveSta(string &pathStr,
                     const string &net, const string &sta,
                     const string &loc, const string &cha,
                     const Time &requestStartTime,
                     int doy, int year, bool first) {
	if ( !isWildcard(sta) ) {
		pathStr += sta + "/";
		return resolveCha(pathStr, net, sta, loc, cha, requestStartTime, doy, year, first);
	}

	try {
		SC_FS_DECLARE_PATH(path, pathStr)
		fs::directory_iterator it(path);
		for ( ; it != fsDirEnd; ++it ) {
			if ( !fs::is_directory(*it) ) continue;

			string leaf = SC_FS_FILE_NAME(SC_FS_DE_PATH(it));
			if ( !wildcmp(sta, leaf) ) continue;

			string staPath = pathStr + leaf + "/";

			// Got a station here
			resolveCha(staPath, net, leaf, loc, cha, requestStartTime, doy, year, first);
		}
	}
	catch ( ... ) {
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SDSArchive::resolveCha(string &pathStr,
                     const string &net, const string &sta,
                     const string &loc, const string &cha,
                     const Time &requestStartTime,
                     int doy, int year, bool first) {
	if ( !isWildcard(cha) ) {
		pathStr += cha + ".D/";
		return resolveLoc(pathStr, net, sta, loc, cha, requestStartTime, doy, year, first);
	}

	try {
		SC_FS_DECLARE_PATH(path, pathStr)
		fs::directory_iterator it(path);
		string chaSel = cha + ".D";
		for ( ; it != fsDirEnd; ++it ) {
			if ( !fs::is_directory(*it) ) continue;

			string leaf = SC_FS_FILE_NAME(SC_FS_DE_PATH(it));
			if ( !wildcmp(chaSel, leaf) ) continue;

			string chaPath = pathStr + leaf + "/";
			leaf.erase(leaf.rfind(".D"));

			// Got a channel here
			resolveLoc(chaPath, net, sta, loc, leaf, requestStartTime, doy, year, first);
		}
	}
	catch ( ... ) {
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SDSArchive::resolveLoc(string &pathStr,
                     const string &net, const string &sta,
                     const string &loc, const string &cha,
                     const Time &requestStartTime,
                     int doy, int year, bool first) {
	char buf[10];
	string filename;
	snprintf(buf, 9, "%d", year);
	filename = net + "." + sta + "." + loc + "." + cha + ".D." + buf + ".";
	snprintf(buf, 9, "%03d", doy);
	filename += buf;

	if ( !isWildcard(loc) ) {
		string fpath = pathStr + filename;
		if ( !Util::fileExists(fpath) ) return true;

		if ( _readFiles.find(filename) == _readFiles.end() ) {
			_readFiles.insert(filename);

			if ( first && getStartTime(fpath) > requestStartTime ) {
				(Time::FromYearDay(year, doy) - TimeSpan(86400,0)).get2(&year, &doy);
				resolveLoc(pathStr, net, sta, loc, cha, requestStartTime, doy+1, year, true);
				first = false;
			}

			SEISCOMP_DEBUG("+ %s", fpath.c_str());
			_fnames.push(File(fpath,first));
		}
		/*
		else
			SEISCOMP_DEBUG("ignore duplicate read: %s", path.c_str());
		*/
		return true;
	}

	try {
		SC_FS_DECLARE_PATH(path, pathStr)
		fs::directory_iterator it(path);
		for ( ; it != fsDirEnd; ++it ) {
			if ( !fs::is_regular_file(*it) ) continue;

			string leaf = SC_FS_FILE_NAME(SC_FS_DE_PATH(it));
			if ( !wildcmp(filename, leaf) ) continue;

			string fpath = pathStr + leaf;

			if ( !Util::fileExists(fpath) ) continue;

			if ( _readFiles.find(fpath) == _readFiles.end() ) {
				_readFiles.insert(fpath);

				if ( first && getStartTime(fpath) > requestStartTime ) {
					(Time::FromYearDay(year, doy) - TimeSpan(86400,0)).get2(&year, &doy);
					resolveLoc(pathStr, net, sta, loc, cha, requestStartTime, doy+1, year, first);
				}

				SEISCOMP_DEBUG("+ %s", fpath.c_str());
				_fnames.push(File(fpath,first));
			}
		}
	}
	catch ( ... ) {
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SDSArchive::resolveFiles(const string &net, const string &sta,
                              const string &loc, const string &cha,
                              const Time &requestStartTime,
                              int doy, int year, bool first) {
	char buf[10];
	snprintf(buf, 9, "%d", year);
	bool result = true;

	for ( const string &root : _arcroots ) {
		string path = root + "/" + buf + "/";
		if ( !resolveNet(path, net, sta, loc, cha, requestStartTime, doy, year, first) )
			result = false;
	}

	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SDSArchive::resolveRequest() {
	Time stime = _curidx->stime.value_or(Time());
	Time etime = _curidx->etime.value_or(Time());

	int sdoy = getDoy(stime);
	int edoy = getDoy(etime);
	int syear, eyear, tmpdoy;

	stime.get(&syear);
	etime.get(&eyear);

	bool first = true;
	for ( int year = syear; year <= eyear; ++year ) {
		tmpdoy = (year == eyear) ? edoy : getDoy(Time(year, 12, 31, 23, 59, 59));
		for ( int doy = sdoy; doy <= tmpdoy; ++doy ) {
			resolveFiles(_curidx->net, _curidx->sta, _curidx->loc, _curidx->cha,
			             stime, doy, year, first);
			first = false;
		}
		sdoy = 1;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SDSArchive::setStart(const string &fname, bool bsearch) {
	double samprate = 0.0;
	Time physFirstStartTime, physFirstEndTime;
	Time recstime, recetime;
	Time stime = !_curidx->stime ? _stime.value_or(Time()) : *_curidx->stime;
	long int offset = 0;
	long int size;
	bool result = true;

	_file.seekg(0, ios::end);
	size = (long int)_file.tellg();
	if ( size <= 0 ) {
		return false;
	}

	if ( bsearch ) {
		_file.seekg(0, ios::beg);

		//! binary search
		IO::MSeedRecord mseed;
		mseed.setHint(Record::META_ONLY);

		try {
			mseed.read(_file);
		}
		catch ( ... ) {
			return false;
		}

		samprate = mseed.samplingFrequency();
		physFirstStartTime = mseed.startTime();
		if ( samprate > 0. ) {
			physFirstEndTime = mseed.endTime();
		}
		else {
			SEISCOMP_WARNING("[%s@0] Wrong sampling frequency %.2f!", fname, samprate);
			physFirstEndTime = physFirstStartTime + TimeSpan(1, 0);
			result = false;
		}

		recstime = physFirstStartTime;

		long start = 0;
		long half = 0;
		long end = 0;
		int reclen = mseed.recordLength();

		if ( recstime < stime ) {
			end = static_cast<long>(size / reclen);
		}

		while ( (end - start) > 1 ) {
			half = start + (end - start) / 2;
			_file.seekg(half * reclen, ios::beg);

			try {
				mseed.read(_file);
				samprate = mseed.samplingFrequency();
				recstime = mseed.startTime();
				if ( samprate > 0. ) {
					recetime = mseed.endTime();
				}
				else {
					SEISCOMP_WARNING("[%s@%ld] Wrong sampling frequency %.2f!", fname.c_str(), half*reclen, samprate);
					recetime = recstime + TimeSpan(1, 0);
					result = false;
				}
			}
			catch ( ... ) {
				SEISCOMP_WARNING("[%s@%d] Couldn't read mseed header", fname, half * reclen);
				break;
			}

			if ( recetime < stime ) {
				start = half;
				if ( (end - start) == 1 ) {
					++half;
				}
			}
			else if ( recstime > stime ) {
				end = half;
			}
			else if ( (recstime <= stime) && (recetime >= stime) ) {
				if ( stime == recetime ) {
					++half;
				}
				break;
			}
		}

		if ( (half == 1) && (recstime > stime) ) {
			if ( physFirstEndTime > stime ) {
				half = 0;
			}
		}

		offset = half * reclen;
	}
	else {
		while ( _file ) {
			IO::MSeedRecord mseed;
			mseed.setHint(Record::META_ONLY);

			offset = _file.tellg();

			try {
				mseed.read(_file);
			}
			catch ( ... ) {
				continue;
			}

			samprate = mseed.samplingFrequency();
			recstime = mseed.startTime();

			if ( recstime > stime ) {
				break;
			}
			else {
				if ( samprate > 0. ) {
					recetime = mseed.endTime();
					if ( recetime > stime ) {
						break;
					}
				}
				else {
					SEISCOMP_WARNING("[%s@%ld] Wrong sampling frequency %.2f!", fname.c_str(), offset, samprate);
					result = false;
				}
			}
		}
	}

	_file.seekg(offset, ios::beg);
	if ( offset == size ) {
		_file.clear(ios::eofbit);
	}

	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Record *SDSArchive::next() {
	lock_guard<mutex> l(_mutex);

	if ( _file.is_open() ) {
		while ( !_closeRequested ) {
			auto *rec = new Seiscomp::IO::MSeedRecord(_dataType, _hint);

			try {
				rec->read(_file);
				if ( rec->startTime() > _curidx->etime ) {
					delete rec;
					break;
				}

				return rec;
			}
			catch ( EndOfStreamException &e ) {
				// EOF for this file, do nothing
				delete rec;
				SEISCOMP_DEBUG("exc: %s", e.what());
				break;
			}
			catch( exception &e ) {
				// Invalid record, delete it
				delete rec;
				SEISCOMP_ERROR("exc: %d, %s", (int)_file.tellg(), e.what());
				if ( !_file.good() )
					break;
			}
		}

		_file.close();
	}
	else {
		_curiter = _orderedRequests.begin();
	}

	while ( !_fnames.empty() || _curiter != _orderedRequests.end() ) {
		while ( _fnames.empty() && _curiter != _orderedRequests.end() ) {
			if ( !_etime ) {
				_etime = Time::UTC();
			}
			if ( !_curiter->stime && !_stime ) {
				SEISCOMP_WARNING("... has invalid time window -> ignore this request above");
				++_curiter;
			}
			else {
				_curidx = &*_curiter;
				// Check start/end times and set globals if not set
				if ( !_curidx->stime ) {
					_curidx->stime = _stime;
				}
				if ( !_curidx->etime ) {
					_curidx->etime = _etime;
				}
				++_curiter;
				resolveRequest();
				break;
			}
		}

		if ( !_fnames.empty() ) {
			while ( !_fnames.empty() ) {
				File file = _fnames.front();
				_fnames.pop();

				_file.open(file.first.c_str(), ifstream::in | ifstream::binary);
				if ( !_file.is_open() ) {
					SEISCOMP_DEBUG("R %s (not found)",file.first);
					_file.clear();
				}
				else {
					SEISCOMP_DEBUG("R %s (first: %d)", file.first, file.second);
					// File part of start time
					if ( file.second ) {
						if ( !setStart(file.first, true) ) {
							if ( !setStart(file.first, false) ) {
								SEISCOMP_WARNING("Error reading file %s; start of time window maybe incorrect",
								                 file.first.c_str());
								_file.close();
								continue;
							}
						}
					}

					while ( !_closeRequested ) {
						auto *rec = new Seiscomp::IO::MSeedRecord(_dataType, _hint);
						try {
							rec->read(_file);
							if ( rec->startTime() > _curidx->etime ) {
								delete rec;
								break;
							}

							return rec;
						}
						catch ( EndOfStreamException &e ) {
							delete rec;
							SEISCOMP_DEBUG("exc: %s", e.what());
							// EOF for this file, do nothing
							break;
						}
						catch( exception &e ) {
							delete rec;
							SEISCOMP_ERROR("exc: %d, %s", (int)_file.tellg(), e.what());
							if ( !_file.good() )
								break;
						}
					}

					_file.close();
				}
			}
		}
	}

	SEISCOMP_DEBUG("[sds] end of data");
	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
