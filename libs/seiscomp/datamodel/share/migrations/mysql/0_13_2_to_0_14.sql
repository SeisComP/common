SELECT 'Drop foreign key contraints of all model tables' AS '';
ALTER TABLE EventDescription DROP FOREIGN KEY IF EXISTS EventDescription_ibfk_1;
ALTER TABLE Comment DROP FOREIGN KEY IF EXISTS Comment_ibfk_1;
ALTER TABLE DataUsed DROP FOREIGN KEY IF EXISTS DataUsed_ibfk_1;
ALTER TABLE CompositeTime DROP FOREIGN KEY IF EXISTS CompositeTime_ibfk_1;
ALTER TABLE PickReference DROP FOREIGN KEY IF EXISTS PickReference_ibfk_1;
ALTER TABLE AmplitudeReference DROP FOREIGN KEY IF EXISTS AmplitudeReference_ibfk_1;
ALTER TABLE Reading DROP FOREIGN KEY IF EXISTS Reading_ibfk_1;
ALTER TABLE MomentTensorComponentContribution DROP FOREIGN KEY IF EXISTS MomentTensorComponentContribution_ibfk_1;
ALTER TABLE MomentTensorStationContribution DROP FOREIGN KEY IF EXISTS MomentTensorStationContribution_ibfk_1;
ALTER TABLE MomentTensorPhaseSetting DROP FOREIGN KEY IF EXISTS MomentTensorPhaseSetting_ibfk_1;
ALTER TABLE MomentTensor DROP FOREIGN KEY IF EXISTS MomentTensor_ibfk_1;
ALTER TABLE FocalMechanism DROP FOREIGN KEY IF EXISTS FocalMechanism_ibfk_1;
ALTER TABLE Amplitude DROP FOREIGN KEY IF EXISTS Amplitude_ibfk_1;
ALTER TABLE StationMagnitudeContribution DROP FOREIGN KEY IF EXISTS StationMagnitudeContribution_ibfk_1;
ALTER TABLE Magnitude DROP FOREIGN KEY IF EXISTS Magnitude_ibfk_1;
ALTER TABLE StationMagnitude DROP FOREIGN KEY IF EXISTS StationMagnitude_ibfk_1;
ALTER TABLE Pick DROP FOREIGN KEY IF EXISTS Pick_ibfk_1;
ALTER TABLE OriginReference DROP FOREIGN KEY IF EXISTS OriginReference_ibfk_1;
ALTER TABLE FocalMechanismReference DROP FOREIGN KEY IF EXISTS FocalMechanismReference_ibfk_1;
ALTER TABLE Event DROP FOREIGN KEY IF EXISTS Event_ibfk_1;
ALTER TABLE Arrival DROP FOREIGN KEY IF EXISTS Arrival_ibfk_1;
ALTER TABLE Origin DROP FOREIGN KEY IF EXISTS Origin_ibfk_1;
ALTER TABLE Parameter DROP FOREIGN KEY IF EXISTS Parameter_ibfk_1;
ALTER TABLE ParameterSet DROP FOREIGN KEY IF EXISTS ParameterSet_ibfk_1;
ALTER TABLE Setup DROP FOREIGN KEY IF EXISTS Setup_ibfk_1;
ALTER TABLE ConfigStation DROP FOREIGN KEY IF EXISTS ConfigStation_ibfk_1;
ALTER TABLE ConfigModule DROP FOREIGN KEY IF EXISTS ConfigModule_ibfk_1;
ALTER TABLE QCLog DROP FOREIGN KEY IF EXISTS QCLog_ibfk_1;
ALTER TABLE WaveformQuality DROP FOREIGN KEY IF EXISTS WaveformQuality_ibfk_1;
ALTER TABLE Outage DROP FOREIGN KEY IF EXISTS Outage_ibfk_1;
ALTER TABLE StationReference DROP FOREIGN KEY IF EXISTS StationReference_ibfk_1;
ALTER TABLE StationGroup DROP FOREIGN KEY IF EXISTS StationGroup_ibfk_1;
ALTER TABLE AuxSource DROP FOREIGN KEY IF EXISTS AuxSource_ibfk_1;
ALTER TABLE AuxDevice DROP FOREIGN KEY IF EXISTS AuxDevice_ibfk_1;
ALTER TABLE SensorCalibration DROP FOREIGN KEY IF EXISTS SensorCalibration_ibfk_1;
ALTER TABLE Sensor DROP FOREIGN KEY IF EXISTS Sensor_ibfk_1;
ALTER TABLE ResponsePAZ DROP FOREIGN KEY IF EXISTS ResponsePAZ_ibfk_1;
ALTER TABLE ResponsePolynomial DROP FOREIGN KEY IF EXISTS ResponsePolynomial_ibfk_1;
ALTER TABLE ResponseFAP DROP FOREIGN KEY IF EXISTS ResponseFAP_ibfk_1;
ALTER TABLE ResponseFIR DROP FOREIGN KEY IF EXISTS ResponseFIR_ibfk_1;
ALTER TABLE ResponseIIR DROP FOREIGN KEY IF EXISTS ResponseIIR_ibfk_1;
ALTER TABLE DataloggerCalibration DROP FOREIGN KEY IF EXISTS DataloggerCalibration_ibfk_1;
ALTER TABLE Decimation DROP FOREIGN KEY IF EXISTS Decimation_ibfk_1;
ALTER TABLE Datalogger DROP FOREIGN KEY IF EXISTS Datalogger_ibfk_1;
ALTER TABLE AuxStream DROP FOREIGN KEY IF EXISTS AuxStream_ibfk_1;
ALTER TABLE Stream DROP FOREIGN KEY IF EXISTS Stream_ibfk_1;
ALTER TABLE SensorLocation DROP FOREIGN KEY IF EXISTS SensorLocation_ibfk_1;
ALTER TABLE Station DROP FOREIGN KEY IF EXISTS Station_ibfk_1;
ALTER TABLE Network DROP FOREIGN KEY IF EXISTS Network_ibfk_1;
ALTER TABLE RouteArclink DROP FOREIGN KEY IF EXISTS RouteArclink_ibfk_1;
ALTER TABLE RouteSeedlink DROP FOREIGN KEY IF EXISTS RouteSeedlink_ibfk_1;
ALTER TABLE Route DROP FOREIGN KEY IF EXISTS Route_ibfk_1;
ALTER TABLE Access DROP FOREIGN KEY IF EXISTS Access_ibfk_1;
ALTER TABLE JournalEntry DROP FOREIGN KEY IF EXISTS JournalEntry_ibfk_1;
ALTER TABLE ArclinkUser DROP FOREIGN KEY IF EXISTS ArclinkUser_ibfk_1;
ALTER TABLE ArclinkStatusLine DROP FOREIGN KEY IF EXISTS ArclinkStatusLine_ibfk_1;
ALTER TABLE ArclinkRequestLine DROP FOREIGN KEY IF EXISTS ArclinkRequestLine_ibfk_1;
ALTER TABLE ArclinkRequest DROP FOREIGN KEY IF EXISTS ArclinkRequest_ibfk_1;
ALTER TABLE DataSegment DROP FOREIGN KEY IF EXISTS DataSegment_ibfk_1;
ALTER TABLE DataAttributeExtent DROP FOREIGN KEY IF EXISTS DataAttributeExtent_ibfk_1;
ALTER TABLE DataExtent DROP FOREIGN KEY IF EXISTS DataExtent_ibfk_1;
ALTER TABLE PublicObject DROP FOREIGN KEY IF EXISTS PublicObject_ibfk_1;

