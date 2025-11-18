\echo Drop foreign key contraints of all model tables
ALTER TABLE EventDescription DROP CONSTRAINT IF EXISTS EventDescription__oid_fkey;
ALTER TABLE Comment DROP CONSTRAINT IF EXISTS Comment__oid_fkey;
ALTER TABLE DataUsed DROP CONSTRAINT IF EXISTS DataUsed__oid_fkey;
ALTER TABLE CompositeTime DROP CONSTRAINT IF EXISTS CompositeTime__oid_fkey;
ALTER TABLE PickReference DROP CONSTRAINT IF EXISTS PickReference__oid_fkey;
ALTER TABLE AmplitudeReference DROP CONSTRAINT IF EXISTS AmplitudeReference__oid_fkey;
ALTER TABLE Reading DROP CONSTRAINT IF EXISTS Reading__oid_fkey;
ALTER TABLE MomentTensorComponentContribution DROP CONSTRAINT IF EXISTS MomentTensorComponentContribution__oid_fkey;
ALTER TABLE MomentTensorStationContribution DROP CONSTRAINT IF EXISTS MomentTensorStationContribution__oid_fkey;
ALTER TABLE MomentTensorPhaseSetting DROP CONSTRAINT IF EXISTS MomentTensorPhaseSetting__oid_fkey;
ALTER TABLE MomentTensor DROP CONSTRAINT IF EXISTS MomentTensor__oid_fkey;
ALTER TABLE FocalMechanism DROP CONSTRAINT IF EXISTS FocalMechanism__oid_fkey;
ALTER TABLE Amplitude DROP CONSTRAINT IF EXISTS Amplitude__oid_fkey;
ALTER TABLE StationMagnitudeContribution DROP CONSTRAINT IF EXISTS StationMagnitudeContribution__oid_fkey;
ALTER TABLE Magnitude DROP CONSTRAINT IF EXISTS Magnitude__oid_fkey;
ALTER TABLE StationMagnitude DROP CONSTRAINT IF EXISTS StationMagnitude__oid_fkey;
ALTER TABLE Pick DROP CONSTRAINT IF EXISTS Pick__oid_fkey;
ALTER TABLE OriginReference DROP CONSTRAINT IF EXISTS OriginReference__oid_fkey;
ALTER TABLE FocalMechanismReference DROP CONSTRAINT IF EXISTS FocalMechanismReference__oid_fkey;
ALTER TABLE Event DROP CONSTRAINT IF EXISTS Event__oid_fkey;
ALTER TABLE Arrival DROP CONSTRAINT IF EXISTS Arrival__oid_fkey;
ALTER TABLE Origin DROP CONSTRAINT IF EXISTS Origin__oid_fkey;
ALTER TABLE Parameter DROP CONSTRAINT IF EXISTS Parameter__oid_fkey;
ALTER TABLE ParameterSet DROP CONSTRAINT IF EXISTS ParameterSet__oid_fkey;
ALTER TABLE Setup DROP CONSTRAINT IF EXISTS Setup__oid_fkey;
ALTER TABLE ConfigStation DROP CONSTRAINT IF EXISTS ConfigStation__oid_fkey;
ALTER TABLE ConfigModule DROP CONSTRAINT IF EXISTS ConfigModule__oid_fkey;
ALTER TABLE QCLog DROP CONSTRAINT IF EXISTS QCLog__oid_fkey;
ALTER TABLE WaveformQuality DROP CONSTRAINT IF EXISTS WaveformQuality__oid_fkey;
ALTER TABLE Outage DROP CONSTRAINT IF EXISTS Outage__oid_fkey;
ALTER TABLE StationReference DROP CONSTRAINT IF EXISTS StationReference__oid_fkey;
ALTER TABLE StationGroup DROP CONSTRAINT IF EXISTS StationGroup__oid_fkey;
ALTER TABLE AuxSource DROP CONSTRAINT IF EXISTS AuxSource__oid_fkey;
ALTER TABLE AuxDevice DROP CONSTRAINT IF EXISTS AuxDevice__oid_fkey;
ALTER TABLE SensorCalibration DROP CONSTRAINT IF EXISTS SensorCalibration__oid_fkey;
ALTER TABLE Sensor DROP CONSTRAINT IF EXISTS Sensor__oid_fkey;
ALTER TABLE ResponsePAZ DROP CONSTRAINT IF EXISTS ResponsePAZ__oid_fkey;
ALTER TABLE ResponsePolynomial DROP CONSTRAINT IF EXISTS ResponsePolynomial__oid_fkey;
ALTER TABLE ResponseFAP DROP CONSTRAINT IF EXISTS ResponseFAP__oid_fkey;
ALTER TABLE ResponseFIR DROP CONSTRAINT IF EXISTS ResponseFIR__oid_fkey;
ALTER TABLE ResponseIIR DROP CONSTRAINT IF EXISTS ResponseIIR__oid_fkey;
ALTER TABLE DataloggerCalibration DROP CONSTRAINT IF EXISTS DataloggerCalibration__oid_fkey;
ALTER TABLE Decimation DROP CONSTRAINT IF EXISTS Decimation__oid_fkey;
ALTER TABLE Datalogger DROP CONSTRAINT IF EXISTS Datalogger__oid_fkey;
ALTER TABLE AuxStream DROP CONSTRAINT IF EXISTS AuxStream__oid_fkey;
ALTER TABLE Stream DROP CONSTRAINT IF EXISTS Stream__oid_fkey;
ALTER TABLE SensorLocation DROP CONSTRAINT IF EXISTS SensorLocation__oid_fkey;
ALTER TABLE Station DROP CONSTRAINT IF EXISTS Station__oid_fkey;
ALTER TABLE Network DROP CONSTRAINT IF EXISTS Network__oid_fkey;
ALTER TABLE RouteArclink DROP CONSTRAINT IF EXISTS RouteArclink__oid_fkey;
ALTER TABLE RouteSeedlink DROP CONSTRAINT IF EXISTS RouteSeedlink__oid_fkey;
ALTER TABLE Route DROP CONSTRAINT IF EXISTS Route__oid_fkey;
ALTER TABLE Access DROP CONSTRAINT IF EXISTS Access__oid_fkey;
ALTER TABLE JournalEntry DROP CONSTRAINT IF EXISTS JournalEntry__oid_fkey;
ALTER TABLE ArclinkUser DROP CONSTRAINT IF EXISTS ArclinkUser__oid_fkey;
ALTER TABLE ArclinkStatusLine DROP CONSTRAINT IF EXISTS ArclinkStatusLine__oid_fkey;
ALTER TABLE ArclinkRequestLine DROP CONSTRAINT IF EXISTS ArclinkRequestLine__oid_fkey;
ALTER TABLE ArclinkRequest DROP CONSTRAINT IF EXISTS ArclinkRequest__oid_fkey;
ALTER TABLE DataSegment DROP CONSTRAINT IF EXISTS DataSegment__oid_fkey;
ALTER TABLE DataAttributeExtent DROP CONSTRAINT IF EXISTS DataAttributeExtent__oid_fkey;
ALTER TABLE DataExtent DROP CONSTRAINT IF EXISTS DataExtent__oid_fkey;
ALTER TABLE PublicObject DROP CONSTRAINT IF EXISTS PublicObject__oid_fkey;

