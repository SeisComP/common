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

%module(package="seiscomp", directors="1") datamodel
%{
#include "seiscomp/datamodel/types.h"
#include "seiscomp/datamodel/eventparameters_package.h"
#include "seiscomp/datamodel/config_package.h"
#include "seiscomp/datamodel/qualitycontrol_package.h"
#include "seiscomp/datamodel/inventory_package.h"
#include "seiscomp/datamodel/routing_package.h"
#include "seiscomp/datamodel/journaling_package.h"
#include "seiscomp/datamodel/arclinklog_package.h"
#include "seiscomp/datamodel/dataavailability_package.h"
#include "seiscomp/datamodel/realarray.h"
#include "seiscomp/datamodel/timearray.h"
#include "seiscomp/datamodel/timepdf1d.h"
#include "seiscomp/datamodel/timequantity.h"
#include "seiscomp/datamodel/creationinfo.h"
#include "seiscomp/datamodel/phase.h"
#include "seiscomp/datamodel/complexarray.h"
#include "seiscomp/datamodel/realpdf1d.h"
#include "seiscomp/datamodel/realquantity.h"
#include "seiscomp/datamodel/integerquantity.h"
#include "seiscomp/datamodel/axis.h"
#include "seiscomp/datamodel/principalaxes.h"
#include "seiscomp/datamodel/tensor.h"
#include "seiscomp/datamodel/originquality.h"
#include "seiscomp/datamodel/nodalplane.h"
#include "seiscomp/datamodel/timewindow.h"
#include "seiscomp/datamodel/waveformstreamid.h"
#include "seiscomp/datamodel/sourcetimefunction.h"
#include "seiscomp/datamodel/nodalplanes.h"
#include "seiscomp/datamodel/confidenceellipsoid.h"
#include "seiscomp/datamodel/originuncertainty.h"
#include "seiscomp/datamodel/blob.h"
#include "seiscomp/datamodel/arclinkrequestsummary.h"
#include "seiscomp/datamodel/databasereader.h"
#include "seiscomp/datamodel/databasequery.h"
#include "seiscomp/datamodel/messages.h"
#include "seiscomp/datamodel/version.h"
%}

%newobject Seiscomp::DataModel::DatabaseReader::loadEventParameters;
%newobject Seiscomp::DataModel::DatabaseReader::loadConfig;
%newobject Seiscomp::DataModel::DatabaseReader::loadQualityControl;
%newobject Seiscomp::DataModel::DatabaseReader::loadInventory;
%newobject Seiscomp::DataModel::DatabaseReader::loadRouting;
%newobject Seiscomp::DataModel::DatabaseReader::loadJournaling;
%newobject Seiscomp::DataModel::DatabaseReader::loadArclinkLog;
%newobject Seiscomp::DataModel::DatabaseReader::loadDataAvailability;

%include "datamodelbase.i"
%include "seiscomp/datamodel/types.h"