SELECT 'Create Catalog table' AS '';
CREATE TABLE Catalog (
       _oid BIGINT(20) NOT NULL,
       _parent_oid BIGINT(20) NOT NULL,
       _last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
       name VARCHAR(255) NOT NULL,
       description LONGTEXT,
       creationInfo_agencyID VARCHAR(64),
       creationInfo_agencyURI VARCHAR(255),
       creationInfo_author VARCHAR(128),
       creationInfo_authorURI VARCHAR(255),
       creationInfo_creationTime DATETIME,
       creationInfo_creationTime_ms INTEGER,
       creationInfo_modificationTime DATETIME,
       creationInfo_modificationTime_ms INTEGER,
       creationInfo_version VARCHAR(64),
       creationInfo_used TINYINT(1) NOT NULL DEFAULT '0',
       start DATETIME NOT NULL,
       start_ms INTEGER NOT NULL,
       end DATETIME,
       end_ms INTEGER,
       dynamic TINYINT(1) NOT NULL,
       PRIMARY KEY(_oid),
       INDEX(_parent_oid)
) ENGINE=INNODB;

SELECT 'Converting QCLog.message to LONGTEXT' AS '';
ALTER TABLE QCLog MODIFY message LONGTEXT NOT NULL;

SELECT 'Drop composite_index from QCLog' AS '';
ALTER TABLE QCLog DROP CONSTRAINT composite_index;