\echo Create catalog table
CREATE TABLE Catalog (
       _oid BIGINT NOT NULL,
       _parent_oid BIGINT NOT NULL,
       _last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
       m_name VARCHAR(255) NOT NULL,
       m_description TEXT,
       m_creationInfo_agencyID VARCHAR(64),
       m_creationInfo_agencyURI VARCHAR(255),
       m_creationInfo_author VARCHAR(128),
       m_creationInfo_authorURI VARCHAR(255),
       m_creationInfo_creationTime TIMESTAMP,
       m_creationInfo_creationTime_ms INTEGER,
       m_creationInfo_modificationTime TIMESTAMP,
       m_creationInfo_modificationTime_ms INTEGER,
       m_creationInfo_version VARCHAR(64),
       m_creationInfo_used BOOLEAN NOT NULL DEFAULT '0',
       m_start TIMESTAMP NOT NULL,
       m_start_ms INTEGER NOT NULL,
       m_end TIMESTAMP,
       m_end_ms INTEGER,
       m_dynamic BOOLEAN NOT NULL,
       PRIMARY KEY(_oid)
);

CREATE INDEX Catalog__parent_oid ON Catalog(_parent_oid);
CREATE TRIGGER Catalog_update BEFORE UPDATE ON Catalog FOR EACH ROW EXECUTE PROCEDURE update_modified();

\echo Drop QCLog composite index
ALTER TABLE QCLog DROP CONSTRAINT qclog_composite_index;

\echo Convert QCLog.message to TEXT
ALTER TABLE QCLog ALTER COLUMN m_message TYPE TEXT;

\echo Create index on QCLog.start
CREATE INDEX qclog_id ON QCLog(m_waveformID_networkCode,m_waveformID_stationCode,m_waveformID_locationCode,m_waveformID_channelCode,m_waveformID_resourceURI);

\echo Create index on QCLog.end
CREATE INDEX qclog_m_start_m_start_ms ON QCLog(m_start,m_start_ms);

\echo Adding index to QCLog
CREATE INDEX qclog_m_end_m_end_ms ON QCLog(m_end,m_end_ms);

