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


#ifndef SC_CORE_VERSION_H
#define SC_CORE_VERSION_H


#include <string>
#include <cstdint>
#include <seiscomp/core.h>


namespace Seiscomp {
namespace Core {


/* #if (SC_API_VERSION >= SC_API_VERSION_CHECK(17, 0, 0)) */
#define SC_API_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))


/* SC_API_VERSION is (major << 16) + (minor << 8) + patch. */
#define SC_API_VERSION 0x110000

#define SC_API_VERSION_MAJOR(v) (v >> 16)
#define SC_API_VERSION_MINOR(v) ((v >> 8) & 0xff)
#define SC_API_VERSION_PATCH(v) (v & 0xff)


/******************************************************************************
 API Changelog
 ******************************************************************************
 "17.0.0"   0x110000
   - Added Seiscomp::Client::Application::handleSOH
   - Added Seiscomp::Processing::MagnitudeProcessor_MLc _c6, _H and _minDepth.
   - Added Seiscomp::Processing::AmplitudeProcessor::parameter
   - Made Seiscomp::IO::DatabaseInterface::escape a const method
   - Increased TILESTORE_API version to 5
   - Added Seiscomp::Processing::WaveformProcessor::Status enumeration PeriodOutOfRange
   - Renamed Seiscomp::Core::Time::seconds() to Time::epochSeconds()
   - Added Seiscomp::Core::Time::epoch()
   - Deprecated Seiscomp::Core::Time::valid()
   - Removed implicit Seiscomp::Core::TimeSpan and Seiscomp::Core::Time double cast
   - Removed Seiscomp::System::CommandLine::parse(inv argc, char **argv, *)
   - Added Seiscomp::System::CommandLine::parse(std::vector<std::string>, *)
   - Changed Seiscomp::Core::Time in Seiscomp::IO::RecordStream::addStream,
     Seiscomp::IO::RecordStream::setStartTime and
     Seiscomp::IO::RecordStream::setEndTime to OPT(Seiscomp::Core::Time)
   - Changed Seiscomp::Core::Time in Seiscomp::Client::StreamApplication::setStartTime
     and Seiscomp::Client::StreamApplication::setEndTime to OPT(Seiscomp::Core::Time)
   - Rename Seiscomp::Wired::ClientSession::inAvail to outputBufferSize and
     remove virtual declaration
   - Added abstract virtual function MagnitudeProcessor::setDefaults() which must
     be implemented by all derived classes.
   - Changed Seiscomp::Core::TimeWindow cast to bool semantic
   - Changed Seiscomp::Core::TimeWindow::length return type, from double to
     Seiscomp::Core::TimeSpan
   - Changed Seiscomp::Core::TimeWindow::setLength length argument from double
     to Seiscomp::Core::TimeSpan
   - Changed Seiscomp::Core::TimeWindow::equals tolerance argument from double
     to Seiscomp::Core::TimeSpan
   - Changed Seiscomp::Core::TimeWindow::contiguous tolerance argument from double
     to Seiscomp::Core::TimeSpan
   - Added Seiscomp::Core::metaValueCast
   - Removed typedef Seiscomp::Core::SmartPointer::Impl
   - Added Seiscomp::Core::SmartPointer as typedef for boost::instrusive_ptr
   - Removed typedef Seiscomp::Core::Optional::Impl
   - Added Seiscomp::Core::Optional as typedef for boost::optional
   - Added Seiscomp::Math::Geo::WGS84_MEAN_RADIUS
   - Added Seiscomp::Math::Geo::WGS84_KM_OF_DEGREE
   - Added Seiscomp::Math::Geo::WGS84_SEMI_MAJOR_AXIS
   - Added Seiscomp::Math::Geo::WGS84_FLATTENING
   - Removed defined KM_OF_DEGREE

 "16.4.0"   0x100400
   - Add Seiscomp::Math::Matrix3<T> ostream output operator
   - Add Seiscomp::Math::Vector3<T> ostream output operator
   - Add Seiscomp::Math::Matrix3<T>::operator*
   - Add Seiscomp::Math::Matrix3<T>::operator=
   - Add static Seiscomp::Math::Matrix3<T>::Rotate[X|Y|Z]
   - Add unary Seiscomp::Math::Vector3<T>::operator+
   - Add unary Seiscomp::Math::Vector3<T>::operator-
   - Add Seiscomp::Math::Vector3<T>::normalized

 "16.3.0"   0x100300
   - Added Seiscomp::Gui::Application::createCSV(view, header)
   - Added Seiscomp::Gui::RecordView::sortByText(item1, item2, item3)
   - Added Seiscomp::Gui::RecordView::sortByText(item1, item2, item3, item4)

 "16.2.0"   0x100200
   - Added Seiscomp::DataModel::PublicObject::Lock
   - Added Seiscomp::DataModel::PublicObject::UnLock

 "16.1.0"   0x100100
   - Added Seiscomp::DataModel::numberOfComponents

 "16.0.0"   0x100000
   - Added Seiscomp::Core::digits10
   - Added Seiscomp::Client::Application::setDatabaseURI
   - Added Seiscomp::Client::Application::setRecordStreamURL
   - Removed Seiscomp::Math::round
   - Renamed GenericMessage::AttachementType to GenericMessage::AttachmentType
   - Renamed GenericMessage::AttachementList to GenericMessage::AttachmentList
   - Made Seiscomp::Gui::Ruler::rulerWidth() public
   - Made Seiscomp::Gui::Ruler::rulerHeight() public
   - Made Seiscomp::Gui::Ruler::isBottom() public
   - Made Seiscomp::Gui::Ruler::isTop() public
   - Made Seiscomp::Gui::Ruler::isLeft() public
   - Made Seiscomp::Gui::Ruler::isRight() public
   - Made Seiscomp::Gui::Ruler::isHorizontal() public
   - Made Seiscomp::Gui::Ruler::isVertical() public
   - Add second parameter to Seiscomp::Gui::Ruler::setPosition(pos, allowRotation)
   - Add Seiscomp::Gui::Ruler::setReverseDirection(bool)
   - Changed KM_OF_DEGREE constant according to WGS84 mean radius definition
   - Removed Seiscomp::Gui::Application::setDatabaseSOHInterval
   - Added Seiscomp::Client::Application::stateOfHealth
   - Removed virtual from Seiscomp::Core::BaseObject::operator=
   - Removed virtual from Seiscomp::Gui::Map::Layer::operator=
   - Changed signature of Seiscomp::Processing::MagnitudeProcessor::estimateMw
   - Removed Seiscomp::DataModel::DatabaseArchive::write(object, parentID)
   - Added Seiscomp::DataModel::DatabaseArchive::insert(object, parentID)
   - Added Seiscomp::Processing::NCompsOperator<>::buffer()
   - Added Seiscomp::Processing::NCompsOperator<>::proc()
   - Added Seiscomp::Processing::NCompsOperator<>::reset()
   - Added Seiscomp::Processing::CodeWrapper<>::reset()
   - Added Seiscomp::Processing::StreamConfigWrapper<>::proc()
   - Added Seiscomp::Processing::StreamConfigWrapper<>::reset()
   - Added Seiscomp::Processing::NoOpWrapper<>::reset()
   - Added Seiscomp::Processing::FilterWrapper<>::proc()
   - Added Seiscomp::Processing::FilterWrapper<>::reset()
   - Seiscomp::Geo::GeoCoordinate::ValueType = double
   - Added Seiscomp::TravelTimeTableInterface::computeTime()
   - Added Seiscomp::TravelTime::azi field
   - Seiscomp::Math::Geo::delazi() allows null arguments
   - Added Seiscomp::Math::Geo::delta()
   - Added Seiscomp::Util::StopWatch::microseconds()
   - Added Seiscomp::Util::StopWatch::nanoseconds()

 "15.6.0"   0x0F0600
   - Added Seiscomp::Gui::PickerView::setAuxiliaryChannels

 "15.5.0"   0x0F0500
   - Added Seiscomp::Utils::TabValues

 "15.4.0"   0x0F0400
   - Added Seiscomp::Gui::RecordView::hasSelectedItems()
   - Added Seiscomp::Gui::RecordView::mapToTime()
   - Added Seiscomp::Gui::RecordView::mapToUnit()
   - Added Seiscomp::Gui::RecordView::setRubberBandSelectionEnabled()
   - Added Seiscomp::Gui::RecordView::SelectionOperation
   - Added Seiscomp::Gui::RecordView::SelectionOperationFlag
   - Added Seiscomp::Gui::RecordView::visibleTimeRange
   - Added signal Seiscomp::Gui::RecordView::selectedRubberBand()
   - Added Seiscomp::Gui::FlowLayout
   - Added Seiscomp::Gui::setBold
   - Added Seiscomp::Gui::setItalic

 "15.3.0"   0x0F0300
   - Added Seiscomp::Client::ThreadedQueue::isClosed
   - Added Seiscomp::Client::ThreadedQueue::reset

 "15.2.0"   0x0F0200
   - Added Seiscomp::Wired::peerCertificate
   - Added Seiscomp::Client::ThreadedQueue::reset
   - Added Seiscomp::DataModel::PublicObject::typeInfo

 "15.1.0"   0x0F0100
   - Added Seiscomp::MetaObject::Find
   - Added Seiscomp::DataModel::Diff4
   - Added Seiscomp::DataModel::QML_NS
   - Added Seiscomp::DataModel::QML_NS_RT
   - Added Seiscomp::DataModel::QML_NS_BED
   - Added Seiscomp::DataModel::QML_NS_BED_RT
   - Added Seiscomp::DataModel::QML_SMIPrefixEnvVar
   - Added Seiscomp::Gui::Map::TileStore::loadingComplete
   - Added Seiscomp::Gui::Map::TileStore::loadingCancelled

 "15.0.0"   0x0F0000
   - Added OriginUncertainty::confidenceLevel
   - Set DataModel version to 0.12
   - Added WindowFunc::apply(..., left, right)
   - Changed WindowFunc::process(int n, T *inout, double width)
     to WindowFunc::process(int n, T *inout, double left, double right)
   - Added locale to Seiscomp::Processing::MagnitudeProcessor::computeMagnitude
   - Removed Seiscomp::Processing::MagnitudeProcessor::correctMagnitude
   - Added signal Seiscomp::Gui::EventListView::visibleEventCountChanged()
   - Added Seiscomp::Gui::StationSymbol::setOutlineColor(QColor)
   - Removed Seiscomp::Core::Generic::Archive::read(int)
   - Removed Seiscomp::Core::Generic::Archive::write(int)
   - Removed Seiscomp::Core::Generic::Archive::read(time_t)
   - Removed Seiscomp::Core::Generic::Archive::write(time_t)
   - Removed Seiscomp::Core::Generic::Archive::read(std::vector<int>)
   - Removed Seiscomp::Core::Generic::Archive::write(std::vector<int>)
   - Added Seiscomp::Core::Generic::Archive::read(int8_t)
   - Added Seiscomp::Core::Generic::Archive::write(int8_t)
   - Added Seiscomp::Core::Generic::Archive::read(int16_t)
   - Added Seiscomp::Core::Generic::Archive::write(int16_t)
   - Added Seiscomp::Core::Generic::Archive::read(int32_t)
   - Added Seiscomp::Core::Generic::Archive::write(int32_t)
   - Added Seiscomp::Core::Generic::Archive::read(int64_t)
   - Added Seiscomp::Core::Generic::Archive::write(int64_t)
   - Added Seiscomp::Core::Generic::Archive::read(std::vector<int8_t>)
   - Added Seiscomp::Core::Generic::Archive::write(std::vector<int8_t>)
   - Added Seiscomp::Core::Generic::Archive::read(std::vector<int16_t>)
   - Added Seiscomp::Core::Generic::Archive::write(std::vector<int16_t>)
   - Added Seiscomp::Core::Generic::Archive::read(std::vector<int32_t>)
   - Added Seiscomp::Core::Generic::Archive::write(std::vector<int32_t>)
   - Added Seiscomp::Core::Generic::Archive::read(std::vector<int64_t>)
   - Added Seiscomp::Core::Generic::Archive::write(std::vector<int64_t>)
   - Added Seiscomp::Gui::Map::AnnotationItem
   - Added Seiscomp::Gui::Map::AnnotationStyle
   - Added Seiscomp::Gui::Map::Annotations
   - Added Seiscomp::Gui::Map::AnnotationLayer
   - Added Seiscomp::Core::Time::utc()
   - Added Seiscomp::Core::Time::UTC()
   - Added Seiscomp::Core::Time::toUTC()
   - Changed signature from
     Seiscomp::Core::trimBack(char *&data, size_t &len) to
     Seiscomp::Core::trimBack(char *data, size_t &len)
   - Set TileStore API to version 3 which is incompatible with previous versions
   - Remove Seiscomp::Gui::Alg::MapTree and Seiscomp::Gui::Alg::MapTreeNode
   - Added Seiscomp::DataModel::id(const Network*, ...)
   - Added Seiscomp::DataModel::id(const Station*, ...)
   - Added Seiscomp::DataModel::id(const SensorLocation*, ...)
   - Added Seiscomp::DataModel::id(const Stream*, ...)
   - Added virtual Seiscomp::Wired::Session::accepted()
   - Added Seiscomp::Wired::Socket::isAccepted()
   - Added Seiscomp::Util::catchBool
   - Fixed Python API for ExportSink::write to always pass bytes and
     remove size parameter
   - Added Regions::getFlinnEngdahlRegion
   - Removed public access of Regions constructor
   - Changed RecordWidget::Filter from float to double
   - Changed SLOT Gui::RecordWidget::setScale(double, float) to
             SLOT Gui::RecordWidget::setScale(double, double)
   - Changed SLOT Gui::RecordWidget::setAmplScale(float) to
             SLOT Gui::RecordWidget::setAmplScale(double)
   - Changed SIGNAL Gui::RecordView::scaleChanged(double, float) to
                    Gui::RecordView::scaleChanged(double, double)
   - Changed SIGNAL Gui::RecordView::amplScaleChanged(float) to
                    Gui::RecordView::amplScaleChanged(double)
   - Changed SLOT Gui::RecordView::setScale(double, float) to
             SLOT Gui::RecordView::setScale(double, double)

 "14.4.0"   0x0E0400
   - Added class Seiscomp::Core::Number<T> (ostream output)
   - Added Seiscomp::Core::number<T>() (Number<T> generator)
   - Added Seiscomp::Processing::Picker::noiseWindow()
   - Added Seiscomp::Processing::Picker::signalWindow()
   - Added Seiscomp::DataModel::PublicObjectCache::contains(PublicObject*)
   - Added Seiscomp::DataModel::PublicObjectCache::contains(string)

 "14.3.0"   0x0E0300
   - Added Seiscomp::RingBuffer::numberOfRecordsToStore()
   - Added Seiscomp::RingBuffer::timeSpanToStore()
   - Added Seiscomp::TimeWindowBuffer::timeWindowToStore()
   - Added Seiscomp::Gui::OriginLocatorMap::stationSymbolToolTip()
   - Added Seiscomp::Gui::MagnitudeMap::stationSymbolToolTip()
   - Added Seiscomp::Environment::resolvePath()
   - Added Seiscomp::Wired::Device::takeFd()
   - Added Seiscomp::Wired::SSLSocket::take()
   - Added Seiscomp::IO::Socket::takeFd()
   - Added Seiscomp::IO::SSLSocket::setFd()
   - Added Seiscomp::Geo::GeoFeature::setAttribute()
   - Added Seiscomp::Geo::GeoFeatureSet::readFile()
   - Added Seiscomp::Geo::GeoFeatureSet::readDir()
   - Added Seiscomp::Geo::readGeoJSON()
   - Deprecated Seiscomp::Geo::GeoFeatureSet::readBNAFile()
   - Deprecated Seiscomp::Geo::GeoFeatureSet::readBNADir()
   - Added Seiscomp::Gui::CM_SHOW_NOTIFICATION enumeration

 "14.2.0"   0x0E0200
   - Added Seiscomp::Client::Application::injectMessage
   - Added RecordStream factory "caps" and "capss"

 "14.1.0"   0x0E0100
   - Added Seiscomp::Wired::Session::handleTimeout
   - Declared Seiscomp::Geo::GeoFeature::area as deprecated
   - Added function Seiscomp::Geo::area which replaces
     Seiscomp::Geo::GeoFeature::area
   - Added functions Seiscomp::Geo::contains
   - Added Seiscomp::Math::Filtering::IIR::ButterworthBandstop
   - Added another Seiscomp::Math::Filtering::cosRamp function
   - Added Seiscomp::Gui::AdvancedOriginSymbol

 "14.0.0"   0x0E0000
   - Added length based string functions to Seiscomp::Core:
     - tokenize
     - tokenize2
     - trimFront
     - trimBack
     - trim
     - strnchr
     - advance
   - Added seiscomp/wire package
   - Moved Client::Commandline to System::Commandline
   - Moved Client::PluginRegistry to System::PluginRegistry
   - Moved all functions from daemon.h from namespace Utils to namespace System
   - Added System::Application
   - Removed Core::Alarmable
   - Removed RecordStream::ODCArchive
   - Removed Seiscomp::Client::Application::sync
   - Removed Seiscomp::Client::Application::requestSync
   - Removed Seiscomp::Client::Application::handleStartSync
   - Removed Seiscomp::Client::Application::handleEndSync
   - Removed Seiscomp::Client::Application::handleSync
   - Renamed Seiscomp::Client::Connection::send to Seiscomp::Client::Connection::sendMessage
   - Added Gui::Map::MercatorProjection
   - Renamed include directories from seiscomp3/ to seiscomp/

 "13.0.0"   0x0D0000
   - Added virtual Seiscomp::IO::Database::escape
   - Renamed Seiscomp::Array::bytes() to elementSize()
   - Added enums ZEP and REP to Seiscomp::Core::GreensfunctionComponent
   - Added Seiscomp::DataModel::touch(obj)
   - Changed database oid type to 64bit
   - Added Record::authentication and Record::authority
   - Seiscomp::Gui::Application does not inherit from QApplication anymore
   - Added Seiscomp::Gui::Map::Symbol::layerVisibilityChanged
   - Added Seiscomp::Gui::Map::Layer::setToolTip
   - Added Seiscomp::Gui::Map::Layer::toolTip
   - Added Seiscomp::Gui::Map::Layer::hasCursorShape
   - Added Seiscomp::Gui::Map::Layer::cursorShape
   - Added Seiscomp::Gui::Map::Layer::setCursorShape
   - Added Seiscomp::Gui::Map::Layer::unsetCursorShape
   - Added Seiscomp::Gui::Map::Layer::update
   - Added Seiscomp::Gui::Axis::ticks
   - Added Seiscomp::Gui::Axis::subTicks
   - Declared Seiscomp::Gui::Axis::updateLayout, sizeHint, draw and drawGrid virtual
   - Declared Seiscomp::Gui::Graph::unproject virtual

 "12.3.0"   0x0C0300
   - Added ArtificialEventParametersMessage

 "12.2.0"   0x0C0200
   - Added Application::waitEvent

 "12.1.1"   0x0C0101
   - Fixed RecordWidget::mouseMoveEvent to not ignore handled events
   - Fixed libtau wrapper setModel initialization bug

 "12.1.0"   0x0C0100
   - Fixed RecordWidget emitting of traceUpdated signal if the record slot to
     be shown has changed
   - Added non-const RecordWidget::traceInfo method
   - Added RecordWidget::recordPen(int) method

 "12.0.0"   0x0C0000
   - Added Seiscomp::Core::Generic::Archive property interface
   - Added Seiscomp::DataModel::DatabaseQuery::getAmplitudes
   - Changed Seiscomp::DataModel::DatabaseArchive::toString() from protected to
     public const
   - Added Seiscomp::Core::Time::localTimeZoneOffset()
   - Removed geo prefix of all headers under <seiscomp/geo/>
   - Added Seiscomp::Util::UnitConverter
   - Added Seiscomp::Processing::WaveformProcessor::Status enumeration TravelTimeEstimateFailed
   - Added Seiscomp::Processing::MagnitudeProcessor::Status enumeration InvalidAmplitudeUnit
   - Added Seiscomp::Processing::MagnitudeProcessor::Status enumeration ReceiverOutOfRegions
   - Added Seiscomp::Processing::MagnitudeProcessor::Status enumeration RayPathOutOfRegions
   - Added Seiscomp::Processing::MagnitudeProcessor::Status enumeration MissingAmplitudeObject
   - Added Geo::GeoFeature::updateBoundingBox method
   - Added static method Seiscomp::Geo::GeoFeatureSet::load
   - Added class Seiscomp::Geo::GeoFeatureSetObserver
   - Added Seiscomp::Gui::Map::StandardLegend::clear
   - Added Seiscomp::Gui::Map::StandardLegend::count
   - Added Seiscomp::Gui::Map::StandardLegend::itemAt
   - Added Seiscomp::Gui::Map::StandardLegend::takeItem
   - Added Seiscomp::Gui::EventLegend
   - Removed Seiscomp::Gui::LayerProperties
   - Removed Seiscomp::Gui::Map::Canvas::drawGeoFeature(LayerProperties ...)
   - Added QPainter reference as 2nd parameter to Seiscomp::Gui::Map::Layer::bufferUpdated
   - Added QPainter reference as 2nd parameter to Seiscomp::Gui::Map::Layer::baseBufferUpdated
   - Added virtual Seiscomp::Gui::Map::Projection::project(QPainterPath, ...) method
   - Added Seiscomp::Gui::Map::Projection::boundingBox() method
   - Added enum Seiscomp::Gui::Map::FilterMode
   - Changed prototype of Seiscomp::Gui::Map::Canvas::drawImage and add filterMode
     parameter
   - Renamed Seiscomp::Gui::Canvas::drawGeoLine to Seiscomp::Gui::Canvas::drawLine
   - Renamed Seiscomp::Gui::Canvas::drawGeoPolyline to Seiscomp::Gui::Canvas::drawPolyline
   - Renamed Seiscomp::Gui::Canvas::drawGeoPolygon to Seiscomp::Gui::Canvas::drawPolygon
   - Renamed Seiscomp::Gui::Canvas::drawGeoFeature to Seiscomp::Gui::Canvas::drawFeature
   - Seiscomp::Gui::Scheme::colors.records.gaps/overlaps is now a brush rather than
     a color
   - Added Seiscomp::Gui::Plot::addAxis
   - Added Seiscomp::Processing::MagnitudeProcessor::Status enumeration IncompleteConfiguration
   - Added Seiscomp::Processing::AmplitudeProcessor::setEnvironment
   - Added Seiscomp::Processing::AmplitudeProcessor::finalizeAmplitude
   - Added amplitude to Seiscomp::Processing::MagnitudeProcessor::computeMagnitude
   - Added unit to Seiscomp::Processing::MagnitudeProcessor::computeMagnitude
   - Added Seiscomp::Processing::MagnitudeProcessor::treatAsValidMagnitude()
   - Added Seiscomp::IO::Exporter::put(std::streambuf* buf, const ObjectList &objects);
   - Added Seiscomp::Gui::RecordMarker::drawBackground
   - Made Seiscomp::Client::Application::version public
   - Changed Seiscomp::Gui::Scheme::colors.map.grid from QColor to QPen
   - Added Seiscomp::Gui::SpectrogramRenderer::range
   - Added parameter stretch to Seiscomp::Gui::SpectrogramRenderer::renderAxis
   - Added overloaded methods to Seiscomp::Gui::Axis::sizeHint and
     Seiscomp::Gui::Axis::updateLayer that only use QFontMetrics

 "11.1.0"   0x0B0100
   - Added Seiscomp::DataModel::StrongMotion::Rupture::_strike
   - Added Seiscomp::Gui::Map::StandardLegend

 "11.0.0"   0x0B0000
   - Remove dynamic type throw declarations from all methods as this is
     deprecated in current C++ standard
   - Added Seiscomp::Gui::Axis::setPen/setGridPen/setSubGridPen
   - Added Seiscomp::Gui::Map::Layer::canvas method to access the parent canvas
   - Added Seiscomp::Gui::Map::Canvas::filterMouseReleaseEvent
   - Added Seiscomp::Gui::Map::Canvas::size
   - Changed Seiscomp::Gui::Map::Canvas::menu parent parameter type from QWidget to QMenu
   - Changed Seiscomp::Gui::Map::Layer::menu parent parameter type from QWidget to QMenu
   - Removed Seiscomp::Gui::Map::Layer RTTI interface
   - Added Seiscomp::Gui::Map::Layer::show
   - Added Seiscomp::Gui::Map::Layer::hide
   - Added Seiscomp::Gui::Map::Layer::baseBufferUpdated
   - Added Seiscomp::Gui::Map::Layer::size
   - Added Seiscomp::Gui::Map::Layer::isInside
   - Added Seiscomp::Gui::Map::Layer::handleEnterEvent
   - Added Seiscomp::Gui::Map::Layer::handleLeaveEvent
   - Added Seiscomp::Gui::Map::Layer::filterMouseMoveEvent
   - Added Seiscomp::Gui::Map::Layer::filterMouseDoubleClickEvent
   - Added Seiscomp::Gui::Map::Layer::filterMousePressEvent
   - Added Seiscomp::Gui::Map::Layer::filterMouseReleaseEvent
   - Added Seiscomp::Gui::Map::Legend::contextResizeEvent
   - Removed virtual declaration of Seiscomp::Gui::Map::Legend::size
   - Removed class Seiscomp::Gui::Map::CanvasDelegate
   - Added class Seiscomp::Gui::EventLayer
   - Added Seiscomp::Gui::OriginSymbol::setColor
   - Added Seiscomp::Gui::OriginSymbol::color
   - Added Seiscomp::Gui::OriginSymbol::setFillColor
   - Added Seiscomp::Gui::OriginSymbol::fillColor
   - Added Seiscomp::Gui::MapWidget::setDrawLegends
   - Added Seiscomp::Gui::RecordView::setMaximumRowHeight
   - Added Seiscomp::Gui::RecordView::setRelativeRowHeight
   - Added Seiscomp::Gui::Application::messageGroups
   - Added Seiscomp::Gui::Application::initLicense
   - Added Seiscomp::Gui::LUT::operator[]
   - Added class Seiscomp::Math::Filtering::Min<T>
   - Added class Seiscomp::Math::Filtering::Max<T>
   - Added Seiscomp::Gui::RecordWidget::setGridVSpacing
   - Added Seiscomp::Gui::RecordWidget::setGridVRange
   - Added Seiscomp::Gui::RecordWidget::setGridVScale
   - Added Seiscomp::Gui::AbstractLegend
   - Added Seiscomp::Gui::Plot::addGraph(graph)
   - Added Seiscomp::Gui::Plot::setLegend
   - Added Seiscomp::Gui::Plot::isInsidePlot
   - Added Seiscomp::Gui::Plot::plotRect
   - Added virtual Seiscomp::Gui::Graph::draw
   - Added virtual Seiscomp::Gui::Graph::drawSymbol
   - Added Seiscomp::Gui::Graph::setName/name
   - Added Seiscomp::Client::Application::reloadBindings
   - Increased datamodel version to 0.10
   - Added class Seiscomp::DataModel::ResponseIIR
   - Inherit class Seiscomp::DataModel::Stream from Seiscomp::DataModel::PublicObject
   - Added hypocenter and receiver to Seiscomp::Processing::MagnitudeProcessor::computeMagnitude
   - Added Seiscomp::Processing::MagnitudeProcessor::Status enumeration EpicenterOutOfRegions
   - Add SC3_LOCATOR_INTERFACE_VERSION define and initialize with version 2
   - Replace LocatorInterface WeightedPick with PickItem
   - Refactored Seiscomp::IO::RecordStream interface
   - Expose PublicObject::registerMe and PublicObject::deregisterMe as public
     methods

 "10.0.0"   0x0A0000
   - Added Seiscomp::Core::Time::LocalTimeZone()
   - Added Seiscomp::IO::GFArchive::getTravelTime(...)
   - Added Seiscomp::Math::WindowFunc and several implementations
   - Changed Seiscomp::Util::Bindings::getKeys to const
   - Added Seiscomp::Gui::Map:Canvas::prependLayer(...)
   - Added Seiscomp::Gui::Map:Canvas::insertLayerBefore(...)
   - Fixed bug in Seiscomp::Gui::Map::TextureCache that affected custom
     Seiscomp::Gui::Map::TileStore implementations
   - Added Seiscomp::Gui::RecordView::coveredTimeRange()
   - Added Seiscomp::Core::stringify(...)
   - Added Seiscomp::Gui::timeToString()
   - Added Seiscomp::Gui::timeToLabel(...)
   - Added Seiscomp::Gui::MapWidget::setGrayScale(...)
   - Added Seiscomp::Gui::Map::Layer::bufferUpdated(...)
   - Added Seiscomp::Gui::Map::CompositionMode (Source, SourceOver, Multiply)
   - Changed prototype of Seiscomp::Gui::Map::Canvas::drawImage(..., +CompositionMode)
   - Changed prototype of Seiscomp::Gui::Map::Projection::drawImage(..., +CompositionMode)
   - Reworked Seiscomp::Gui::Map::Symbol and add geodetic location and screen
     position attributes (Symbol API v2)
   - Add default implementation of Seiscomp::Gui::Map::Symbol::calculateMapPosition

 "9.1.0"   0x090100
   - Added Seiscomp::Client::Application::Stage enum PLUGINS
   - Added Seiscomp::Gui::TensorRenderer::renderNP
   - Fixed parameter const'ness of Seiscomp::Math::Tensor2S

 "9.0.0"   0x090000
   - Added member creationInfo to class Seiscomp::DataModel::ConfigStation
     and increased datamodel version to 0.9
   - Added optional error code to Seiscomp::Communication class methods
   - Changed internal Seiscomp::Util::Timer API
   - Added Seiscomp::Util::Timer::setTimeout2
   - Added Seiscomp::Core::Archive::setStrictMode
   - Added Seiscomp::Core::Archive::isStrictMode
   - Added Seiscomp::IO::DatabaseInterface::numberOfAffectedRows
   - Added optional error code to Seiscomp::Inventory class
   - Added macro REREGISTER_CLASS which allows to overwrite class registrations
   - Added method Seiscomp::Gui::Alg::MapTreeNode::parent()
   - Allow Seiscomp::Gui::Map::TileStore::load to return null images
   - Increased TILESTORE version to 2

 "8.0.0"   0x080000
   - Added class Seiscomp::DataModel::ResponseFAP
   - Changed Seiscomp::IO::GFArchive::addRequest from 1D request signature to
     3D request signature (distance,depth) -> (source,receiver)
   - Made Seiscomp::RecordStream::SDSArchive private members and methods
     protected
   - Added GUI plotting library

 "7.0.0"   0x070000
   - Added support for httpmsgbus messaging protocol
   - Added Seiscomp::IO::HttpSocket
   - Added Seiscomp::Communication::NetworkInterface::setSequenceNumber
   - Added Seiscomp::Communication::NetworkInterface::getSequenceNumber
   - Added Seiscomp::Communication::SystemConnection::setSequenceNumber
   - Added Seiscomp::Communication::SystemConnection::getSequenceNumber
   - Modify Seiscomp::IO::Socket don't start timeout stopwatch automatically

 "6.1.0"   0x060100
   - Added Seiscomp::Gui::RecordWidget::setRecordStepFunction

 "6.0.0"   0x060000
   - Added virtual method Seiscomp::Record::clipMask()

 "5.1.0"   0x050100
   - Added Seiscomp::Core::BitSet

 "5.0.1"   0x050001
   - Renamed seiscomp/math/filtering/rca.h to seiscomp/math/filtering/average.h
     and made it a "real" average instead of an average of absolute values

 "5.0.0"   0x050000
   - Removed Seiscomp::Core::RecordSequence::continuousRecord(...)
   - Removed Seiscomp::Core::RecordSequence::fillGaps
   - Added template method Seiscomp::Core::RecordSequence::continuousRecord<T>(...)
   - Added Seiscomp::Processing::Picker::Config::noiseBegin
   - Added Seiscomp::Gui::Ruler::pixelPerUnit
   - Added Seiscomp::Gui::Ruler::scale

 "4.0.0"   0x040000
   - Added Seiscomp::System::ConfigDelegate::aboutToWrite
   - Added Seiscomp::System::ConfigDelegate::finishedWriting
   - Added Seiscomp::System::ConfigDelegate::hasWriteError
   - Added Seiscomp::System::ConfigDelegate::handleWriteTimeMismatch
   - Added delegate parameter to Seiscomp::System::*::writeConfig
   - Added Seiscomp::Gui::RecordStreamThread::setRecordHint
   - Added Seiscomp::Client::StreamApplication::setRecordDatatype
   - Added Seiscomp::Client::StreamApplication::recordDataType
   - Added Seiscomp::IO::DatabaseInterface::getRowFieldName virtual abstract
     method
   - Fixed Gui::RecordPolyline rendering with large offset
   - Added Seiscomp::Logging::Output::setUTCEnabled
   - Added Seiscomp::IO::QuakeLink::Response::disposed field

 "3.1.0"   0x030100
   - Change private to protected access in Seiscomp::IO::QuakeLink::Connection

 "3.0.0"   0x030000
   - Added Seiscomp::IO::RecordStream::filterRecord virtual method
   - Fixed bug in Seiscomp::IO::QuakeLink::Connection
   - Added Processing::Picker::Result::polarity

 "2.5.0"   0x020500
   - Added Seiscomp::Client::Application::configGetPath

 "2.4.0"   0x020400
   - Added Seiscomp::IO::BSONArchive
   - Added Seiscomp::IO::JSONArchive

 "2.3.0"   0x020300
   - Added Seiscomp::DataModel::Object::setLastModifiedInArchive
   - Added Seiscomp::DataModel::Object::lastModifiedInArchive
   - Populate Seiscomp::DataModel::Object::_lastModifiedInArchive
     with Seiscomp::DataModel::DatabaseArchive

 "2.2.0"   0x020200
   - Added optimization flag to Seiscomp::Gui::RecordPolyline::create
   - Added Seiscomp::Gui::RecordWidget::setRecordOptimization
   - Added Seiscomp::Gui::RecordWidget::traceInfo
   - Added Seiscomp::Gui::RecordWidget::areScaledValuesShown
   - Implement Seiscomp::Gui::Ruler::sizeHint for vertical layout

 "2.1.0"   0x020100
   - Removed Seiscomp::MultiComponentArray

 "2.0.0"   0x020000
   - Moved Processing::Parameters to Util::KeyValues
   - Renamed Processing::Parameters::readFrom to Util::KeyValues::init
   - Added Util::Bindings class

 "1.16.0"  0x011000
   - Added Seiscomp::IO::Spectralizer
   - Added Seiscomp::Gui::LUT
   - Added Seiscomp::Gui::StaticLUT
   - Added Seiscomp::Gui::StaticColorLUT
   - Added Seiscomp::Gui::SpectrogramRenderer
   - Added Seiscomp::Gui::SpectrogramWidget

 "1.15.0"  0x010F00
   - Added Seiscomp::Math::Geo::delazi_wgs84

 "1.14.0"  0x010E00
   - Changed return type of Seiscomp::DataModel::getThreeComponents to bool

 "1.13.0"  0x010D00
   - Changed Seiscomp::DataModel::DiffMerge::compareNonArrayProperty signature
     from Object* to BaseObject *
   - Added method Seiscomp::DataModel::DatabaseArchive::parentPublicID(...)

 "1.12.0"  0x010C00
   - Set Seiscomp::Seismology::LocatorInterface::locate(...) initTime to const

 "1.11.0"  0x010B00
   - Added const Record * as first parameter to
     Seiscomp::Processing::NCompsOperator<...>::Proc::operator()

 "1.10.0"  0x010A00
   - Derive Seiscomp::IO::QuakeLink::Connection from BaseObject

 "1.9.0"   0x010900
   - Renamed Seiscomp::Client::Application::version to
             Seiscomp::Client::Application::frameworkVersion
   - Added Seiscomp::Client::Application::version virtual method
   - Added Seiscomp::Client::Application::reloadInventory method

 "1.8.0"   0x010800
   - Added Seiscomp::Client::Application::reloadInventory method

 "1.7.0"   0x010700
   - Added Seiscomp::Processing::Picker::filterID method
   - Added Seiscomp::Processing::SecondaryPicker::filterID method

 "1.6.0"   0x010600
   - Added Seiscomp::IO::QuakeLink keep alive option

 "1.5.0"   0x010500
   - Added Seiscomp::Core::TimeSpan::MinTime and
           Seiscomp::Core::TimeSpan::MaxTime

 "1.4.0"   0x010400
   - Added clone method to Seiscomp::IO::RecordFilterInterface
   - Added Seiscomp::IO::RecordDemuxFilter

 "1.3.1"   0x010301
   - Fixed severe bug in Seiscomp::RecordStream::Decimation

 "1.3.0"   0x010300
   - Added Seiscomp::IO::RecordFilterInterface
     Added Seiscomp::IO::RecordIIRFilter
   - Added Seiscomp::IO::RecordResampler

 "1.2.0"   0x010200
   - Added Diff2 class to Seiscomp::DataModel
   - Added canPush, canPop methods to ThreadedQueue
   - Added acquititionError signal to RecordStreamThread

 "1.1.0"   0x010100
   - Added function DataModel::PublicObjectCache::cached()
   - Added function Gui::ImageTree::hasPendingRequests()
   - Added function Gui::Canvas::reload()
   - Added function Gui::Canvas::renderingComplete()
   - Added signal   Gui::Canvas::renderingCompleted()
   - Added function Gui::TextureCache::invalidateTexture(...)
 */


template <std::uint16_t major, std::uint8_t minor, std::uint8_t patch>
class VersionPacker {
	public:
		enum { Value = major << 0x10 | minor << 0x08 | patch };
};


class SC_SYSTEM_CORE_API Version {
	// ----------------------------------------------------------------------
	// Traits
	// ----------------------------------------------------------------------
	public:
		typedef uint32_t PackType;
		typedef uint16_t MajorType;
		typedef uint8_t  MinorType;
		typedef uint8_t  PatchType;