SELECT 'Create index on QCLog.start' AS '';
CREATE INDEX QCLog_start ON QCLog(start,start_ms);

SELECT 'Create index on QCLog.end' AS '';
CREATE INDEX QCLog_end ON QCLog(end,end_ms);

SELECT 'Adding index to QCLog' AS '';
CREATE INDEX QCLog_id ON QCLog(waveformID_networkCode,waveformID_stationCode,waveformID_locationCode,waveformID_channelCode,waveformID_resourceURI);

SELECT 'Convert BLOBS to LONGTEXT' AS '';
ALTER TABLE CompositeTime MODIFY second_pdf_variable_content LONGTEXT;
ALTER TABLE CompositeTime MODIFY second_pdf_probability_content LONGTEXT;
ALTER TABLE MomentTensorComponentContribution MODIFY dataTimeWindow LONGTEXT;
ALTER TABLE MomentTensor MODIFY scalarMoment_pdf_variable_content LONGTEXT;
ALTER TABLE MomentTensor MODIFY scalarMoment_pdf_probability_content LONGTEXT;
ALTER TABLE MomentTensor MODIFY tensor_Mrr_pdf_variable_content LONGTEXT;
ALTER TABLE MomentTensor MODIFY tensor_Mrr_pdf_probability_content LONGTEXT;
ALTER TABLE MomentTensor MODIFY tensor_Mtt_pdf_variable_content LONGTEXT;
ALTER TABLE MomentTensor MODIFY tensor_Mtt_pdf_probability_content LONGTEXT;
ALTER TABLE MomentTensor MODIFY tensor_Mpp_pdf_variable_content LONGTEXT;
ALTER TABLE MomentTensor MODIFY tensor_Mpp_pdf_probability_content LONGTEXT;
ALTER TABLE MomentTensor MODIFY tensor_Mrt_pdf_variable_content LONGTEXT;
ALTER TABLE MomentTensor MODIFY tensor_Mrt_pdf_probability_content LONGTEXT;
ALTER TABLE MomentTensor MODIFY tensor_Mrp_pdf_variable_content LONGTEXT;
ALTER TABLE MomentTensor MODIFY tensor_Mrp_pdf_probability_content LONGTEXT;
ALTER TABLE MomentTensor MODIFY tensor_Mtp_pdf_variable_content LONGTEXT;
ALTER TABLE MomentTensor MODIFY tensor_Mtp_pdf_probability_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY nodalPlanes_nodalPlane1_strike_pdf_variable_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY nodalPlanes_nodalPlane1_strike_pdf_probability_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY nodalPlanes_nodalPlane1_dip_pdf_variable_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY nodalPlanes_nodalPlane1_dip_pdf_probability_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY nodalPlanes_nodalPlane1_rake_pdf_variable_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY nodalPlanes_nodalPlane1_rake_pdf_probability_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY nodalPlanes_nodalPlane2_strike_pdf_variable_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY nodalPlanes_nodalPlane2_strike_pdf_probability_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY nodalPlanes_nodalPlane2_dip_pdf_variable_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY nodalPlanes_nodalPlane2_dip_pdf_probability_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY nodalPlanes_nodalPlane2_rake_pdf_variable_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY nodalPlanes_nodalPlane2_rake_pdf_probability_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY principalAxes_tAxis_azimuth_pdf_variable_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY principalAxes_tAxis_azimuth_pdf_probability_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY principalAxes_tAxis_plunge_pdf_variable_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY principalAxes_tAxis_plunge_pdf_probability_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY principalAxes_tAxis_length_pdf_variable_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY principalAxes_tAxis_length_pdf_probability_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY principalAxes_pAxis_azimuth_pdf_variable_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY principalAxes_pAxis_azimuth_pdf_probability_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY principalAxes_pAxis_plunge_pdf_variable_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY principalAxes_pAxis_plunge_pdf_probability_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY principalAxes_pAxis_length_pdf_variable_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY principalAxes_pAxis_length_pdf_probability_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY principalAxes_nAxis_azimuth_pdf_variable_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY principalAxes_nAxis_azimuth_pdf_probability_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY principalAxes_nAxis_plunge_pdf_variable_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY principalAxes_nAxis_plunge_pdf_probability_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY principalAxes_nAxis_length_pdf_variable_content LONGTEXT;
ALTER TABLE FocalMechanism MODIFY principalAxes_nAxis_length_pdf_probability_content LONGTEXT;
ALTER TABLE Amplitude MODIFY amplitude_pdf_variable_content LONGTEXT;
ALTER TABLE Amplitude MODIFY amplitude_pdf_probability_content LONGTEXT;
ALTER TABLE Amplitude MODIFY period_pdf_variable_content LONGTEXT;
ALTER TABLE Amplitude MODIFY period_pdf_probability_content LONGTEXT;
ALTER TABLE Amplitude MODIFY scalingTime_pdf_variable_content LONGTEXT;
ALTER TABLE Amplitude MODIFY scalingTime_pdf_probability_content LONGTEXT;
ALTER TABLE Magnitude MODIFY magnitude_pdf_variable_content LONGTEXT;
ALTER TABLE Magnitude MODIFY magnitude_pdf_probability_content LONGTEXT;
ALTER TABLE StationMagnitude MODIFY magnitude_pdf_variable_content LONGTEXT;
ALTER TABLE StationMagnitude MODIFY magnitude_pdf_probability_content LONGTEXT;
ALTER TABLE Pick MODIFY time_pdf_variable_content LONGTEXT;
ALTER TABLE Pick MODIFY time_pdf_probability_content LONGTEXT;
ALTER TABLE Pick MODIFY horizontalSlowness_pdf_variable_content LONGTEXT;
ALTER TABLE Pick MODIFY horizontalSlowness_pdf_probability_content LONGTEXT;
ALTER TABLE Pick MODIFY backazimuth_pdf_variable_content LONGTEXT;
ALTER TABLE Pick MODIFY backazimuth_pdf_probability_content LONGTEXT;
ALTER TABLE Origin MODIFY time_pdf_variable_content LONGTEXT;
ALTER TABLE Origin MODIFY time_pdf_probability_content LONGTEXT;
ALTER TABLE Origin MODIFY latitude_pdf_variable_content LONGTEXT;
ALTER TABLE Origin MODIFY latitude_pdf_probability_content LONGTEXT;
ALTER TABLE Origin MODIFY longitude_pdf_variable_content LONGTEXT;
ALTER TABLE Origin MODIFY longitude_pdf_probability_content LONGTEXT;
ALTER TABLE Origin MODIFY depth_pdf_variable_content LONGTEXT;
ALTER TABLE Origin MODIFY depth_pdf_probability_content LONGTEXT;
ALTER TABLE Parameter MODIFY value LONGTEXT;
ALTER TABLE AuxSource MODIFY remark_content LONGTEXT;
ALTER TABLE AuxDevice MODIFY remark_content LONGTEXT;
ALTER TABLE SensorCalibration MODIFY remark_content LONGTEXT;
ALTER TABLE Sensor MODIFY remark_content LONGTEXT;
ALTER TABLE ResponsePAZ MODIFY zeros_content LONGTEXT;
ALTER TABLE ResponsePAZ MODIFY poles_content LONGTEXT;
ALTER TABLE ResponsePAZ MODIFY remark_content LONGTEXT;
ALTER TABLE ResponsePolynomial MODIFY coefficients_content LONGTEXT;
ALTER TABLE ResponsePolynomial MODIFY remark_content LONGTEXT;
ALTER TABLE ResponseFAP MODIFY tuples_content LONGTEXT;
ALTER TABLE ResponseFAP MODIFY remark_content LONGTEXT;
ALTER TABLE ResponseFIR MODIFY coefficients_content LONGTEXT;
ALTER TABLE ResponseFIR MODIFY remark_content LONGTEXT;
ALTER TABLE ResponseIIR MODIFY numerators_content LONGTEXT;
ALTER TABLE ResponseIIR MODIFY denominators_content LONGTEXT;
ALTER TABLE ResponseIIR MODIFY remark_content LONGTEXT;
ALTER TABLE DataloggerCalibration MODIFY remark_content LONGTEXT;
ALTER TABLE Decimation MODIFY analogueFilterChain_content LONGTEXT;
ALTER TABLE Decimation MODIFY digitalFilterChain_content LONGTEXT;
ALTER TABLE Datalogger MODIFY remark_content LONGTEXT;
ALTER TABLE AuxStream MODIFY code CHAR(8) NOT NULL;
ALTER TABLE Stream MODIFY code CHAR(8) NOT NULL;
ALTER TABLE Station MODIFY remark_content LONGTEXT;
ALTER TABLE Network MODIFY remark_content LONGTEXT;

SELECT 'Updating Meta' AS '';
UPDATE Meta SET value='0.14.0' WHERE name='Schema-Version';
