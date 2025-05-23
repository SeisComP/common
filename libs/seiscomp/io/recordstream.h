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


#ifndef SEISCOMP_IO_RECORDSTREAM_H
#define SEISCOMP_IO_RECORDSTREAM_H


#include <seiscomp/core/interruptible.h>
#include <seiscomp/core/record.h>
#include <seiscomp/io/recordstreamexceptions.h>


namespace Seiscomp {
namespace IO {


DEFINE_SMARTPOINTER(RecordStream);


/**
 * @brief The RecordStream class defines an abstract interface to read data
 *        records from arbitrary sources.
 *
 * \code{.cpp}
 * RecordStreamPtr rs = RecordStream::Open(URL);
 * if ( rs != nullptr ) {
 *   rs->addStream("XY", "ABCD", "", "BHZ");
 *   RecordPtr rec;
 *   while ( (rec = rs->next()) ) {
 *     // Do something with rec
 *   }
 *   rs->close();
 * }
 * \endcode
 */
class SC_SYSTEM_CORE_API RecordStream : public Core::InterruptibleObject {
	DECLARE_SC_CLASS(RecordStream)

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	protected:
		/**
		 * @brief The constructor is protected because this is an
		 *        abstract base class.
		 */
		RecordStream();


	public:
		//! D'tor
		virtual ~RecordStream() {}


	// ------------------------------------------------------------------
	//  RecordStream interface
	// ------------------------------------------------------------------
	public:
		/**
		 * @brief Sets the source location of the data. This is implementation
		 *        specific. It can be a path to a file on disk or a hostname
		 *        with port or something else.
		 * @param source The source definition.
		 * @return Status flag
		 */
		virtual bool setSource(const std::string &source) = 0;

		/**
		 * @brief Closes the recordstream. This method will usually be called
		 *        from within another thread while reading data. So it must be
		 *        implemented with thread safety in mind.
		 */
		virtual void close() = 0;

		/**
		 * @brief Adds a data channel to the request. This will not yet start
		 *        the request. Some implementations may support wildcard
		 *        characters (* and ?) at any level.
		 *
		 * The time window request for this channel will be using the globally
		 * configured time window, see setStartTime(const Seiscomp::Core::Time &),
		 * setEndTime(const Seiscomp::Core::Time &) and  setTimeWindow(const Seiscomp::Core::TimeWindow &timeWindow).
		 *
		 * If addStream() is called another time with the same channel
		 * identifiers then most implementations will overwrite the previous
		 * request. In short: multiple requests (different time windows) for
		 * the same channel are not supported.
		 *
		 * @param networkCode The network code
		 * @param stationCode The station code
		 * @param locationCode The location code
		 * @param channelCode The channel code
		 * @return Status flag
		 */
		virtual bool addStream(const std::string &networkCode,
		                       const std::string &stationCode,
		                       const std::string &locationCode,
		                       const std::string &channelCode) = 0;

		/**
		 * @brief Same as addStream(const std::string &, const std::string &, const std::string &, const std::string &)
		 *        but with an additional time window for this particular channel.
		 *
		 * If addStream() is called another time with the same channel
		 * identifiers then most implementations will overwrite the previous
		 * request. In short: multiple requests (different time windows) for
		 * the same channel are not supported.
		 *
		 * @param networkCode The network code
		 * @param stationCode The station code
		 * @param locationCode The location code
		 * @param channelCode The channel code
		 * @param startTime The start time for this particular stream
		 * @param endTime The end time for this particular stream
		 * @return
		 */
		virtual bool addStream(const std::string &networkCode,
		                       const std::string &stationCode,
		                       const std::string &locationCode,
		                       const std::string &channelCode,
		                       const OPT(Core::Time) &startTime,
		                       const OPT(Core::Time) &endTime) = 0;

		/**
		 * @brief Sets the start time for all streams that haven't been
		 *        requested with a specific time window.
		 * @param startTime The start time.
		 * @return Status flag
		 */
		virtual bool setStartTime(const OPT(Core::Time) &startTime) = 0;