optional(Seiscomp::DataModel::RealArray);
optional(Seiscomp::DataModel::TimeArray);
optional(Seiscomp::DataModel::TimePDF1D);
optional(Seiscomp::DataModel::TimeQuantity);
optional(Seiscomp::DataModel::CreationInfo);
optional(Seiscomp::DataModel::Phase);
optional(Seiscomp::DataModel::ComplexArray);
optional(Seiscomp::DataModel::RealPDF1D);
optional(Seiscomp::DataModel::RealQuantity);
optional(Seiscomp::DataModel::IntegerQuantity);
optional(Seiscomp::DataModel::Axis);
optional(Seiscomp::DataModel::PrincipalAxes);
optional(Seiscomp::DataModel::Tensor);
optional(Seiscomp::DataModel::OriginQuality);
optional(Seiscomp::DataModel::NodalPlane);
optional(Seiscomp::DataModel::TimeWindow);
optional(Seiscomp::DataModel::WaveformStreamID);
optional(Seiscomp::DataModel::SourceTimeFunction);
optional(Seiscomp::DataModel::NodalPlanes);
optional(Seiscomp::DataModel::ConfidenceEllipsoid);
optional(Seiscomp::DataModel::OriginUncertainty);
optional(Seiscomp::DataModel::Blob);
optional(Seiscomp::DataModel::ArclinkRequestSummary);
enum(Seiscomp::DataModel::OriginUncertaintyDescription);
optional_enum(Seiscomp::DataModel::OriginUncertaintyDescription);
enum(Seiscomp::DataModel::MomentTensorStatus);
optional_enum(Seiscomp::DataModel::MomentTensorStatus);
enum(Seiscomp::DataModel::OriginDepthType);
optional_enum(Seiscomp::DataModel::OriginDepthType);
enum(Seiscomp::DataModel::OriginType);
optional_enum(Seiscomp::DataModel::OriginType);
enum(Seiscomp::DataModel::EvaluationMode);
optional_enum(Seiscomp::DataModel::EvaluationMode);
enum(Seiscomp::DataModel::EvaluationStatus);
optional_enum(Seiscomp::DataModel::EvaluationStatus);
enum(Seiscomp::DataModel::PickOnset);
optional_enum(Seiscomp::DataModel::PickOnset);
enum(Seiscomp::DataModel::MomentTensorMethod);
optional_enum(Seiscomp::DataModel::MomentTensorMethod);
enum(Seiscomp::DataModel::DataUsedWaveType);
optional_enum(Seiscomp::DataModel::DataUsedWaveType);
enum(Seiscomp::DataModel::EventDescriptionType);
optional_enum(Seiscomp::DataModel::EventDescriptionType);
enum(Seiscomp::DataModel::EventType);
optional_enum(Seiscomp::DataModel::EventType);
enum(Seiscomp::DataModel::EventTypeCertainty);
optional_enum(Seiscomp::DataModel::EventTypeCertainty);
enum(Seiscomp::DataModel::SourceTimeFunctionType);
optional_enum(Seiscomp::DataModel::SourceTimeFunctionType);
enum(Seiscomp::DataModel::PickPolarity);
optional_enum(Seiscomp::DataModel::PickPolarity);
enum(Seiscomp::DataModel::StationGroupType);
optional_enum(Seiscomp::DataModel::StationGroupType);

// package base
%include "seiscomp/datamodel/realarray.h"
%include "seiscomp/datamodel/timearray.h"
%include "seiscomp/datamodel/timepdf1d.h"
%include "seiscomp/datamodel/timequantity.h"
%include "seiscomp/datamodel/creationinfo.h"
%include "seiscomp/datamodel/phase.h"
%include "seiscomp/datamodel/complexarray.h"
%include "seiscomp/datamodel/realpdf1d.h"
%include "seiscomp/datamodel/realquantity.h"
%include "seiscomp/datamodel/integerquantity.h"
%include "seiscomp/datamodel/axis.h"
%include "seiscomp/datamodel/principalaxes.h"
%include "seiscomp/datamodel/tensor.h"
%include "seiscomp/datamodel/originquality.h"
%include "seiscomp/datamodel/nodalplane.h"
%include "seiscomp/datamodel/timewindow.h"
%include "seiscomp/datamodel/waveformstreamid.h"
%include "seiscomp/datamodel/sourcetimefunction.h"
%include "seiscomp/datamodel/nodalplanes.h"
%include "seiscomp/datamodel/confidenceellipsoid.h"
%include "seiscomp/datamodel/originuncertainty.h"
%include "seiscomp/datamodel/blob.h"
%include "seiscomp/datamodel/arclinkrequestsummary.h"