\echo Change AuxStream and Stream code length to 8
ALTER TABLE AuxStream ALTER COLUMN m_code TYPE VARCHAR(8);
ALTER TABLE Stream ALTER COLUMN m_code TYPE VARCHAR(8);

\echo Convert BYTEA to TEXT
ALTER TABLE CompositeTime ALTER COLUMN m_second_pdf_variable_content TYPE TEXT;
ALTER TABLE CompositeTime ALTER COLUMN m_second_pdf_probability_content TYPE TEXT;
ALTER TABLE MomentTensorComponentContribution ALTER COLUMN m_dataTimeWindow TYPE TEXT;
ALTER TABLE MomentTensor ALTER COLUMN m_scalarMoment_pdf_variable_content TYPE TEXT;
ALTER TABLE MomentTensor ALTER COLUMN m_scalarMoment_pdf_probability_content TYPE TEXT;
ALTER TABLE MomentTensor ALTER COLUMN m_tensor_Mrr_pdf_variable_content TYPE TEXT;
ALTER TABLE MomentTensor ALTER COLUMN m_tensor_Mrr_pdf_probability_content TYPE TEXT;
ALTER TABLE MomentTensor ALTER COLUMN m_tensor_Mtt_pdf_variable_content TYPE TEXT;
ALTER TABLE MomentTensor ALTER COLUMN m_tensor_Mtt_pdf_probability_content TYPE TEXT;
ALTER TABLE MomentTensor ALTER COLUMN m_tensor_Mpp_pdf_variable_content TYPE TEXT;
ALTER TABLE MomentTensor ALTER COLUMN m_tensor_Mpp_pdf_probability_content TYPE TEXT;
ALTER TABLE MomentTensor ALTER COLUMN m_tensor_Mrt_pdf_variable_content TYPE TEXT;
ALTER TABLE MomentTensor ALTER COLUMN m_tensor_Mrt_pdf_probability_content TYPE TEXT;
ALTER TABLE MomentTensor ALTER COLUMN m_tensor_Mrp_pdf_variable_content TYPE TEXT;
ALTER TABLE MomentTensor ALTER COLUMN m_tensor_Mrp_pdf_probability_content TYPE TEXT;
ALTER TABLE MomentTensor ALTER COLUMN m_tensor_Mtp_pdf_variable_content TYPE TEXT;
ALTER TABLE MomentTensor ALTER COLUMN m_tensor_Mtp_pdf_probability_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_nodalPlanes_nodalPlane1_strike_pdf_variable_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_nodalPlanes_nodalPlane1_strike_pdf_probability_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_nodalPlanes_nodalPlane1_dip_pdf_variable_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_nodalPlanes_nodalPlane1_dip_pdf_probability_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_nodalPlanes_nodalPlane1_rake_pdf_variable_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_nodalPlanes_nodalPlane1_rake_pdf_probability_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_nodalPlanes_nodalPlane2_strike_pdf_variable_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_nodalPlanes_nodalPlane2_strike_pdf_probability_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_nodalPlanes_nodalPlane2_dip_pdf_variable_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_nodalPlanes_nodalPlane2_dip_pdf_probability_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_nodalPlanes_nodalPlane2_rake_pdf_variable_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_nodalPlanes_nodalPlane2_rake_pdf_probability_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_principalAxes_tAxis_azimuth_pdf_variable_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_principalAxes_tAxis_azimuth_pdf_probability_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_principalAxes_tAxis_plunge_pdf_variable_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_principalAxes_tAxis_plunge_pdf_probability_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_principalAxes_tAxis_length_pdf_variable_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_principalAxes_tAxis_length_pdf_probability_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_principalAxes_pAxis_azimuth_pdf_variable_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_principalAxes_pAxis_azimuth_pdf_probability_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_principalAxes_pAxis_plunge_pdf_variable_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_principalAxes_pAxis_plunge_pdf_probability_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_principalAxes_pAxis_length_pdf_variable_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_principalAxes_pAxis_length_pdf_probability_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_principalAxes_nAxis_azimuth_pdf_variable_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_principalAxes_nAxis_azimuth_pdf_probability_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_principalAxes_nAxis_plunge_pdf_variable_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_principalAxes_nAxis_plunge_pdf_probability_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_principalAxes_nAxis_length_pdf_variable_content TYPE TEXT;
ALTER TABLE FocalMechanism ALTER COLUMN m_principalAxes_nAxis_length_pdf_probability_content TYPE TEXT;
ALTER TABLE Amplitude ALTER COLUMN m_amplitude_pdf_variable_content TYPE TEXT;
ALTER TABLE Amplitude ALTER COLUMN m_amplitude_pdf_probability_content TYPE TEXT;
ALTER TABLE Amplitude ALTER COLUMN m_period_pdf_variable_content TYPE TEXT;
ALTER TABLE Amplitude ALTER COLUMN m_period_pdf_probability_content TYPE TEXT;
ALTER TABLE Amplitude ALTER COLUMN m_scalingTime_pdf_variable_content TYPE TEXT;
ALTER TABLE Amplitude ALTER COLUMN m_scalingTime_pdf_probability_content TYPE TEXT;
ALTER TABLE Magnitude ALTER COLUMN m_magnitude_pdf_variable_content TYPE TEXT;
ALTER TABLE Magnitude ALTER COLUMN m_magnitude_pdf_probability_content TYPE TEXT;
ALTER TABLE StationMagnitude ALTER COLUMN m_magnitude_pdf_variable_content TYPE TEXT;
ALTER TABLE StationMagnitude ALTER COLUMN m_magnitude_pdf_probability_content TYPE TEXT;
ALTER TABLE Pick ALTER COLUMN m_time_pdf_variable_content TYPE TEXT;
ALTER TABLE Pick ALTER COLUMN m_time_pdf_probability_content TYPE TEXT;
ALTER TABLE Pick ALTER COLUMN m_horizontalSlowness_pdf_variable_content TYPE TEXT;
ALTER TABLE Pick ALTER COLUMN m_horizontalSlowness_pdf_probability_content TYPE TEXT;
ALTER TABLE Pick ALTER COLUMN m_backazimuth_pdf_variable_content TYPE TEXT;
ALTER TABLE Pick ALTER COLUMN m_backazimuth_pdf_probability_content TYPE TEXT;
ALTER TABLE Origin ALTER COLUMN m_time_pdf_variable_content TYPE TEXT;
ALTER TABLE Origin ALTER COLUMN m_time_pdf_probability_content TYPE TEXT;
ALTER TABLE Origin ALTER COLUMN m_latitude_pdf_variable_content TYPE TEXT;
ALTER TABLE Origin ALTER COLUMN m_latitude_pdf_probability_content TYPE TEXT;
ALTER TABLE Origin ALTER COLUMN m_longitude_pdf_variable_content TYPE TEXT;
ALTER TABLE Origin ALTER COLUMN m_longitude_pdf_probability_content TYPE TEXT;
ALTER TABLE Origin ALTER COLUMN m_depth_pdf_variable_content TYPE TEXT;
ALTER TABLE Origin ALTER COLUMN m_depth_pdf_probability_content TYPE TEXT;
ALTER TABLE Parameter ALTER COLUMN m_value TYPE TEXT;
ALTER TABLE AuxSource ALTER COLUMN m_remark_content TYPE TEXT;
ALTER TABLE AuxDevice ALTER COLUMN m_remark_content TYPE TEXT;
ALTER TABLE SensorCalibration ALTER COLUMN m_remark_content TYPE TEXT;
ALTER TABLE Sensor ALTER COLUMN m_remark_content TYPE TEXT;
ALTER TABLE ResponsePAZ ALTER COLUMN m_zeros_content TYPE TEXT;
ALTER TABLE ResponsePAZ ALTER COLUMN m_poles_content TYPE TEXT;
ALTER TABLE ResponsePAZ ALTER COLUMN m_remark_content TYPE TEXT;
ALTER TABLE ResponsePolynomial ALTER COLUMN m_coefficients_content TYPE TEXT;
ALTER TABLE ResponsePolynomial ALTER COLUMN m_remark_content TYPE TEXT;
ALTER TABLE ResponseFAP ALTER COLUMN m_tuples_content TYPE TEXT;
ALTER TABLE ResponseFAP ALTER COLUMN m_remark_content TYPE TEXT;
ALTER TABLE ResponseFIR ALTER COLUMN m_coefficients_content TYPE TEXT;
ALTER TABLE ResponseFIR ALTER COLUMN m_remark_content TYPE TEXT;
ALTER TABLE ResponseIIR ALTER COLUMN m_numerators_content TYPE TEXT;
ALTER TABLE ResponseIIR ALTER COLUMN m_denominators_content TYPE TEXT;
ALTER TABLE ResponseIIR ALTER COLUMN m_remark_content TYPE TEXT;
ALTER TABLE DataloggerCalibration ALTER COLUMN m_remark_content TYPE TEXT;
ALTER TABLE Decimation ALTER COLUMN m_analogueFilterChain_content TYPE TEXT;
ALTER TABLE Decimation ALTER COLUMN m_digitalFilterChain_content TYPE TEXT;
ALTER TABLE Datalogger ALTER COLUMN m_remark_content TYPE TEXT;
ALTER TABLE Station ALTER COLUMN m_remark_content TYPE TEXT;
ALTER TABLE Network ALTER COLUMN m_remark_content TYPE TEXT;

\echo Updating Meta
UPDATE Meta SET value='0.14.0' WHERE name='Schema-Version';