		/**
		 * @brief Sets the end time for all streams that haven't been
		 *        requested with a specific time window.
		 * @param endTime The end time. And invalid time is treated as open
		 *                end time and will return as much data as is available.
		 * @return Status flag
		 */
		virtual bool setEndTime(const OPT(Core::Time) &endTime) = 0;

		/**
		 * @brief Convenience function to set start time and end time.
		 * @param timeWindow The time window
		 * @return Status flag
		 */
		virtual bool setTimeWindow(const Core::TimeWindow &timeWindow);

		/**
		 * @brief Sets an optional timeout for data retrieval. If within \p seconds
		 *        seconds no data is returned then the recordstream will abort.
		 *        The default implementation return false.
		 * @param seconds The maximum number of seconds to wait for data.
		 * @return Status flag.
		 */
		virtual bool setTimeout(int seconds);

		/**
		 * @brief Sets the type of the record to be generated. Not all
		 *        implementations support this call or will just ignore it as
		 *        the type of data is defined in the data protocol. This is
		 *        most useful for files.
		 * @param type The type name. Currently the following record types are
		 *        supported:
		 *        - 'mseed'  MiniSeed
		 *        - 'ah'     AH format
		 *        - 'sac'  SAC
		 * @return Status flag. The default implementation will return false.
		 */
		virtual bool setRecordType(const char *type);


	// ------------------------------------------------------------------
	//  Data retrieval interface
	// ------------------------------------------------------------------
	public:
		/**
		 * @brief Sets the desired data type of the records returned. The
		 *        default is DOUBLE. This method must be called before
		 *        calling next().
		 * @param dataType The data type
		 */
		void setDataType(Array::DataType dataType);

		/**
		 * @brief Sets the hint how records should be created. The default
		 *        is SAVE_RAW. This method must be called before calling
		 *        next().
		 * @param hint The record creation hint
		 */
		void setDataHint(Record::Hint hint);

		/**
		 * @brief Returns the next record from the source.
		 * @return The ownership of the returned instance goes to the
		 *         caller. Iteration stops of nullptr is returned.
		 */
		virtual Record *next() = 0;


	// ------------------------------------------------------------------
	//  RecordStream static interface
	// ------------------------------------------------------------------
	public:
		/**
		 * @brief Creates a recordstream for the given service.
		 * @param service The service name
		 * @return A pointer to the recordstream object
		 *
		 *         \note
		 *         The returned pointer has to be deleted by the caller!
		 */
		static RecordStream *Create(const char *service);

		/**
		 * @brief Opens a recordstream at source. This will call @Create
		 * @param url A source URL of format [service://]address[#type],
		//!           e.g. file:///data/record.mseed#mseed. Service defaults
		//!           'file' and the default type is 'mseed'.
		 * @return A pointer to the recordstream object. If the recordstream
		 *         does not support the requested type, nullptr will be returned.
		 *
		 *         \note
		 *         The returned pointer has to be deleted by the caller!
		 */
		static RecordStream *Open(const char *url);


	// ------------------------------------------------------------------
	//  Protected interface
	// ------------------------------------------------------------------
	protected:
		// Does nothing
		virtual void handleInterrupt(int) override;

		/**
		 * @brief Helper function to set up a created record. Basically
		 *        this will set the desired data type and hint.
		 * @param rec
		 */
		void setupRecord(Record *rec);


	// ------------------------------------------------------------------
	//  Protected members
	// ------------------------------------------------------------------
	protected:
		Array::DataType  _dataType;
		Record::Hint     _hint;
};


DEFINE_INTERFACE_FACTORY(RecordStream);


#define REGISTER_RECORDSTREAM_VAR(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::IO::RecordStream, Class> __##Class##InterfaceFactory__(Service)

#define REGISTER_RECORDSTREAM(Class, Service) \
static REGISTER_RECORDSTREAM_VAR(Class, Service)


inline void RecordStream::setupRecord(Record *rec) {
	rec->setDataType(_dataType);
	rec->setHint(_hint);
}


}
}

#endif