// package EventParameters
%include "seiscomp/datamodel/eventdescription.h"
%include "seiscomp/datamodel/comment.h"
%include "seiscomp/datamodel/dataused.h"
%include "seiscomp/datamodel/compositetime.h"
%include "seiscomp/datamodel/pickreference.h"
%include "seiscomp/datamodel/amplitudereference.h"
%include "seiscomp/datamodel/reading.h"
%include "seiscomp/datamodel/momenttensorcomponentcontribution.h"
%include "seiscomp/datamodel/momenttensorstationcontribution.h"
%include "seiscomp/datamodel/momenttensorphasesetting.h"
%include "seiscomp/datamodel/momenttensor.h"
%include "seiscomp/datamodel/focalmechanism.h"
%include "seiscomp/datamodel/amplitude.h"
%include "seiscomp/datamodel/stationmagnitudecontribution.h"
%include "seiscomp/datamodel/magnitude.h"
%include "seiscomp/datamodel/stationmagnitude.h"
%include "seiscomp/datamodel/pick.h"
%include "seiscomp/datamodel/originreference.h"
%include "seiscomp/datamodel/focalmechanismreference.h"
%include "seiscomp/datamodel/event.h"
%include "seiscomp/datamodel/catalog.h"
%include "seiscomp/datamodel/arrival.h"
%include "seiscomp/datamodel/origin.h"
%include "seiscomp/datamodel/eventparameters.h"

// package Config
%include "seiscomp/datamodel/parameter.h"
%include "seiscomp/datamodel/parameterset.h"
%include "seiscomp/datamodel/setup.h"
%include "seiscomp/datamodel/configstation.h"
%include "seiscomp/datamodel/configmodule.h"
%include "seiscomp/datamodel/config.h"

// package QualityControl
%include "seiscomp/datamodel/qclog.h"
%include "seiscomp/datamodel/waveformquality.h"
%include "seiscomp/datamodel/outage.h"
%include "seiscomp/datamodel/qualitycontrol.h"

// package Inventory
%include "seiscomp/datamodel/stationreference.h"
%include "seiscomp/datamodel/stationgroup.h"
%include "seiscomp/datamodel/auxsource.h"
%include "seiscomp/datamodel/auxdevice.h"
%include "seiscomp/datamodel/sensorcalibration.h"
%include "seiscomp/datamodel/sensor.h"
%include "seiscomp/datamodel/responsepaz.h"
%include "seiscomp/datamodel/responsepolynomial.h"
%include "seiscomp/datamodel/responsefap.h"
%include "seiscomp/datamodel/responsefir.h"
%include "seiscomp/datamodel/responseiir.h"
%include "seiscomp/datamodel/dataloggercalibration.h"
%include "seiscomp/datamodel/decimation.h"
%include "seiscomp/datamodel/datalogger.h"
%include "seiscomp/datamodel/auxstream.h"
%include "seiscomp/datamodel/stream.h"
%include "seiscomp/datamodel/sensorlocation.h"
%include "seiscomp/datamodel/station.h"
%include "seiscomp/datamodel/network.h"
%include "seiscomp/datamodel/inventory.h"

// package Routing
%include "seiscomp/datamodel/routearclink.h"
%include "seiscomp/datamodel/routeseedlink.h"
%include "seiscomp/datamodel/route.h"
%include "seiscomp/datamodel/access.h"
%include "seiscomp/datamodel/routing.h"

// package Journaling
%include "seiscomp/datamodel/journalentry.h"
%include "seiscomp/datamodel/journaling.h"

// package ArclinkLog
%include "seiscomp/datamodel/arclinkuser.h"
%include "seiscomp/datamodel/arclinkstatusline.h"
%include "seiscomp/datamodel/arclinkrequestline.h"
%include "seiscomp/datamodel/arclinkrequest.h"
%include "seiscomp/datamodel/arclinklog.h"

// package DataAvailability
%include "seiscomp/datamodel/datasegment.h"
%include "seiscomp/datamodel/dataattributeextent.h"
%include "seiscomp/datamodel/dataextent.h"
%include "seiscomp/datamodel/dataavailability.h"

// additional headers
%include "seiscomp/datamodel/databasereader.h"
%include "seiscomp/datamodel/databasequery.h"
%include "seiscomp/datamodel/messages.h"
%include "seiscomp/datamodel/version.h"

%include "datamodelext.i"