	// ----------------------------------------------------------------------
	// Xstruction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		Version(PackType packed_version = 0) : packed(packed_version) {}
		Version(MajorType major, MinorType minor, PatchType patch = 0) {
			packed = pack(major, minor, patch);
		}


	// ----------------------------------------------------------------------
	// Public interface
	// ----------------------------------------------------------------------
	public:
		MajorType majorTag() const { return packed >> 0x10; }
		MinorType minorTag() const { return (packed >> 0x08) & 0xff; }
		PatchType patchTag() const { return packed & 0xff; }

		PackType majorMinor() const { return packed & ~0xff; }

		std::string toString() const;
		bool fromString(const std::string &str);

		static PackType pack(MajorType major, MinorType minor, PatchType patch = 0) {
			return (major << 0x10) | (minor << 0x08) | patch;
		}


	// ----------------------------------------------------------------------
	// Operators
	// ----------------------------------------------------------------------
	public:
		// Operators
		bool operator==(const Version &other) const { return packed == other.packed; }
		bool operator!=(const Version &other) const { return packed != other.packed; }
		bool operator<(const Version &other) const { return packed < other.packed; }
		bool operator>(const Version &other) const { return packed > other.packed; }
		bool operator<=(const Version &other) const { return packed <= other.packed; }
		bool operator>=(const Version &other) const { return packed >= other.packed; }


	// ----------------------------------------------------------------------
	// Public members
	// ----------------------------------------------------------------------
	public:
		PackType packed;
};


class SC_SYSTEM_CORE_API FrameworkVersion {
	public:
		FrameworkVersion();

		//! Returns the version string
		std::string toString() const;

		//! Returns additional system build information
		std::string systemInfo() const;

		const Version &version() const;
		const Version &api() const;


	private:
		Version     _version;
		Version     _api;
		std::string _release;
};

extern FrameworkVersion CurrentVersion;


}
}


#endif
