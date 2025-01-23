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

#ifndef TEST_IO_ARCHIVE_EVENTXML_H
#define TEST_IO_ARCHIVE_EVENTXML_H

#include <string>

std::string gempa2021ijvk = R"EVENTXML(<?xml version="1.0" encoding="UTF-8"?>
<seiscomp xmlns="http://geofon.gfz-potsdam.de/ns/seiscomp3-schema/0.11" version="0.11">
  <EventParameters>
    <pick publicID="20210430.051926.35-AIC-G.SANVU.00.BHZ">
      <time>
        <value>2021-04-30T05:19:26.35Z</value>
      </time>
      <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:20:08.460104Z</creationTime>
      </creationInfo>
      <comment>
        <text>516411.2691</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052008.06-AIC-IU.HNR.00.BHZ">
      <time>
        <value>2021-04-30T05:20:08.069538Z</value>
      </time>
      <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:20:39.163413Z</creationTime>
      </creationInfo>
      <comment>
        <text>1785.345446</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052157.79-AIC-IU.TARA.00.BHZ">
      <time>
        <value>2021-04-30T05:21:57.794539Z</value>
      </time>
      <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:22:46.367507Z</creationTime>
      </creationInfo>
      <comment>
        <text>38580.54497</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052259.26-AIC-AU.EIDS..BHZ">
      <time>
        <value>2021-04-30T05:22:59.269538Z</value>
      </time>
      <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:23:40.601582Z</creationTime>
      </creationInfo>
      <comment>
        <text>129.3693006</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052313.39-AIC-IU.CTAO.00.BHZ">
      <time>
        <value>2021-04-30T05:23:13.394538Z</value>
      </time>
      <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:24:08.840893Z</creationTime>
      </creationInfo>
      <comment>
        <text>89.92931146</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052327.21-AIC-AU.ARMA..BHZ">
      <time>
        <value>2021-04-30T05:23:27.219538Z</value>
      </time>
      <waveformID networkCode="AU" stationCode="ARMA" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:23:59.142929Z</creationTime>
      </creationInfo>
      <comment>
        <text>255.1357532</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052400.85-AIC-G.CAN.00.BHZ">
      <time>
        <value>2021-04-30T05:24:00.85Z</value>
      </time>
      <waveformID networkCode="G" stationCode="CAN" locationCode="00" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:24:31.286786Z</creationTime>
      </creationInfo>
      <comment>
        <text>221.9124626</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052411.24-AIC-AU.CMSA..BHZ">
      <time>
        <value>2021-04-30T05:24:11.244538Z</value>
      </time>
      <waveformID networkCode="AU" stationCode="CMSA" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:24:37.04041Z</creationTime>
      </creationInfo>
      <comment>
        <text>67.7292807</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052449.69-AIC-II.WRAB.00.BHZ">
      <time>
        <value>2021-04-30T05:24:49.694536Z</value>
      </time>
      <waveformID networkCode="II" stationCode="WRAB" locationCode="00" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:25:21.612124Z</creationTime>
      </creationInfo>
      <comment>
        <text>17.56457314</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052611.16-AIC-GE.SANI..BHZ">
      <time>
        <value>2021-04-30T05:26:11.169536Z</value>
      </time>
      <waveformID networkCode="GE" stationCode="SANI" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:26:34.254369Z</creationTime>
      </creationInfo>
      <comment>
        <text>128.7597509</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052645.86-AIC-IA.SMSI..BHZ">
      <time>
        <value>2021-04-30T05:26:45.8625Z</value>
      </time>
      <waveformID networkCode="IA" stationCode="SMSI" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:27:32.560241Z</creationTime>
      </creationInfo>
      <comment>
        <text>28.4251654</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052657.94-AIC-II.KAPI.00.BHZ">
      <time>
        <value>2021-04-30T05:26:57.944538Z</value>
      </time>
      <waveformID networkCode="II" stationCode="KAPI" locationCode="00" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:27:34.926047Z</creationTime>
      </creationInfo>
      <comment>
        <text>7.848832438</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052658.56-AIC-AU.MEEK..BHZ">
      <time>
        <value>2021-04-30T05:26:58.569538Z</value>
      </time>
      <waveformID networkCode="AU" stationCode="MEEK" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <horizontalSlowness>
        <value>6.803328368</value>
      </horizontalSlowness>
      <backazimuth>
        <value>74.10479815</value>
      </backazimuth>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick@gempa-proc</author>
        <creationTime>2021-04-30T05:27:26.05046Z</creationTime>
      </creationInfo>
      <comment>
        <text>666.5150362</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052659.61-AIC-IA.DBNI..BHZ">
      <time>
        <value>2021-04-30T05:26:59.6125Z</value>
      </time>
      <waveformID networkCode="IA" stationCode="DBNI" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:27:32.91164Z</creationTime>
      </creationInfo>
      <comment>
        <text>251.4197006</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052702.21-AIC-IA.MPSI..BHZ">
      <time>
        <value>2021-04-30T05:27:02.2125Z</value>
      </time>
      <waveformID networkCode="IA" stationCode="MPSI" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:27:27.543321Z</creationTime>
      </creationInfo>
      <comment>
        <text>108.4924793</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052742.81-AIC-MY.KKM..BHZ">
      <time>
        <value>2021-04-30T05:27:42.819538Z</value>
      </time>
      <waveformID networkCode="MY" stationCode="KKM" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:28:52.241276Z</creationTime>
      </creationInfo>
      <comment>
        <text>2638.551647</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052807.66-AIC-TW.NACB..BHZ">
      <time>
        <value>2021-04-30T05:28:07.669538Z</value>
      </time>
      <waveformID networkCode="TW" stationCode="NACB" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:28:44.490996Z</creationTime>
      </creationInfo>
      <comment>
        <text>203.2994739</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052809.21-AIC-TW.TPUB..BHZ">
      <time>
        <value>2021-04-30T05:28:09.219538Z</value>
      </time>
      <waveformID networkCode="TW" stationCode="TPUB" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:28:51.847629Z</creationTime>
      </creationInfo>
      <comment>
        <text>912.8995708</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052810.81-AIC-TW.YHNB..BHZ">
      <time>
        <value>2021-04-30T05:28:10.819538Z</value>
      </time>
      <waveformID networkCode="TW" stationCode="YHNB" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:28:48.567577Z</creationTime>
      </creationInfo>
      <comment>
        <text>394.6947297</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052559.83-AIC-IA.OBMI..BHZ">
      <time>
        <value>2021-04-30T05:25:59.8375Z</value>
      </time>
      <waveformID networkCode="IA" stationCode="OBMI" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:26:29.045092Z</creationTime>
      </creationInfo>
      <comment>
        <text>417.7432823</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052715.96-AIC-IA.SMKI..BHZ">
      <time>
        <value>2021-04-30T05:27:15.9625Z</value>
      </time>
      <waveformID networkCode="IA" stationCode="SMKI" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:27:35.795926Z</creationTime>
      </creationInfo>
      <comment>
        <text>130463.6632</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052806.81-AIC-TW.YULB..BHZ">
      <time>
        <value>2021-04-30T05:28:06.819539Z</value>
      </time>
      <waveformID networkCode="TW" stationCode="YULB" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <horizontalSlowness>
        <value>6.159321384</value>
      </horizontalSlowness>
      <backazimuth>
        <value>143.7394071</value>
      </backazimuth>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick@gempa-proc</author>
        <creationTime>2021-04-30T05:28:53.568273Z</creationTime>
      </creationInfo>
      <comment>
        <text>86.58728009</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052809.66-AIC-TW.SSLB..BHZ">
      <time>
        <value>2021-04-30T05:28:09.669539Z</value>
      </time>
      <waveformID networkCode="TW" stationCode="SSLB" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:29:02.249124Z</creationTime>
      </creationInfo>
    </pick>
    <pick publicID="20210430.052825.23-AIC-IA.CGJI..BHZ">
      <time>
        <value>2021-04-30T05:28:25.2375Z</value>
      </time>
      <waveformID networkCode="IA" stationCode="CGJI" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:29:06.556062Z</creationTime>
      </creationInfo>
      <comment>
        <text>67.5911587</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052909.46-AIC-IU.CASY.00.BHZ">
      <time>
        <value>2021-04-30T05:29:09.469538Z</value>
      </time>
      <waveformID networkCode="IU" stationCode="CASY" locationCode="00" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:29:56.272739Z</creationTime>
      </creationInfo>
      <comment>
        <text>136.3591703</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052921.26-AIC-IA.MNSI..BHZ">
      <time>
        <value>2021-04-30T05:29:21.2625Z</value>
      </time>
      <waveformID networkCode="IA" stationCode="MNSI" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:29:43.428444Z</creationTime>
      </creationInfo>
      <comment>
        <text>582.4690158</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052911.41-AIC-IU.SBA.00.BHZ">
      <time>
        <value>2021-04-30T05:29:11.419538Z</value>
      </time>
      <waveformID networkCode="IU" stationCode="SBA" locationCode="00" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:30:04.835466Z</creationTime>
      </creationInfo>
    </pick>
    <pick publicID="20210430.052936.16-AIC-RM.SLV..BHZ">
      <time>
        <value>2021-04-30T05:29:36.169538Z</value>
      </time>
      <waveformID networkCode="RM" stationCode="SLV" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:30:33.69901Z</creationTime>
      </creationInfo>
      <comment>
        <text>22.26028018</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052956.89-AIC-IU.CHTO.00.BHZ">
      <time>
        <value>2021-04-30T05:29:56.894538Z</value>
      </time>
      <waveformID networkCode="IU" stationCode="CHTO" locationCode="00" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:30:40.618042Z</creationTime>
      </creationInfo>
      <comment>
        <text>25.04695345</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052908.75-AIC-GT.VNDA.00.BHZ">
      <time>
        <value>2021-04-30T05:29:08.75Z</value>
      </time>
      <waveformID networkCode="GT" stationCode="VNDA" locationCode="00" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:30:58.040533Z</creationTime>
      </creationInfo>
      <comment>
        <text>9.715289576</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.053031.16-AIC-IU.ULN.00.BHZ">
      <time>
        <value>2021-04-30T05:30:31.169538Z</value>
      </time>
      <waveformID networkCode="IU" stationCode="ULN" locationCode="00" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:31:39.82858Z</creationTime>
      </creationInfo>
    </pick>
    <pick publicID="20210430.053054.31-AIC-IU.COLA.00.BHZ">
      <time>
        <value>2021-04-30T05:30:54.319536Z</value>
      </time>
      <waveformID networkCode="IU" stationCode="COLA" locationCode="00" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:31:44.491043Z</creationTime>
      </creationInfo>
    </pick>
    <pick publicID="20210430.053746.90-AIC-HU.TRPA..BHZ">
      <time>
        <value>2021-04-30T05:37:46.9Z</value>
      </time>
      <waveformID networkCode="HU" stationCode="TRPA" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:38:29.656714Z</creationTime>
      </creationInfo>
      <comment>
        <text>72.43481722</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <pick publicID="20210430.052930.06-AIC-IC.ENH.00.BHZ">
      <time>
        <value>2021-04-30T05:29:30.069538Z</value>
      </time>
      <waveformID networkCode="IC" stationCode="ENH" locationCode="00" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T06:03:21.871343Z</creationTime>
      </creationInfo>
    </pick>
    <pick publicID="20210430.052250.71-AIC-GE.PMG.00.BHZ">
      <time>
        <value>2021-04-30T05:22:50.719538Z</value>
      </time>
      <waveformID networkCode="GE" stationCode="PMG" locationCode="00" channelCode="BHZ"/>
      <filterID>BW(4,0.7,2)</filterID>
      <methodID>AIC</methodID>
      <phaseHint>P</phaseHint>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T09:49:23.37817Z</creationTime>
      </creationInfo>
      <comment>
        <text>215.9705406</text>
        <id>DFX:rectilinearity</id>
      </comment>
    </pick>
    <amplitude publicID="20210430.051926.35-AIC-G.SANVU.00.BHZ.MLv">
      <type>MLv</type>
      <amplitude>
        <value>14.62909143</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:20:39.950001Z</reference>
        <begin>-78.6</begin>
        <end>76.4</end>
      </timeWindow>
      <snr>72.28517741</snr>
      <pickID>20210430.051926.35-AIC-G.SANVU.00.BHZ</pickID>
      <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHE"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick@gempa-proc</author>
        <creationTime>2021-04-30T05:22:32.627918Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052008.06-AIC-IU.HNR.00.BHZ.MLv">
      <type>MLv</type>
      <amplitude>
        <value>0.2140626461</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:21:28.544538Z</reference>
        <begin>-85.475</begin>
        <end>69.525</end>
      </timeWindow>
      <snr>13.15651728</snr>
      <pickID>20210430.052008.06-AIC-IU.HNR.00.BHZ</pickID>
      <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BH1"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick@gempa-proc</author>
        <creationTime>2021-04-30T05:23:21.795376Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052259.26-AIC-AU.EIDS..BHZ.Mjma">
      <type>Mjma</type>
      <amplitude>
        <value>0.4558309835</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:23:50.519538Z</reference>
        <begin>-56.25</begin>
        <end>98.75</end>
      </timeWindow>
      <snr>1.444974195</snr>
      <pickID>20210430.052259.26-AIC-AU.EIDS..BHZ</pickID>
      <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:25:49.894124Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.051926.35-AIC-G.SANVU.00.BHZ.Mjma">
      <type>Mjma</type>
      <amplitude>
        <value>10.52311843</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:20:26.55Z</reference>
        <begin>-65.2</begin>
        <end>89.8</end>
      </timeWindow>
      <snr>17.90946808</snr>
      <pickID>20210430.051926.35-AIC-G.SANVU.00.BHZ</pickID>
      <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:22:28.81448Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052008.06-AIC-IU.HNR.00.BHZ.Mjma">
      <type>Mjma</type>
      <amplitude>
        <value>0.185209429</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:20:07.894538Z</reference>
        <begin>-4.825</begin>
        <end>150.175</end>
      </timeWindow>
      <snr>3.321550049</snr>
      <pickID>20210430.052008.06-AIC-IU.HNR.00.BHZ</pickID>
      <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:23:06.520926Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052157.79-AIC-IU.TARA.00.BHZ.Mjma">
      <type>Mjma</type>
      <amplitude>
        <value>2.989746328</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:22:20.169539Z</reference>
        <begin>-27.375</begin>
        <end>127.625</end>
      </timeWindow>
      <snr>2.444952353</snr>
      <pickID>20210430.052157.79-AIC-IU.TARA.00.BHZ</pickID>
      <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:25:00.923516Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052008.06-AIC-IU.HNR.00.BHZ.mB">
      <type>mB</type>
      <amplitude>
        <value>349.581575</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:20:26.794538Z</reference>
        <begin>-23.725</begin>
        <end>41.275</end>
      </timeWindow>
      <snr>4.29662629</snr>
      <pickID>20210430.052008.06-AIC-IU.HNR.00.BHZ</pickID>
      <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BH1"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick@gempa-proc</author>
        <creationTime>2021-04-30T05:21:58.46752Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052157.79-AIC-IU.TARA.00.BHZ.mB">
      <type>mB</type>
      <amplitude>
        <value>6371.494598</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:22:49.294539Z</reference>
        <begin>-56.5</begin>
        <end>8.5</end>
      </timeWindow>
      <snr>3.305312741</snr>
      <pickID>20210430.052157.79-AIC-IU.TARA.00.BHZ</pickID>
      <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:23:46.906412Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052327.21-AIC-AU.ARMA..BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>43.66808098</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:23:33.294538Z</reference>
        <begin>-11.075</begin>
        <end>23.925</end>
      </timeWindow>
      <period>
        <value>0.8625</value>
      </period>
      <snr>6.897593303</snr>
      <pickID>20210430.052327.21-AIC-AU.ARMA..BHZ</pickID>
      <waveformID networkCode="AU" stationCode="ARMA" channelCode="BHE"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick@gempa-proc</author>
        <creationTime>2021-04-30T05:24:19.182722Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052411.24-AIC-AU.CMSA..BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>35.75319204</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:24:25.319539Z</reference>
        <begin>-19.075</begin>
        <end>15.925</end>
      </timeWindow>
      <period>
        <value>1.03125</value>
      </period>
      <snr>7.069458651</snr>
      <pickID>20210430.052411.24-AIC-AU.CMSA..BHZ</pickID>
      <waveformID networkCode="AU" stationCode="CMSA" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:25:06.319108Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052259.26-AIC-AU.EIDS..BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>18.38902195</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:23:14.594538Z</reference>
        <begin>-20.325</begin>
        <end>14.675</end>
      </timeWindow>
      <period>
        <value>0.6375</value>
      </period>
      <snr>1.491807803</snr>
      <pickID>20210430.052259.26-AIC-AU.EIDS..BHZ</pickID>
      <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:23:49.182707Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052658.56-AIC-AU.MEEK..BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>32.42962688</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:26:58.394538Z</reference>
        <begin>-4.825</begin>
        <end>30.175</end>
      </timeWindow>
      <period>
        <value>0.85</value>
      </period>
      <snr>2.12573443</snr>
      <pickID>20210430.052658.56-AIC-AU.MEEK..BHZ</pickID>
      <waveformID networkCode="AU" stationCode="MEEK" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:27:44.083264Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052611.16-AIC-GE.SANI..BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>24.03440182</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:26:25.769536Z</reference>
        <begin>-19.6</begin>
        <end>15.4</end>
      </timeWindow>
      <period>
        <value>1.025</value>
      </period>
      <snr>3.925169426</snr>
      <pickID>20210430.052611.16-AIC-GE.SANI..BHZ</pickID>
      <waveformID networkCode="GE" stationCode="SANI" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:26:54.981909Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052657.94-AIC-II.KAPI.00.BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>10.20030079</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:27:05.769538Z</reference>
        <begin>-12.825</begin>
        <end>22.175</end>
      </timeWindow>
      <period>
        <value>1.24375</value>
      </period>
      <snr>4.466271015</snr>
      <pickID>20210430.052657.94-AIC-II.KAPI.00.BHZ</pickID>
      <waveformID networkCode="II" stationCode="KAPI" locationCode="00" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:27:45.951586Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052449.69-AIC-II.WRAB.00.BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>5.947492013</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:24:49.919536Z</reference>
        <begin>-5.225</begin>
        <end>29.775</end>
      </timeWindow>
      <period>
        <value>1.33125</value>
      </period>
      <snr>2.70977146</snr>
      <pickID>20210430.052449.69-AIC-II.WRAB.00.BHZ</pickID>
      <waveformID networkCode="II" stationCode="WRAB" locationCode="00" channelCode="BH2"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick@gempa-proc</author>
        <creationTime>2021-04-30T05:25:40.133696Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052313.39-AIC-IU.CTAO.00.BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>10.43868701</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:23:14.869538Z</reference>
        <begin>-6.475</begin>
        <end>28.525</end>
      </timeWindow>
      <period>
        <value>0.9</value>
      </period>
      <snr>3.347318266</snr>
      <pickID>20210430.052313.39-AIC-IU.CTAO.00.BHZ</pickID>
      <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:24:32.283717Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052008.06-AIC-IU.HNR.00.BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>55.44284562</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:20:08.819538Z</reference>
        <begin>-5.75</begin>
        <end>29.25</end>
      </timeWindow>
      <period>
        <value>0.95</value>
      </period>
      <snr>4.397379224</snr>
      <pickID>20210430.052008.06-AIC-IU.HNR.00.BHZ</pickID>
      <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:21:27.131583Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052157.79-AIC-IU.TARA.00.BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>361.6859723</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:21:58.394539Z</reference>
        <begin>-5.6</begin>
        <end>29.4</end>
      </timeWindow>
      <period>
        <value>1.0125</value>
      </period>
      <snr>5.691724457</snr>
      <pickID>20210430.052157.79-AIC-IU.TARA.00.BHZ</pickID>
      <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:22:58.694485Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="Amplitude/20210430052823.012927.113567">
      <type>ML</type>
      <amplitude>
        <value>8.018396266</value>
        <lowerUncertainty>0.7022851286</lowerUncertainty>
        <upperUncertainty>0.7022851286</upperUncertainty>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:20:33.675001Z</reference>
        <begin>-72.324999</begin>
        <end>82.675</end>
      </timeWindow>
      <snr>0</snr>
      <pickID>20210430.051926.35-AIC-G.SANVU.00.BHZ</pickID>
      <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scamp</author>
        <creationTime>2021-04-30T05:28:23.012955Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="Amplitude/20210430052823.068657.113575">
      <type>ML</type>
      <amplitude>
        <value>0.08976952536</value>
        <lowerUncertainty>0.01726179769</lowerUncertainty>
        <upperUncertainty>0.01726179769</upperUncertainty>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:21:25.857038Z</reference>
        <begin>-82.787499</begin>
        <end>72.2125</end>
      </timeWindow>
      <snr>0</snr>
      <pickID>20210430.052008.06-AIC-IU.HNR.00.BHZ</pickID>
      <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BH"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scamp</author>
        <creationTime>2021-04-30T05:28:23.068684Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="Amplitude/20210430052822.994143.113564">
      <type>AMN</type>
      <amplitude>
        <value>1.752602771e-05</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:20:23.15Z</reference>
        <begin>-3.85</begin>
        <end>10</end>
      </timeWindow>
      <period>
        <value>0.7</value>
      </period>
      <snr>13.92031443</snr>
      <unit>m/s</unit>
      <pickID>20210430.051926.35-AIC-G.SANVU.00.BHZ</pickID>
      <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scamp</author>
        <creationTime>2021-04-30T05:28:22.994179Z</creationTime>
        <version>0.2.0</version>
      </creationInfo>
    </amplitude>
    <amplitude publicID="Amplitude/20210430052823.066898.113574">
      <type>AMN</type>
      <amplitude>
        <value>2.08488202e-07</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:22:00.494538Z</reference>
        <begin>-5.825</begin>
        <end>19.95</end>
      </timeWindow>
      <period>
        <value>0.95</value>
      </period>
      <snr>0.9880351338</snr>
      <unit>m/s</unit>
      <pickID>20210430.052008.06-AIC-IU.HNR.00.BHZ</pickID>
      <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scamp</author>
        <creationTime>2021-04-30T05:28:23.066931Z</creationTime>
        <version>0.2.0</version>
      </creationInfo>
    </amplitude>
    <amplitude publicID="Amplitude/20210430052823.094023.113580">
      <type>AMN</type>
      <amplitude>
        <value>2.645376479e-06</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:26:26.569539Z</reference>
        <begin>-21.325</begin>
        <end>35.775</end>
      </timeWindow>
      <period>
        <value>0.1</value>
      </period>
      <snr>1.187873258</snr>
      <unit>m/s</unit>
      <pickID>20210430.052157.79-AIC-IU.TARA.00.BHZ</pickID>
      <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scamp</author>
        <creationTime>2021-04-30T05:28:23.094055Z</creationTime>
        <version>0.2.0</version>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052659.61-AIC-IA.DBNI..BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>21.15591556</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:27:00.2125Z</reference>
        <begin>-5.6</begin>
        <end>29.4</end>
      </timeWindow>
      <period>
        <value>0.8625</value>
      </period>
      <snr>3.892127935</snr>
      <pickID>20210430.052659.61-AIC-IA.DBNI..BHZ</pickID>
      <waveformID networkCode="IA" stationCode="DBNI" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:27:57.627987Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052702.21-AIC-IA.MPSI..BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>10.24641003</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:27:26.4125Z</reference>
        <begin>-29.2</begin>
        <end>5.8</end>
      </timeWindow>
      <period>
        <value>0.8875</value>
      </period>
      <snr>4.154908099</snr>
      <pickID>20210430.052702.21-AIC-IA.MPSI..BHZ</pickID>
      <waveformID networkCode="IA" stationCode="MPSI" channelCode="BHN"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick@gempa-proc</author>
        <creationTime>2021-04-30T05:27:57.520265Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052645.86-AIC-IA.SMSI..BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>6.460316297</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:27:07.0375Z</reference>
        <begin>-26.15</begin>
        <end>8.85</end>
      </timeWindow>
      <period>
        <value>0.75</value>
      </period>
      <snr>2.13191063</snr>
      <pickID>20210430.052645.86-AIC-IA.SMSI..BHZ</pickID>
      <waveformID networkCode="IA" stationCode="SMSI" channelCode="BHE"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick@gempa-proc</author>
        <creationTime>2021-04-30T05:27:52.435174Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052742.81-AIC-MY.KKM..BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>27.87066016</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:27:43.619538Z</reference>
        <begin>-5.8</begin>
        <end>29.2</end>
      </timeWindow>
      <period>
        <value>1</value>
      </period>
      <snr>5.515766491</snr>
      <pickID>20210430.052742.81-AIC-MY.KKM..BHZ</pickID>
      <waveformID networkCode="MY" stationCode="KKM" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:28:52.26235Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052715.96-AIC-IA.SMKI..BHZ.mB">
      <type>mB</type>
      <amplitude>
        <value>13407.07143</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:27:46.2625Z</reference>
        <begin>-35.3</begin>
        <end>29.7</end>
      </timeWindow>
      <snr>4.478169551</snr>
      <pickID>20210430.052715.96-AIC-IA.SMKI..BHZ</pickID>
      <waveformID networkCode="IA" stationCode="SMKI" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:28:30.785912Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052559.83-AIC-IA.OBMI..BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>39.34274411</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:26:00.0375Z</reference>
        <begin>-5.2</begin>
        <end>29.8</end>
      </timeWindow>
      <period>
        <value>0.975</value>
      </period>
      <snr>7.845427662</snr>
      <pickID>20210430.052559.83-AIC-IA.OBMI..BHZ</pickID>
      <waveformID networkCode="IA" stationCode="OBMI" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:26:54.031823Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052715.96-AIC-IA.SMKI..BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>225.169672</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:27:34.6625Z</reference>
        <begin>-23.7</begin>
        <end>11.3</end>
      </timeWindow>
      <period>
        <value>0.4375</value>
      </period>
      <snr>1.820825486</snr>
      <pickID>20210430.052715.96-AIC-IA.SMKI..BHZ</pickID>
      <waveformID networkCode="IA" stationCode="SMKI" channelCode="BHN"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick@gempa-proc</author>
        <creationTime>2021-04-30T05:28:00.778393Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052825.23-AIC-IA.CGJI..BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>42.64781767</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:28:32.0875Z</reference>
        <begin>-11.85</begin>
        <end>23.15</end>
      </timeWindow>
      <period>
        <value>1.325</value>
      </period>
      <snr>3.533761817</snr>
      <pickID>20210430.052825.23-AIC-IA.CGJI..BHZ</pickID>
      <waveformID networkCode="IA" stationCode="CGJI" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:29:06.559968Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052909.46-AIC-IU.CASY.00.BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>24.54842846</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:29:10.069538Z</reference>
        <begin>-5.6</begin>
        <end>29.4</end>
      </timeWindow>
      <period>
        <value>1.5125</value>
      </period>
      <snr>2.655302456</snr>
      <pickID>20210430.052909.46-AIC-IU.CASY.00.BHZ</pickID>
      <waveformID networkCode="IU" stationCode="CASY" locationCode="00" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:30:45.48666Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052956.89-AIC-IU.CHTO.00.BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>11.48601042</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:29:57.794538Z</reference>
        <begin>-5.9</begin>
        <end>29.1</end>
      </timeWindow>
      <period>
        <value>1.225</value>
      </period>
      <snr>11.48709386</snr>
      <pickID>20210430.052956.89-AIC-IU.CHTO.00.BHZ</pickID>
      <waveformID networkCode="IU" stationCode="CHTO" locationCode="00" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:31:05.687743Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052911.41-AIC-IU.SBA.00.BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>4.381251034</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:29:12.769536Z</reference>
        <begin>-6.35</begin>
        <end>28.65</end>
      </timeWindow>
      <period>
        <value>1.1125</value>
      </period>
      <snr>1.468795842</snr>
      <pickID>20210430.052911.41-AIC-IU.SBA.00.BHZ</pickID>
      <waveformID networkCode="IU" stationCode="SBA" locationCode="00" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:30:04.844031Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052936.16-AIC-RM.SLV..BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>4.32492644</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:30:03.119538Z</reference>
        <begin>-31.95</begin>
        <end>3.05</end>
      </timeWindow>
      <period>
        <value>0.9625</value>
      </period>
      <snr>3.209449547</snr>
      <pickID>20210430.052936.16-AIC-RM.SLV..BHZ</pickID>
      <waveformID networkCode="RM" stationCode="SLV" channelCode="BHN"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick@gempa-proc</author>
        <creationTime>2021-04-30T05:30:38.794035Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052807.66-AIC-TW.NACB..BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>26.40865456</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:28:11.619538Z</reference>
        <begin>-8.95</begin>
        <end>26.05</end>
      </timeWindow>
      <period>
        <value>0.9875</value>
      </period>
      <snr>2.909264347</snr>
      <pickID>20210430.052807.66-AIC-TW.NACB..BHZ</pickID>
      <waveformID networkCode="TW" stationCode="NACB" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:29:03.753072Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052809.66-AIC-TW.SSLB..BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>18.48259311</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:28:10.769539Z</reference>
        <begin>-6.1</begin>
        <end>28.9</end>
      </timeWindow>
      <period>
        <value>1.1</value>
      </period>
      <snr>3.235455766</snr>
      <pickID>20210430.052809.66-AIC-TW.SSLB..BHZ</pickID>
      <waveformID networkCode="TW" stationCode="SSLB" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:29:02.253781Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052809.21-AIC-TW.TPUB..BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>64.37275829</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:28:09.919538Z</reference>
        <begin>-5.7</begin>
        <end>29.3</end>
      </timeWindow>
      <period>
        <value>1.2</value>
      </period>
      <snr>5.143576028</snr>
      <pickID>20210430.052809.21-AIC-TW.TPUB..BHZ</pickID>
      <waveformID networkCode="TW" stationCode="TPUB" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:29:06.727282Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052810.81-AIC-TW.YHNB..BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>18.19682163</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:28:16.569538Z</reference>
        <begin>-10.75</begin>
        <end>24.25</end>
      </timeWindow>
      <period>
        <value>0.925</value>
      </period>
      <snr>2.363270852</snr>
      <pickID>20210430.052810.81-AIC-TW.YHNB..BHZ</pickID>
      <waveformID networkCode="TW" stationCode="YHNB" channelCode="BHE"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick@gempa-proc</author>
        <creationTime>2021-04-30T05:29:13.642733Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="Amplitude/20210430053140.209493.114038">
      <type>AMN</type>
      <amplitude>
        <value>1.283514247e-07</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:29:04.219539Z</reference>
        <begin>-23.025</begin>
        <end>53.05</end>
      </timeWindow>
      <period>
        <value>0.9</value>
      </period>
      <snr>0.8075</snr>
      <unit>m/s</unit>
      <pickID>20210430.052259.26-AIC-AU.EIDS..BHZ</pickID>
      <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scamp</author>
        <creationTime>2021-04-30T05:31:40.209557Z</creationTime>
        <version>0.2.0</version>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052956.89-AIC-IU.CHTO.00.BHZ.mB">
      <type>mB</type>
      <amplitude>
        <value>145.3844009</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:29:56.769538Z</reference>
        <begin>-4.875</begin>
        <end>60.125</end>
      </timeWindow>
      <snr>3.462214042</snr>
      <pickID>20210430.052956.89-AIC-IU.CHTO.00.BHZ</pickID>
      <waveformID networkCode="IU" stationCode="CHTO" locationCode="00" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:31:54.867186Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052908.75-AIC-GT.VNDA.00.BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>8.501078336</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:29:23.9Z</reference>
        <begin>-20.15</begin>
        <end>14.85</end>
      </timeWindow>
      <period>
        <value>1.075</value>
      </period>
      <snr>2.916634577</snr>
      <pickID>20210430.052908.75-AIC-GT.VNDA.00.BHZ</pickID>
      <waveformID networkCode="GT" stationCode="VNDA" locationCode="00" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:30:58.050584Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="Amplitude/20210430053131.536021.113922">
      <type>mb</type>
      <amplitude>
        <value>24.54842846</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:29:10.069538Z</reference>
        <begin>-5.6</begin>
        <end>29.4</end>
      </timeWindow>
      <period>
        <value>1.5125</value>
      </period>
      <snr>2.655302456</snr>
      <pickID>20210430.052909.46-AIC-IU.CASY.00.BHZ</pickID>
      <waveformID networkCode="IU" stationCode="CASY" locationCode="00" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scamp</author>
        <creationTime>2021-04-30T05:31:31.536053Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="Amplitude/20210430053158.72278.114063">
      <type>AMN</type>
      <amplitude>
        <value>9.688321078e-08</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:29:23.669538Z</reference>
        <begin>-3.575</begin>
        <end>77.35</end>
      </timeWindow>
      <period>
        <value>2.35</value>
      </period>
      <snr>0.6197674419</snr>
      <unit>m/s</unit>
      <pickID>20210430.052313.39-AIC-IU.CTAO.00.BHZ</pickID>
      <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scamp</author>
        <creationTime>2021-04-30T05:31:58.722839Z</creationTime>
        <version>0.2.0</version>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.053031.16-AIC-IU.ULN.00.BHZ.mB">
      <type>mB</type>
      <amplitude>
        <value>961.0055331</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:30:30.919538Z</reference>
        <begin>-4.75</begin>
        <end>60.25</end>
      </timeWindow>
      <snr>3.505988967</snr>
      <pickID>20210430.053031.16-AIC-IU.ULN.00.BHZ</pickID>
      <waveformID networkCode="IU" stationCode="ULN" locationCode="00" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:32:05.889669Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.053054.31-AIC-IU.COLA.00.BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>2.282514843</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:30:55.944538Z</reference>
        <begin>-6.625</begin>
        <end>28.375</end>
      </timeWindow>
      <period>
        <value>0.66875</value>
      </period>
      <snr>1.384748419</snr>
      <pickID>20210430.053054.31-AIC-IU.COLA.00.BHZ</pickID>
      <waveformID networkCode="IU" stationCode="COLA" locationCode="00" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:31:44.496495Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.053031.16-AIC-IU.ULN.00.BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>55.68894732</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:30:31.419538Z</reference>
        <begin>-5.25</begin>
        <end>29.75</end>
      </timeWindow>
      <period>
        <value>0.9125</value>
      </period>
      <snr>9.586594101</snr>
      <pickID>20210430.053031.16-AIC-IU.ULN.00.BHZ</pickID>
      <waveformID networkCode="IU" stationCode="ULN" locationCode="00" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T05:32:05.883375Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="Amplitude/20210430053830.002206.114525">
      <type>AMN</type>
      <amplitude>
        <value>3.961025708e-07</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:32:47.194539Z</reference>
        <begin>-11.225</begin>
        <end>94.2</end>
      </timeWindow>
      <period>
        <value>3.9</value>
      </period>
      <snr>3.216129032</snr>
      <unit>m/s</unit>
      <pickID>20210430.052411.24-AIC-AU.CMSA..BHZ</pickID>
      <waveformID networkCode="AU" stationCode="CMSA" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scamp</author>
        <creationTime>2021-04-30T05:38:30.002251Z</creationTime>
        <version>0.2.0</version>
      </creationInfo>
    </amplitude>
    <amplitude publicID="Amplitude/20210430053830.009.114526">
      <type>AMN</type>
      <amplitude>
        <value>2.248765878e-07</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:31:00.169539Z</reference>
        <begin>-38.9</begin>
        <end>49.675</end>
      </timeWindow>
      <period>
        <value>1.9</value>
      </period>
      <snr>2.084714549</snr>
      <unit>m/s</unit>
      <pickID>20210430.052327.21-AIC-AU.ARMA..BHZ</pickID>
      <waveformID networkCode="AU" stationCode="ARMA" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scamp</author>
        <creationTime>2021-04-30T05:38:30.009038Z</creationTime>
        <version>0.2.0</version>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052930.06-AIC-IC.ENH.00.BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>249.882019</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:29:30.619538Z</reference>
        <begin>-5.55</begin>
        <end>29.45</end>
      </timeWindow>
      <period>
        <value>0.9125</value>
      </period>
      <snr>6.233028308</snr>
      <pickID>20210430.052930.06-AIC-IC.ENH.00.BHZ</pickID>
      <waveformID networkCode="IC" stationCode="ENH" locationCode="00" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T06:03:21.886172Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052250.71-AIC-GE.PMG.00.BHZ.Mjma">
      <type>Mjma</type>
      <amplitude>
        <value>0.4652322066</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:23:22.019538Z</reference>
        <begin>-36.3</begin>
        <end>118.7</end>
      </timeWindow>
      <snr>1.190461349</snr>
      <pickID>20210430.052250.71-AIC-GE.PMG.00.BHZ</pickID>
      <waveformID networkCode="GE" stationCode="PMG" locationCode="00" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T09:49:47.454008Z</creationTime>
      </creationInfo>
    </amplitude>
    <amplitude publicID="20210430.052250.71-AIC-GE.PMG.00.BHZ.mb">
      <type>mb</type>
      <amplitude>
        <value>36.93509833</value>
      </amplitude>
      <timeWindow>
        <reference>2021-04-30T05:23:07.169538Z</reference>
        <begin>-21.45</begin>
        <end>13.55</end>
      </timeWindow>
      <period>
        <value>1.1625</value>
      </period>
      <snr>3.969661699</snr>
      <pickID>20210430.052250.71-AIC-GE.PMG.00.BHZ</pickID>
      <waveformID networkCode="GE" stationCode="PMG" locationCode="00" channelCode="BHZ"/>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautopick</author>
        <creationTime>2021-04-30T09:49:23.38231Z</creationTime>
      </creationInfo>
    </amplitude>
    <origin publicID="Origin/20210430052734.985523.85085">
      <time>
        <value>2021-04-30T05:18:28.470972Z</value>
        <uncertainty>0.4527335614</uncertainty>
      </time>
      <latitude>
        <value>-11.95924759</value>
        <uncertainty>4.962128264</uncertainty>
      </latitude>
      <longitude>
        <value>166.247757</value>
        <uncertainty>6.254166092</uncertainty>
      </longitude>
      <depth>
        <value>10</value>
        <uncertainty>0</uncertainty>
      </depth>
      <methodID>LOCSAT</methodID>
      <earthModelID>iasp91</earthModelID>
      <quality>
        <associatedPhaseCount>15</associatedPhaseCount>
        <usedPhaseCount>11</usedPhaseCount>
        <associatedStationCount>15</associatedStationCount>
        <usedStationCount>11</usedStationCount>
        <standardError>2.274534135</standardError>
        <azimuthalGap>138.2900352</azimuthalGap>
        <maximumDistance>46.98543549</maximumDistance>
        <minimumDistance>3.609270334</minimumDistance>
        <medianDistance>22.89196587</medianDistance>
      </quality>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>autoloc</author>
        <creationTime>2021-04-30T05:27:34.985636Z</creationTime>
      </creationInfo>
      <arrival>
        <pickID>20210430.051926.35-AIC-G.SANVU.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>165.2073517</azimuth>
        <distance>3.609270334</distance>
        <timeResidual>2.145641327</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052008.06-AIC-IU.HNR.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>291.5462952</azimuth>
        <distance>6.683574677</distance>
        <timeResidual>1.358402252</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052157.79-AIC-IU.TARA.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>26.91731644</azimuth>
        <distance>14.87410355</distance>
        <timeResidual>-0.1393585205</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052259.26-AIC-AU.EIDS..BHZ</pickID>
        <phase>P</phase>
        <azimuth>224.7529449</azimuth>
        <distance>19.6184597</distance>
        <timeResidual>2.529037476</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052313.39-AIC-IU.CTAO.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>244.5045471</azimuth>
        <distance>20.83959389</distance>
        <timeResidual>2.966781616</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052327.21-AIC-AU.ARMA..BHZ</pickID>
        <phase>P</phase>
        <azimuth>214.0210114</azimuth>
        <distance>22.89196587</distance>
        <timeResidual>-4.499908447</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052400.85-AIC-G.CAN.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>210.9136047</azimuth>
        <distance>28.10027313</distance>
        <timeResidual>-18.95721436</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052411.24-AIC-AU.CMSA..BHZ</pickID>
        <phase>P</phase>
        <azimuth>220.8295288</azimuth>
        <distance>27.24019623</distance>
        <timeResidual>-1.105285645</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052449.69-AIC-II.WRAB.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>251.2962952</azimuth>
        <distance>31.62089539</distance>
        <timeResidual>-2.216125488</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052611.16-AIC-GE.SANI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>280.786438</azimuth>
        <distance>41.10609055</distance>
        <timeResidual>-1.622711182</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052645.86-AIC-IA.SMSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>283.4856262</azimuth>
        <distance>45.45728683</distance>
        <timeResidual>-2.042907715</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052657.94-AIC-II.KAPI.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>274.4787903</azimuth>
        <distance>46.44925308</distance>
        <timeResidual>2.031890869</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052658.56-AIC-AU.MEEK..BHZ</pickID>
        <phase>P</phase>
        <azimuth>244.5848389</azimuth>
        <distance>46.98543549</distance>
        <timeResidual>-1.448608398</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052659.61-AIC-IA.DBNI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>269.427948</azimuth>
        <distance>47.24565506</distance>
        <timeResidual>-2.597045898</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052702.21-AIC-IA.MPSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>281.6201782</azimuth>
        <distance>47.62031174</distance>
        <timeResidual>-2.725524902</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <stationMagnitude publicID="Origin/20210430052734.985523.85085/staMag/mb/AU.ARMA">
        <magnitude>
          <value>4.912059716</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052327.21-AIC-AU.ARMA..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="ARMA" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:27:34.998152Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052734.985523.85085/staMag/mb/AU.CMSA">
        <magnitude>
          <value>5.045564648</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052411.24-AIC-AU.CMSA..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="CMSA" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:27:34.998567Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052734.985523.85085/staMag/Mjma/AU.EIDS">
        <magnitude>
          <value>5.044422011</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052259.26-AIC-AU.EIDS..BHZ.Mjma</amplitudeID>
        <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:27:34.998883Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052734.985523.85085/staMag/mb/AU.EIDS">
        <magnitude>
          <value>4.537063088</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052259.26-AIC-AU.EIDS..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:27:34.999024Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052734.985523.85085/staMag/MLv/G.SANVU">
        <magnitude>
          <value>5.440807683</value>
        </magnitude>
        <type>MLv</type>
        <amplitudeID>20210430.051926.35-AIC-G.SANVU.00.BHZ.MLv</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:27:34.999313Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052734.985523.85085/staMag/Mjma/G.SANVU">
        <magnitude>
          <value>5.135787921</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.051926.35-AIC-G.SANVU.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:27:34.999442Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052734.985523.85085/staMag/mb/GE.SANI">
        <magnitude>
          <value>5.04450942</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052611.16-AIC-GE.SANI..BHZ.mb</amplitudeID>
        <waveformID networkCode="GE" stationCode="SANI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:27:34.999877Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052734.985523.85085/staMag/mb/II.WRAB">
        <magnitude>
          <value>4.572225141</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052449.69-AIC-II.WRAB.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="II" stationCode="WRAB" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:27:35.000043Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052734.985523.85085/staMag/mb/IU.CTAO">
        <magnitude>
          <value>4.166696894</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052313.39-AIC-IU.CTAO.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:27:35.000464Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052734.985523.85085/staMag/MLv/IU.HNR">
        <magnitude>
          <value>4.489427983</value>
        </magnitude>
        <type>MLv</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.MLv</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:27:35.001247Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052734.985523.85085/staMag/Mjma/IU.HNR">
        <magnitude>
          <value>3.844236206</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:27:35.001377Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052734.985523.85085/staMag/mB/IU.HNR">
        <magnitude>
          <value>4.609033781</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:27:35.001515Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052734.985523.85085/staMag/mb/IU.HNR">
        <magnitude>
          <value>4.684600197</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:27:35.001648Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052734.985523.85085/staMag/Mjma/IU.TARA">
        <magnitude>
          <value>5.653247507</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:27:35.00201Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052734.985523.85085/staMag/mB/IU.TARA">
        <magnitude>
          <value>6.03421546</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:27:35.002157Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052734.985523.85085/staMag/mb/IU.TARA">
        <magnitude>
          <value>5.721090643</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:27:35.002281Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052734.985523.85085/staMag/mb/AU.MEEK">
        <magnitude>
          <value>5.301377175</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052658.56-AIC-AU.MEEK..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="MEEK" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:27:44.087941Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052734.985523.85085/staMag/mb/II.KAPI">
        <magnitude>
          <value>4.628372342</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052657.94-AIC-II.KAPI.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="II" stationCode="KAPI" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:27:45.95516Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052734.985523.85085/staMag/ML/G.SANVU">
        <magnitude>
          <value>5.406582164</value>
        </magnitude>
        <type>ML</type>
        <amplitudeID>Amplitude/20210430052823.012927.113567</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:23.016834Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052734.985523.85085/staMag/ML/IU.HNR">
        <magnitude>
          <value>4.224350484</value>
        </magnitude>
        <type>ML</type>
        <amplitudeID>Amplitude/20210430052823.068657.113575</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BH"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:23.080859Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052734.985523.85085/staMag/MN/G.SANVU">
        <magnitude>
          <value>4.666824187</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052822.994143.113564</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:22.998728Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052734.985523.85085/staMag/MN/IU.HNR">
        <magnitude>
          <value>3.189921589</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052823.066898.113574</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:23.071023Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052734.985523.85085/staMag/MN/IU.TARA">
        <magnitude>
          <value>4.866780119</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052823.094023.113580</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:23.098072Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <magnitude publicID="Origin/20210430052734.985523.85085/netMag/MLv">
        <magnitude>
          <value>4.965117833</value>
          <uncertainty>0.6727270375</uncertainty>
        </magnitude>
        <type>MLv</type>
        <methodID>mean</methodID>
        <stationCount>2</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:27:35.002319Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052734.985523.85085/staMag/MLv/G.SANVU</stationMagnitudeID>
          <residual>0.4756898501</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052734.985523.85085/staMag/MLv/IU.HNR</stationMagnitudeID>
          <residual>-0.4756898501</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430052734.985523.85085/netMag/Mjma">
        <magnitude>
          <value>4.976317263</value>
          <uncertainty>0.6708167354</uncertainty>
        </magnitude>
        <type>Mjma</type>
        <methodID>trimmed mean(25)</methodID>
        <stationCount>4</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:27:35.002402Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052734.985523.85085/staMag/Mjma/AU.EIDS</stationMagnitudeID>
          <residual>0.06810474803</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052734.985523.85085/staMag/Mjma/G.SANVU</stationMagnitudeID>
          <residual>0.1594706585</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052734.985523.85085/staMag/Mjma/IU.HNR</stationMagnitudeID>
          <residual>-1.132081057</residual>
          <weight>0.5</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052734.985523.85085/staMag/Mjma/IU.TARA</stationMagnitudeID>
          <residual>0.6769302439</residual>
          <weight>0.5</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430052734.985523.85085/netMag/mB">
        <magnitude>
          <value>5.321624621</value>
          <uncertainty>1.00775563</uncertainty>
        </magnitude>
        <type>mB</type>
        <methodID>mean</methodID>
        <stationCount>2</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:27:35.002461Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052734.985523.85085/staMag/mB/IU.HNR</stationMagnitudeID>
          <residual>-0.7125908394</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052734.985523.85085/staMag/mB/IU.TARA</stationMagnitudeID>
          <residual>0.7125908394</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430052734.985523.85085/netMag/Mw(mB)">
        <magnitude>
          <value>4.738112007</value>
          <uncertainty>1.00775563</uncertainty>
        </magnitude>
        <type>Mw(mB)</type>
        <stationCount>2</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:27:35.002509Z</creationTime>
        </creationInfo>
      </magnitude>
      <magnitude publicID="Origin/20210430052734.985523.85085/netMag/mb">
        <magnitude>
          <value>4.835488222</value>
          <uncertainty>0.2655152756</uncertainty>
        </magnitude>
        <type>mb</type>
        <methodID>trimmed mean(25)</methodID>
        <stationCount>8</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:27:35.002531Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052734.985523.85085/staMag/mb/AU.ARMA</stationMagnitudeID>
          <residual>0.0765714948</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052734.985523.85085/staMag/mb/AU.CMSA</stationMagnitudeID>
          <residual>0.2100764264</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052734.985523.85085/staMag/mb/AU.EIDS</stationMagnitudeID>
          <residual>-0.2984251333</residual>
          <weight>0.75</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052734.985523.85085/staMag/mb/AU.MEEK</stationMagnitudeID>
          <residual>0.4658889532</residual>
          <weight>0.75</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052734.985523.85085/staMag/mb/GE.SANI</stationMagnitudeID>
          <residual>0.2090211979</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052734.985523.85085/staMag/mb/II.KAPI</stationMagnitudeID>
          <residual>-0.2071158792</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052734.985523.85085/staMag/mb/II.WRAB</stationMagnitudeID>
          <residual>-0.2632630804</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052734.985523.85085/staMag/mb/IU.CTAO</stationMagnitudeID>
          <residual>-0.6687913276</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052734.985523.85085/staMag/mb/IU.HNR</stationMagnitudeID>
          <residual>-0.1508880244</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052734.985523.85085/staMag/mb/IU.TARA</stationMagnitudeID>
          <residual>0.885602421</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430052734.985523.85085/netMag/M">
        <magnitude>
          <value>4.870721944</value>
        </magnitude>
        <type>M</type>
        <methodID>weighted average</methodID>
        <stationCount>8</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:27:35.002631Z</creationTime>
          <modificationTime>2021-04-30T05:28:23.081008Z</modificationTime>
        </creationInfo>
      </magnitude>
      <magnitude publicID="Origin/20210430052734.985523.85085/netMag/ML">
        <magnitude>
          <value>4.815466324</value>
          <uncertainty>0.8359640377</uncertainty>
        </magnitude>
        <type>ML</type>
        <methodID>mean</methodID>
        <stationCount>2</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:23.016873Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052734.985523.85085/staMag/ML/G.SANVU</stationMagnitudeID>
          <residual>0.5911158399</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052734.985523.85085/staMag/ML/IU.HNR</stationMagnitudeID>
          <residual>-0.5911158399</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430052734.985523.85085/netMag/MN">
        <magnitude>
          <value>4.666824187</value>
          <uncertainty>0</uncertainty>
        </magnitude>
        <type>MN</type>
        <methodID>mean</methodID>
        <stationCount>1</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:22.998779Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052734.985523.85085/staMag/MN/G.SANVU</stationMagnitudeID>
          <residual>0</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052734.985523.85085/staMag/MN/IU.HNR</stationMagnitudeID>
          <residual>-1.476902598</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052734.985523.85085/staMag/MN/IU.TARA</stationMagnitudeID>
          <residual>0.199955932</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
      </magnitude>
    </origin>
    <origin publicID="Origin/20210430052852.335811.85253">
      <time>
        <value>2021-04-30T05:18:28.470972Z</value>
        <uncertainty>0.4527335614</uncertainty>
      </time>
      <latitude>
        <value>-11.95924759</value>
        <uncertainty>4.962128264</uncertainty>
      </latitude>
      <longitude>
        <value>166.247757</value>
        <uncertainty>6.254166092</uncertainty>
      </longitude>
      <depth>
        <value>10</value>
        <uncertainty>0</uncertainty>
      </depth>
      <methodID>LOCSAT</methodID>
      <earthModelID>iasp91</earthModelID>
      <quality>
        <associatedPhaseCount>19</associatedPhaseCount>
        <usedPhaseCount>15</usedPhaseCount>
        <associatedStationCount>19</associatedStationCount>
        <usedStationCount>15</usedStationCount>
        <standardError>2.303505354</standardError>
        <azimuthalGap>138.2900352</azimuthalGap>
        <maximumDistance>52.90841293</maximumDistance>
        <minimumDistance>3.609270334</minimumDistance>
        <medianDistance>31.62089539</medianDistance>
      </quality>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>autoloc</author>
        <creationTime>2021-04-30T05:28:52.335946Z</creationTime>
      </creationInfo>
      <arrival>
        <pickID>20210430.051926.35-AIC-G.SANVU.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>165.2073517</azimuth>
        <distance>3.609270334</distance>
        <timeResidual>2.145641327</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052008.06-AIC-IU.HNR.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>291.5462952</azimuth>
        <distance>6.683574677</distance>
        <timeResidual>1.358402252</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052157.79-AIC-IU.TARA.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>26.91731644</azimuth>
        <distance>14.87410355</distance>
        <timeResidual>-0.1393585205</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052259.26-AIC-AU.EIDS..BHZ</pickID>
        <phase>P</phase>
        <azimuth>224.7529449</azimuth>
        <distance>19.6184597</distance>
        <timeResidual>2.529037476</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052313.39-AIC-IU.CTAO.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>244.5045471</azimuth>
        <distance>20.83959389</distance>
        <timeResidual>2.966781616</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052327.21-AIC-AU.ARMA..BHZ</pickID>
        <phase>P</phase>
        <azimuth>214.0210114</azimuth>
        <distance>22.89196587</distance>
        <timeResidual>-4.499908447</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052400.85-AIC-G.CAN.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>210.9136047</azimuth>
        <distance>28.10027313</distance>
        <timeResidual>-18.95721436</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052411.24-AIC-AU.CMSA..BHZ</pickID>
        <phase>P</phase>
        <azimuth>220.8295288</azimuth>
        <distance>27.24019623</distance>
        <timeResidual>-1.105285645</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052449.69-AIC-II.WRAB.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>251.2962952</azimuth>
        <distance>31.62089539</distance>
        <timeResidual>-2.216125488</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052611.16-AIC-GE.SANI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>280.786438</azimuth>
        <distance>41.10609055</distance>
        <timeResidual>-1.622711182</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052645.86-AIC-IA.SMSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>283.4856262</azimuth>
        <distance>45.45728683</distance>
        <timeResidual>-2.042907715</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052657.94-AIC-II.KAPI.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>274.4787903</azimuth>
        <distance>46.44925308</distance>
        <timeResidual>2.031890869</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052658.56-AIC-AU.MEEK..BHZ</pickID>
        <phase>P</phase>
        <azimuth>244.5848389</azimuth>
        <distance>46.98543549</distance>
        <timeResidual>-1.448608398</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052659.61-AIC-IA.DBNI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>269.427948</azimuth>
        <distance>47.24565506</distance>
        <timeResidual>-2.597045898</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052702.21-AIC-IA.MPSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>281.6201782</azimuth>
        <distance>47.62031174</distance>
        <timeResidual>-2.725524902</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052742.81-AIC-MY.KKM..BHZ</pickID>
        <phase>P</phase>
        <azimuth>287.162262</azimuth>
        <distance>52.90841293</distance>
        <timeResidual>-2.082519531</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052807.66-AIC-TW.NACB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>309.8468018</azimuth>
        <distance>56.62950134</distance>
        <timeResidual>-3.370387554</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052809.21-AIC-TW.TPUB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>308.3904114</azimuth>
        <distance>56.873806</distance>
        <timeResidual>-3.620946646</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052810.81-AIC-TW.YHNB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>310.1983032</azimuth>
        <distance>57.07614517</distance>
        <timeResidual>-3.378571272</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/mb/AU.ARMA">
        <magnitude>
          <value>4.912059716</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052327.21-AIC-AU.ARMA..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="ARMA" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.352694Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/mb/AU.CMSA">
        <magnitude>
          <value>5.045564648</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052411.24-AIC-AU.CMSA..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="CMSA" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.353106Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/Mjma/AU.EIDS">
        <magnitude>
          <value>5.044422011</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052259.26-AIC-AU.EIDS..BHZ.Mjma</amplitudeID>
        <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.353431Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/mb/AU.EIDS">
        <magnitude>
          <value>4.537063088</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052259.26-AIC-AU.EIDS..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.353576Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/mb/AU.MEEK">
        <magnitude>
          <value>5.301377175</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052658.56-AIC-AU.MEEK..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="MEEK" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.353715Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/MN/G.SANVU">
        <magnitude>
          <value>4.666824187</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052822.994143.113564</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.353998Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/ML/G.SANVU">
        <magnitude>
          <value>5.406582164</value>
        </magnitude>
        <type>ML</type>
        <amplitudeID>Amplitude/20210430052823.012927.113567</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.354185Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/MLv/G.SANVU">
        <magnitude>
          <value>5.440807683</value>
        </magnitude>
        <type>MLv</type>
        <amplitudeID>20210430.051926.35-AIC-G.SANVU.00.BHZ.MLv</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.354388Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/Mjma/G.SANVU">
        <magnitude>
          <value>5.135787921</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.051926.35-AIC-G.SANVU.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.354541Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/mb/GE.SANI">
        <magnitude>
          <value>5.04450942</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052611.16-AIC-GE.SANI..BHZ.mb</amplitudeID>
        <waveformID networkCode="GE" stationCode="SANI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.354975Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/mb/IA.DBNI">
        <magnitude>
          <value>5.111834274</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052659.61-AIC-IA.DBNI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="DBNI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.355134Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/mb/IA.MPSI">
        <magnitude>
          <value>5.014668345</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052702.21-AIC-IA.MPSI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="MPSI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.355914Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/mb/IA.SMSI">
        <magnitude>
          <value>4.710833956</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052645.86-AIC-IA.SMSI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="SMSI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.356078Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/mb/II.KAPI">
        <magnitude>
          <value>4.628372342</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052657.94-AIC-II.KAPI.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="II" stationCode="KAPI" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.356237Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/mb/II.WRAB">
        <magnitude>
          <value>4.572225141</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052449.69-AIC-II.WRAB.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="II" stationCode="WRAB" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.356639Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/mb/IU.CTAO">
        <magnitude>
          <value>4.166696894</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052313.39-AIC-IU.CTAO.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.357039Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/MN/IU.HNR">
        <magnitude>
          <value>3.189921589</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052823.066898.113574</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.357329Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/ML/IU.HNR">
        <magnitude>
          <value>4.224350484</value>
        </magnitude>
        <type>ML</type>
        <amplitudeID>Amplitude/20210430052823.068657.113575</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BH"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.357521Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/MLv/IU.HNR">
        <magnitude>
          <value>4.489427983</value>
        </magnitude>
        <type>MLv</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.MLv</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.357707Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/Mjma/IU.HNR">
        <magnitude>
          <value>3.844236206</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.357824Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/mB/IU.HNR">
        <magnitude>
          <value>4.609033781</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.357964Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/mb/IU.HNR">
        <magnitude>
          <value>4.684600197</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.358097Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/MN/IU.TARA">
        <magnitude>
          <value>4.866780119</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052823.094023.113580</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.358421Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/Mjma/IU.TARA">
        <magnitude>
          <value>5.653247507</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.358698Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/mB/IU.TARA">
        <magnitude>
          <value>6.03421546</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.358836Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/mb/IU.TARA">
        <magnitude>
          <value>5.721090643</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.358967Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052852.335811.85253/staMag/mb/MY.KKM">
        <magnitude>
          <value>5.209831479</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052742.81-AIC-MY.KKM..BHZ.mb</amplitudeID>
        <waveformID networkCode="MY" stationCode="KKM" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.359111Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <magnitude publicID="Origin/20210430052852.335811.85253/netMag/ML">
        <magnitude>
          <value>4.815466324</value>
          <uncertainty>0.8359640377</uncertainty>
        </magnitude>
        <type>ML</type>
        <methodID>mean</methodID>
        <stationCount>2</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.359142Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/ML/G.SANVU</stationMagnitudeID>
          <residual>0.5911158399</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/ML/IU.HNR</stationMagnitudeID>
          <residual>-0.5911158399</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430052852.335811.85253/netMag/MLv">
        <magnitude>
          <value>4.965117833</value>
          <uncertainty>0.6727270375</uncertainty>
        </magnitude>
        <type>MLv</type>
        <methodID>mean</methodID>
        <stationCount>2</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.359229Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/MLv/G.SANVU</stationMagnitudeID>
          <residual>0.4756898501</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/MLv/IU.HNR</stationMagnitudeID>
          <residual>-0.4756898501</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430052852.335811.85253/netMag/MN">
        <magnitude>
          <value>4.666824187</value>
          <uncertainty>0</uncertainty>
        </magnitude>
        <type>MN</type>
        <methodID>mean</methodID>
        <stationCount>1</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.359283Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/MN/G.SANVU</stationMagnitudeID>
          <residual>0</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/MN/IU.HNR</stationMagnitudeID>
          <residual>-1.476902598</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/MN/IU.TARA</stationMagnitudeID>
          <residual>0.199955932</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430052852.335811.85253/netMag/Mjma">
        <magnitude>
          <value>4.976317263</value>
          <uncertainty>0.6708167354</uncertainty>
        </magnitude>
        <type>Mjma</type>
        <methodID>trimmed mean(25)</methodID>
        <stationCount>4</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.359357Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/Mjma/AU.EIDS</stationMagnitudeID>
          <residual>0.06810474803</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/Mjma/G.SANVU</stationMagnitudeID>
          <residual>0.1594706585</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/Mjma/IU.HNR</stationMagnitudeID>
          <residual>-1.132081057</residual>
          <weight>0.5</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/Mjma/IU.TARA</stationMagnitudeID>
          <residual>0.6769302439</residual>
          <weight>0.5</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430052852.335811.85253/netMag/mB">
        <magnitude>
          <value>5.321624621</value>
          <uncertainty>1.00775563</uncertainty>
        </magnitude>
        <type>mB</type>
        <methodID>mean</methodID>
        <stationCount>2</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.359416Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/mB/IU.HNR</stationMagnitudeID>
          <residual>-0.7125908394</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/mB/IU.TARA</stationMagnitudeID>
          <residual>0.7125908394</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430052852.335811.85253/netMag/Mw(mB)">
        <magnitude>
          <value>4.738112007</value>
          <uncertainty>1.00775563</uncertainty>
        </magnitude>
        <type>Mw(mB)</type>
        <stationCount>2</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.359473Z</creationTime>
        </creationInfo>
      </magnitude>
      <magnitude publicID="Origin/20210430052852.335811.85253/netMag/mb">
        <magnitude>
          <value>4.894677103</value>
          <uncertainty>0.2367640687</uncertainty>
        </magnitude>
        <type>mb</type>
        <methodID>trimmed mean(25)</methodID>
        <stationCount>12</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.359498Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/mb/AU.ARMA</stationMagnitudeID>
          <residual>0.01738261319</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/mb/AU.CMSA</stationMagnitudeID>
          <residual>0.1508875447</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/mb/AU.EIDS</stationMagnitudeID>
          <residual>-0.3576140149</residual>
          <weight>0.25</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/mb/AU.MEEK</stationMagnitudeID>
          <residual>0.4067000716</residual>
          <weight>0.25</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/mb/GE.SANI</stationMagnitudeID>
          <residual>0.1498323163</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/mb/IA.DBNI</stationMagnitudeID>
          <residual>0.2171571706</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/mb/IA.MPSI</stationMagnitudeID>
          <residual>0.1199912417</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/mb/IA.SMSI</stationMagnitudeID>
          <residual>-0.1838431473</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/mb/II.KAPI</stationMagnitudeID>
          <residual>-0.2663047608</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/mb/II.WRAB</stationMagnitudeID>
          <residual>-0.322451962</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/mb/IU.CTAO</stationMagnitudeID>
          <residual>-0.7279802092</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/mb/IU.HNR</stationMagnitudeID>
          <residual>-0.210076906</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/mb/IU.TARA</stationMagnitudeID>
          <residual>0.8264135394</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052852.335811.85253/staMag/mb/MY.KKM</stationMagnitudeID>
          <residual>0.3151543755</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430052852.335811.85253/netMag/M">
        <magnitude>
          <value>4.880586757</value>
        </magnitude>
        <type>M</type>
        <methodID>weighted average</methodID>
        <stationCount>12</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:28:52.359655Z</creationTime>
        </creationInfo>
      </magnitude>
    </origin>
    <origin publicID="Origin/20210430052906.647836.85374">
      <time>
        <value>2021-04-30T05:18:28.014623Z</value>
        <uncertainty>0.4341970148</uncertainty>
      </time>
      <latitude>
        <value>-11.90989971</value>
        <uncertainty>4.735218137</uncertainty>
      </latitude>
      <longitude>
        <value>166.2073059</value>
        <uncertainty>6.118732905</uncertainty>
      </longitude>
      <depth>
        <value>10</value>
        <uncertainty>0</uncertainty>
      </depth>
      <methodID>LOCSAT</methodID>
      <earthModelID>iasp91</earthModelID>
      <quality>
        <associatedPhaseCount>24</associatedPhaseCount>
        <usedPhaseCount>17</usedPhaseCount>
        <associatedStationCount>24</associatedStationCount>
        <usedStationCount>17</usedStationCount>
        <standardError>2.585901856</standardError>
        <azimuthalGap>137.6721325</azimuthalGap>
        <maximumDistance>52.85604477</maximumDistance>
        <minimumDistance>3.667175055</minimumDistance>
        <medianDistance>39.67186356</medianDistance>
      </quality>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>autoloc</author>
        <creationTime>2021-04-30T05:29:06.647975Z</creationTime>
      </creationInfo>
      <arrival>
        <pickID>20210430.051926.35-AIC-G.SANVU.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>164.8151855</azimuth>
        <distance>3.667175055</distance>
        <timeResidual>1.808414459</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052008.06-AIC-IU.HNR.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>291.2848206</azimuth>
        <distance>6.628712177</distance>
        <timeResidual>2.566513062</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052157.79-AIC-IU.TARA.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>27.14305305</azimuth>
        <distance>14.84813213</distance>
        <timeResidual>0.6653938293</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052259.26-AIC-AU.EIDS..BHZ</pickID>
        <phase>P</phase>
        <azimuth>224.5850525</azimuth>
        <distance>19.62573242</distance>
        <timeResidual>2.910434723</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052313.39-AIC-IU.CTAO.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>244.3510284</azimuth>
        <distance>20.82519531</distance>
        <timeResidual>3.58221817</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052327.21-AIC-AU.ARMA..BHZ</pickID>
        <phase>P</phase>
        <azimuth>213.8864136</azimuth>
        <distance>22.9107933</distance>
        <timeResidual>-4.238445282</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052400.85-AIC-G.CAN.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>210.8109894</azimuth>
        <distance>28.12233162</distance>
        <timeResidual>-18.6938591</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052411.24-AIC-AU.CMSA..BHZ</pickID>
        <phase>P</phase>
        <azimuth>220.7171021</azimuth>
        <distance>27.25172234</distance>
        <timeResidual>-0.7485466003</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052449.69-AIC-II.WRAB.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>251.2080688</azimuth>
        <distance>31.5992794</distance>
        <timeResidual>-1.567913055</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052559.83-AIC-IA.OBMI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>282.5227051</azimuth>
        <distance>39.67186356</distance>
        <timeResidual>-0.5866203308</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052611.16-AIC-GE.SANI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>280.7476501</azimuth>
        <distance>41.05799103</distance>
        <timeResidual>-0.7712516785</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052645.86-AIC-IA.SMSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>283.45578</azimuth>
        <distance>45.40730286</distance>
        <timeResidual>-1.191112518</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052657.94-AIC-II.KAPI.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>274.4432678</azimuth>
        <distance>46.40596008</distance>
        <timeResidual>2.828205109</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052658.56-AIC-AU.MEEK..BHZ</pickID>
        <phase>P</phase>
        <azimuth>244.5357513</azimuth>
        <distance>46.97089386</distance>
        <timeResidual>-0.8763542175</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052659.61-AIC-IA.DBNI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>269.3902588</azimuth>
        <distance>47.20658875</distance>
        <timeResidual>-1.835826874</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052702.21-AIC-IA.MPSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>281.5916748</azimuth>
        <distance>47.57161713</distance>
        <timeResidual>-1.891307831</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052715.96-AIC-IA.SMKI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>279.6096497</azimuth>
        <distance>49.94377899</distance>
        <timeResidual>-6.440624237</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052742.81-AIC-MY.KKM..BHZ</pickID>
        <phase>P</phase>
        <azimuth>287.1437683</azimuth>
        <distance>52.85604477</distance>
        <timeResidual>-1.240001678</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052806.81-AIC-TW.YULB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>308.8881836</azimuth>
        <distance>56.35274887</distance>
        <timeResidual>-1.833287239</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052807.66-AIC-TW.NACB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>309.8468933</azimuth>
        <distance>56.56749725</distance>
        <timeResidual>-2.47309494</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052809.21-AIC-TW.TPUB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>308.3895569</azimuth>
        <distance>56.81214142</distance>
        <timeResidual>-2.72700119</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052809.66-AIC-TW.SSLB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>309.0708008</azimuth>
        <distance>56.83424759</distance>
        <timeResidual>-2.405428648</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052810.81-AIC-TW.YHNB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>310.1987915</azimuth>
        <distance>57.01406479</distance>
        <timeResidual>-2.4829216</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052825.23-AIC-IA.CGJI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>269.2183533</azimuth>
        <distance>59.85900879</distance>
        <timeResidual>-9.226207733</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/mb/AU.ARMA">
        <magnitude>
          <value>4.912918244</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052327.21-AIC-AU.ARMA..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="ARMA" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.668812Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/mb/AU.CMSA">
        <magnitude>
          <value>5.046371478</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052411.24-AIC-AU.CMSA..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="CMSA" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.669238Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/Mjma/AU.EIDS">
        <magnitude>
          <value>5.044700483</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052259.26-AIC-AU.EIDS..BHZ.Mjma</amplitudeID>
        <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.669512Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/mb/AU.EIDS">
        <magnitude>
          <value>4.537135816</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052259.26-AIC-AU.EIDS..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.66964Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/mb/AU.MEEK">
        <magnitude>
          <value>5.301231762</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052658.56-AIC-AU.MEEK..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="MEEK" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.669781Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/MN/G.SANVU">
        <magnitude>
          <value>4.678312761</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052822.994143.113564</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.670037Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/ML/G.SANVU">
        <magnitude>
          <value>5.421061185</value>
        </magnitude>
        <type>ML</type>
        <amplitudeID>Amplitude/20210430052823.012927.113567</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.670216Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/MLv/G.SANVU">
        <magnitude>
          <value>5.455286704</value>
        </magnitude>
        <type>MLv</type>
        <amplitudeID>20210430.051926.35-AIC-G.SANVU.00.BHZ.MLv</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.670403Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/Mjma/G.SANVU">
        <magnitude>
          <value>5.147746079</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.051926.35-AIC-G.SANVU.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.670515Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/mb/GE.SANI">
        <magnitude>
          <value>5.04450942</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052611.16-AIC-GE.SANI..BHZ.mb</amplitudeID>
        <waveformID networkCode="GE" stationCode="SANI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.671148Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/mb/IA.DBNI">
        <magnitude>
          <value>5.111490491</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052659.61-AIC-IA.DBNI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="DBNI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.671304Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/mb/IA.MPSI">
        <magnitude>
          <value>5.014239833</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052702.21-AIC-IA.MPSI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="MPSI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.671434Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/mb/IA.OBMI">
        <magnitude>
          <value>5.276103705</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052559.83-AIC-IA.OBMI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="OBMI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.671809Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/mB/IA.SMKI">
        <magnitude>
          <value>6.928838996</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052715.96-AIC-IA.SMKI..BHZ.mB</amplitudeID>
        <waveformID networkCode="IA" stationCode="SMKI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.672267Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/mb/IA.SMKI">
        <magnitude>
          <value>6.040956493</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052715.96-AIC-IA.SMKI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="SMKI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.672429Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/mb/IA.SMSI">
        <magnitude>
          <value>4.710334105</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052645.86-AIC-IA.SMSI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="SMSI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.67256Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/mb/II.KAPI">
        <magnitude>
          <value>4.627939423</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052657.94-AIC-II.KAPI.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="II" stationCode="KAPI" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.672703Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/mb/II.WRAB">
        <magnitude>
          <value>4.572008976</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052449.69-AIC-II.WRAB.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="II" stationCode="WRAB" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.673065Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/mb/IU.CTAO">
        <magnitude>
          <value>4.166328293</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052313.39-AIC-IU.CTAO.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.673442Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/MN/IU.HNR">
        <magnitude>
          <value>3.183993557</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052823.066898.113574</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.673688Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/ML/IU.HNR">
        <magnitude>
          <value>4.210632168</value>
        </magnitude>
        <type>ML</type>
        <amplitudeID>Amplitude/20210430052823.068657.113575</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BH"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.673866Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/MLv/IU.HNR">
        <magnitude>
          <value>4.475709667</value>
        </magnitude>
        <type>MLv</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.MLv</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.674045Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/Mjma/IU.HNR">
        <magnitude>
          <value>3.838043424</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.674155Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/mB/IU.HNR">
        <magnitude>
          <value>4.604886175</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.674289Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/mb/IU.HNR">
        <magnitude>
          <value>4.68045259</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.674441Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/MN/IU.TARA">
        <magnitude>
          <value>4.865534342</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052823.094023.113580</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.674728Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/Mjma/IU.TARA">
        <magnitude>
          <value>5.651934477</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.674979Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/mB/IU.TARA">
        <magnitude>
          <value>6.034329732</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.675122Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/mb/IU.TARA">
        <magnitude>
          <value>5.721204914</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.675258Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052906.647836.85374/staMag/mb/MY.KKM">
        <magnitude>
          <value>5.209307799</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052742.81-AIC-MY.KKM..BHZ.mb</amplitudeID>
        <waveformID networkCode="MY" stationCode="KKM" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.675411Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <magnitude publicID="Origin/20210430052906.647836.85374/netMag/ML">
        <magnitude>
          <value>4.815846677</value>
          <uncertainty>0.8559025659</uncertainty>
        </magnitude>
        <type>ML</type>
        <methodID>mean</methodID>
        <stationCount>2</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.675438Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/ML/G.SANVU</stationMagnitudeID>
          <residual>0.6052145084</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/ML/IU.HNR</stationMagnitudeID>
          <residual>-0.6052145084</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430052906.647836.85374/netMag/MLv">
        <magnitude>
          <value>4.965498186</value>
          <uncertainty>0.6926655657</uncertainty>
        </magnitude>
        <type>MLv</type>
        <methodID>mean</methodID>
        <stationCount>2</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.675509Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/MLv/G.SANVU</stationMagnitudeID>
          <residual>0.4897885186</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/MLv/IU.HNR</stationMagnitudeID>
          <residual>-0.4897885186</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430052906.647836.85374/netMag/MN">
        <magnitude>
          <value>4.678312761</value>
          <uncertainty>0</uncertainty>
        </magnitude>
        <type>MN</type>
        <methodID>mean</methodID>
        <stationCount>1</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.675567Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/MN/G.SANVU</stationMagnitudeID>
          <residual>0</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/MN/IU.HNR</stationMagnitudeID>
          <residual>-1.494319204</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/MN/IU.TARA</stationMagnitudeID>
          <residual>0.1872215811</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430052906.647836.85374/netMag/Mjma">
        <magnitude>
          <value>4.979145171</value>
          <uncertainty>0.6745750843</uncertainty>
        </magnitude>
        <type>Mjma</type>
        <methodID>trimmed mean(25)</methodID>
        <stationCount>4</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.67562Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/Mjma/AU.EIDS</stationMagnitudeID>
          <residual>0.06555531232</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/Mjma/G.SANVU</stationMagnitudeID>
          <residual>0.1686009082</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/Mjma/IU.HNR</stationMagnitudeID>
          <residual>-1.141101747</residual>
          <weight>0.5</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/Mjma/IU.TARA</stationMagnitudeID>
          <residual>0.672789306</residual>
          <weight>0.5</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430052906.647836.85374/netMag/mB">
        <magnitude>
          <value>5.856018301</value>
          <uncertainty>1.172192563</uncertainty>
        </magnitude>
        <type>mB</type>
        <methodID>mean</methodID>
        <stationCount>3</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.675681Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/mB/IA.SMKI</stationMagnitudeID>
          <residual>1.072820695</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/mB/IU.HNR</stationMagnitudeID>
          <residual>-1.251132126</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/mB/IU.TARA</stationMagnitudeID>
          <residual>0.1783114308</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430052906.647836.85374/netMag/Mw(mB)">
        <magnitude>
          <value>5.432823791</value>
          <uncertainty>1.172192563</uncertainty>
        </magnitude>
        <type>Mw(mB)</type>
        <stationCount>3</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.675737Z</creationTime>
        </creationInfo>
      </magnitude>
      <magnitude publicID="Origin/20210430052906.647836.85374/netMag/mb">
        <magnitude>
          <value>4.958908985</value>
          <uncertainty>0.2562831763</uncertainty>
        </magnitude>
        <type>mb</type>
        <methodID>trimmed mean(25)</methodID>
        <stationCount>12</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.675756Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/mb/AU.ARMA</stationMagnitudeID>
          <residual>-0.04599074118</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/mb/AU.CMSA</stationMagnitudeID>
          <residual>0.08746249204</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/mb/AU.EIDS</stationMagnitudeID>
          <residual>-0.4217731698</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/mb/AU.MEEK</stationMagnitudeID>
          <residual>0.3423227767</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/mb/GE.SANI</stationMagnitudeID>
          <residual>0.08560043407</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/mb/IA.DBNI</stationMagnitudeID>
          <residual>0.1525815052</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/mb/IA.MPSI</stationMagnitudeID>
          <residual>0.05533084728</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/mb/IA.OBMI</stationMagnitudeID>
          <residual>0.3171947194</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/mb/IA.SMKI</stationMagnitudeID>
          <residual>1.082047508</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/mb/IA.SMSI</stationMagnitudeID>
          <residual>-0.2485748808</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/mb/II.KAPI</stationMagnitudeID>
          <residual>-0.3309695623</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/mb/II.WRAB</stationMagnitudeID>
          <residual>-0.386900009</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/mb/IU.CTAO</stationMagnitudeID>
          <residual>-0.7925806927</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/mb/IU.HNR</stationMagnitudeID>
          <residual>-0.278456395</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/mb/IU.TARA</stationMagnitudeID>
          <residual>0.7622959286</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052906.647836.85374/staMag/mb/MY.KKM</stationMagnitudeID>
          <residual>0.2503988136</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430052906.647836.85374/netMag/M">
        <magnitude>
          <value>4.911253988</value>
        </magnitude>
        <type>M</type>
        <methodID>weighted average</methodID>
        <stationCount>12</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:06.675933Z</creationTime>
        </creationInfo>
      </magnitude>
    </origin>
    <origin publicID="Origin/20210430052956.353827.85503">
      <time>
        <value>2021-04-30T05:18:27.718645Z</value>
        <uncertainty>0.4292210388</uncertainty>
      </time>
      <latitude>
        <value>-11.87887096</value>
        <uncertainty>4.668046384</uncertainty>
      </latitude>
      <longitude>
        <value>166.1829681</value>
        <uncertainty>6.058027053</uncertainty>
      </longitude>
      <depth>
        <value>10</value>
        <uncertainty>0</uncertainty>
      </depth>
      <methodID>LOCSAT</methodID>
      <earthModelID>iasp91</earthModelID>
      <quality>
        <associatedPhaseCount>26</associatedPhaseCount>
        <usedPhaseCount>18</usedPhaseCount>
        <associatedStationCount>26</associatedStationCount>
        <usedStationCount>18</usedStationCount>
        <standardError>2.579014925</standardError>
        <azimuthalGap>137.3090954</azimuthalGap>
        <maximumDistance>65.75802612</maximumDistance>
        <minimumDistance>3.703387737</minimumDistance>
        <medianDistance>40.33535194</medianDistance>
      </quality>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>autoloc</author>
        <creationTime>2021-04-30T05:29:56.353982Z</creationTime>
      </creationInfo>
      <arrival>
        <pickID>20210430.051926.35-AIC-G.SANVU.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>164.5906525</azimuth>
        <distance>3.703387737</distance>
        <timeResidual>1.608146667</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052008.06-AIC-IU.HNR.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>291.114563</azimuth>
        <distance>6.5952878</distance>
        <timeResidual>3.320453644</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052157.79-AIC-IU.TARA.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>27.28155708</azimuth>
        <distance>14.83142853</distance>
        <timeResidual>1.185569763</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052259.26-AIC-AU.EIDS..BHZ</pickID>
        <phase>P</phase>
        <azimuth>224.4814453</azimuth>
        <distance>19.63114929</distance>
        <timeResidual>3.150047302</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052313.39-AIC-IU.CTAO.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>244.2553711</azimuth>
        <distance>20.81719017</distance>
        <timeResidual>3.966880798</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052327.21-AIC-AU.ARMA..BHZ</pickID>
        <phase>P</phase>
        <azimuth>213.803772</azimuth>
        <distance>22.9233017</distance>
        <timeResidual>-4.072044373</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052400.85-AIC-G.CAN.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>210.7480316</azimuth>
        <distance>28.13680267</distance>
        <timeResidual>-18.52458954</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052411.24-AIC-AU.CMSA..BHZ</pickID>
        <phase>P</phase>
        <azimuth>220.6477966</azimuth>
        <distance>27.25972748</distance>
        <timeResidual>-0.5220565796</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052449.69-AIC-II.WRAB.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>251.1528473</azimuth>
        <distance>31.58674622</distance>
        <timeResidual>-1.160606384</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052559.83-AIC-IA.OBMI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>282.497406</azimuth>
        <distance>39.64189148</distance>
        <timeResidual>-0.04164886475</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052611.16-AIC-GE.SANI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>280.7227478</azimuth>
        <distance>41.02881241</distance>
        <timeResidual>-0.2354660034</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052645.86-AIC-IA.SMSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>283.4364929</azimuth>
        <distance>45.37692642</distance>
        <timeResidual>-0.6547164917</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052657.94-AIC-II.KAPI.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>274.4205933</azimuth>
        <distance>46.37981796</distance>
        <timeResidual>3.329475403</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052658.56-AIC-AU.MEEK..BHZ</pickID>
        <phase>P</phase>
        <azimuth>244.5050659</azimuth>
        <distance>46.96274567</distance>
        <timeResidual>-0.5152511597</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052659.61-AIC-IA.DBNI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>269.3663025</azimuth>
        <distance>47.1831131</distance>
        <timeResidual>-1.356620789</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052702.21-AIC-IA.MPSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>281.5732727</azimuth>
        <distance>47.54205704</distance>
        <timeResidual>-1.365959167</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052715.96-AIC-IA.SMKI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>279.5922546</azimuth>
        <distance>49.91511917</distance>
        <timeResidual>-5.927055359</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052742.81-AIC-MY.KKM..BHZ</pickID>
        <phase>P</phase>
        <azimuth>287.1316223</azimuth>
        <distance>52.82414246</distance>
        <timeResidual>-0.7087936401</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052806.81-AIC-TW.YULB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>308.887085</azimuth>
        <distance>56.3147316</distance>
        <timeResidual>-1.265922546</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052807.66-AIC-TW.NACB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>309.8462524</azimuth>
        <distance>56.52933121</distance>
        <timeResidual>-1.90599823</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052809.21-AIC-TW.TPUB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>308.3883362</azimuth>
        <distance>56.77420425</distance>
        <timeResidual>-2.162223816</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052809.66-AIC-TW.SSLB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>309.0698853</azimuth>
        <distance>56.7961998</distance>
        <timeResidual>-1.839347839</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052810.81-AIC-TW.YHNB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>310.1983948</azimuth>
        <distance>56.97584534</distance>
        <timeResidual>-1.916557312</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052825.23-AIC-IA.CGJI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>269.2051697</azimuth>
        <distance>59.83562088</distance>
        <timeResidual>-8.76915741</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052909.46-AIC-IU.CASY.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>201.3604279</azimuth>
        <distance>65.75802612</distance>
        <timeResidual>-3.109855652</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052921.26-AIC-IA.MNSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>275.9299622</azimuth>
        <distance>67.31272888</distance>
        <timeResidual>-2.152730942</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/mb/AU.ARMA">
        <magnitude>
          <value>4.913488625</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052327.21-AIC-AU.ARMA..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="ARMA" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.371038Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/mb/AU.CMSA">
        <magnitude>
          <value>5.046931839</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052411.24-AIC-AU.CMSA..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="CMSA" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.371468Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/Mjma/AU.EIDS">
        <magnitude>
          <value>5.044907828</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052259.26-AIC-AU.EIDS..BHZ.Mjma</amplitudeID>
        <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.37177Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/mb/AU.EIDS">
        <magnitude>
          <value>4.537189984</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052259.26-AIC-AU.EIDS..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.371913Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/mb/AU.MEEK">
        <magnitude>
          <value>5.301150282</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052658.56-AIC-AU.MEEK..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="MEEK" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.372289Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/MN/G.SANVU">
        <magnitude>
          <value>4.685405078</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052822.994143.113564</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.37262Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/ML/G.SANVU">
        <magnitude>
          <value>5.430116132</value>
        </magnitude>
        <type>ML</type>
        <amplitudeID>Amplitude/20210430052823.012927.113567</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.372815Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/MLv/G.SANVU">
        <magnitude>
          <value>5.464341651</value>
        </magnitude>
        <type>MLv</type>
        <amplitudeID>20210430.051926.35-AIC-G.SANVU.00.BHZ.MLv</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.372999Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/Mjma/G.SANVU">
        <magnitude>
          <value>5.155128927</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.051926.35-AIC-G.SANVU.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.373124Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/mb/GE.SANI">
        <magnitude>
          <value>5.04450942</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052611.16-AIC-GE.SANI..BHZ.mb</amplitudeID>
        <waveformID networkCode="GE" stationCode="SANI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.373801Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/mb/IA.DBNI">
        <magnitude>
          <value>5.111283905</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052659.61-AIC-IA.DBNI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="DBNI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.374168Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/mb/IA.MPSI">
        <magnitude>
          <value>5.013979704</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052702.21-AIC-IA.MPSI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="MPSI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.374331Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/mb/IA.OBMI">
        <magnitude>
          <value>5.276235585</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052559.83-AIC-IA.OBMI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="OBMI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.374699Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/mB/IA.SMKI">
        <magnitude>
          <value>6.928678505</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052715.96-AIC-IA.SMKI..BHZ.mB</amplitudeID>
        <waveformID networkCode="IA" stationCode="SMKI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.374863Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/mb/IA.SMKI">
        <magnitude>
          <value>6.040796002</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052715.96-AIC-IA.SMKI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="SMKI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.37499Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/mb/IA.SMSI">
        <magnitude>
          <value>4.710030333</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052645.86-AIC-IA.SMSI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="SMSI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.375138Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/mb/II.KAPI">
        <magnitude>
          <value>4.627678008</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052657.94-AIC-II.KAPI.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="II" stationCode="KAPI" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.375536Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/mb/II.WRAB">
        <magnitude>
          <value>4.571883642</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052449.69-AIC-II.WRAB.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="II" stationCode="WRAB" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.375942Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/mb/IU.CTAO">
        <magnitude>
          <value>4.166123362</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052313.39-AIC-IU.CTAO.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.376363Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/MN/IU.HNR">
        <magnitude>
          <value>3.180358372</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052823.066898.113574</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.376656Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/ML/IU.HNR">
        <magnitude>
          <value>4.202274434</value>
        </magnitude>
        <type>ML</type>
        <amplitudeID>Amplitude/20210430052823.068657.113575</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BH"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.376845Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/MLv/IU.HNR">
        <magnitude>
          <value>4.467351933</value>
        </magnitude>
        <type>MLv</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.MLv</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.377031Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/Mjma/IU.HNR">
        <magnitude>
          <value>3.834245364</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.377153Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/mB/IU.HNR">
        <magnitude>
          <value>4.602359291</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.377286Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/mb/IU.HNR">
        <magnitude>
          <value>4.677925706</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.377452Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/MN/IU.TARA">
        <magnitude>
          <value>4.864731554</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052823.094023.113580</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.3778Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/Mjma/IU.TARA">
        <magnitude>
          <value>5.651088783</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.378093Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/mB/IU.TARA">
        <magnitude>
          <value>6.034403226</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.378243Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/mb/IU.TARA">
        <magnitude>
          <value>5.721278408</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.37888Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430052956.353827.85503/staMag/mb/MY.KKM">
        <magnitude>
          <value>5.208988777</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052742.81-AIC-MY.KKM..BHZ.mb</amplitudeID>
        <waveformID networkCode="MY" stationCode="KKM" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.379042Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <magnitude publicID="Origin/20210430052956.353827.85503/netMag/ML">
        <magnitude>
          <value>4.816195283</value>
          <uncertainty>0.868215191</uncertainty>
        </magnitude>
        <type>ML</type>
        <methodID>mean</methodID>
        <stationCount>2</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.379068Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/ML/G.SANVU</stationMagnitudeID>
          <residual>0.6139208491</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/ML/IU.HNR</stationMagnitudeID>
          <residual>-0.6139208491</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430052956.353827.85503/netMag/MLv">
        <magnitude>
          <value>4.965846792</value>
          <uncertainty>0.7049781908</uncertainty>
        </magnitude>
        <type>MLv</type>
        <methodID>mean</methodID>
        <stationCount>2</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.379145Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/MLv/G.SANVU</stationMagnitudeID>
          <residual>0.4984948593</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/MLv/IU.HNR</stationMagnitudeID>
          <residual>-0.4984948593</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430052956.353827.85503/netMag/MN">
        <magnitude>
          <value>4.685405078</value>
          <uncertainty>0</uncertainty>
        </magnitude>
        <type>MN</type>
        <methodID>mean</methodID>
        <stationCount>1</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.379201Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/MN/G.SANVU</stationMagnitudeID>
          <residual>0</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/MN/IU.HNR</stationMagnitudeID>
          <residual>-1.505046706</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/MN/IU.TARA</stationMagnitudeID>
          <residual>0.1793264757</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430052956.353827.85503/netMag/Mjma">
        <magnitude>
          <value>4.980901276</value>
          <uncertainty>0.6769186254</uncertainty>
        </magnitude>
        <type>Mjma</type>
        <methodID>trimmed mean(25)</methodID>
        <stationCount>4</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.379256Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/Mjma/AU.EIDS</stationMagnitudeID>
          <residual>0.06400655165</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/Mjma/G.SANVU</stationMagnitudeID>
          <residual>0.1742276512</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/Mjma/IU.HNR</stationMagnitudeID>
          <residual>-1.146655912</residual>
          <weight>0.5</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/Mjma/IU.TARA</stationMagnitudeID>
          <residual>0.6701875067</residual>
          <weight>0.5</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430052956.353827.85503/netMag/mB">
        <magnitude>
          <value>5.855147007</value>
          <uncertainty>1.173473419</uncertainty>
        </magnitude>
        <type>mB</type>
        <methodID>mean</methodID>
        <stationCount>3</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.379322Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/mB/IA.SMKI</stationMagnitudeID>
          <residual>1.073531498</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/mB/IU.HNR</stationMagnitudeID>
          <residual>-1.252787717</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/mB/IU.TARA</stationMagnitudeID>
          <residual>0.1792562185</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430052956.353827.85503/netMag/Mw(mB)">
        <magnitude>
          <value>5.431691109</value>
          <uncertainty>1.173473419</uncertainty>
        </magnitude>
        <type>Mw(mB)</type>
        <stationCount>3</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.379386Z</creationTime>
        </creationInfo>
      </magnitude>
      <magnitude publicID="Origin/20210430052956.353827.85503/netMag/mb">
        <magnitude>
          <value>4.958673819</value>
          <uncertainty>0.2565768792</uncertainty>
        </magnitude>
        <type>mb</type>
        <methodID>trimmed mean(25)</methodID>
        <stationCount>12</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.379406Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/mb/AU.ARMA</stationMagnitudeID>
          <residual>-0.04518519415</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/mb/AU.CMSA</stationMagnitudeID>
          <residual>0.08825801993</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/mb/AU.EIDS</stationMagnitudeID>
          <residual>-0.4214838344</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/mb/AU.MEEK</stationMagnitudeID>
          <residual>0.3424764634</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/mb/GE.SANI</stationMagnitudeID>
          <residual>0.08583560063</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/mb/IA.DBNI</stationMagnitudeID>
          <residual>0.1526100862</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/mb/IA.MPSI</stationMagnitudeID>
          <residual>0.0553058853</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/mb/IA.OBMI</stationMagnitudeID>
          <residual>0.3175617661</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/mb/IA.SMKI</stationMagnitudeID>
          <residual>1.082122183</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/mb/IA.SMSI</stationMagnitudeID>
          <residual>-0.2486434855</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/mb/II.KAPI</stationMagnitudeID>
          <residual>-0.3309958105</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/mb/II.WRAB</stationMagnitudeID>
          <residual>-0.3867901772</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/mb/IU.CTAO</stationMagnitudeID>
          <residual>-0.7925504565</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/mb/IU.HNR</stationMagnitudeID>
          <residual>-0.2807481124</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/mb/IU.TARA</stationMagnitudeID>
          <residual>0.7626045892</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430052956.353827.85503/staMag/mb/MY.KKM</stationMagnitudeID>
          <residual>0.2503149582</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430052956.353827.85503/netMag/M">
        <magnitude>
          <value>4.912775365</value>
        </magnitude>
        <type>M</type>
        <methodID>weighted average</methodID>
        <stationCount>12</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:29:56.379582Z</creationTime>
        </creationInfo>
      </magnitude>
    </origin>
    <origin publicID="Origin/20210430053040.704638.85662">
      <time>
        <value>2021-04-30T05:18:27.029985Z</value>
        <uncertainty>0.3530697619</uncertainty>
      </time>
      <latitude>
        <value>-11.91822624</value>
        <uncertainty>4.308476216</uncertainty>
      </latitude>
      <longitude>
        <value>165.9821625</value>
        <uncertainty>6.178548175</uncertainty>
      </longitude>
      <depth>
        <value>10</value>
        <uncertainty>0</uncertainty>
      </depth>
      <methodID>LOCSAT</methodID>
      <earthModelID>iasp91</earthModelID>
      <quality>
        <associatedPhaseCount>29</associatedPhaseCount>
        <usedPhaseCount>24</usedPhaseCount>
        <associatedStationCount>29</associatedStationCount>
        <usedStationCount>24</usedStationCount>
        <standardError>1.991229918</standardError>
        <azimuthalGap>133.6494522</azimuthalGap>
        <maximumDistance>72.85942841</maximumDistance>
        <minimumDistance>3.722947836</minimumDistance>
        <medianDistance>51.18802834</medianDistance>
      </quality>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>autoloc</author>
        <creationTime>2021-04-30T05:30:40.704804Z</creationTime>
      </creationInfo>
      <arrival>
        <pickID>20210430.051926.35-AIC-G.SANVU.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>161.5585938</azimuth>
        <distance>3.722947836</distance>
        <timeResidual>2.019290924</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052008.06-AIC-IU.HNR.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>292.1106873</azimuth>
        <distance>6.427100658</distance>
        <timeResidual>6.326396942</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052157.79-AIC-IU.TARA.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>27.90914154</azimuth>
        <distance>14.95732784</distance>
        <timeResidual>0.1535720825</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052259.26-AIC-AU.EIDS..BHZ</pickID>
        <phase>P</phase>
        <azimuth>224.2045593</azimuth>
        <distance>19.46566391</distance>
        <timeResidual>5.652580261</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052313.39-AIC-IU.CTAO.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>244.1644592</azimuth>
        <distance>20.62314796</distance>
        <timeResidual>6.760185242</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052327.21-AIC-AU.ARMA..BHZ</pickID>
        <phase>P</phase>
        <azimuth>213.5088348</azimuth>
        <distance>22.78164482</distance>
        <timeResidual>-1.87877655</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052400.85-AIC-G.CAN.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>210.5099792</azimuth>
        <distance>28.0028286</distance>
        <timeResidual>-16.63677216</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052411.24-AIC-AU.CMSA..BHZ</pickID>
        <phase>P</phase>
        <azimuth>220.4482727</azimuth>
        <distance>27.10208702</distance>
        <timeResidual>1.586006165</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052449.69-AIC-II.WRAB.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>251.151413</azimuth>
        <distance>31.38807678</distance>
        <timeResidual>1.275672913</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052559.83-AIC-IA.OBMI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>282.6371155</azimuth>
        <distance>39.45867157</distance>
        <timeResidual>2.177375793</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052611.16-AIC-GE.SANI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>280.8511353</azimuth>
        <distance>40.84315872</distance>
        <timeResidual>1.98651886</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052645.86-AIC-IA.SMSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>283.5611877</azimuth>
        <distance>45.19504929</distance>
        <timeResidual>1.481819153</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052657.94-AIC-II.KAPI.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>274.5141296</azimuth>
        <distance>46.18698883</distance>
        <timeResidual>5.538002014</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052658.56-AIC-AU.MEEK..BHZ</pickID>
        <phase>P</phase>
        <azimuth>244.5004883</azimuth>
        <distance>46.76844788</distance>
        <timeResidual>1.695899963</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052659.61-AIC-IA.DBNI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>269.4424133</azimuth>
        <distance>46.98622131</distance>
        <timeResidual>0.8720474243</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052702.21-AIC-IA.MPSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>281.6864319</azimuth>
        <distance>47.3575325</distance>
        <timeResidual>0.7628250122</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052715.96-AIC-IA.SMKI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>279.6942139</azimuth>
        <distance>49.72799683</distance>
        <timeResidual>-3.81023407</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052742.81-AIC-MY.KKM..BHZ</pickID>
        <phase>P</phase>
        <azimuth>287.2458191</azimuth>
        <distance>52.64805984</distance>
        <timeResidual>1.287849426</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052806.81-AIC-TW.YULB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>309.0314636</azimuth>
        <distance>56.18668365</distance>
        <timeResidual>0.3454055786</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052807.66-AIC-TW.NACB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>309.9912109</azimuth>
        <distance>56.40388489</distance>
        <timeResidual>-0.3152389526</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052809.21-AIC-TW.TPUB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>308.5302124</azimuth>
        <distance>56.64481735</distance>
        <timeResidual>-0.5457687378</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052809.66-AIC-TW.SSLB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>309.2127075</azimuth>
        <distance>56.66864777</distance>
        <timeResidual>-0.236076355</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052810.81-AIC-TW.YHNB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>310.3421326</azimuth>
        <distance>56.85136032</distance>
        <timeResidual>-0.3367233276</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052825.23-AIC-IA.CGJI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>269.2680054</azimuth>
        <distance>59.6386261</distance>
        <timeResidual>-6.72026825</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052909.46-AIC-IU.CASY.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>201.3256836</azimuth>
        <distance>65.64985657</distance>
        <timeResidual>-1.721000671</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052911.41-AIC-IU.SBA.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>179.8213348</azimuth>
        <distance>65.93215942</distance>
        <timeResidual>-1.398490906</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052921.26-AIC-IA.MNSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>275.9963989</azimuth>
        <distance>67.12138367</distance>
        <timeResidual>-0.2424240112</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052936.16-AIC-RM.SLV..BHZ</pickID>
        <phase>P</phase>
        <azimuth>298.4552917</azimuth>
        <distance>69.41108704</distance>
        <timeResidual>0.9630813599</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052956.89-AIC-IU.CHTO.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>294.2060547</azimuth>
        <distance>72.85942841</distance>
        <timeResidual>0.5972976685</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/mb/AU.ARMA">
        <magnitude>
          <value>4.907029096</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052327.21-AIC-AU.ARMA..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="ARMA" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.723598Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/mb/AU.CMSA">
        <magnitude>
          <value>5.03589698</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052411.24-AIC-AU.CMSA..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="CMSA" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.723994Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/mb/AU.MEEK">
        <magnitude>
          <value>5.299207353</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052658.56-AIC-AU.MEEK..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="MEEK" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.72441Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/MN/G.SANVU">
        <magnitude>
          <value>4.689341524</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052822.994143.113564</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.724696Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/ML/G.SANVU">
        <magnitude>
          <value>5.435007116</value>
        </magnitude>
        <type>ML</type>
        <amplitudeID>Amplitude/20210430052823.012927.113567</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.724888Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/MLv/G.SANVU">
        <magnitude>
          <value>5.469232636</value>
        </magnitude>
        <type>MLv</type>
        <amplitudeID>20210430.051926.35-AIC-G.SANVU.00.BHZ.MLv</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.725085Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/Mjma/G.SANVU">
        <magnitude>
          <value>5.159086764</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.051926.35-AIC-G.SANVU.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.725205Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/mb/GE.SANI">
        <magnitude>
          <value>5.043631088</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052611.16-AIC-GE.SANI..BHZ.mb</amplitudeID>
        <waveformID networkCode="GE" stationCode="SANI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.725895Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/mb/IA.CGJI">
        <magnitude>
          <value>5.310867268</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052825.23-AIC-IA.CGJI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="CGJI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.726062Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/mb/IA.DBNI">
        <magnitude>
          <value>5.109534728</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052659.61-AIC-IA.DBNI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="DBNI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.726456Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/mb/IA.MPSI">
        <magnitude>
          <value>5.01235589</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052702.21-AIC-IA.MPSI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="MPSI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.72704Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/mb/IA.OBMI">
        <magnitude>
          <value>5.277041771</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052559.83-AIC-IA.OBMI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="OBMI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.727438Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/mB/IA.SMKI">
        <magnitude>
          <value>6.927630646</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052715.96-AIC-IA.SMKI..BHZ.mB</amplitudeID>
        <waveformID networkCode="IA" stationCode="SMKI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.727854Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/mb/IA.SMKI">
        <magnitude>
          <value>6.039748143</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052715.96-AIC-IA.SMKI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="SMKI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.727994Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/mb/IA.SMSI">
        <magnitude>
          <value>4.70821152</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052645.86-AIC-IA.SMSI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="SMSI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.72839Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/mb/II.WRAB">
        <magnitude>
          <value>4.569896902</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052449.69-AIC-II.WRAB.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="II" stationCode="WRAB" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.728783Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/mb/IU.SBA">
        <magnitude>
          <value>4.426199774</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052911.41-AIC-IU.SBA.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="SBA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.728945Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/MN/IU.TARA">
        <magnitude>
          <value>4.870867904</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052823.094023.113580</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.729284Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/Mjma/IU.TARA">
        <magnitude>
          <value>5.657439664</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.729583Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/mB/IU.TARA">
        <magnitude>
          <value>6.033849282</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.729716Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/mb/IU.TARA">
        <magnitude>
          <value>5.720724465</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.729855Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/mb/MY.KKM">
        <magnitude>
          <value>5.207227958</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052742.81-AIC-MY.KKM..BHZ.mb</amplitudeID>
        <waveformID networkCode="MY" stationCode="KKM" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.729999Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/mb/RM.SLV">
        <magnitude>
          <value>4.845626496</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052936.16-AIC-RM.SLV..BHZ.mb</amplitudeID>
        <waveformID networkCode="RM" stationCode="SLV" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.73063Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/mb/TW.NACB">
        <magnitude>
          <value>5.212809242</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052807.66-AIC-TW.NACB..BHZ.mb</amplitudeID>
        <waveformID networkCode="TW" stationCode="NACB" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.730794Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/mb/TW.SSLB">
        <magnitude>
          <value>5.010970289</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052809.66-AIC-TW.SSLB..BHZ.mb</amplitudeID>
        <waveformID networkCode="TW" stationCode="SSLB" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.73094Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/mb/TW.TPUB">
        <magnitude>
          <value>5.515120943</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052809.21-AIC-TW.TPUB..BHZ.mb</amplitudeID>
        <waveformID networkCode="TW" stationCode="TPUB" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.731073Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/mb/TW.YHNB">
        <magnitude>
          <value>5.301132721</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052810.81-AIC-TW.YHNB..BHZ.mb</amplitudeID>
        <waveformID networkCode="TW" stationCode="YHNB" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.731225Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/mb/IU.CASY">
        <magnitude>
          <value>5.039987806</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052909.46-AIC-IU.CASY.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="CASY" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:45.492563Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053040.704638.85662/staMag/mb/IU.CHTO">
        <magnitude>
          <value>4.82443302</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052956.89-AIC-IU.CHTO.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="CHTO" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:05.692897Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <magnitude publicID="Origin/20210430053040.704638.85662/netMag/ML">
        <magnitude>
          <value>5.435007116</value>
          <uncertainty>0</uncertainty>
        </magnitude>
        <type>ML</type>
        <methodID>mean</methodID>
        <stationCount>1</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.731372Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/ML/G.SANVU</stationMagnitudeID>
          <residual>0</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430053040.704638.85662/netMag/MLv">
        <magnitude>
          <value>5.469232636</value>
          <uncertainty>0</uncertainty>
        </magnitude>
        <type>MLv</type>
        <methodID>mean</methodID>
        <stationCount>1</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.731441Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/MLv/G.SANVU</stationMagnitudeID>
          <residual>0</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430053040.704638.85662/netMag/MN">
        <magnitude>
          <value>4.689341524</value>
          <uncertainty>0</uncertainty>
        </magnitude>
        <type>MN</type>
        <methodID>mean</methodID>
        <stationCount>1</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.731482Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/MN/G.SANVU</stationMagnitudeID>
          <residual>0</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/MN/IU.TARA</stationMagnitudeID>
          <residual>0.1815263796</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430053040.704638.85662/netMag/Mjma">
        <magnitude>
          <value>5.408263214</value>
          <uncertainty>0.3523887149</uncertainty>
        </magnitude>
        <type>Mjma</type>
        <methodID>mean</methodID>
        <stationCount>2</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.731523Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/Mjma/G.SANVU</stationMagnitudeID>
          <residual>-0.2491764499</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/Mjma/IU.TARA</stationMagnitudeID>
          <residual>0.2491764499</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430053040.704638.85662/netMag/mB">
        <magnitude>
          <value>6.480739964</value>
          <uncertainty>0.6319988632</uncertainty>
        </magnitude>
        <type>mB</type>
        <methodID>mean</methodID>
        <stationCount>2</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.731574Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/mB/IA.SMKI</stationMagnitudeID>
          <residual>0.4468906818</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/mB/IU.TARA</stationMagnitudeID>
          <residual>-0.4468906818</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430053040.704638.85662/netMag/Mw(mB)">
        <magnitude>
          <value>6.244961953</value>
          <uncertainty>0.6319988632</uncertainty>
        </magnitude>
        <type>Mw(mB)</type>
        <stationCount>2</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.731618Z</creationTime>
        </creationInfo>
      </magnitude>
      <magnitude publicID="Origin/20210430053040.704638.85662/netMag/mb">
        <magnitude>
          <value>5.096603262</value>
          <uncertainty>0.185333459</uncertainty>
        </magnitude>
        <type>mb</type>
        <methodID>trimmed mean(25)</methodID>
        <stationCount>17</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.731643Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/mb/AU.ARMA</stationMagnitudeID>
          <residual>-0.1895741658</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/mb/AU.CMSA</stationMagnitudeID>
          <residual>-0.06070628235</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/mb/AU.MEEK</stationMagnitudeID>
          <residual>0.2026040904</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/mb/GE.SANI</stationMagnitudeID>
          <residual>-0.05297217386</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/mb/IA.CGJI</stationMagnitudeID>
          <residual>0.2142640058</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/mb/IA.DBNI</stationMagnitudeID>
          <residual>0.01293146581</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/mb/IA.MPSI</stationMagnitudeID>
          <residual>-0.08424737229</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/mb/IA.OBMI</stationMagnitudeID>
          <residual>0.1804385089</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/mb/IA.SMKI</stationMagnitudeID>
          <residual>0.9431448809</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/mb/IA.SMSI</stationMagnitudeID>
          <residual>-0.3883917417</residual>
          <weight>0.375</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/mb/II.WRAB</stationMagnitudeID>
          <residual>-0.5267063602</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/mb/IU.CASY</stationMagnitudeID>
          <residual>-0.05661545615</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/mb/IU.CHTO</stationMagnitudeID>
          <residual>-0.2721702426</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/mb/IU.SBA</stationMagnitudeID>
          <residual>-0.6704034883</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/mb/IU.TARA</stationMagnitudeID>
          <residual>0.6241212027</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/mb/MY.KKM</stationMagnitudeID>
          <residual>0.1106246956</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/mb/RM.SLV</stationMagnitudeID>
          <residual>-0.250976766</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/mb/TW.NACB</stationMagnitudeID>
          <residual>0.1162059804</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/mb/TW.SSLB</stationMagnitudeID>
          <residual>-0.0856329736</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/mb/TW.TPUB</stationMagnitudeID>
          <residual>0.4185176813</residual>
          <weight>0.375</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053040.704638.85662/staMag/mb/TW.YHNB</stationMagnitudeID>
          <residual>0.2045294584</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430053040.704638.85662/netMag/M">
        <magnitude>
          <value>5.261280065</value>
        </magnitude>
        <type>M</type>
        <methodID>weighted average</methodID>
        <stationCount>17</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:30:40.731852Z</creationTime>
          <modificationTime>2021-04-30T05:31:05.693186Z</modificationTime>
        </creationInfo>
      </magnitude>
    </origin>
    <origin publicID="Origin/20210430053139.929603.85951">
      <time>
        <value>2021-04-30T05:18:32.704008Z</value>
        <uncertainty>0.6388640275</uncertainty>
      </time>
      <latitude>
        <value>-11.89531898</value>
        <uncertainty>3.913870009</uncertainty>
      </latitude>
      <longitude>
        <value>166.3618927</value>
        <uncertainty>5.595278739</uncertainty>
      </longitude>
      <depth>
        <value>69.88449097</value>
        <uncertainty>6.381146329</uncertainty>
      </depth>
      <depthType>from location</depthType>
      <methodID>LOCSAT</methodID>
      <earthModelID>iasp91</earthModelID>
      <quality>
        <associatedPhaseCount>31</associatedPhaseCount>
        <usedPhaseCount>29</usedPhaseCount>
        <associatedStationCount>31</associatedStationCount>
        <usedStationCount>29</usedStationCount>
        <standardError>1.705153594</standardError>
        <azimuthalGap>140.5098438</azimuthalGap>
        <maximumDistance>79.50111389</maximumDistance>
        <minimumDistance>3.644659519</minimumDistance>
        <medianDistance>47.71689224</medianDistance>
      </quality>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>autoloc</author>
        <creationTime>2021-04-30T05:31:39.929868Z</creationTime>
      </creationInfo>
      <arrival>
        <pickID>20210430.051926.35-AIC-G.SANVU.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>167.1355896</azimuth>
        <distance>3.644659519</distance>
        <timeResidual>-0.07741165161</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052008.06-AIC-IU.HNR.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>290.6757202</azimuth>
        <distance>6.764728069</distance>
        <timeResidual>-1.270423889</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052157.79-AIC-IU.TARA.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>26.62574577</azimuth>
        <distance>14.76672649</distance>
        <timeResidual>0.9380455017</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052259.26-AIC-AU.EIDS..BHZ</pickID>
        <phase>P</phase>
        <azimuth>224.8250122</azimuth>
        <distance>19.74250221</distance>
        <timeResidual>2.786022186</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052313.39-AIC-IU.CTAO.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>244.4558716</azimuth>
        <distance>20.96791267</distance>
        <timeResidual>3.254863739</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052327.21-AIC-AU.ARMA..BHZ</pickID>
        <phase>P</phase>
        <azimuth>214.1312714</azimuth>
        <distance>23.00748634</distance>
        <timeResidual>-3.906925201</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052400.85-AIC-G.CAN.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>211.0074921</azimuth>
        <distance>28.21254158</distance>
        <timeResidual>-17.38473892</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052411.24-AIC-AU.CMSA..BHZ</pickID>
        <phase>P</phase>
        <azimuth>220.8885345</azimuth>
        <distance>27.36160278</distance>
        <timeResidual>0.3583335876</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052449.69-AIC-II.WRAB.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>251.2327423</azimuth>
        <distance>31.74718285</distance>
        <timeResidual>-0.6990699768</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052559.83-AIC-IA.OBMI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>282.4344177</azimuth>
        <distance>39.81640244</distance>
        <timeResidual>0.5498008728</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052611.16-AIC-GE.SANI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>280.6672058</azimuth>
        <distance>41.20391083</distance>
        <timeResidual>0.4002952576</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052645.86-AIC-IA.SMSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>283.3754578</azimuth>
        <distance>45.55105591</distance>
        <timeResidual>0.1438865662</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052657.94-AIC-II.KAPI.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>274.3865356</azimuth>
        <distance>46.55565262</distance>
        <timeResidual>4.153865814</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052658.56-AIC-AU.MEEK..BHZ</pickID>
        <phase>P</phase>
        <azimuth>244.5521545</azimuth>
        <distance>47.11373901</distance>
        <timeResidual>0.5256919861</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052659.61-AIC-IA.DBNI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>269.3464661</azimuth>
        <distance>47.35800552</distance>
        <timeResidual>-0.4945411682</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052702.21-AIC-IA.MPSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>281.5191345</azimuth>
        <distance>47.71689224</distance>
        <timeResidual>-0.4927101135</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052715.96-AIC-IA.SMKI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>279.5445862</azimuth>
        <distance>50.09050751</distance>
        <timeResidual>-4.970218658</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052742.81-AIC-MY.KKM..BHZ</pickID>
        <phase>P</phase>
        <azimuth>287.067749</azimuth>
        <distance>52.99632645</distance>
        <timeResidual>0.3718223572</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052806.81-AIC-TW.YULB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>308.7857666</azimuth>
        <distance>56.4614296</distance>
        <timeResidual>0.1097373962</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052807.66-AIC-TW.NACB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>309.7438354</azimuth>
        <distance>56.67438507</distance>
        <timeResidual>-0.5123329163</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052809.21-AIC-TW.TPUB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>308.2889709</azimuth>
        <distance>56.92173767</distance>
        <timeResidual>-0.7783851624</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052809.66-AIC-TW.SSLB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>308.9694214</azimuth>
        <distance>56.94258881</distance>
        <timeResidual>-0.4469032288</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052810.81-AIC-TW.YHNB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>310.0965271</azimuth>
        <distance>57.12028503</distance>
        <timeResidual>-0.5051307678</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052825.23-AIC-IA.CGJI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>269.1792603</azimuth>
        <distance>60.0104599</distance>
        <timeResidual>-7.473026276</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052908.75-AIC-GT.VNDA.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>181.068634</azimuth>
        <distance>65.66309357</distance>
        <timeResidual>-0.3920326233</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052909.46-AIC-IU.CASY.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>201.3995972</azimuth>
        <distance>65.80653381</distance>
        <timeResidual>-0.8057899475</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052911.41-AIC-IU.SBA.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>179.9088898</azimuth>
        <distance>65.95418549</distance>
        <timeResidual>0.3958702087</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052921.26-AIC-IA.MNSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>275.8924255</azimuth>
        <distance>67.4885788</distance>
        <timeResidual>-0.6223182678</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052936.16-AIC-RM.SLV..BHZ</pickID>
        <phase>P</phase>
        <azimuth>298.3036499</azimuth>
        <distance>69.72710419</distance>
        <timeResidual>0.9947471619</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052956.89-AIC-IU.CHTO.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>294.0749512</azimuth>
        <distance>73.18912506</distance>
        <timeResidual>0.690486908</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.053031.16-AIC-IU.ULN.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>324.0746155</azimuth>
        <distance>79.50111389</distance>
        <timeResidual>0.300655365</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mb/AU.ARMA">
        <magnitude>
          <value>4.920211725</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052327.21-AIC-AU.ARMA..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="ARMA" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.954809Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mb/AU.CMSA">
        <magnitude>
          <value>5.02901118</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052411.24-AIC-AU.CMSA..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="CMSA" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.955277Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/Mjma/AU.EIDS">
        <magnitude>
          <value>5.04915752</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052259.26-AIC-AU.EIDS..BHZ.Mjma</amplitudeID>
        <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.955618Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mb/AU.EIDS">
        <magnitude>
          <value>4.52885747</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052259.26-AIC-AU.EIDS..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.95577Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mb/AU.MEEK">
        <magnitude>
          <value>5.247786168</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052658.56-AIC-AU.MEEK..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="MEEK" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.956216Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/MN/G.SANVU">
        <magnitude>
          <value>4.673783476</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052822.994143.113564</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.956579Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/ML/G.SANVU">
        <magnitude>
          <value>5.415431197</value>
        </magnitude>
        <type>ML</type>
        <amplitudeID>Amplitude/20210430052823.012927.113567</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.956796Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/MLv/G.SANVU">
        <magnitude>
          <value>5.449656716</value>
        </magnitude>
        <type>MLv</type>
        <amplitudeID>20210430.051926.35-AIC-G.SANVU.00.BHZ.MLv</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.957007Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/Mjma/G.SANVU">
        <magnitude>
          <value>5.143118885</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.051926.35-AIC-G.SANVU.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.957142Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mb/GE.SANI">
        <magnitude>
          <value>5.010387843</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052611.16-AIC-GE.SANI..BHZ.mb</amplitudeID>
        <waveformID networkCode="GE" stationCode="SANI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.9579Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mb/GT.VNDA">
        <magnitude>
          <value>4.64354098</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052908.75-AIC-GT.VNDA.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="GT" stationCode="VNDA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.958077Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mb/IA.DBNI">
        <magnitude>
          <value>5.058780687</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052659.61-AIC-IA.DBNI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="DBNI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.958537Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mb/IA.MPSI">
        <magnitude>
          <value>4.962497517</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052702.21-AIC-IA.MPSI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="MPSI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.959041Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mb/IA.OBMI">
        <magnitude>
          <value>5.240798714</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052559.83-AIC-IA.OBMI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="OBMI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.959435Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mB/IA.SMKI">
        <magnitude>
          <value>6.914107408</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052715.96-AIC-IA.SMKI..BHZ.mB</amplitudeID>
        <waveformID networkCode="IA" stationCode="SMKI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.959889Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mb/IA.SMKI">
        <magnitude>
          <value>6.026224905</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052715.96-AIC-IA.SMKI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="SMKI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.960028Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mb/IA.SMSI">
        <magnitude>
          <value>4.664325025</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052645.86-AIC-IA.SMSI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="SMSI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.960425Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mb/II.KAPI">
        <magnitude>
          <value>4.578086875</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052657.94-AIC-II.KAPI.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="II" stationCode="KAPI" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.960844Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mb/II.WRAB">
        <magnitude>
          <value>4.556682163</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052449.69-AIC-II.WRAB.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="II" stationCode="WRAB" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.961261Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mb/IU.CASY">
        <magnitude>
          <value>4.956276</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>Amplitude/20210430053131.536021.113922</amplitudeID>
        <waveformID networkCode="IU" stationCode="CASY" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.961436Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mb/IU.CHTO">
        <magnitude>
          <value>4.728551711</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052956.89-AIC-IU.CHTO.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="CHTO" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.961585Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mb/IU.CTAO">
        <magnitude>
          <value>4.17816422</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052313.39-AIC-IU.CTAO.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.961998Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/MN/IU.HNR">
        <magnitude>
          <value>3.198669548</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052823.066898.113574</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.962327Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/ML/IU.HNR">
        <magnitude>
          <value>4.244642814</value>
        </magnitude>
        <type>ML</type>
        <amplitudeID>Amplitude/20210430052823.068657.113575</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BH"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.962523Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/MLv/IU.HNR">
        <magnitude>
          <value>4.509720313</value>
        </magnitude>
        <type>MLv</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.MLv</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.962728Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/Mjma/IU.HNR">
        <magnitude>
          <value>3.853304067</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.962853Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mB/IU.HNR">
        <magnitude>
          <value>4.520763029</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.96299Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mb/IU.HNR">
        <magnitude>
          <value>4.596329445</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.963132Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mb/IU.SBA">
        <magnitude>
          <value>4.341731936</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052911.41-AIC-IU.SBA.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="SBA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.969416Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/MN/IU.TARA">
        <magnitude>
          <value>4.861536816</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052823.094023.113580</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.969861Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/Mjma/IU.TARA">
        <magnitude>
          <value>5.647803942</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.970176Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mB/IU.TARA">
        <magnitude>
          <value>5.974804146</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.970346Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mb/IU.TARA">
        <magnitude>
          <value>5.661679329</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.970497Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mb/MY.KKM">
        <magnitude>
          <value>5.186830005</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052742.81-AIC-MY.KKM..BHZ.mb</amplitudeID>
        <waveformID networkCode="MY" stationCode="KKM" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.970895Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mb/RM.SLV">
        <magnitude>
          <value>4.748709101</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052936.16-AIC-RM.SLV..BHZ.mb</amplitudeID>
        <waveformID networkCode="RM" stationCode="SLV" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.971048Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mb/TW.NACB">
        <magnitude>
          <value>5.157781412</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052807.66-AIC-TW.NACB..BHZ.mb</amplitudeID>
        <waveformID networkCode="TW" stationCode="NACB" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.971456Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mb/TW.SSLB">
        <magnitude>
          <value>4.958183028</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052809.66-AIC-TW.SSLB..BHZ.mb</amplitudeID>
        <waveformID networkCode="TW" stationCode="SSLB" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.97186Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mb/TW.TPUB">
        <magnitude>
          <value>5.462159493</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052809.21-AIC-TW.TPUB..BHZ.mb</amplitudeID>
        <waveformID networkCode="TW" stationCode="TPUB" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.972231Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mb/TW.YHNB">
        <magnitude>
          <value>5.248018231</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052810.81-AIC-TW.YHNB..BHZ.mb</amplitudeID>
        <waveformID networkCode="TW" stationCode="YHNB" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.972653Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/MN/AU.EIDS">
        <magnitude>
          <value>3.759087071</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430053140.209493.114038</amplitudeID>
        <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:40.21642Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053139.929603.85951/staMag/mB/IU.CHTO">
        <magnitude>
          <value>4.980856537</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052956.89-AIC-IU.CHTO.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="CHTO" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:54.87284Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <magnitude publicID="Origin/20210430053139.929603.85951/netMag/ML">
        <magnitude>
          <value>4.830037005</value>
          <uncertainty>0.8278724047</uncertainty>
        </magnitude>
        <type>ML</type>
        <methodID>mean</methodID>
        <stationCount>2</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.973051Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/ML/G.SANVU</stationMagnitudeID>
          <residual>0.5853941913</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/ML/IU.HNR</stationMagnitudeID>
          <residual>-0.5853941913</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430053139.929603.85951/netMag/MLv">
        <magnitude>
          <value>4.979688514</value>
          <uncertainty>0.6646354045</uncertainty>
        </magnitude>
        <type>MLv</type>
        <methodID>mean</methodID>
        <stationCount>2</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.973155Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/MLv/G.SANVU</stationMagnitudeID>
          <residual>0.4699682015</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/MLv/IU.HNR</stationMagnitudeID>
          <residual>-0.4699682015</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430053139.929603.85951/netMag/MN">
        <magnitude>
          <value>4.673783476</value>
          <uncertainty>0</uncertainty>
        </magnitude>
        <type>MN</type>
        <methodID>mean</methodID>
        <stationCount>1</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.973212Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/MN/AU.EIDS</stationMagnitudeID>
          <residual>-0.9146964054</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/MN/G.SANVU</stationMagnitudeID>
          <residual>0</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/MN/IU.HNR</stationMagnitudeID>
          <residual>-1.475113929</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/MN/IU.TARA</stationMagnitudeID>
          <residual>0.1877533396</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430053139.929603.85951/netMag/Mjma">
        <magnitude>
          <value>4.98094347</value>
          <uncertainty>0.6667423705</uncertainty>
        </magnitude>
        <type>Mjma</type>
        <methodID>trimmed mean(25)</methodID>
        <stationCount>4</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.973271Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/Mjma/AU.EIDS</stationMagnitudeID>
          <residual>0.06821405028</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/Mjma/G.SANVU</stationMagnitudeID>
          <residual>0.1621754151</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/Mjma/IU.HNR</stationMagnitudeID>
          <residual>-1.127639402</residual>
          <weight>0.5</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/Mjma/IU.TARA</stationMagnitudeID>
          <residual>0.6668604717</residual>
          <weight>0.5</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430053139.929603.85951/netMag/mB">
        <magnitude>
          <value>5.557698634</value>
          <uncertainty>0.9910257108</uncertainty>
        </magnitude>
        <type>mB</type>
        <methodID>trimmed mean(25)</methodID>
        <stationCount>4</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.973343Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mB/IA.SMKI</stationMagnitudeID>
          <residual>1.356408774</residual>
          <weight>0.5</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mB/IU.CHTO</stationMagnitudeID>
          <residual>-0.576842097</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mB/IU.HNR</stationMagnitudeID>
          <residual>-1.036935605</residual>
          <weight>0.5</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mB/IU.TARA</stationMagnitudeID>
          <residual>0.4171055126</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430053139.929603.85951/netMag/Mw(mB)">
        <magnitude>
          <value>5.045008224</value>
          <uncertainty>0.9910257108</uncertainty>
        </magnitude>
        <type>Mw(mB)</type>
        <stationCount>4</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.973406Z</creationTime>
          <modificationTime>2021-04-30T05:31:54.872914Z</modificationTime>
        </creationInfo>
      </magnitude>
      <magnitude publicID="Origin/20210430053139.929603.85951/netMag/mb">
        <magnitude>
          <value>4.920917347</value>
          <uncertainty>0.2398447108</uncertainty>
        </magnitude>
        <type>mb</type>
        <methodID>trimmed mean(25)</methodID>
        <stationCount>19</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.973428Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mb/AU.ARMA</stationMagnitudeID>
          <residual>-0.0007056218427</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mb/AU.CMSA</stationMagnitudeID>
          <residual>0.1080938324</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mb/AU.EIDS</stationMagnitudeID>
          <residual>-0.3920598773</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mb/AU.MEEK</stationMagnitudeID>
          <residual>0.3268688209</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mb/GE.SANI</stationMagnitudeID>
          <residual>0.08947049608</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mb/GT.VNDA</stationMagnitudeID>
          <residual>-0.2773763676</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mb/IA.DBNI</stationMagnitudeID>
          <residual>0.1378633394</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mb/IA.MPSI</stationMagnitudeID>
          <residual>0.04158017012</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mb/IA.OBMI</stationMagnitudeID>
          <residual>0.3198813666</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mb/IA.SMKI</stationMagnitudeID>
          <residual>1.105307557</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mb/IA.SMSI</stationMagnitudeID>
          <residual>-0.2565923219</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mb/II.KAPI</stationMagnitudeID>
          <residual>-0.3428304725</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mb/II.WRAB</stationMagnitudeID>
          <residual>-0.3642351844</residual>
          <weight>0.875</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mb/IU.CASY</stationMagnitudeID>
          <residual>0.03535865314</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mb/IU.CHTO</stationMagnitudeID>
          <residual>-0.1923656362</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mb/IU.CTAO</stationMagnitudeID>
          <residual>-0.7427531275</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mb/IU.HNR</stationMagnitudeID>
          <residual>-0.3245879026</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mb/IU.SBA</stationMagnitudeID>
          <residual>-0.5791854111</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mb/IU.TARA</stationMagnitudeID>
          <residual>0.7407619816</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mb/MY.KKM</stationMagnitudeID>
          <residual>0.2659126575</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mb/RM.SLV</stationMagnitudeID>
          <residual>-0.1722082464</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mb/TW.NACB</stationMagnitudeID>
          <residual>0.2368640645</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mb/TW.SSLB</stationMagnitudeID>
          <residual>0.03726568119</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mb/TW.TPUB</stationMagnitudeID>
          <residual>0.5412421459</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053139.929603.85951/staMag/mb/TW.YHNB</stationMagnitudeID>
          <residual>0.3271008841</residual>
          <weight>0.875</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430053139.929603.85951/netMag/M">
        <magnitude>
          <value>4.907888373</value>
        </magnitude>
        <type>M</type>
        <methodID>weighted average</methodID>
        <stationCount>19</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:39.973674Z</creationTime>
          <modificationTime>2021-04-30T05:31:54.872974Z</modificationTime>
        </creationInfo>
      </magnitude>
    </origin>
    <origin publicID="Origin/20210430053158.499233.85979">
      <time>
        <value>2021-04-30T05:18:32.698114Z</value>
        <uncertainty>0.6375871131</uncertainty>
      </time>
      <latitude>
        <value>-11.89444256</value>
        <uncertainty>3.859532527</uncertainty>
      </latitude>
      <longitude>
        <value>166.3646393</value>
        <uncertainty>5.222600556</uncertainty>
      </longitude>
      <depth>
        <value>70.03338623</value>
        <uncertainty>6.298131926</uncertainty>
      </depth>
      <depthType>from location</depthType>
      <methodID>LOCSAT</methodID>
      <earthModelID>iasp91</earthModelID>
      <quality>
        <associatedPhaseCount>32</associatedPhaseCount>
        <usedPhaseCount>30</usedPhaseCount>
        <associatedStationCount>32</associatedStationCount>
        <usedStationCount>30</usedStationCount>
        <standardError>1.676706938</standardError>
        <azimuthalGap>140.561676</azimuthalGap>
        <maximumDistance>84.0763855</maximumDistance>
        <minimumDistance>3.644916773</minimumDistance>
        <medianDistance>48.90618324</medianDistance>
      </quality>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>autoloc</author>
        <creationTime>2021-04-30T05:31:58.499938Z</creationTime>
      </creationInfo>
      <arrival>
        <pickID>20210430.051926.35-AIC-G.SANVU.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>167.1792297</azimuth>
        <distance>3.644916773</distance>
        <timeResidual>-0.07614135742</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052008.06-AIC-IU.HNR.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>290.6602478</azimuth>
        <distance>6.766933441</distance>
        <timeResidual>-1.294589996</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052157.79-AIC-IU.TARA.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>26.61755371</azimuth>
        <distance>14.76473904</distance>
        <timeResidual>0.9756469727</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052259.26-AIC-AU.EIDS..BHZ</pickID>
        <phase>P</phase>
        <azimuth>224.8280487</azimuth>
        <distance>19.74501991</distance>
        <timeResidual>2.775772095</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052313.39-AIC-IU.CTAO.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>244.4562683</azimuth>
        <distance>20.97071648</distance>
        <timeResidual>3.242050171</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052327.21-AIC-AU.ARMA..BHZ</pickID>
        <phase>P</phase>
        <azimuth>214.1347809</azimuth>
        <distance>23.00971985</distance>
        <timeResidual>-3.912628174</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052400.85-AIC-G.CAN.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>211.010376</azimuth>
        <distance>28.21467781</distance>
        <timeResidual>-17.38381958</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052411.24-AIC-AU.CMSA..BHZ</pickID>
        <phase>P</phase>
        <azimuth>220.8907928</azimuth>
        <distance>27.36402512</distance>
        <timeResidual>0.3565979004</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052449.69-AIC-II.WRAB.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>251.2322235</azimuth>
        <distance>31.75000954</distance>
        <timeResidual>-0.703704834</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052559.83-AIC-IA.OBMI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>282.4321289</azimuth>
        <distance>39.81884003</distance>
        <timeResidual>0.5501403809</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052611.16-AIC-GE.SANI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>280.6650696</azimuth>
        <distance>41.20639038</distance>
        <timeResidual>0.4006347656</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052645.86-AIC-IA.SMSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>283.3734436</azimuth>
        <distance>45.5534668</distance>
        <timeResidual>0.1458129883</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052657.94-AIC-II.KAPI.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>274.3849487</azimuth>
        <distance>46.55826569</distance>
        <timeResidual>4.154510498</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052658.56-AIC-AU.MEEK..BHZ</pickID>
        <phase>P</phase>
        <azimuth>244.5519257</azimuth>
        <distance>47.11654282</distance>
        <timeResidual>0.5249938965</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052659.61-AIC-IA.DBNI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>269.3451233</azimuth>
        <distance>47.36070251</distance>
        <timeResidual>-0.4943847656</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052702.21-AIC-IA.MPSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>281.5173035</azimuth>
        <distance>47.71935272</distance>
        <timeResidual>-0.4906921387</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052715.96-AIC-IA.SMKI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>279.5429382</azimuth>
        <distance>50.09301376</distance>
        <timeResidual>-4.96786499</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052742.81-AIC-MY.KKM..BHZ</pickID>
        <phase>P</phase>
        <azimuth>287.0659485</azimuth>
        <distance>52.99863815</distance>
        <timeResidual>0.3761901855</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052806.81-AIC-TW.YULB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>308.7836304</azimuth>
        <distance>56.46297455</distance>
        <timeResidual>0.1202697754</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052807.66-AIC-TW.NACB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>309.7416992</azimuth>
        <distance>56.67589188</distance>
        <timeResidual>-0.501373291</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052809.21-AIC-TW.TPUB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>308.2868652</azimuth>
        <distance>56.92330551</distance>
        <timeResidual>-0.7678527832</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052809.66-AIC-TW.SSLB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>308.9673157</azimuth>
        <distance>56.94412613</distance>
        <timeResidual>-0.436126709</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052810.81-AIC-TW.YHNB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>310.0943909</azimuth>
        <distance>57.12177658</distance>
        <timeResidual>-0.4940490723</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052825.23-AIC-IA.CGJI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>269.1782227</azimuth>
        <distance>60.01316071</distance>
        <timeResidual>-7.469573975</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052908.75-AIC-GT.VNDA.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>181.0692749</azimuth>
        <distance>65.66401672</distance>
        <timeResidual>-0.3756408691</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052909.46-AIC-IU.CASY.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>201.4000092</azimuth>
        <distance>65.80833435</distance>
        <timeResidual>-0.7950744629</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052911.41-AIC-IU.SBA.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>179.9095154</azimuth>
        <distance>65.95506287</distance>
        <timeResidual>0.4126281738</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052921.26-AIC-IA.MNSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>275.8913879</azimuth>
        <distance>67.49115753</distance>
        <timeResidual>-0.6163024902</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052936.16-AIC-RM.SLV..BHZ</pickID>
        <phase>P</phase>
        <azimuth>298.3023071</azimuth>
        <distance>69.72905731</distance>
        <timeResidual>1.005096436</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052956.89-AIC-IU.CHTO.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>294.073822</azimuth>
        <distance>73.19121552</distance>
        <timeResidual>0.7007141113</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.053031.16-AIC-IU.ULN.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>324.0735474</azimuth>
        <distance>79.50198364</distance>
        <timeResidual>0.3186950684</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.053054.31-AIC-IU.COLA.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>17.81292152</azimuth>
        <distance>84.0763855</distance>
        <timeResidual>-0.1329040527</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/AU.ARMA">
        <magnitude>
          <value>4.920119292</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052327.21-AIC-AU.ARMA..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="ARMA" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.525202Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/AU.CMSA">
        <magnitude>
          <value>5.029037456</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052411.24-AIC-AU.CMSA..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="CMSA" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.525623Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/MN/AU.EIDS">
        <magnitude>
          <value>3.759179174</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430053140.209493.114038</amplitudeID>
        <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.526001Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/Mjma/AU.EIDS">
        <magnitude>
          <value>5.049253329</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052259.26-AIC-AU.EIDS..BHZ.Mjma</amplitudeID>
        <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.526267Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/AU.EIDS">
        <magnitude>
          <value>4.528621727</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052259.26-AIC-AU.EIDS..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.526416Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/AU.MEEK">
        <magnitude>
          <value>5.247633206</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052658.56-AIC-AU.MEEK..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="MEEK" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.526784Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/MN/G.SANVU">
        <magnitude>
          <value>4.673832763</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052822.994143.113564</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.527018Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/ML/G.SANVU">
        <magnitude>
          <value>5.415495523</value>
        </magnitude>
        <type>ML</type>
        <amplitudeID>Amplitude/20210430052823.012927.113567</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.527193Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/MLv/G.SANVU">
        <magnitude>
          <value>5.449721042</value>
        </magnitude>
        <type>MLv</type>
        <amplitudeID>20210430.051926.35-AIC-G.SANVU.00.BHZ.MLv</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.527387Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/Mjma/G.SANVU">
        <magnitude>
          <value>5.143171915</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.051926.35-AIC-G.SANVU.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.527505Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/GE.SANI">
        <magnitude>
          <value>5.010287069</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052611.16-AIC-GE.SANI..BHZ.mb</amplitudeID>
        <waveformID networkCode="GE" stationCode="SANI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.528154Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/GT.VNDA">
        <magnitude>
          <value>4.643345805</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052908.75-AIC-GT.VNDA.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="GT" stationCode="VNDA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.528326Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/IA.DBNI">
        <magnitude>
          <value>5.058611939</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052659.61-AIC-IA.DBNI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="DBNI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.528693Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/IA.MPSI">
        <magnitude>
          <value>4.962304655</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052702.21-AIC-IA.MPSI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="MPSI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.529171Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/IA.OBMI">
        <magnitude>
          <value>5.240620041</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052559.83-AIC-IA.OBMI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="OBMI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.529544Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mB/IA.SMKI">
        <magnitude>
          <value>6.91403983</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052715.96-AIC-IA.SMKI..BHZ.mB</amplitudeID>
        <waveformID networkCode="IA" stationCode="SMKI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.529916Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/IA.SMKI">
        <magnitude>
          <value>6.026157327</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052715.96-AIC-IA.SMKI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="SMKI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.530049Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/IA.SMSI">
        <magnitude>
          <value>4.664260582</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052645.86-AIC-IA.SMSI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="SMSI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.530418Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/II.KAPI">
        <magnitude>
          <value>4.57793881</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052657.94-AIC-II.KAPI.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="II" stationCode="KAPI" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.530795Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/II.WRAB">
        <magnitude>
          <value>4.556579564</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052449.69-AIC-II.WRAB.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="II" stationCode="WRAB" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.531155Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/IU.CASY">
        <magnitude>
          <value>4.956066524</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>Amplitude/20210430053131.536021.113922</amplitudeID>
        <waveformID networkCode="IU" stationCode="CASY" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.531317Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mB/IU.CHTO">
        <magnitude>
          <value>4.980706713</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052956.89-AIC-IU.CHTO.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="CHTO" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.531473Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/IU.CHTO">
        <magnitude>
          <value>4.728401888</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052956.89-AIC-IU.CHTO.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="CHTO" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.531598Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/IU.COLA">
        <magnitude>
          <value>4.309316275</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.053054.31-AIC-IU.COLA.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="COLA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.531741Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/IU.CTAO">
        <magnitude>
          <value>4.17808498</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052313.39-AIC-IU.CTAO.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.532129Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/MN/IU.HNR">
        <magnitude>
          <value>3.198905354</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052823.066898.113574</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.532409Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/ML/IU.HNR">
        <magnitude>
          <value>4.245194265</value>
        </magnitude>
        <type>ML</type>
        <amplitudeID>Amplitude/20210430052823.068657.113575</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BH"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.532606Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/MLv/IU.HNR">
        <magnitude>
          <value>4.510271764</value>
        </magnitude>
        <type>MLv</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.MLv</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.532798Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/Mjma/IU.HNR">
        <magnitude>
          <value>3.853548969</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.53292Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mB/IU.HNR">
        <magnitude>
          <value>4.520281602</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.533065Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/IU.HNR">
        <magnitude>
          <value>4.595848017</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.53321Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/IU.SBA">
        <magnitude>
          <value>4.341501942</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052911.41-AIC-IU.SBA.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="SBA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.53338Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/MN/IU.TARA">
        <magnitude>
          <value>4.861439215</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052823.094023.113580</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.533687Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/Mjma/IU.TARA">
        <magnitude>
          <value>5.647702813</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.533948Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mB/IU.TARA">
        <magnitude>
          <value>5.974637949</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.534088Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/IU.TARA">
        <magnitude>
          <value>5.661513132</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.534221Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/MY.KKM">
        <magnitude>
          <value>5.186747328</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052742.81-AIC-MY.KKM..BHZ.mb</amplitudeID>
        <waveformID networkCode="MY" stationCode="KKM" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.541675Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/RM.SLV">
        <magnitude>
          <value>4.748470866</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052936.16-AIC-RM.SLV..BHZ.mb</amplitudeID>
        <waveformID networkCode="RM" stationCode="SLV" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.54186Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/TW.NACB">
        <magnitude>
          <value>5.15765558</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052807.66-AIC-TW.NACB..BHZ.mb</amplitudeID>
        <waveformID networkCode="TW" stationCode="NACB" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.542231Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/TW.SSLB">
        <magnitude>
          <value>4.958073427</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052809.66-AIC-TW.SSLB..BHZ.mb</amplitudeID>
        <waveformID networkCode="TW" stationCode="SSLB" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.542616Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/TW.TPUB">
        <magnitude>
          <value>5.462048907</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052809.21-AIC-TW.TPUB..BHZ.mb</amplitudeID>
        <waveformID networkCode="TW" stationCode="TPUB" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.542984Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/TW.YHNB">
        <magnitude>
          <value>5.247874604</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052810.81-AIC-TW.YHNB..BHZ.mb</amplitudeID>
        <waveformID networkCode="TW" stationCode="YHNB" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.543378Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/MN/IU.CTAO">
        <magnitude>
          <value>3.681643004</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430053158.72278.114063</amplitudeID>
        <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.729409Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mB/IU.ULN">
        <magnitude>
          <value>5.807719537</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.053031.16-AIC-IU.ULN.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="ULN" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:32:05.893497Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053158.499233.85979/staMag/mb/IU.ULN">
        <magnitude>
          <value>5.548709652</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.053031.16-AIC-IU.ULN.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="ULN" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:32:05.888404Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <magnitude publicID="Origin/20210430053158.499233.85979/netMag/ML">
        <magnitude>
          <value>4.830344894</value>
          <uncertainty>0.8275279552</uncertainty>
        </magnitude>
        <type>ML</type>
        <methodID>mean</methodID>
        <stationCount>2</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.543747Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/ML/G.SANVU</stationMagnitudeID>
          <residual>0.5851506288</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/ML/IU.HNR</stationMagnitudeID>
          <residual>-0.5851506288</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430053158.499233.85979/netMag/MLv">
        <magnitude>
          <value>4.979996403</value>
          <uncertainty>0.664290955</uncertainty>
        </magnitude>
        <type>MLv</type>
        <methodID>mean</methodID>
        <stationCount>2</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.543838Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/MLv/G.SANVU</stationMagnitudeID>
          <residual>0.469724639</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/MLv/IU.HNR</stationMagnitudeID>
          <residual>-0.469724639</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430053158.499233.85979/netMag/MN">
        <magnitude>
          <value>4.673832763</value>
          <uncertainty>0</uncertainty>
        </magnitude>
        <type>MN</type>
        <methodID>mean</methodID>
        <stationCount>1</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.543893Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/MN/AU.EIDS</stationMagnitudeID>
          <residual>-0.9146535894</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/MN/G.SANVU</stationMagnitudeID>
          <residual>0</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/MN/IU.CTAO</stationMagnitudeID>
          <residual>-0.9921897589</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/MN/IU.HNR</stationMagnitudeID>
          <residual>-1.474927409</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/MN/IU.TARA</stationMagnitudeID>
          <residual>0.1876064522</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430053158.499233.85979/netMag/Mjma">
        <magnitude>
          <value>4.981017045</value>
          <uncertainty>0.6666248872</uncertainty>
        </magnitude>
        <type>Mjma</type>
        <methodID>trimmed mean(25)</methodID>
        <stationCount>4</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.543948Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/Mjma/AU.EIDS</stationMagnitudeID>
          <residual>0.06823628395</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/Mjma/G.SANVU</stationMagnitudeID>
          <residual>0.1621548699</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/Mjma/IU.HNR</stationMagnitudeID>
          <residual>-1.127468076</residual>
          <weight>0.5</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/Mjma/IU.TARA</stationMagnitudeID>
          <residual>0.6666857683</residual>
          <weight>0.5</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430053158.499233.85979/netMag/mB">
        <magnitude>
          <value>5.613582596</value>
          <uncertainty>0.7748355917</uncertainty>
        </magnitude>
        <type>mB</type>
        <methodID>trimmed mean(25)</methodID>
        <stationCount>5</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.544004Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mB/IA.SMKI</stationMagnitudeID>
          <residual>1.300457234</residual>
          <weight>0.375</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mB/IU.CHTO</stationMagnitudeID>
          <residual>-0.632875883</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mB/IU.HNR</stationMagnitudeID>
          <residual>-1.093300995</residual>
          <weight>0.375</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mB/IU.TARA</stationMagnitudeID>
          <residual>0.3610553529</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mB/IU.ULN</stationMagnitudeID>
          <residual>0.1941369403</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430053158.499233.85979/netMag/Mw(mB)">
        <magnitude>
          <value>5.117657375</value>
          <uncertainty>0.7748355917</uncertainty>
        </magnitude>
        <type>Mw(mB)</type>
        <stationCount>5</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.544071Z</creationTime>
          <modificationTime>2021-04-30T05:32:05.893587Z</modificationTime>
        </creationInfo>
      </magnitude>
      <magnitude publicID="Origin/20210430053158.499233.85979/netMag/mb">
        <magnitude>
          <value>4.925138064</value>
          <uncertainty>0.262864812</uncertainty>
        </magnitude>
        <type>mb</type>
        <methodID>trimmed mean(25)</methodID>
        <stationCount>21</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.544092Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/AU.ARMA</stationMagnitudeID>
          <residual>-0.005018772218</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/AU.CMSA</stationMagnitudeID>
          <residual>0.1038993924</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/AU.EIDS</stationMagnitudeID>
          <residual>-0.3965163367</residual>
          <weight>0.625</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/AU.MEEK</stationMagnitudeID>
          <residual>0.3224951417</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/GE.SANI</stationMagnitudeID>
          <residual>0.08514900441</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/GT.VNDA</stationMagnitudeID>
          <residual>-0.2817922591</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/IA.DBNI</stationMagnitudeID>
          <residual>0.1334738749</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/IA.MPSI</stationMagnitudeID>
          <residual>0.03716659109</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/IA.OBMI</stationMagnitudeID>
          <residual>0.3154819769</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/IA.SMKI</stationMagnitudeID>
          <residual>1.101019263</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/IA.SMSI</stationMagnitudeID>
          <residual>-0.2608774825</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/II.KAPI</stationMagnitudeID>
          <residual>-0.3471992539</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/II.WRAB</stationMagnitudeID>
          <residual>-0.3685585004</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/IU.CASY</stationMagnitudeID>
          <residual>0.03092845971</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/IU.CHTO</stationMagnitudeID>
          <residual>-0.1967361764</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/IU.COLA</stationMagnitudeID>
          <residual>-0.615821789</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/IU.CTAO</stationMagnitudeID>
          <residual>-0.7470530846</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/IU.HNR</stationMagnitudeID>
          <residual>-0.3292900467</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/IU.SBA</stationMagnitudeID>
          <residual>-0.5836361216</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/IU.TARA</stationMagnitudeID>
          <residual>0.7363750676</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/IU.ULN</stationMagnitudeID>
          <residual>0.6235715882</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/MY.KKM</stationMagnitudeID>
          <residual>0.2616092635</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/RM.SLV</stationMagnitudeID>
          <residual>-0.1766671984</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/TW.NACB</stationMagnitudeID>
          <residual>0.2325175155</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/TW.SSLB</stationMagnitudeID>
          <residual>0.03293536309</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/TW.TPUB</stationMagnitudeID>
          <residual>0.5369108427</residual>
          <weight>0.625</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053158.499233.85979/staMag/mb/TW.YHNB</stationMagnitudeID>
          <residual>0.3227365402</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430053158.499233.85979/netMag/M">
        <magnitude>
          <value>4.926854707</value>
        </magnitude>
        <type>M</type>
        <methodID>weighted average</methodID>
        <stationCount>21</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:31:58.544396Z</creationTime>
          <modificationTime>2021-04-30T05:32:05.893656Z</modificationTime>
        </creationInfo>
      </magnitude>
    </origin>
    <origin publicID="Origin/20210430053829.67918.86662">
      <time>
        <value>2021-04-30T05:18:32.698118Z</value>
        <uncertainty>0.6375871131</uncertainty>
      </time>
      <latitude>
        <value>-11.89444256</value>
        <uncertainty>3.859532527</uncertainty>
      </latitude>
      <longitude>
        <value>166.3646393</value>
        <uncertainty>5.222600556</uncertainty>
      </longitude>
      <depth>
        <value>70.03339386</value>
        <uncertainty>6.298131926</uncertainty>
      </depth>
      <depthType>from location</depthType>
      <methodID>LOCSAT</methodID>
      <earthModelID>iasp91</earthModelID>
      <quality>
        <associatedPhaseCount>33</associatedPhaseCount>
        <usedPhaseCount>30</usedPhaseCount>
        <associatedStationCount>33</associatedStationCount>
        <usedStationCount>30</usedStationCount>
        <standardError>1.676706938</standardError>
        <azimuthalGap>140.561676</azimuthalGap>
        <maximumDistance>84.0763855</maximumDistance>
        <minimumDistance>3.644916773</minimumDistance>
        <medianDistance>48.90618324</medianDistance>
      </quality>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>autoloc</author>
        <creationTime>2021-04-30T05:38:29.679436Z</creationTime>
      </creationInfo>
      <arrival>
        <pickID>20210430.051926.35-AIC-G.SANVU.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>167.1792297</azimuth>
        <distance>3.644916773</distance>
        <timeResidual>-0.07614517212</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052008.06-AIC-IU.HNR.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>290.6602478</azimuth>
        <distance>6.766933441</distance>
        <timeResidual>-1.294593811</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052157.79-AIC-IU.TARA.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>26.61755371</azimuth>
        <distance>14.76473904</distance>
        <timeResidual>0.975643158</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052259.26-AIC-AU.EIDS..BHZ</pickID>
        <phase>P</phase>
        <azimuth>224.8280487</azimuth>
        <distance>19.74501991</distance>
        <timeResidual>2.77576828</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052313.39-AIC-IU.CTAO.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>244.4562683</azimuth>
        <distance>20.97071648</distance>
        <timeResidual>3.242046356</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052327.21-AIC-AU.ARMA..BHZ</pickID>
        <phase>P</phase>
        <azimuth>214.1347809</azimuth>
        <distance>23.00971985</distance>
        <timeResidual>-3.912631989</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052411.24-AIC-AU.CMSA..BHZ</pickID>
        <phase>P</phase>
        <azimuth>220.8907928</azimuth>
        <distance>27.36402512</distance>
        <timeResidual>0.3565940857</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052400.85-AIC-G.CAN.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>211.010376</azimuth>
        <distance>28.21467781</distance>
        <timeResidual>-17.38382339</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052449.69-AIC-II.WRAB.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>251.2322235</azimuth>
        <distance>31.75000954</distance>
        <timeResidual>-0.7037086487</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052559.83-AIC-IA.OBMI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>282.4321289</azimuth>
        <distance>39.81884003</distance>
        <timeResidual>0.5501365662</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052611.16-AIC-GE.SANI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>280.6650696</azimuth>
        <distance>41.20639038</distance>
        <timeResidual>0.4006309509</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052645.86-AIC-IA.SMSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>283.3734436</azimuth>
        <distance>45.5534668</distance>
        <timeResidual>0.1458091736</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052657.94-AIC-II.KAPI.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>274.3849487</azimuth>
        <distance>46.55826569</distance>
        <timeResidual>4.154506683</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052658.56-AIC-AU.MEEK..BHZ</pickID>
        <phase>P</phase>
        <azimuth>244.5519257</azimuth>
        <distance>47.11654282</distance>
        <timeResidual>0.5249900818</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052659.61-AIC-IA.DBNI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>269.3451233</azimuth>
        <distance>47.36070251</distance>
        <timeResidual>-0.4943885803</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052702.21-AIC-IA.MPSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>281.5173035</azimuth>
        <distance>47.71935272</distance>
        <timeResidual>-0.4906959534</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052715.96-AIC-IA.SMKI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>279.5429382</azimuth>
        <distance>50.09301376</distance>
        <timeResidual>-4.967868805</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052742.81-AIC-MY.KKM..BHZ</pickID>
        <phase>P</phase>
        <azimuth>287.0659485</azimuth>
        <distance>52.99863815</distance>
        <timeResidual>0.3761863708</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052806.81-AIC-TW.YULB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>308.7836304</azimuth>
        <distance>56.46297455</distance>
        <timeResidual>0.1202659607</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052807.66-AIC-TW.NACB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>309.7416992</azimuth>
        <distance>56.67589188</distance>
        <timeResidual>-0.5013771057</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052809.21-AIC-TW.TPUB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>308.2868652</azimuth>
        <distance>56.92330551</distance>
        <timeResidual>-0.7678565979</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052809.66-AIC-TW.SSLB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>308.9673157</azimuth>
        <distance>56.94412613</distance>
        <timeResidual>-0.4361305237</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052810.81-AIC-TW.YHNB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>310.0943909</azimuth>
        <distance>57.12177658</distance>
        <timeResidual>-0.494052887</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052825.23-AIC-IA.CGJI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>269.1782227</azimuth>
        <distance>60.01316071</distance>
        <timeResidual>-7.469577789</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052908.75-AIC-GT.VNDA.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>181.0692749</azimuth>
        <distance>65.66401672</distance>
        <timeResidual>-0.3756446838</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052909.46-AIC-IU.CASY.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>201.4000092</azimuth>
        <distance>65.80833435</distance>
        <timeResidual>-0.7950782776</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052911.41-AIC-IU.SBA.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>179.9095154</azimuth>
        <distance>65.95506287</distance>
        <timeResidual>0.4126243591</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052921.26-AIC-IA.MNSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>275.8913879</azimuth>
        <distance>67.49115753</distance>
        <timeResidual>-0.6163063049</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052936.16-AIC-RM.SLV..BHZ</pickID>
        <phase>P</phase>
        <azimuth>298.3023071</azimuth>
        <distance>69.72905731</distance>
        <timeResidual>1.005092621</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052956.89-AIC-IU.CHTO.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>294.073822</azimuth>
        <distance>73.19121552</distance>
        <timeResidual>0.7007102966</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.053031.16-AIC-IU.ULN.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>324.0735474</azimuth>
        <distance>79.50198364</distance>
        <timeResidual>0.3186912537</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.053054.31-AIC-IU.COLA.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>17.81292152</azimuth>
        <distance>84.0763855</distance>
        <timeResidual>-0.1329078674</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.053746.90-AIC-HU.TRPA..BHZ</pickID>
        <phase>PKP</phase>
        <azimuth>327.4684143</azimuth>
        <distance>132.8972778</distance>
        <timeResidual>7.229756832</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/AU.ARMA">
        <magnitude>
          <value>4.920119283</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052327.21-AIC-AU.ARMA..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="ARMA" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.714408Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/AU.CMSA">
        <magnitude>
          <value>5.02903745</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052411.24-AIC-AU.CMSA..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="CMSA" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.714835Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/MN/AU.EIDS">
        <magnitude>
          <value>3.759179174</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430053140.209493.114038</amplitudeID>
        <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.715253Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/Mjma/AU.EIDS">
        <magnitude>
          <value>5.049253329</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052259.26-AIC-AU.EIDS..BHZ.Mjma</amplitudeID>
        <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.715537Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/AU.EIDS">
        <magnitude>
          <value>4.528621711</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052259.26-AIC-AU.EIDS..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.715681Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/AU.MEEK">
        <magnitude>
          <value>5.247633196</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052658.56-AIC-AU.MEEK..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="MEEK" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.716082Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/MN/G.SANVU">
        <magnitude>
          <value>4.673832763</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052822.994143.113564</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.716391Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/ML/G.SANVU">
        <magnitude>
          <value>5.415495523</value>
        </magnitude>
        <type>ML</type>
        <amplitudeID>Amplitude/20210430052823.012927.113567</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.716599Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/MLv/G.SANVU">
        <magnitude>
          <value>5.449721042</value>
        </magnitude>
        <type>MLv</type>
        <amplitudeID>20210430.051926.35-AIC-G.SANVU.00.BHZ.MLv</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.716791Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/Mjma/G.SANVU">
        <magnitude>
          <value>5.143171915</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.051926.35-AIC-G.SANVU.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.716913Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/GE.SANI">
        <magnitude>
          <value>5.010287064</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052611.16-AIC-GE.SANI..BHZ.mb</amplitudeID>
        <waveformID networkCode="GE" stationCode="SANI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.717596Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/GT.VNDA">
        <magnitude>
          <value>4.643345795</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052908.75-AIC-GT.VNDA.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="GT" stationCode="VNDA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.717996Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/IA.DBNI">
        <magnitude>
          <value>5.058611929</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052659.61-AIC-IA.DBNI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="DBNI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.718392Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/IA.MPSI">
        <magnitude>
          <value>4.962304644</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052702.21-AIC-IA.MPSI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="MPSI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.719122Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/IA.OBMI">
        <magnitude>
          <value>5.240620032</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052559.83-AIC-IA.OBMI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="OBMI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.719508Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mB/IA.SMKI">
        <magnitude>
          <value>6.914039825</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052715.96-AIC-IA.SMKI..BHZ.mB</amplitudeID>
        <waveformID networkCode="IA" stationCode="SMKI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.719921Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/IA.SMKI">
        <magnitude>
          <value>6.026157322</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052715.96-AIC-IA.SMKI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="SMKI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.720058Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/IA.SMSI">
        <magnitude>
          <value>4.664260577</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052645.86-AIC-IA.SMSI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="SMSI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.720442Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/II.KAPI">
        <magnitude>
          <value>4.577938802</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052657.94-AIC-II.KAPI.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="II" stationCode="KAPI" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.720838Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/II.WRAB">
        <magnitude>
          <value>4.556579555</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052449.69-AIC-II.WRAB.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="II" stationCode="WRAB" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.721228Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/IU.CASY">
        <magnitude>
          <value>4.956066513</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>Amplitude/20210430053131.536021.113922</amplitudeID>
        <waveformID networkCode="IU" stationCode="CASY" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.721654Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mB/IU.CHTO">
        <magnitude>
          <value>4.980706705</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052956.89-AIC-IU.CHTO.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="CHTO" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.722094Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/IU.CHTO">
        <magnitude>
          <value>4.728401879</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052956.89-AIC-IU.CHTO.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="CHTO" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.722258Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/IU.COLA">
        <magnitude>
          <value>4.309316266</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.053054.31-AIC-IU.COLA.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="COLA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.735888Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/MN/IU.CTAO">
        <magnitude>
          <value>3.681643004</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430053158.72278.114063</amplitudeID>
        <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.736393Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/IU.CTAO">
        <magnitude>
          <value>4.17808497</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052313.39-AIC-IU.CTAO.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.73678Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/MN/IU.HNR">
        <magnitude>
          <value>3.198905354</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052823.066898.113574</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.73708Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/ML/IU.HNR">
        <magnitude>
          <value>4.245194265</value>
        </magnitude>
        <type>ML</type>
        <amplitudeID>Amplitude/20210430052823.068657.113575</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BH"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.737276Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/MLv/IU.HNR">
        <magnitude>
          <value>4.510271764</value>
        </magnitude>
        <type>MLv</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.MLv</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.737484Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/Mjma/IU.HNR">
        <magnitude>
          <value>3.853548969</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.737611Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mB/IU.HNR">
        <magnitude>
          <value>4.520281574</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.737746Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/IU.HNR">
        <magnitude>
          <value>4.59584799</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.737897Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/IU.SBA">
        <magnitude>
          <value>4.341501931</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052911.41-AIC-IU.SBA.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="SBA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.738315Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/MN/IU.TARA">
        <magnitude>
          <value>4.861439215</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052823.094023.113580</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.738679Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/Mjma/IU.TARA">
        <magnitude>
          <value>5.647702813</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.738971Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mB/IU.TARA">
        <magnitude>
          <value>5.974637936</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.739126Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/IU.TARA">
        <magnitude>
          <value>5.661513119</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.739275Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mB/IU.ULN">
        <magnitude>
          <value>5.807719531</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.053031.16-AIC-IU.ULN.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="ULN" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.73971Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/IU.ULN">
        <magnitude>
          <value>5.548709646</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.053031.16-AIC-IU.ULN.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="ULN" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.739854Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/MY.KKM">
        <magnitude>
          <value>5.186747325</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052742.81-AIC-MY.KKM..BHZ.mb</amplitudeID>
        <waveformID networkCode="MY" stationCode="KKM" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.740252Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/RM.SLV">
        <magnitude>
          <value>4.748470854</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052936.16-AIC-RM.SLV..BHZ.mb</amplitudeID>
        <waveformID networkCode="RM" stationCode="SLV" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.740679Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/TW.NACB">
        <magnitude>
          <value>5.157655573</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052807.66-AIC-TW.NACB..BHZ.mb</amplitudeID>
        <waveformID networkCode="TW" stationCode="NACB" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.741083Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/TW.SSLB">
        <magnitude>
          <value>4.958073421</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052809.66-AIC-TW.SSLB..BHZ.mb</amplitudeID>
        <waveformID networkCode="TW" stationCode="SSLB" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.741483Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/TW.TPUB">
        <magnitude>
          <value>5.4620489</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052809.21-AIC-TW.TPUB..BHZ.mb</amplitudeID>
        <waveformID networkCode="TW" stationCode="TPUB" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.741873Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/mb/TW.YHNB">
        <magnitude>
          <value>5.247874598</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052810.81-AIC-TW.YHNB..BHZ.mb</amplitudeID>
        <waveformID networkCode="TW" stationCode="YHNB" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.742258Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/MN/AU.ARMA">
        <magnitude>
          <value>4.112490252</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430053830.009.114526</amplitudeID>
        <waveformID networkCode="AU" stationCode="ARMA" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:30.018393Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430053829.67918.86662/staMag/MN/AU.CMSA">
        <magnitude>
          <value>4.483911138</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430053830.002206.114525</amplitudeID>
        <waveformID networkCode="AU" stationCode="CMSA" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:30.01129Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <magnitude publicID="Origin/20210430053829.67918.86662/netMag/ML">
        <magnitude>
          <value>4.830344894</value>
          <uncertainty>0.8275279552</uncertainty>
        </magnitude>
        <type>ML</type>
        <methodID>mean</methodID>
        <stationCount>2</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.742649Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/ML/G.SANVU</stationMagnitudeID>
          <residual>0.5851506288</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/ML/IU.HNR</stationMagnitudeID>
          <residual>-0.5851506288</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430053829.67918.86662/netMag/MLv">
        <magnitude>
          <value>4.979996403</value>
          <uncertainty>0.664290955</uncertainty>
        </magnitude>
        <type>MLv</type>
        <methodID>mean</methodID>
        <stationCount>2</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.742737Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/MLv/G.SANVU</stationMagnitudeID>
          <residual>0.469724639</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/MLv/IU.HNR</stationMagnitudeID>
          <residual>-0.469724639</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430053829.67918.86662/netMag/MN">
        <magnitude>
          <value>4.673832763</value>
          <uncertainty>0</uncertainty>
        </magnitude>
        <type>MN</type>
        <methodID>mean</methodID>
        <stationCount>1</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.742792Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/MN/G.SANVU</stationMagnitudeID>
          <residual>0</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/MN/AU.EIDS</stationMagnitudeID>
          <residual>-0.9146535894</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/MN/IU.CTAO</stationMagnitudeID>
          <residual>-0.9921897589</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/MN/IU.HNR</stationMagnitudeID>
          <residual>-1.474927409</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/MN/IU.TARA</stationMagnitudeID>
          <residual>0.1876064522</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/MN/AU.CMSA</stationMagnitudeID>
          <residual>-0.1899216255</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/MN/AU.ARMA</stationMagnitudeID>
          <residual>-0.5613425109</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430053829.67918.86662/netMag/Mjma">
        <magnitude>
          <value>4.981017045</value>
          <uncertainty>0.6666248872</uncertainty>
        </magnitude>
        <type>Mjma</type>
        <methodID>trimmed mean(25)</methodID>
        <stationCount>4</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.742854Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/Mjma/AU.EIDS</stationMagnitudeID>
          <residual>0.06823628395</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/Mjma/G.SANVU</stationMagnitudeID>
          <residual>0.1621548699</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/Mjma/IU.HNR</stationMagnitudeID>
          <residual>-1.127468076</residual>
          <weight>0.5</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/Mjma/IU.TARA</stationMagnitudeID>
          <residual>0.6666857683</residual>
          <weight>0.5</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430053829.67918.86662/netMag/mB">
        <magnitude>
          <value>5.613582586</value>
          <uncertainty>0.7748355954</uncertainty>
        </magnitude>
        <type>mB</type>
        <methodID>trimmed mean(25)</methodID>
        <stationCount>5</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.742922Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mB/IA.SMKI</stationMagnitudeID>
          <residual>1.300457239</residual>
          <weight>0.375</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mB/IU.CHTO</stationMagnitudeID>
          <residual>-0.6328758808</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mB/IU.HNR</stationMagnitudeID>
          <residual>-1.093301012</residual>
          <weight>0.375</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mB/IU.TARA</stationMagnitudeID>
          <residual>0.3610553505</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mB/IU.ULN</stationMagnitudeID>
          <residual>0.194136945</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430053829.67918.86662/netMag/Mw(mB)">
        <magnitude>
          <value>5.117657361</value>
          <uncertainty>0.7748355954</uncertainty>
        </magnitude>
        <type>Mw(mB)</type>
        <stationCount>5</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.742994Z</creationTime>
        </creationInfo>
      </magnitude>
      <magnitude publicID="Origin/20210430053829.67918.86662/netMag/mb">
        <magnitude>
          <value>4.925138055</value>
          <uncertainty>0.2628648141</uncertainty>
        </magnitude>
        <type>mb</type>
        <methodID>trimmed mean(25)</methodID>
        <stationCount>21</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.743021Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/AU.ARMA</stationMagnitudeID>
          <residual>-0.005018772012</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/AU.CMSA</stationMagnitudeID>
          <residual>0.1038993948</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/AU.EIDS</stationMagnitudeID>
          <residual>-0.3965163442</residual>
          <weight>0.625</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/AU.MEEK</stationMagnitudeID>
          <residual>0.3224951415</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/GE.SANI</stationMagnitudeID>
          <residual>0.08514900943</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/GT.VNDA</stationMagnitudeID>
          <residual>-0.28179226</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/IA.DBNI</stationMagnitudeID>
          <residual>0.1334738739</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/IA.MPSI</stationMagnitudeID>
          <residual>0.03716658907</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/IA.OBMI</stationMagnitudeID>
          <residual>0.3154819771</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/IA.SMKI</stationMagnitudeID>
          <residual>1.101019267</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/IA.SMSI</stationMagnitudeID>
          <residual>-0.2608774779</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/II.KAPI</stationMagnitudeID>
          <residual>-0.3471992523</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/II.WRAB</stationMagnitudeID>
          <residual>-0.3685584995</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/IU.CASY</stationMagnitudeID>
          <residual>0.030928458</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/IU.CHTO</stationMagnitudeID>
          <residual>-0.1967361756</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/IU.COLA</stationMagnitudeID>
          <residual>-0.6158217886</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/IU.CTAO</stationMagnitudeID>
          <residual>-0.7470530846</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/IU.HNR</stationMagnitudeID>
          <residual>-0.3292900651</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/IU.SBA</stationMagnitudeID>
          <residual>-0.5836361242</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/IU.TARA</stationMagnitudeID>
          <residual>0.7363750638</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/IU.ULN</stationMagnitudeID>
          <residual>0.6235715914</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/MY.KKM</stationMagnitudeID>
          <residual>0.2616092697</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/RM.SLV</stationMagnitudeID>
          <residual>-0.1766672012</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/TW.NACB</stationMagnitudeID>
          <residual>0.2325175178</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/TW.SSLB</stationMagnitudeID>
          <residual>0.03293536615</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/TW.TPUB</stationMagnitudeID>
          <residual>0.5369108457</residual>
          <weight>0.625</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430053829.67918.86662/staMag/mb/TW.YHNB</stationMagnitudeID>
          <residual>0.3227365427</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430053829.67918.86662/netMag/M">
        <magnitude>
          <value>4.926854703</value>
        </magnitude>
        <type>M</type>
        <methodID>weighted average</methodID>
        <stationCount>21</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T05:38:29.743329Z</creationTime>
        </creationInfo>
      </magnitude>
    </origin>
    <origin publicID="Origin/20210430060321.912081.87745">
      <time>
        <value>2021-04-30T05:18:32.700063Z</value>
        <uncertainty>0.637446894</uncertainty>
      </time>
      <latitude>
        <value>-11.89324379</value>
        <uncertainty>3.834316805</uncertainty>
      </latitude>
      <longitude>
        <value>166.3651733</value>
        <uncertainty>5.219074171</uncertainty>
      </longitude>
      <depth>
        <value>70.18717957</value>
        <uncertainty>6.274564155</uncertainty>
      </depth>
      <depthType>from location</depthType>
      <methodID>LOCSAT</methodID>
      <earthModelID>iasp91</earthModelID>
      <quality>
        <associatedPhaseCount>34</associatedPhaseCount>
        <usedPhaseCount>31</usedPhaseCount>
        <associatedStationCount>34</associatedStationCount>
        <usedStationCount>31</usedStationCount>
        <standardError>1.650300786</standardError>
        <azimuthalGap>140.573576</azimuthalGap>
        <maximumDistance>84.0750885</maximumDistance>
        <minimumDistance>3.645969629</minimumDistance>
        <medianDistance>50.09333038</medianDistance>
      </quality>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>autoloc</author>
        <creationTime>2021-04-30T06:03:21.912386Z</creationTime>
      </creationInfo>
      <arrival>
        <pickID>20210430.051926.35-AIC-G.SANVU.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>167.1912842</azimuth>
        <distance>3.645969629</distance>
        <timeResidual>-0.09363555908</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052008.06-AIC-IU.HNR.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>290.6491394</azimuth>
        <distance>6.766999722</distance>
        <timeResidual>-1.297317505</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052157.79-AIC-IU.TARA.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>26.61770821</azimuth>
        <distance>14.76343346</distance>
        <timeResidual>0.9966621399</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052259.26-AIC-AU.EIDS..BHZ</pickID>
        <phase>P</phase>
        <azimuth>224.8266144</azimuth>
        <distance>19.7462368</distance>
        <timeResidual>2.772266388</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052313.39-AIC-IU.CTAO.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>244.4539337</azimuth>
        <distance>20.97170448</distance>
        <timeResidual>3.241382599</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052327.21-AIC-AU.ARMA..BHZ</pickID>
        <phase>P</phase>
        <azimuth>214.1341095</azimuth>
        <distance>23.0110054</distance>
        <timeResidual>-3.915737152</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052400.85-AIC-G.CAN.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>211.0099487</azimuth>
        <distance>28.21597481</distance>
        <timeResidual>-17.38268661</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052411.24-AIC-AU.CMSA..BHZ</pickID>
        <phase>P</phase>
        <azimuth>220.8899231</azimuth>
        <distance>27.36527443</distance>
        <timeResidual>0.3580360413</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052449.69-AIC-II.WRAB.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>251.2305603</azimuth>
        <distance>31.75089073</distance>
        <timeResidual>-0.6986045837</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052559.83-AIC-IA.OBMI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>282.430481</azimuth>
        <distance>39.8190918</distance>
        <timeResidual>0.5612831116</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052611.16-AIC-GE.SANI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>280.6635132</azimuth>
        <distance>41.2066803</distance>
        <timeResidual>0.4115333557</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052645.86-AIC-IA.SMSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>283.3720703</azimuth>
        <distance>45.55369949</distance>
        <timeResidual>0.1575965881</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052657.94-AIC-II.KAPI.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>274.383667</azimuth>
        <distance>46.55869293</distance>
        <timeResidual>4.164890289</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052658.56-AIC-AU.MEEK..BHZ</pickID>
        <phase>P</phase>
        <azimuth>244.5510254</azimuth>
        <distance>47.11753082</distance>
        <timeResidual>0.531162262</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052659.61-AIC-IA.DBNI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>269.3439026</azimuth>
        <distance>47.36123657</distance>
        <timeResidual>-0.4847373962</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052702.21-AIC-IA.MPSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>281.5160522</azimuth>
        <distance>47.71962357</distance>
        <timeResidual>-0.478969574</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052715.96-AIC-IA.SMKI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>279.541748</azimuth>
        <distance>50.09333038</distance>
        <timeResidual>-4.956264496</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052742.81-AIC-MY.KKM..BHZ</pickID>
        <phase>P</phase>
        <azimuth>287.0648499</azimuth>
        <distance>52.99878693</distance>
        <timeResidual>0.3893165588</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052806.81-AIC-TW.YULB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>308.7826843</azimuth>
        <distance>56.46263123</distance>
        <timeResidual>0.1371192932</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052807.66-AIC-TW.NACB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>309.7407837</azimuth>
        <distance>56.67552567</distance>
        <timeResidual>-0.4845237732</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052809.21-AIC-TW.TPUB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>308.2859497</azimuth>
        <distance>56.92297363</distance>
        <timeResidual>-0.7511253357</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052809.66-AIC-TW.SSLB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>308.9664001</azimuth>
        <distance>56.94377899</distance>
        <timeResidual>-0.4192771912</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052810.81-AIC-TW.YHNB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>310.0934753</azimuth>
        <distance>57.12140656</distance>
        <timeResidual>-0.4770774841</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052825.23-AIC-IA.CGJI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>269.1773987</azimuth>
        <distance>60.01370239</distance>
        <timeResidual>-7.458705902</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052908.75-AIC-GT.VNDA.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>181.0693817</azimuth>
        <distance>65.6652298</distance>
        <timeResidual>-0.3683738708</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052909.46-AIC-IU.CASY.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>201.3999329</azimuth>
        <distance>65.80963898</distance>
        <timeResidual>-0.7884788513</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052911.41-AIC-IU.SBA.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>179.9096375</azimuth>
        <distance>65.95626068</distance>
        <timeResidual>0.4199562073</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052921.26-AIC-IA.MNSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>275.8907471</azimuth>
        <distance>67.49155426</distance>
        <timeResidual>-0.6037864685</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052930.06-AIC-IC.ENH.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>309.2368774</azimuth>
        <distance>69.02565765</distance>
        <timeResidual>-0.2879905701</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052936.16-AIC-RM.SLV..BHZ</pickID>
        <phase>P</phase>
        <azimuth>298.3017273</azimuth>
        <distance>69.7289505</distance>
        <timeResidual>1.020847321</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052956.89-AIC-IU.CHTO.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>294.0733032</azimuth>
        <distance>73.19120789</distance>
        <timeResidual>0.7160377502</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.053031.16-AIC-IU.ULN.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>324.0732422</azimuth>
        <distance>79.50131226</distance>
        <timeResidual>0.3379859924</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.053054.31-AIC-IU.COLA.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>17.81279755</azimuth>
        <distance>84.0750885</distance>
        <timeResidual>-0.1105003357</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.053746.90-AIC-HU.TRPA..BHZ</pickID>
        <phase>PKP</phase>
        <azimuth>327.4692993</azimuth>
        <distance>132.8965607</distance>
        <timeResidual>6.915561676</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/MN/AU.ARMA">
        <magnitude>
          <value>4.112530431</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430053830.009.114526</amplitudeID>
        <waveformID networkCode="AU" stationCode="ARMA" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.946661Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/AU.ARMA">
        <magnitude>
          <value>4.919984798</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052327.21-AIC-AU.ARMA..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="ARMA" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.94717Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/MN/AU.CMSA">
        <magnitude>
          <value>4.483943914</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430053830.002206.114525</amplitudeID>
        <waveformID networkCode="AU" stationCode="CMSA" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.947604Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/AU.CMSA">
        <magnitude>
          <value>5.028981335</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052411.24-AIC-AU.CMSA..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="CMSA" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.947989Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/MN/AU.EIDS">
        <magnitude>
          <value>3.759223517</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430053140.209493.114038</amplitudeID>
        <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.948367Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/Mjma/AU.EIDS">
        <magnitude>
          <value>5.049299632</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052259.26-AIC-AU.EIDS..BHZ.Mjma</amplitudeID>
        <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.948669Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/AU.EIDS">
        <magnitude>
          <value>4.528327735</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052259.26-AIC-AU.EIDS..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.948816Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/AU.MEEK">
        <magnitude>
          <value>5.24745287</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052658.56-AIC-AU.MEEK..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="MEEK" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.949217Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/MN/G.SANVU">
        <magnitude>
          <value>4.674040539</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052822.994143.113564</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.949495Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/ML/G.SANVU">
        <magnitude>
          <value>5.415758788</value>
        </magnitude>
        <type>ML</type>
        <amplitudeID>Amplitude/20210430052823.012927.113567</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.949709Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/MLv/G.SANVU">
        <magnitude>
          <value>5.449984308</value>
        </magnitude>
        <type>MLv</type>
        <amplitudeID>20210430.051926.35-AIC-G.SANVU.00.BHZ.MLv</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.949908Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/Mjma/G.SANVU">
        <magnitude>
          <value>5.143388909</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.051926.35-AIC-G.SANVU.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.950033Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/GE.SANI">
        <magnitude>
          <value>5.010198142</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052611.16-AIC-GE.SANI..BHZ.mb</amplitudeID>
        <waveformID networkCode="GE" stationCode="SANI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.950701Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/GT.VNDA">
        <magnitude>
          <value>4.643144771</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052908.75-AIC-GT.VNDA.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="GT" stationCode="VNDA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.95111Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/IA.DBNI">
        <magnitude>
          <value>5.058411352</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052659.61-AIC-IA.DBNI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="DBNI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.951507Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/IA.MPSI">
        <magnitude>
          <value>4.962078972</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052702.21-AIC-IA.MPSI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="MPSI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.952237Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/IA.OBMI">
        <magnitude>
          <value>5.240435491</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052559.83-AIC-IA.OBMI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="OBMI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.952626Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mB/IA.SMKI">
        <magnitude>
          <value>6.913928367</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052715.96-AIC-IA.SMKI..BHZ.mB</amplitudeID>
        <waveformID networkCode="IA" stationCode="SMKI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.953042Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/IA.SMKI">
        <magnitude>
          <value>6.026045864</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052715.96-AIC-IA.SMKI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="SMKI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.953179Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/IA.SMSI">
        <magnitude>
          <value>4.664167699</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052645.86-AIC-IA.SMSI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="SMSI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.953565Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/IC.ENH">
        <magnitude>
          <value>6.183602484</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052930.06-AIC-IC.ENH.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IC" stationCode="ENH" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.953715Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/II.KAPI">
        <magnitude>
          <value>4.577782084</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052657.94-AIC-II.KAPI.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="II" stationCode="KAPI" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.954109Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/II.WRAB">
        <magnitude>
          <value>4.556429357</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052449.69-AIC-II.WRAB.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="II" stationCode="WRAB" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.955886Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/IU.CASY">
        <magnitude>
          <value>4.955848013</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>Amplitude/20210430053131.536021.113922</amplitudeID>
        <waveformID networkCode="IU" stationCode="CASY" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.956339Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mB/IU.CHTO">
        <magnitude>
          <value>4.980533859</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052956.89-AIC-IU.CHTO.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="CHTO" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.956773Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/IU.CHTO">
        <magnitude>
          <value>4.728229033</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052956.89-AIC-IU.CHTO.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="CHTO" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.956919Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/IU.COLA">
        <magnitude>
          <value>4.309112459</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.053054.31-AIC-IU.COLA.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="COLA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.957326Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/MN/IU.CTAO">
        <magnitude>
          <value>3.681676821</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430053158.72278.114063</amplitudeID>
        <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.957709Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/IU.CTAO">
        <magnitude>
          <value>4.177933333</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052313.39-AIC-IU.CTAO.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.958075Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/MN/IU.HNR">
        <magnitude>
          <value>3.19891299</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052823.066898.113574</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.958348Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/ML/IU.HNR">
        <magnitude>
          <value>4.245210838</value>
        </magnitude>
        <type>ML</type>
        <amplitudeID>Amplitude/20210430052823.068657.113575</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BH"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.95854Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/MLv/IU.HNR">
        <magnitude>
          <value>4.510288337</value>
        </magnitude>
        <type>MLv</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.MLv</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.958733Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/Mjma/IU.HNR">
        <magnitude>
          <value>3.853556328</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.958856Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mB/IU.HNR">
        <magnitude>
          <value>4.519725509</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.959Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/IU.HNR">
        <magnitude>
          <value>4.595291925</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.959144Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/IU.SBA">
        <magnitude>
          <value>4.341265053</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052911.41-AIC-IU.SBA.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="SBA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.95955Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/MN/IU.TARA">
        <magnitude>
          <value>4.861375443</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052823.094023.113580</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.959911Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/Mjma/IU.TARA">
        <magnitude>
          <value>5.647636373</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.960219Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mB/IU.TARA">
        <magnitude>
          <value>5.974433087</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.960381Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/IU.TARA">
        <magnitude>
          <value>5.661308269</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.960541Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mB/IU.ULN">
        <magnitude>
          <value>5.807596502</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.053031.16-AIC-IU.ULN.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="ULN" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.960942Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/IU.ULN">
        <magnitude>
          <value>5.548586618</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.053031.16-AIC-IU.ULN.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="ULN" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.961083Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/MY.KKM">
        <magnitude>
          <value>5.186684321</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052742.81-AIC-MY.KKM..BHZ.mb</amplitudeID>
        <waveformID networkCode="MY" stationCode="KKM" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.961473Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/RM.SLV">
        <magnitude>
          <value>4.748224794</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052936.16-AIC-RM.SLV..BHZ.mb</amplitudeID>
        <waveformID networkCode="RM" stationCode="SLV" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.961882Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/TW.NACB">
        <magnitude>
          <value>5.157509503</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052807.66-AIC-TW.NACB..BHZ.mb</amplitudeID>
        <waveformID networkCode="TW" stationCode="NACB" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.962302Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/TW.SSLB">
        <magnitude>
          <value>4.957944013</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052809.66-AIC-TW.SSLB..BHZ.mb</amplitudeID>
        <waveformID networkCode="TW" stationCode="SSLB" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.962684Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/TW.TPUB">
        <magnitude>
          <value>5.461918341</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052809.21-AIC-TW.TPUB..BHZ.mb</amplitudeID>
        <waveformID networkCode="TW" stationCode="TPUB" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.963053Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430060321.912081.87745/staMag/mb/TW.YHNB">
        <magnitude>
          <value>5.247739158</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052810.81-AIC-TW.YHNB..BHZ.mb</amplitudeID>
        <waveformID networkCode="TW" stationCode="YHNB" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.963448Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <magnitude publicID="Origin/20210430060321.912081.87745/netMag/ML">
        <magnitude>
          <value>4.830484813</value>
          <uncertainty>0.8277023931</uncertainty>
        </magnitude>
        <type>ML</type>
        <methodID>mean</methodID>
        <stationCount>2</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.963828Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/ML/G.SANVU</stationMagnitudeID>
          <residual>0.585273975</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/ML/IU.HNR</stationMagnitudeID>
          <residual>-0.585273975</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430060321.912081.87745/netMag/MLv">
        <magnitude>
          <value>4.980136322</value>
          <uncertainty>0.6644653929</uncertainty>
        </magnitude>
        <type>MLv</type>
        <methodID>mean</methodID>
        <stationCount>2</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.963995Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/MLv/G.SANVU</stationMagnitudeID>
          <residual>0.4698479852</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/MLv/IU.HNR</stationMagnitudeID>
          <residual>-0.4698479852</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430060321.912081.87745/netMag/MN">
        <magnitude>
          <value>4.674040539</value>
          <uncertainty>0</uncertainty>
        </magnitude>
        <type>MN</type>
        <methodID>mean</methodID>
        <stationCount>1</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.964048Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/MN/AU.ARMA</stationMagnitudeID>
          <residual>-0.5615101077</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/MN/AU.CMSA</stationMagnitudeID>
          <residual>-0.1900966249</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/MN/AU.EIDS</stationMagnitudeID>
          <residual>-0.9148170219</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/MN/G.SANVU</stationMagnitudeID>
          <residual>0</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/MN/IU.CTAO</stationMagnitudeID>
          <residual>-0.9923637182</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/MN/IU.HNR</stationMagnitudeID>
          <residual>-1.475127548</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/MN/IU.TARA</stationMagnitudeID>
          <residual>0.1873349039</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430060321.912081.87745/netMag/Mjma">
        <magnitude>
          <value>4.981094964</value>
          <uncertainty>0.6666339381</uncertainty>
        </magnitude>
        <type>Mjma</type>
        <methodID>trimmed mean(25)</methodID>
        <stationCount>4</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.964142Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/Mjma/AU.EIDS</stationMagnitudeID>
          <residual>0.06820466787</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/Mjma/G.SANVU</stationMagnitudeID>
          <residual>0.1622939455</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/Mjma/IU.HNR</stationMagnitudeID>
          <residual>-1.127538636</residual>
          <weight>0.5</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/Mjma/IU.TARA</stationMagnitudeID>
          <residual>0.6665414095</residual>
          <weight>0.5</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430060321.912081.87745/netMag/mB">
        <magnitude>
          <value>5.613382307</value>
          <uncertainty>0.7749225052</uncertainty>
        </magnitude>
        <type>mB</type>
        <methodID>trimmed mean(25)</methodID>
        <stationCount>5</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.964214Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mB/IA.SMKI</stationMagnitudeID>
          <residual>1.30054606</residual>
          <weight>0.375</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mB/IU.CHTO</stationMagnitudeID>
          <residual>-0.6328484483</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mB/IU.HNR</stationMagnitudeID>
          <residual>-1.093656798</residual>
          <weight>0.375</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mB/IU.TARA</stationMagnitudeID>
          <residual>0.3610507799</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mB/IU.ULN</stationMagnitudeID>
          <residual>0.1942141951</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430060321.912081.87745/netMag/Mw(mB)">
        <magnitude>
          <value>5.117396999</value>
          <uncertainty>0.7749225052</uncertainty>
        </magnitude>
        <type>Mw(mB)</type>
        <stationCount>5</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.964319Z</creationTime>
        </creationInfo>
      </magnitude>
      <magnitude publicID="Origin/20210430060321.912081.87745/netMag/mb">
        <magnitude>
          <value>4.951757769</value>
          <uncertainty>0.2827002089</uncertainty>
        </magnitude>
        <type>mb</type>
        <methodID>trimmed mean(25)</methodID>
        <stationCount>22</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.964345Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/AU.ARMA</stationMagnitudeID>
          <residual>-0.03177297105</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/AU.CMSA</stationMagnitudeID>
          <residual>0.07722356623</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/AU.EIDS</stationMagnitudeID>
          <residual>-0.4234300342</residual>
          <weight>0.5</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/AU.MEEK</stationMagnitudeID>
          <residual>0.2956951014</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/GE.SANI</stationMagnitudeID>
          <residual>0.05844037344</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/GT.VNDA</stationMagnitudeID>
          <residual>-0.3086129983</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/IA.DBNI</stationMagnitudeID>
          <residual>0.106653583</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/IA.MPSI</stationMagnitudeID>
          <residual>0.0103212029</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/IA.OBMI</stationMagnitudeID>
          <residual>0.2886777217</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/IA.SMKI</stationMagnitudeID>
          <residual>1.074288095</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/IA.SMSI</stationMagnitudeID>
          <residual>-0.28759007</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/IC.ENH</stationMagnitudeID>
          <residual>1.231844715</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/II.KAPI</stationMagnitudeID>
          <residual>-0.3739756855</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/II.WRAB</stationMagnitudeID>
          <residual>-0.3953284117</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/IU.CASY</stationMagnitudeID>
          <residual>0.00409024373</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/IU.CHTO</stationMagnitudeID>
          <residual>-0.2235287359</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/IU.COLA</stationMagnitudeID>
          <residual>-0.6426453098</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/IU.CTAO</stationMagnitudeID>
          <residual>-0.7738244362</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/IU.HNR</stationMagnitudeID>
          <residual>-0.3564658439</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/IU.SBA</stationMagnitudeID>
          <residual>-0.6104927161</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/IU.TARA</stationMagnitudeID>
          <residual>0.7095505005</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/IU.ULN</stationMagnitudeID>
          <residual>0.5968288488</residual>
          <weight>0.5</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/MY.KKM</stationMagnitudeID>
          <residual>0.2349265521</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/RM.SLV</stationMagnitudeID>
          <residual>-0.2035329753</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/TW.NACB</stationMagnitudeID>
          <residual>0.2057517337</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/TW.SSLB</stationMagnitudeID>
          <residual>0.006186244359</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/TW.TPUB</stationMagnitudeID>
          <residual>0.5101605724</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430060321.912081.87745/staMag/mb/TW.YHNB</stationMagnitudeID>
          <residual>0.2959813894</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430060321.912081.87745/netMag/M">
        <magnitude>
          <value>4.930721104</value>
        </magnitude>
        <type>M</type>
        <methodID>weighted average</methodID>
        <stationCount>22</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T06:03:21.964675Z</creationTime>
        </creationInfo>
      </magnitude>
    </origin>
    <origin publicID="Origin/20210430094923.406014.99645">
      <time>
        <value>2021-04-30T05:18:32.764684Z</value>
        <uncertainty>0.6348486743</uncertainty>
      </time>
      <latitude>
        <value>-11.89557266</value>
        <uncertainty>3.830244077</uncertainty>
      </latitude>
      <longitude>
        <value>166.3815613</value>
        <uncertainty>5.041867255</uncertainty>
      </longitude>
      <depth>
        <value>71.3624649</value>
        <uncertainty>6.194593891</uncertainty>
      </depth>
      <depthType>from location</depthType>
      <methodID>LOCSAT</methodID>
      <earthModelID>iasp91</earthModelID>
      <quality>
        <associatedPhaseCount>35</associatedPhaseCount>
        <usedPhaseCount>32</usedPhaseCount>
        <associatedStationCount>35</associatedStationCount>
        <usedStationCount>32</usedStationCount>
        <standardError>1.640247658</standardError>
        <azimuthalGap>140.8696346</azimuthalGap>
        <maximumDistance>84.07239532</maximumDistance>
        <minimumDistance>3.640174627</minimumDistance>
        <medianDistance>48.92266464</medianDistance>
      </quality>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>autoloc</author>
        <creationTime>2021-04-30T09:49:23.406262Z</creationTime>
      </creationInfo>
      <arrival>
        <pickID>20210430.051926.35-AIC-G.SANVU.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>167.4255829</azimuth>
        <distance>3.640174627</distance>
        <timeResidual>-0.08920669556</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052008.06-AIC-IU.HNR.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>290.6165466</azimuth>
        <distance>6.782827854</distance>
        <timeResidual>-1.577148438</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052157.79-AIC-IU.TARA.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>26.55594826</azimuth>
        <distance>14.75833893</distance>
        <timeResidual>1.045612335</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052250.71-AIC-GE.PMG.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>275.6318054</azimuth>
        <distance>19.05451584</distance>
        <timeResidual>1.215900421</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052259.26-AIC-AU.EIDS..BHZ</pickID>
        <phase>P</phase>
        <azimuth>224.8594666</azimuth>
        <distance>19.75589371</distance>
        <timeResidual>2.690799713</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052313.39-AIC-IU.CTAO.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>244.4740601</azimuth>
        <distance>20.98517036</distance>
        <timeResidual>3.121982574</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052327.21-AIC-AU.ARMA..BHZ</pickID>
        <phase>P</phase>
        <azimuth>214.1650543</azimuth>
        <distance>23.01807976</distance>
        <timeResidual>-3.961406708</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052400.85-AIC-G.CAN.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>211.0344238</azimuth>
        <distance>28.22224236</distance>
        <timeResidual>-17.39252853</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052411.24-AIC-AU.CMSA..BHZ</pickID>
        <phase>P</phase>
        <azimuth>220.9129181</azimuth>
        <distance>27.3740139</distance>
        <timeResidual>0.3253974915</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052449.69-AIC-II.WRAB.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>251.2390747</azimuth>
        <distance>31.76532555</distance>
        <timeResidual>-0.7775993347</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052559.83-AIC-IA.OBMI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>282.4256897</azimuth>
        <distance>39.83525467</distance>
        <timeResidual>0.4787483215</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052611.16-AIC-GE.SANI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>280.6593628</azimuth>
        <distance>41.22286987</distance>
        <timeResidual>0.3310432434</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052645.86-AIC-IA.SMSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>283.3672791</azimuth>
        <distance>45.56983948</distance>
        <timeResidual>0.0848274231</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052657.94-AIC-II.KAPI.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>274.3813171</azimuth>
        <distance>46.57485962</distance>
        <timeResidual>4.093647003</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052658.56-AIC-AU.MEEK..BHZ</pickID>
        <phase>P</phase>
        <azimuth>244.5559845</azimuth>
        <distance>47.13100815</distance>
        <timeResidual>0.4816169739</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052659.61-AIC-IA.DBNI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>269.3428345</azimuth>
        <distance>47.37724686</distance>
        <timeResidual>-0.5533561707</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052702.21-AIC-IA.MPSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>281.5118408</azimuth>
        <distance>47.7358017</distance>
        <timeResidual>-0.5483512878</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052715.96-AIC-IA.SMKI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>279.5380859</azimuth>
        <distance>50.10952759</distance>
        <timeResidual>-5.02186203</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052742.81-AIC-MY.KKM..BHZ</pickID>
        <phase>P</phase>
        <azimuth>287.0596008</azimuth>
        <distance>53.01480103</distance>
        <timeResidual>0.3299446106</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052806.81-AIC-TW.YULB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>308.7738342</azimuth>
        <distance>56.4765892</distance>
        <timeResidual>0.09776687622</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052807.66-AIC-TW.NACB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>309.731842</azimuth>
        <distance>56.68934631</distance>
        <timeResidual>-0.5224723816</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052809.21-AIC-TW.TPUB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>308.2772827</azimuth>
        <distance>56.93700409</distance>
        <timeResidual>-0.7902336121</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052809.66-AIC-TW.SSLB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>308.9576416</azimuth>
        <distance>56.95771027</distance>
        <timeResidual>-0.4577140808</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052810.81-AIC-TW.YHNB..BHZ</pickID>
        <phase>P</phase>
        <azimuth>310.0845642</azimuth>
        <distance>57.1351738</distance>
        <timeResidual>-0.51404953</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052825.23-AIC-IA.CGJI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>269.1755066</azimuth>
        <distance>60.02970123</distance>
        <timeResidual>-7.506481171</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052908.75-AIC-GT.VNDA.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>181.0732727</azimuth>
        <distance>65.66320038</distance>
        <timeResidual>-0.2906608582</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052909.46-AIC-IU.CASY.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>201.4036407</azimuth>
        <distance>65.81332397</distance>
        <timeResidual>-0.7478141785</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052911.41-AIC-IU.SBA.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>179.9134216</azimuth>
        <distance>65.9539032</distance>
        <timeResidual>0.4998054504</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052921.26-AIC-IA.MNSI..BHZ</pickID>
        <phase>P</phase>
        <azimuth>275.8876648</azimuth>
        <distance>67.50775146</distance>
        <timeResidual>-0.641002655</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052930.06-AIC-IC.ENH.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>309.2302856</azimuth>
        <distance>69.03955078</distance>
        <timeResidual>-0.3087272644</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052936.16-AIC-RM.SLV..BHZ</pickID>
        <phase>P</phase>
        <azimuth>298.2962952</azimuth>
        <distance>69.74417114</distance>
        <timeResidual>0.9930305481</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.052956.89-AIC-IU.CHTO.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>294.0686035</azimuth>
        <distance>73.20680237</distance>
        <timeResidual>0.6912727356</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.053031.16-AIC-IU.ULN.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>324.0677185</azimuth>
        <distance>79.51261139</distance>
        <timeResidual>0.3461799622</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.053054.31-AIC-IU.COLA.00.BHZ</pickID>
        <phase>P</phase>
        <azimuth>17.80776215</azimuth>
        <distance>84.07239532</distance>
        <timeResidual>-0.02534103394</timeResidual>
        <timeUsed>true</timeUsed>
        <weight>1</weight>
      </arrival>
      <arrival>
        <pickID>20210430.053746.90-AIC-HU.TRPA..BHZ</pickID>
        <phase>PKP</phase>
        <azimuth>327.4773254</azimuth>
        <distance>132.907135</distance>
        <timeResidual>6.975452423</timeResidual>
        <timeUsed>false</timeUsed>
        <weight>0</weight>
      </arrival>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/MN/AU.ARMA">
        <magnitude>
          <value>4.112754493</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430053830.009.114526</amplitudeID>
        <waveformID networkCode="AU" stationCode="ARMA" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.540943Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/AU.ARMA">
        <magnitude>
          <value>4.888274011</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052327.21-AIC-AU.ARMA..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="ARMA" channelCode="BHE"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.541485Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/MN/AU.CMSA">
        <magnitude>
          <value>4.484176084</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430053830.002206.114525</amplitudeID>
        <waveformID networkCode="AU" stationCode="CMSA" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.541983Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/AU.CMSA">
        <magnitude>
          <value>5.02850777</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052411.24-AIC-AU.CMSA..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="CMSA" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.542377Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/MN/AU.EIDS">
        <magnitude>
          <value>3.759578749</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430053140.209493.114038</amplitudeID>
        <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.542783Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/Mjma/AU.EIDS">
        <magnitude>
          <value>5.04966698</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052259.26-AIC-AU.EIDS..BHZ.Mjma</amplitudeID>
        <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.543081Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/AU.EIDS">
        <magnitude>
          <value>4.526104482</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052259.26-AIC-AU.EIDS..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="EIDS" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.543223Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/AU.MEEK">
        <magnitude>
          <value>5.246136261</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052658.56-AIC-AU.MEEK..BHZ.mb</amplitudeID>
        <waveformID networkCode="AU" stationCode="MEEK" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.54364Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/MN/G.SANVU">
        <magnitude>
          <value>4.672885638</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052822.994143.113564</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.5439Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/ML/G.SANVU">
        <magnitude>
          <value>5.414309754</value>
        </magnitude>
        <type>ML</type>
        <amplitudeID>Amplitude/20210430052823.012927.113567</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.544109Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/MLv/G.SANVU">
        <magnitude>
          <value>5.675439593</value>
        </magnitude>
        <type>MLv</type>
        <amplitudeID>20210430.051926.35-AIC-G.SANVU.00.BHZ.MLv</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHE"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.544342Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/Mjma/G.SANVU">
        <magnitude>
          <value>5.142193776</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.051926.35-AIC-G.SANVU.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.544452Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/GE.PMG">
        <magnitude>
          <value>4.541497175</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052250.71-AIC-GE.PMG.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="GE" stationCode="PMG" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.545321Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/GE.SANI">
        <magnitude>
          <value>5.009405964</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052611.16-AIC-GE.SANI..BHZ.mb</amplitudeID>
        <waveformID networkCode="GE" stationCode="SANI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.545734Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/GT.VNDA">
        <magnitude>
          <value>4.641574771</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052908.75-AIC-GT.VNDA.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="GT" stationCode="VNDA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.546144Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/IA.DBNI">
        <magnitude>
          <value>5.057008179</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052659.61-AIC-IA.DBNI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="DBNI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.54658Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/IA.MPSI">
        <magnitude>
          <value>4.733702728</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052702.21-AIC-IA.MPSI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="MPSI" channelCode="BHN"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.547359Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/IA.OBMI">
        <magnitude>
          <value>5.239025161</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052559.83-AIC-IA.OBMI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="OBMI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.547757Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mB/IA.SMKI">
        <magnitude>
          <value>6.913338871</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052715.96-AIC-IA.SMKI..BHZ.mB</amplitudeID>
        <waveformID networkCode="IA" stationCode="SMKI" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.548141Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/IA.SMKI">
        <magnitude>
          <value>6.435716653</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052715.96-AIC-IA.SMKI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="SMKI" channelCode="BHN"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.548274Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/IA.SMSI">
        <magnitude>
          <value>4.592547057</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052645.86-AIC-IA.SMSI..BHZ.mb</amplitudeID>
        <waveformID networkCode="IA" stationCode="SMSI" channelCode="BHE"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.548667Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/IC.ENH">
        <magnitude>
          <value>6.181722007</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052930.06-AIC-IC.ENH.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IC" stationCode="ENH" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.549037Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/II.KAPI">
        <magnitude>
          <value>4.576596257</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052657.94-AIC-II.KAPI.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="II" stationCode="KAPI" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.549427Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/II.WRAB">
        <magnitude>
          <value>4.308298083</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052449.69-AIC-II.WRAB.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="II" stationCode="WRAB" locationCode="00" channelCode="BH2"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.5498Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/IU.CASY">
        <magnitude>
          <value>4.954154294</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>Amplitude/20210430053131.536021.113922</amplitudeID>
        <waveformID networkCode="IU" stationCode="CASY" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.550268Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mB/IU.CHTO">
        <magnitude>
          <value>4.979352894</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052956.89-AIC-IU.CHTO.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="CHTO" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.576112Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/IU.CHTO">
        <magnitude>
          <value>4.727048068</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052956.89-AIC-IU.CHTO.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="CHTO" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.576281Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/IU.COLA">
        <magnitude>
          <value>4.307686386</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.053054.31-AIC-IU.COLA.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="COLA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.576709Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/MN/IU.CTAO">
        <magnitude>
          <value>3.68214089</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430053158.72278.114063</amplitudeID>
        <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.577112Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/IU.CTAO">
        <magnitude>
          <value>4.17700662</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052313.39-AIC-IU.CTAO.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.577513Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/MN/IU.HNR">
        <magnitude>
          <value>3.200599027</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052823.066898.113574</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.577772Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/ML/IU.HNR">
        <magnitude>
          <value>4.249168648</value>
        </magnitude>
        <type>ML</type>
        <amplitudeID>Amplitude/20210430052823.068657.113575</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BH"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.577962Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/MLv/IU.HNR">
        <magnitude>
          <value>4.626580609</value>
        </magnitude>
        <type>MLv</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.MLv</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BH1"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.578165Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/Mjma/IU.HNR">
        <magnitude>
          <value>3.855311649</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.578285Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mB/IU.HNR">
        <magnitude>
          <value>4.430659052</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BH1"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.578469Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/IU.HNR">
        <magnitude>
          <value>4.591412293</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052008.06-AIC-IU.HNR.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="HNR" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.578609Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/IU.SBA">
        <magnitude>
          <value>4.339420726</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052911.41-AIC-IU.SBA.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="SBA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.578999Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/MN/IU.TARA">
        <magnitude>
          <value>4.861122646</value>
        </magnitude>
        <type>MN</type>
        <amplitudeID>Amplitude/20210430052823.094023.113580</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>false</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.579358Z</creationTime>
          <version>0.2.0</version>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/Mjma/IU.TARA">
        <magnitude>
          <value>5.647377062</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.579646Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mB/IU.TARA">
        <magnitude>
          <value>5.972654767</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.579787Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/IU.TARA">
        <magnitude>
          <value>5.65952995</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052157.79-AIC-IU.TARA.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.579919Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mB/IU.ULN">
        <magnitude>
          <value>5.806656275</value>
        </magnitude>
        <type>mB</type>
        <amplitudeID>20210430.053031.16-AIC-IU.ULN.00.BHZ.mB</amplitudeID>
        <waveformID networkCode="IU" stationCode="ULN" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.58032Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/IU.ULN">
        <magnitude>
          <value>5.54764639</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.053031.16-AIC-IU.ULN.00.BHZ.mb</amplitudeID>
        <waveformID networkCode="IU" stationCode="ULN" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.58046Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/MY.KKM">
        <magnitude>
          <value>5.186186449</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052742.81-AIC-MY.KKM..BHZ.mb</amplitudeID>
        <waveformID networkCode="MY" stationCode="KKM" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.580847Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/RM.SLV">
        <magnitude>
          <value>4.396797861</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052936.16-AIC-RM.SLV..BHZ.mb</amplitudeID>
        <waveformID networkCode="RM" stationCode="SLV" channelCode="BHN"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.581244Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/TW.NACB">
        <magnitude>
          <value>5.15654036</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052807.66-AIC-TW.NACB..BHZ.mb</amplitudeID>
        <waveformID networkCode="TW" stationCode="NACB" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.581652Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/TW.SSLB">
        <magnitude>
          <value>4.957101973</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052809.66-AIC-TW.SSLB..BHZ.mb</amplitudeID>
        <waveformID networkCode="TW" stationCode="SSLB" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.582047Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/TW.TPUB">
        <magnitude>
          <value>5.461067407</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052809.21-AIC-TW.TPUB..BHZ.mb</amplitudeID>
        <waveformID networkCode="TW" stationCode="TPUB" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.582437Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/mb/TW.YHNB">
        <magnitude>
          <value>5.024897309</value>
        </magnitude>
        <type>mb</type>
        <amplitudeID>20210430.052810.81-AIC-TW.YHNB..BHZ.mb</amplitudeID>
        <waveformID networkCode="TW" stationCode="YHNB" channelCode="BHE"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.582825Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <stationMagnitude publicID="Origin/20210430094923.406014.99645/staMag/Mjma/GE.PMG">
        <magnitude>
          <value>5.031374051</value>
        </magnitude>
        <type>Mjma</type>
        <amplitudeID>20210430.052250.71-AIC-GE.PMG.00.BHZ.Mjma</amplitudeID>
        <waveformID networkCode="GE" stationCode="PMG" locationCode="00" channelCode="BHZ"/>
        <passedQC>true</passedQC>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:47.460988Z</creationTime>
        </creationInfo>
      </stationMagnitude>
      <magnitude publicID="Origin/20210430094923.406014.99645/netMag/ML">
        <magnitude>
          <value>4.831739201</value>
          <uncertainty>0.8238791768</uncertainty>
        </magnitude>
        <type>ML</type>
        <methodID>mean</methodID>
        <stationCount>2</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.583218Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/ML/G.SANVU</stationMagnitudeID>
          <residual>0.5825705528</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/ML/IU.HNR</stationMagnitudeID>
          <residual>-0.5825705528</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430094923.406014.99645/netMag/MLv">
        <magnitude>
          <value>5.151010101</value>
          <uncertainty>0.7416553001</uncertainty>
        </magnitude>
        <type>MLv</type>
        <methodID>mean</methodID>
        <stationCount>2</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.583356Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/MLv/G.SANVU</stationMagnitudeID>
          <residual>0.524429492</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/MLv/IU.HNR</stationMagnitudeID>
          <residual>-0.524429492</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430094923.406014.99645/netMag/MN">
        <magnitude>
          <value>4.672885638</value>
          <uncertainty>0</uncertainty>
        </magnitude>
        <type>MN</type>
        <methodID>mean</methodID>
        <stationCount>1</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.583408Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/MN/AU.ARMA</stationMagnitudeID>
          <residual>-0.5601311451</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/MN/AU.CMSA</stationMagnitudeID>
          <residual>-0.1887095539</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/MN/AU.EIDS</stationMagnitudeID>
          <residual>-0.9133068892</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/MN/G.SANVU</stationMagnitudeID>
          <residual>0</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/MN/IU.CTAO</stationMagnitudeID>
          <residual>-0.9907447484</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/MN/IU.HNR</stationMagnitudeID>
          <residual>-1.472286612</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/MN/IU.TARA</stationMagnitudeID>
          <residual>0.1882370079</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430094923.406014.99645/netMag/Mjma">
        <magnitude>
          <value>5.009798153</value>
          <uncertainty>0.4942720548</uncertainty>
        </magnitude>
        <type>Mjma</type>
        <methodID>trimmed mean(25)</methodID>
        <stationCount>5</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.583481Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/Mjma/AU.EIDS</stationMagnitudeID>
          <residual>0.03986882701</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/Mjma/G.SANVU</stationMagnitudeID>
          <residual>0.1323956233</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/Mjma/GE.PMG</stationMagnitudeID>
          <residual>0.02157589802</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/Mjma/IU.HNR</stationMagnitudeID>
          <residual>-1.154486504</residual>
          <weight>0.375</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/Mjma/IU.TARA</stationMagnitudeID>
          <residual>0.6375789088</residual>
          <weight>0.375</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430094923.406014.99645/netMag/mB">
        <magnitude>
          <value>5.603376842</value>
          <uncertainty>0.792308463</uncertainty>
        </magnitude>
        <type>mB</type>
        <methodID>trimmed mean(25)</methodID>
        <stationCount>5</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.583537Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mB/IA.SMKI</stationMagnitudeID>
          <residual>1.309962029</residual>
          <weight>0.375</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mB/IU.CHTO</stationMagnitudeID>
          <residual>-0.6240239481</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mB/IU.HNR</stationMagnitudeID>
          <residual>-1.17271779</residual>
          <weight>0.375</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mB/IU.TARA</stationMagnitudeID>
          <residual>0.3692779255</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mB/IU.ULN</stationMagnitudeID>
          <residual>0.203279433</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430094923.406014.99645/netMag/Mw(mB)">
        <magnitude>
          <value>5.104389894</value>
          <uncertainty>0.792308463</uncertainty>
        </magnitude>
        <type>Mw(mB)</type>
        <stationCount>5</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.583604Z</creationTime>
        </creationInfo>
      </magnitude>
      <magnitude publicID="Origin/20210430094923.406014.99645/netMag/mb">
        <magnitude>
          <value>4.884746483</value>
          <uncertainty>0.3091091184</uncertainty>
        </magnitude>
        <type>mb</type>
        <methodID>trimmed mean(25)</methodID>
        <stationCount>23</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.583624Z</creationTime>
        </creationInfo>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/AU.ARMA</stationMagnitudeID>
          <residual>0.00352752793</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/AU.CMSA</stationMagnitudeID>
          <residual>0.1437612876</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/AU.EIDS</stationMagnitudeID>
          <residual>-0.3586420007</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/AU.MEEK</stationMagnitudeID>
          <residual>0.3613897786</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/GE.PMG</stationMagnitudeID>
          <residual>-0.3432493073</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/GE.SANI</stationMagnitudeID>
          <residual>0.1246594808</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/GT.VNDA</stationMagnitudeID>
          <residual>-0.2431717112</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/IA.DBNI</stationMagnitudeID>
          <residual>0.1722616959</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/IA.MPSI</stationMagnitudeID>
          <residual>-0.1510437545</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/IA.OBMI</stationMagnitudeID>
          <residual>0.3542786781</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/IA.SMKI</stationMagnitudeID>
          <residual>1.550970171</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/IA.SMSI</stationMagnitudeID>
          <residual>-0.2921994254</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/IC.ENH</stationMagnitudeID>
          <residual>1.296975524</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/II.KAPI</stationMagnitudeID>
          <residual>-0.3081502256</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/II.WRAB</stationMagnitudeID>
          <residual>-0.5764483994</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/IU.CASY</stationMagnitudeID>
          <residual>0.06940781095</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/IU.CHTO</stationMagnitudeID>
          <residual>-0.1576984146</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/IU.COLA</stationMagnitudeID>
          <residual>-0.5770600969</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/IU.CTAO</stationMagnitudeID>
          <residual>-0.7077398626</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/IU.HNR</stationMagnitudeID>
          <residual>-0.2933341901</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/IU.SBA</stationMagnitudeID>
          <residual>-0.5453257565</residual>
          <weight>0.375</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/IU.TARA</stationMagnitudeID>
          <residual>0.7747834671</residual>
          <weight>0</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/IU.ULN</stationMagnitudeID>
          <residual>0.6628999077</residual>
          <weight>0.375</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/MY.KKM</stationMagnitudeID>
          <residual>0.3014399667</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/RM.SLV</stationMagnitudeID>
          <residual>-0.4879486221</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/TW.NACB</stationMagnitudeID>
          <residual>0.2717938775</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/TW.SSLB</stationMagnitudeID>
          <residual>0.07235549027</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/TW.TPUB</stationMagnitudeID>
          <residual>0.5763209244</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
        <stationMagnitudeContribution>
          <stationMagnitudeID>Origin/20210430094923.406014.99645/staMag/mb/TW.YHNB</stationMagnitudeID>
          <residual>0.1401508258</residual>
          <weight>1</weight>
        </stationMagnitudeContribution>
      </magnitude>
      <magnitude publicID="Origin/20210430094923.406014.99645/netMag/M">
        <magnitude>
          <value>4.972225653</value>
        </magnitude>
        <type>M</type>
        <methodID>weighted average</methodID>
        <stationCount>23</stationCount>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scmag</author>
          <creationTime>2021-04-30T09:49:23.583944Z</creationTime>
          <modificationTime>2021-04-30T09:49:47.46116Z</modificationTime>
        </creationInfo>
      </magnitude>
    </origin>
    <origin publicID="Origin/20210430054646.973859.613244">
      <time>
        <value>2021-04-30T05:18:32.698118Z</value>
      </time>
      <latitude>
        <value>-11.89444256</value>
      </latitude>
      <longitude>
        <value>166.3646393</value>
      </longitude>
      <depth>
        <value>31</value>
      </depth>
      <methodID>FocalMechanism</methodID>
      <earthModelID>qseis_ak135f_continental</earthModelID>
      <quality>
        <usedPhaseCount>8</usedPhaseCount>
        <associatedStationCount>23</associatedStationCount>
        <usedStationCount>7</usedStationCount>
        <azimuthalGap>142.1660848</azimuthalGap>
        <maximumDistance>28.13891734</maximumDistance>
        <minimumDistance>3.624386503</minimumDistance>
      </quality>
      <type>hypocenter</type>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T05:46:46.973516Z</creationTime>
      </creationInfo>
      <magnitude publicID="Magnitude/20210430054646.973972.613245">
        <magnitude>
          <value>5.054353539</value>
        </magnitude>
        <type>Mw</type>
        <methodID>MT</methodID>
        <stationCount>7</stationCount>
        <azimuthalGap>142.1660848</azimuthalGap>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T05:46:46.973516Z</creationTime>
        </creationInfo>
      </magnitude>
    </origin>
    <origin publicID="Origin/20210430054719.938975.613259">
      <time>
        <value>2021-04-30T05:18:32.698118Z</value>
      </time>
      <latitude>
        <value>-11.89444256</value>
      </latitude>
      <longitude>
        <value>166.3646393</value>
      </longitude>
      <depth>
        <value>31</value>
      </depth>
      <methodID>FocalMechanism</methodID>
      <earthModelID>qseis_ak135f_continental</earthModelID>
      <quality>
        <usedPhaseCount>8</usedPhaseCount>
        <associatedStationCount>25</associatedStationCount>
        <usedStationCount>7</usedStationCount>
        <azimuthalGap>142.1660848</azimuthalGap>
        <maximumDistance>28.13891734</maximumDistance>
        <minimumDistance>3.624386503</minimumDistance>
      </quality>
      <type>hypocenter</type>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T05:47:19.938545Z</creationTime>
      </creationInfo>
      <magnitude publicID="Magnitude/20210430054719.939079.613260">
        <magnitude>
          <value>5.05462205</value>
        </magnitude>
        <type>Mw</type>
        <methodID>MT</methodID>
        <stationCount>7</stationCount>
        <azimuthalGap>142.1660848</azimuthalGap>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T05:47:19.938545Z</creationTime>
        </creationInfo>
      </magnitude>
    </origin>
    <origin publicID="Origin/20210430054758.637903.613272">
      <time>
        <value>2021-04-30T05:18:32.698118Z</value>
      </time>
      <latitude>
        <value>-11.89444256</value>
      </latitude>
      <longitude>
        <value>166.3646393</value>
      </longitude>
      <depth>
        <value>31</value>
      </depth>
      <methodID>FocalMechanism</methodID>
      <earthModelID>qseis_ak135f_continental</earthModelID>
      <quality>
        <usedPhaseCount>9</usedPhaseCount>
        <associatedStationCount>26</associatedStationCount>
        <usedStationCount>8</usedStationCount>
        <azimuthalGap>142.1660848</azimuthalGap>
        <maximumDistance>29.96626382</maximumDistance>
        <minimumDistance>3.624386503</minimumDistance>
      </quality>
      <type>hypocenter</type>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T05:47:58.637516Z</creationTime>
      </creationInfo>
      <magnitude publicID="Magnitude/20210430054758.638028.613273">
        <magnitude>
          <value>5.055288022</value>
        </magnitude>
        <type>Mw</type>
        <methodID>MT</methodID>
        <stationCount>8</stationCount>
        <azimuthalGap>142.1660848</azimuthalGap>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T05:47:58.637516Z</creationTime>
        </creationInfo>
      </magnitude>
    </origin>
    <origin publicID="Origin/20210430054838.925868.613372">
      <time>
        <value>2021-04-30T05:18:32.698118Z</value>
      </time>
      <latitude>
        <value>-11.89444256</value>
      </latitude>
      <longitude>
        <value>166.3646393</value>
      </longitude>
      <depth>
        <value>31</value>
      </depth>
      <methodID>FocalMechanism</methodID>
      <earthModelID>qseis_ak135f_continental</earthModelID>
      <quality>
        <usedPhaseCount>8</usedPhaseCount>
        <associatedStationCount>27</associatedStationCount>
        <usedStationCount>7</usedStationCount>
        <azimuthalGap>142.1660848</azimuthalGap>
        <maximumDistance>28.13891734</maximumDistance>
        <minimumDistance>3.624386503</minimumDistance>
      </quality>
      <type>hypocenter</type>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T05:48:38.925529Z</creationTime>
      </creationInfo>
      <magnitude publicID="Magnitude/20210430054838.925934.613373">
        <magnitude>
          <value>5.0549905</value>
        </magnitude>
        <type>Mw</type>
        <methodID>MT</methodID>
        <stationCount>7</stationCount>
        <azimuthalGap>142.1660848</azimuthalGap>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T05:48:38.925529Z</creationTime>
        </creationInfo>
      </magnitude>
    </origin>
    <origin publicID="Origin/20210430054919.510829.613513">
      <time>
        <value>2021-04-30T05:18:32.698118Z</value>
      </time>
      <latitude>
        <value>-11.89444256</value>
      </latitude>
      <longitude>
        <value>166.3646393</value>
      </longitude>
      <depth>
        <value>31</value>
      </depth>
      <methodID>FocalMechanism</methodID>
      <earthModelID>qseis_ak135f_continental</earthModelID>
      <quality>
        <usedPhaseCount>8</usedPhaseCount>
        <associatedStationCount>29</associatedStationCount>
        <usedStationCount>7</usedStationCount>
        <azimuthalGap>142.1660848</azimuthalGap>
        <maximumDistance>28.13891734</maximumDistance>
        <minimumDistance>3.624386503</minimumDistance>
      </quality>
      <type>hypocenter</type>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T05:49:19.510573Z</creationTime>
      </creationInfo>
      <magnitude publicID="Magnitude/20210430054919.510879.613514">
        <magnitude>
          <value>5.0549905</value>
        </magnitude>
        <type>Mw</type>
        <methodID>MT</methodID>
        <stationCount>7</stationCount>
        <azimuthalGap>142.1660848</azimuthalGap>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T05:49:19.510573Z</creationTime>
        </creationInfo>
      </magnitude>
    </origin>
    <origin publicID="Origin/20210430055022.509769.613624">
      <time>
        <value>2021-04-30T05:18:32.698118Z</value>
      </time>
      <latitude>
        <value>-11.89444256</value>
      </latitude>
      <longitude>
        <value>166.3646393</value>
      </longitude>
      <depth>
        <value>31</value>
      </depth>
      <methodID>FocalMechanism</methodID>
      <earthModelID>qseis_ak135f_continental</earthModelID>
      <quality>
        <usedPhaseCount>8</usedPhaseCount>
        <associatedStationCount>30</associatedStationCount>
        <usedStationCount>7</usedStationCount>
        <azimuthalGap>142.1660848</azimuthalGap>
        <maximumDistance>28.13891734</maximumDistance>
        <minimumDistance>3.624386503</minimumDistance>
      </quality>
      <type>hypocenter</type>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T05:50:22.509494Z</creationTime>
      </creationInfo>
      <magnitude publicID="Magnitude/20210430055022.509822.613625">
        <magnitude>
          <value>5.0549905</value>
        </magnitude>
        <type>Mw</type>
        <methodID>MT</methodID>
        <stationCount>7</stationCount>
        <azimuthalGap>142.1660848</azimuthalGap>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T05:50:22.509494Z</creationTime>
        </creationInfo>
      </magnitude>
    </origin>
    <origin publicID="Origin/20210430055105.175114.613661">
      <time>
        <value>2021-04-30T05:18:32.698118Z</value>
      </time>
      <latitude>
        <value>-11.89444256</value>
      </latitude>
      <longitude>
        <value>166.3646393</value>
      </longitude>
      <depth>
        <value>31</value>
      </depth>
      <methodID>FocalMechanism</methodID>
      <earthModelID>qseis_ak135f_continental</earthModelID>
      <quality>
        <usedPhaseCount>8</usedPhaseCount>
        <associatedStationCount>32</associatedStationCount>
        <usedStationCount>7</usedStationCount>
        <azimuthalGap>142.1660848</azimuthalGap>
        <maximumDistance>28.13891734</maximumDistance>
        <minimumDistance>3.624386503</minimumDistance>
      </quality>
      <type>hypocenter</type>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T05:51:05.174557Z</creationTime>
      </creationInfo>
      <magnitude publicID="Magnitude/20210430055105.175223.613662">
        <magnitude>
          <value>5.0549905</value>
        </magnitude>
        <type>Mw</type>
        <methodID>MT</methodID>
        <stationCount>7</stationCount>
        <azimuthalGap>142.1660848</azimuthalGap>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T05:51:05.174557Z</creationTime>
        </creationInfo>
      </magnitude>
    </origin>
    <origin publicID="Origin/20210430055144.024927.613677">
      <time>
        <value>2021-04-30T05:18:32.698118Z</value>
      </time>
      <latitude>
        <value>-11.89444256</value>
      </latitude>
      <longitude>
        <value>166.3646393</value>
      </longitude>
      <depth>
        <value>31</value>
      </depth>
      <methodID>FocalMechanism</methodID>
      <earthModelID>qseis_ak135f_continental</earthModelID>
      <quality>
        <usedPhaseCount>8</usedPhaseCount>
        <associatedStationCount>34</associatedStationCount>
        <usedStationCount>7</usedStationCount>
        <azimuthalGap>142.1660848</azimuthalGap>
        <maximumDistance>28.13891734</maximumDistance>
        <minimumDistance>3.624386503</minimumDistance>
      </quality>
      <type>hypocenter</type>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T05:51:44.024554Z</creationTime>
      </creationInfo>
      <magnitude publicID="Magnitude/20210430055144.02503.613678">
        <magnitude>
          <value>5.0549905</value>
        </magnitude>
        <type>Mw</type>
        <methodID>MT</methodID>
        <stationCount>7</stationCount>
        <azimuthalGap>142.1660848</azimuthalGap>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T05:51:44.024554Z</creationTime>
        </creationInfo>
      </magnitude>
    </origin>
    <origin publicID="Origin/20210430055224.677935.613690">
      <time>
        <value>2021-04-30T05:18:32.698118Z</value>
      </time>
      <latitude>
        <value>-11.89444256</value>
      </latitude>
      <longitude>
        <value>166.3646393</value>
      </longitude>
      <depth>
        <value>31</value>
      </depth>
      <methodID>FocalMechanism</methodID>
      <earthModelID>qseis_ak135f_continental</earthModelID>
      <quality>
        <usedPhaseCount>8</usedPhaseCount>
        <associatedStationCount>37</associatedStationCount>
        <usedStationCount>7</usedStationCount>
        <azimuthalGap>142.1660848</azimuthalGap>
        <maximumDistance>28.13891734</maximumDistance>
        <minimumDistance>3.624386503</minimumDistance>
      </quality>
      <type>hypocenter</type>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T05:52:24.677544Z</creationTime>
      </creationInfo>
      <magnitude publicID="Magnitude/20210430055224.678039.613691">
        <magnitude>
          <value>5.0549905</value>
        </magnitude>
        <type>Mw</type>
        <methodID>MT</methodID>
        <stationCount>7</stationCount>
        <azimuthalGap>142.1660848</azimuthalGap>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T05:52:24.677544Z</creationTime>
        </creationInfo>
      </magnitude>
    </origin>
    <origin publicID="Origin/20210430055237.798824.613703">
      <time>
        <value>2021-04-30T05:18:51.031451Z</value>
      </time>
      <latitude>
        <value>-11.89444256</value>
      </latitude>
      <longitude>
        <value>166.3938843</value>
      </longitude>
      <depth>
        <value>34.25008328</value>
      </depth>
      <methodID>FocalMechanism</methodID>
      <earthModelID>qseis_ak135f_continental</earthModelID>
      <quality>
        <usedPhaseCount>8</usedPhaseCount>
        <associatedStationCount>37</associatedStationCount>
        <usedStationCount>7</usedStationCount>
        <azimuthalGap>239.9937659</azimuthalGap>
        <maximumDistance>34.72866335</maximumDistance>
        <minimumDistance>3.618101529</minimumDistance>
      </quality>
      <type>centroid</type>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T05:52:37.798509Z</creationTime>
      </creationInfo>
      <magnitude publicID="Magnitude/20210430055237.798878.613704">
        <magnitude>
          <value>5.020915353</value>
        </magnitude>
        <type>Mw</type>
        <methodID>CMT</methodID>
        <stationCount>7</stationCount>
        <azimuthalGap>239.9937659</azimuthalGap>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T05:52:37.798509Z</creationTime>
        </creationInfo>
      </magnitude>
    </origin>
    <origin publicID="Origin/20210430055304.912021.613716">
      <time>
        <value>2021-04-30T05:18:32.698118Z</value>
      </time>
      <latitude>
        <value>-11.89444256</value>
      </latitude>
      <longitude>
        <value>166.3646393</value>
      </longitude>
      <depth>
        <value>31</value>
      </depth>
      <methodID>FocalMechanism</methodID>
      <earthModelID>qseis_ak135f_continental</earthModelID>
      <quality>
        <usedPhaseCount>8</usedPhaseCount>
        <associatedStationCount>38</associatedStationCount>
        <usedStationCount>7</usedStationCount>
        <azimuthalGap>142.1660848</azimuthalGap>
        <maximumDistance>28.13891734</maximumDistance>
        <minimumDistance>3.624386503</minimumDistance>
      </quality>
      <type>hypocenter</type>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T05:53:04.911698Z</creationTime>
      </creationInfo>
      <magnitude publicID="Magnitude/20210430055304.912111.613717">
        <magnitude>
          <value>5.0549905</value>
        </magnitude>
        <type>Mw</type>
        <methodID>MT</methodID>
        <stationCount>7</stationCount>
        <azimuthalGap>142.1660848</azimuthalGap>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T05:53:04.911698Z</creationTime>
        </creationInfo>
      </magnitude>
    </origin>
    <origin publicID="Origin/20210430055315.679349.613729">
      <time>
        <value>2021-04-30T05:18:51.031451Z</value>
      </time>
      <latitude>
        <value>-11.89444256</value>
      </latitude>
      <longitude>
        <value>166.3938843</value>
      </longitude>
      <depth>
        <value>34.25008328</value>
      </depth>
      <methodID>FocalMechanism</methodID>
      <earthModelID>qseis_ak135f_continental</earthModelID>
      <quality>
        <usedPhaseCount>8</usedPhaseCount>
        <associatedStationCount>38</associatedStationCount>
        <usedStationCount>7</usedStationCount>
        <azimuthalGap>239.9937659</azimuthalGap>
        <maximumDistance>34.72866335</maximumDistance>
        <minimumDistance>3.618101529</minimumDistance>
      </quality>
      <type>centroid</type>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T05:53:15.679051Z</creationTime>
      </creationInfo>
      <magnitude publicID="Magnitude/20210430055315.679402.613730">
        <magnitude>
          <value>5.021097167</value>
        </magnitude>
        <type>Mw</type>
        <methodID>CMT</methodID>
        <stationCount>7</stationCount>
        <azimuthalGap>239.9937659</azimuthalGap>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T05:53:15.679051Z</creationTime>
        </creationInfo>
      </magnitude>
    </origin>
    <origin publicID="Origin/20210430060422.075945.613831">
      <time>
        <value>2021-04-30T05:18:32.700063Z</value>
      </time>
      <latitude>
        <value>-11.89324379</value>
      </latitude>
      <longitude>
        <value>166.3651733</value>
      </longitude>
      <depth>
        <value>74</value>
      </depth>
      <methodID>FocalMechanism</methodID>
      <earthModelID>qseis_ak135f_continental</earthModelID>
      <quality>
        <usedPhaseCount>11</usedPhaseCount>
        <associatedStationCount>70</associatedStationCount>
        <usedStationCount>6</usedStationCount>
        <azimuthalGap>275.0755894</azimuthalGap>
        <maximumDistance>45.50692922</maximumDistance>
        <minimumDistance>3.625431235</minimumDistance>
      </quality>
      <type>hypocenter</type>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T06:04:22.075526Z</creationTime>
      </creationInfo>
      <magnitude publicID="Magnitude/20210430060422.076007.613832">
        <magnitude>
          <value>4.930974717</value>
        </magnitude>
        <type>Mw</type>
        <methodID>MT</methodID>
        <stationCount>6</stationCount>
        <azimuthalGap>275.0755894</azimuthalGap>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T06:04:22.075526Z</creationTime>
        </creationInfo>
      </magnitude>
    </origin>
    <origin publicID="Origin/20210430060441.059734.613898">
      <time>
        <value>2021-04-30T05:18:31.91435Z</value>
      </time>
      <latitude>
        <value>-11.89324379</value>
      </latitude>
      <longitude>
        <value>166.3775639</value>
      </longitude>
      <depth>
        <value>76.75400888</value>
      </depth>
      <methodID>FocalMechanism</methodID>
      <earthModelID>qseis_ak135f_continental</earthModelID>
      <quality>
        <usedPhaseCount>9</usedPhaseCount>
        <associatedStationCount>70</associatedStationCount>
        <usedStationCount>6</usedStationCount>
        <azimuthalGap>181.5093784</azimuthalGap>
        <maximumDistance>45.51846606</maximumDistance>
        <minimumDistance>3.622744988</minimumDistance>
      </quality>
      <type>centroid</type>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T06:04:41.059503Z</creationTime>
      </creationInfo>
      <magnitude publicID="Magnitude/20210430060441.059771.613899">
        <magnitude>
          <value>4.987024437</value>
        </magnitude>
        <type>Mw</type>
        <methodID>CMT</methodID>
        <stationCount>6</stationCount>
        <azimuthalGap>181.5093784</azimuthalGap>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T06:04:41.059503Z</creationTime>
        </creationInfo>
      </magnitude>
    </origin>
    <origin publicID="Origin/20210430060540.614867.614027">
      <time>
        <value>2021-04-30T05:18:32.700063Z</value>
      </time>
      <latitude>
        <value>-11.89324379</value>
      </latitude>
      <longitude>
        <value>166.3651733</value>
      </longitude>
      <depth>
        <value>74</value>
      </depth>
      <methodID>FocalMechanism</methodID>
      <earthModelID>qseis_ak135f_continental</earthModelID>
      <quality>
        <usedPhaseCount>11</usedPhaseCount>
        <associatedStationCount>76</associatedStationCount>
        <usedStationCount>6</usedStationCount>
        <azimuthalGap>275.0755894</azimuthalGap>
        <maximumDistance>45.50692922</maximumDistance>
        <minimumDistance>3.625431235</minimumDistance>
      </quality>
      <type>hypocenter</type>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T06:05:40.614507Z</creationTime>
      </creationInfo>
      <magnitude publicID="Magnitude/20210430060540.614927.614028">
        <magnitude>
          <value>4.930974717</value>
        </magnitude>
        <type>Mw</type>
        <methodID>MT</methodID>
        <stationCount>6</stationCount>
        <azimuthalGap>275.0755894</azimuthalGap>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T06:05:40.614507Z</creationTime>
        </creationInfo>
      </magnitude>
    </origin>
    <focalMechanism publicID="FocalMechanism/20210430054646.973651.613243">
      <triggeringOriginID>Origin/20210430053829.67918.86662</triggeringOriginID>
      <nodalPlanes>
        <nodalPlane1>
          <strike>
            <value>177.281775</value>
          </strike>
          <dip>
            <value>76.58290404</value>
          </dip>
          <rake>
            <value>174.2624477</value>
          </rake>
        </nodalPlane1>
        <nodalPlane2>
          <strike>
            <value>268.6173321</value>
          </strike>
          <dip>
            <value>84.41954865</value>
          </dip>
          <rake>
            <value>13.48219016</value>
          </rake>
        </nodalPlane2>
      </nodalPlanes>
      <principalAxes>
        <tAxis>
          <azimuth>
            <value>133.6216502</value>
          </azimuth>
          <plunge>
            <value>13.46416562</value>
          </plunge>
          <length>
            <value>3.108041651e+16</value>
          </length>
        </tAxis>
        <pAxis>
          <azimuth>
            <value>42.30807085</value>
          </azimuth>
          <plunge>
            <value>5.469411628</value>
          </plunge>
          <length>
            <value>-4.568628066e+16</value>
          </length>
        </pAxis>
        <nAxis>
          <azimuth>
            <value>290.6949551</value>
          </azimuth>
          <plunge>
            <value>75.42832792</value>
          </plunge>
          <length>
            <value>1.460586416e+16</value>
          </length>
        </nAxis>
      </principalAxes>
      <azimuthalGap>142.1660848</azimuthalGap>
      <misfit>0.04442736373</misfit>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T05:46:46.973516Z</creationTime>
      </creationInfo>
      <momentTensor publicID="MomentTensor/20210430054646.974052.613246">
        <derivedOriginID>Origin/20210430054646.973859.613244</derivedOriginID>
        <momentMagnitudeID>Magnitude/20210430054646.973972.613245</momentMagnitudeID>
        <scalarMoment>
          <value>4.041385776e+16</value>
        </scalarMoment>
        <tensor>
          <Mrr>
            <value>1.495125814e+16</value>
          </Mrr>
          <Mtt>
            <value>-1.065316645e+16</value>
          </Mtt>
          <Mpp>
            <value>-4.298091692e+15</value>
          </Mpp>
          <Mrt>
            <value>-6.804168123e+15</value>
          </Mrt>
          <Mrp>
            <value>1.150069208e+15</value>
          </Mrp>
          <Mtp>
            <value>3.752210952e+16</value>
          </Mtp>
        </tensor>
        <varianceReduction>0.9555726363</varianceReduction>
        <doubleCouple>0.3606017411</doubleCouple>
        <clvd>0.6393982589</clvd>
        <greensFunctionID>sc3gf1d:/qseis_ak135f_continental</greensFunctionID>
        <filterID>BP 40s-100s</filterID>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T05:46:46.973516Z</creationTime>
        </creationInfo>
        <dataUsed>
          <waveType>body waves</waveType>
          <stationCount>7</stationCount>
          <componentCount>7</componentCount>
        </dataUsed>
        <dataUsed>
          <waveType>surface waves</waveType>
          <stationCount>1</stationCount>
          <componentCount>1</componentCount>
        </dataUsed>
        <phaseSetting code="L" lowerPeriod="60" upperPeriod="150" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="P" lowerPeriod="40" upperPeriod="100" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="R" lowerPeriod="60" upperPeriod="150" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="S" lowerPeriod="40" upperPeriod="100" minimumSNR="1" maximumTimeShift="10"/>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054646.974151.613247" active="true" weight="1">
          <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH">
            <resourceURI>G.SANVU</resourceURI>
          </waveformID>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-4</timeShift>
            <dataTimeWindow>54.01234519 155.3729706</dataTimeWindow>
            <misfit>0.04565406799</misfit>
            <snr>27.23679924</snr>
          </component>
          <component phaseCode="R" component="0" active="true">
            <weight>1</weight>
            <timeShift>-4</timeShift>
            <dataTimeWindow>95.90205638 234.3056838</dataTimeWindow>
            <misfit>0.02108879089</misfit>
            <snr>7.709475517</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.123312071</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>96.43622093 234.3056838</dataTimeWindow>
            <misfit>0.8464826965</misfit>
            <snr>2.305141687</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.2406651974</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>96.43622093 234.3056838</dataTimeWindow>
            <misfit>1</misfit>
            <snr>4.024114132</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054646.974238.613248" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BH">
            <resourceURI>IU.TARA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.007092492655</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>326.4520581 872.8422378</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.073470592</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>6</timeShift>
            <dataTimeWindow>205.7445822 306.9925842</dataTimeWindow>
            <misfit>0.4962920761</misfit>
            <snr>1.363570571</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>388.6334025 872.8422378</dataTimeWindow>
            <misfit>0.8863717079</misfit>
            <snr>4.783790112</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.003497791942</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>388.6334025 872.8422378</dataTimeWindow>
            <misfit>0.9628971267</misfit>
            <snr>2.31493187</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.004891633056</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>368.4220656 568.4220656</dataTimeWindow>
            <misfit>0.7426436996</misfit>
            <snr>0.6371799707</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.009299323894</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>368.4220656 568.4220656</dataTimeWindow>
            <misfit>1</misfit>
            <snr>0.82813555</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054646.97428.613249" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="EIDS" channelCode="BH">
            <resourceURI>AU.EIDS</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.1044915617</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>437.9514809 1154.246532</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.215333939</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>5</timeShift>
            <dataTimeWindow>266.9266902 369.2104797</dataTimeWindow>
            <misfit>0.2396282959</misfit>
            <snr>4.692846775</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>521.3708106 1154.246532</dataTimeWindow>
            <misfit>0.7552874374</misfit>
            <snr>1.796907663</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.08761250228</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>521.3708106 1154.246532</dataTimeWindow>
            <misfit>0.763740406</misfit>
            <snr>2.604088306</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.03381812572</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>488.3866849 688.3866849</dataTimeWindow>
            <misfit>0.3428308105</misfit>
            <snr>1.600562096</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.03597045317</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>488.3866849 688.3866849</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.383267522</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054646.974327.613250" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BH">
            <resourceURI>IU.CTAO</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.02272730693</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>465.9110441 1222.790913</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.926164627</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>1</timeShift>
            <dataTimeWindow>280.6353776 382.8960571</dataTimeWindow>
            <misfit>0.5432185364</misfit>
            <snr>7.60748148</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>554.6560049 1222.790913</dataTimeWindow>
            <misfit>0.5438781357</misfit>
            <snr>1.595830441</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.031402722</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>554.6560049 1222.790913</dataTimeWindow>
            <misfit>0.8834739971</misfit>
            <snr>1.921049237</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.01518514752</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>513.5637303 713.5637303</dataTimeWindow>
            <misfit>0.9644511843</misfit>
            <snr>0.9940682054</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.01513855904</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>513.5637303 713.5637303</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.104261518</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054646.974373.613251" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="ARMA" channelCode="BH">
            <resourceURI>AU.ARMA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.09928009659</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>509.9773866 1330.301247</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.778090715</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>1</timeShift>
            <dataTimeWindow>301.7205975 404.132782</dataTimeWindow>
            <misfit>0.3358093262</misfit>
            <snr>1.406597495</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.1159364 1330.301247</dataTimeWindow>
            <misfit>0.9172219753</misfit>
            <snr>2.54085803</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.04847697541</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.1159364 1330.301247</dataTimeWindow>
            <misfit>0.7893798637</misfit>
            <snr>2.103213787</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.02705101855</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>550.9146855 750.9146855</dataTimeWindow>
            <misfit>0.9281681299</misfit>
            <snr>2.0030725</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.04330698028</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>550.9146855 750.9146855</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.200439453</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054646.97441.613252" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="CMSA" channelCode="BH">
            <resourceURI>AU.CMSA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.2612291873</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>606.998931 1560.886236</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.590667725</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-2</timeShift>
            <dataTimeWindow>342.0220828 444.8430176</dataTimeWindow>
            <misfit>0.3899413681</misfit>
            <snr>2.734752893</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>722.617775 1560.886236</dataTimeWindow>
            <misfit>0.6833830261</misfit>
            <snr>1.75257659</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.1859548837</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>722.617775 1560.886236</dataTimeWindow>
            <misfit>0.5847386932</misfit>
            <snr>2.04881382</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.09062115848</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>620.6214221 820.6214221</dataTimeWindow>
            <misfit>0.6709676361</misfit>
            <snr>1.990436196</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.112135753</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>620.6214221 820.6214221</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.806781292</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054646.97445.613253" active="true" weight="1">
          <waveformID networkCode="G" stationCode="CAN" locationCode="00" channelCode="BH">
            <resourceURI>G.CAN</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>625.4319812 1604.400325</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.339976072</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-2</timeShift>
            <dataTimeWindow>349.3843523 452.2696228</dataTimeWindow>
            <misfit>0.1505408478</misfit>
            <snr>1.981705666</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>744.5618824 1604.400325</dataTimeWindow>
            <misfit>0.8060004425</misfit>
            <snr>1.968536258</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>744.5618824 1604.400325</dataTimeWindow>
            <misfit>0.8817236328</misfit>
            <snr>1.264317036</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>633.5868385 833.5868385</dataTimeWindow>
            <misfit>0.5142569351</misfit>
            <snr>1.669034362</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>633.5868385 833.5868385</dataTimeWindow>
            <misfit>1</misfit>
            <snr>4.945378304</snr>
          </component>
        </stationMomentTensorContribution>
      </momentTensor>
    </focalMechanism>
    <focalMechanism publicID="FocalMechanism/20210430054719.938764.613258">
      <triggeringOriginID>Origin/20210430053829.67918.86662</triggeringOriginID>
      <nodalPlanes>
        <nodalPlane1>
          <strike>
            <value>177.2742726</value>
          </strike>
          <dip>
            <value>76.61342939</value>
          </dip>
          <rake>
            <value>174.2963807</value>
          </rake>
        </nodalPlane1>
        <nodalPlane2>
          <strike>
            <value>268.5989171</value>
          </strike>
          <dip>
            <value>84.4518401</value>
          </dip>
          <rake>
            <value>13.45075871</value>
          </rake>
        </nodalPlane2>
      </nodalPlanes>
      <principalAxes>
        <tAxis>
          <azimuth>
            <value>133.6064521</value>
          </azimuth>
          <plunge>
            <value>13.41921138</value>
          </plunge>
          <length>
            <value>3.111600737e+16</value>
          </length>
        </tAxis>
        <pAxis>
          <azimuth>
            <value>42.29700587</value>
          </azimuth>
          <plunge>
            <value>5.47114855</value>
          </plunge>
          <length>
            <value>-4.572682366e+16</value>
          </length>
        </pAxis>
        <nAxis>
          <azimuth>
            <value>290.6095121</value>
          </azimuth>
          <plunge>
            <value>75.46944394</value>
          </plunge>
          <length>
            <value>1.461081629e+16</value>
          </length>
        </nAxis>
      </principalAxes>
      <azimuthalGap>142.1660848</azimuthalGap>
      <misfit>0.04443451728</misfit>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T05:47:19.938545Z</creationTime>
      </creationInfo>
      <momentTensor publicID="MomentTensor/20210430054719.939157.613261">
        <derivedOriginID>Origin/20210430054719.938975.613259</derivedOriginID>
        <momentMagnitudeID>Magnitude/20210430054719.939079.613260</momentMagnitudeID>
        <scalarMoment>
          <value>4.045135516e+16</value>
        </scalarMoment>
        <tensor>
          <Mrr>
            <value>1.495125517e+16</value>
          </Mrr>
          <Mtt>
            <value>-1.067173412e+16</value>
          </Mtt>
          <Mpp>
            <value>-4.279521045e+15</value>
          </Mpp>
          <Mrt>
            <value>-6.80553508e+15</value>
          </Mrt>
          <Mrp>
            <value>1.15602311e+15</value>
          </Mrp>
          <Mtp>
            <value>3.756048878e+16</value>
          </Mtp>
        </tensor>
        <varianceReduction>0.9555654827</varianceReduction>
        <doubleCouple>0.3609520575</doubleCouple>
        <clvd>0.6390479425</clvd>
        <greensFunctionID>sc3gf1d:/qseis_ak135f_continental</greensFunctionID>
        <filterID>BP 40s-100s</filterID>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T05:47:19.938545Z</creationTime>
        </creationInfo>
        <dataUsed>
          <waveType>body waves</waveType>
          <stationCount>7</stationCount>
          <componentCount>7</componentCount>
        </dataUsed>
        <dataUsed>
          <waveType>surface waves</waveType>
          <stationCount>1</stationCount>
          <componentCount>1</componentCount>
        </dataUsed>
        <phaseSetting code="L" lowerPeriod="60" upperPeriod="150" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="P" lowerPeriod="40" upperPeriod="100" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="R" lowerPeriod="60" upperPeriod="150" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="S" lowerPeriod="40" upperPeriod="100" minimumSNR="1" maximumTimeShift="10"/>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054719.939259.613262" active="true" weight="1">
          <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH">
            <resourceURI>G.SANVU</resourceURI>
          </waveformID>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-4</timeShift>
            <dataTimeWindow>54.01234519 155.3729706</dataTimeWindow>
            <misfit>0.04565406799</misfit>
            <snr>27.23679924</snr>
          </component>
          <component phaseCode="R" component="0" active="true">
            <weight>1</weight>
            <timeShift>-4</timeShift>
            <dataTimeWindow>95.90205638 234.3056838</dataTimeWindow>
            <misfit>0.02108886719</misfit>
            <snr>7.709475517</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.123312071</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>96.43622093 234.3056838</dataTimeWindow>
            <misfit>0.8464826965</misfit>
            <snr>2.305141687</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.2406651974</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>96.43622093 234.3056838</dataTimeWindow>
            <misfit>1</misfit>
            <snr>4.024114132</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054719.939345.613263" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BH">
            <resourceURI>IU.TARA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.007092492655</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>326.4520581 872.8422378</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.073470592</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>6</timeShift>
            <dataTimeWindow>205.7445822 306.9925842</dataTimeWindow>
            <misfit>0.4963317871</misfit>
            <snr>1.363570571</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>388.6334025 872.8422378</dataTimeWindow>
            <misfit>0.8863717079</misfit>
            <snr>4.783790112</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.003497791942</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>388.6334025 872.8422378</dataTimeWindow>
            <misfit>0.9628971267</misfit>
            <snr>2.31493187</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.004891633056</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>368.4220656 568.4220656</dataTimeWindow>
            <misfit>0.7426436996</misfit>
            <snr>0.6371799707</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.009299323894</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>368.4220656 568.4220656</dataTimeWindow>
            <misfit>1</misfit>
            <snr>0.82813555</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054719.939403.613264" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="EIDS" channelCode="BH">
            <resourceURI>AU.EIDS</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.1044915617</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>437.9514809 1154.246532</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.215333939</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>5</timeShift>
            <dataTimeWindow>266.9266902 369.2104797</dataTimeWindow>
            <misfit>0.2394352722</misfit>
            <snr>4.692846775</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>521.3708106 1154.246532</dataTimeWindow>
            <misfit>0.7552874374</misfit>
            <snr>1.796907663</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.08761250228</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>521.3708106 1154.246532</dataTimeWindow>
            <misfit>0.763740406</misfit>
            <snr>2.604088306</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.03381812572</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>488.3866849 688.3866849</dataTimeWindow>
            <misfit>0.3428308105</misfit>
            <snr>1.600562096</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.03597045317</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>488.3866849 688.3866849</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.383267522</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054719.939454.613265" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BH">
            <resourceURI>IU.CTAO</resourceURI>
          </waveformID>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>1</timeShift>
            <dataTimeWindow>280.6353776 382.8960571</dataTimeWindow>
            <misfit>0.5432176971</misfit>
            <snr>7.60748148</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>554.6560049 1222.790913</dataTimeWindow>
            <misfit>0.5438781357</misfit>
            <snr>1.595830441</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.01518514752</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>513.5637303 713.5637303</dataTimeWindow>
            <misfit>0.9644511843</misfit>
            <snr>0.9940682054</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.031402722</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>554.6560049 1222.790913</dataTimeWindow>
            <misfit>0.8834739971</misfit>
            <snr>1.921049237</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.01513855904</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>513.5637303 713.5637303</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.104261518</snr>
          </component>
          <component phaseCode="L" component="2" active="false">
            <weight>0.02272730693</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>465.9110441 1222.790913</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.926164627</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054719.939531.613266" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="ARMA" channelCode="BH">
            <resourceURI>AU.ARMA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.09928009659</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>509.9773866 1330.301247</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.778090715</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>1</timeShift>
            <dataTimeWindow>301.7205975 404.132782</dataTimeWindow>
            <misfit>0.3359051514</misfit>
            <snr>1.406597495</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.1159364 1330.301247</dataTimeWindow>
            <misfit>0.9172219753</misfit>
            <snr>2.54085803</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.04847697541</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.1159364 1330.301247</dataTimeWindow>
            <misfit>0.7893798637</misfit>
            <snr>2.103213787</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.02705101855</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>550.9146855 750.9146855</dataTimeWindow>
            <misfit>0.9281681299</misfit>
            <snr>2.0030725</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.04330698028</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>550.9146855 750.9146855</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.200439453</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054719.939588.613267" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="CMSA" channelCode="BH">
            <resourceURI>AU.CMSA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.2612291873</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>606.998931 1560.886236</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.590667725</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-2</timeShift>
            <dataTimeWindow>342.0220828 444.8430176</dataTimeWindow>
            <misfit>0.3898883438</misfit>
            <snr>2.734752893</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>722.617775 1560.886236</dataTimeWindow>
            <misfit>0.6833830261</misfit>
            <snr>1.75257659</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.1859548837</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>722.617775 1560.886236</dataTimeWindow>
            <misfit>0.5847386932</misfit>
            <snr>2.04881382</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.09062115848</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>620.6214221 820.6214221</dataTimeWindow>
            <misfit>0.6709676361</misfit>
            <snr>1.990436196</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.112135753</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>620.6214221 820.6214221</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.806781292</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054719.939629.613268" active="true" weight="1">
          <waveformID networkCode="G" stationCode="CAN" locationCode="00" channelCode="BH">
            <resourceURI>G.CAN</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>625.4319812 1604.400325</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.33833766</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-2</timeShift>
            <dataTimeWindow>349.3843523 452.2696228</dataTimeWindow>
            <misfit>0.1503552246</misfit>
            <snr>1.981676102</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>744.5618824 1604.400325</dataTimeWindow>
            <misfit>0.8058838272</misfit>
            <snr>1.965732336</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>744.5618824 1604.400325</dataTimeWindow>
            <misfit>0.841143961</misfit>
            <snr>1.085965395</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>633.5868385 833.5868385</dataTimeWindow>
            <misfit>0.5143440247</misfit>
            <snr>1.668998122</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>633.5868385 833.5868385</dataTimeWindow>
            <misfit>1</misfit>
            <snr>4.945716858</snr>
          </component>
        </stationMomentTensorContribution>
      </momentTensor>
    </focalMechanism>
    <focalMechanism publicID="FocalMechanism/20210430054758.63766.613271">
      <triggeringOriginID>Origin/20210430053829.67918.86662</triggeringOriginID>
      <nodalPlanes>
        <nodalPlane1>
          <strike>
            <value>177.2555043</value>
          </strike>
          <dip>
            <value>76.69040849</value>
          </dip>
          <rake>
            <value>174.3811364</value>
          </rake>
        </nodalPlane1>
        <nodalPlane2>
          <strike>
            <value>268.5529786</value>
          </strike>
          <dip>
            <value>84.53252334</value>
          </dip>
          <rake>
            <value>13.37154722</value>
          </rake>
        </nodalPlane2>
      </nodalPlanes>
      <principalAxes>
        <tAxis>
          <azimuth>
            <value>133.5684198</value>
          </azimuth>
          <plunge>
            <value>13.30640707</value>
          </plunge>
          <length>
            <value>3.120591818e+16</value>
          </length>
        </tAxis>
        <pAxis>
          <azimuth>
            <value>42.26947302</value>
          </azimuth>
          <plunge>
            <value>5.474992362</value>
          </plunge>
          <length>
            <value>-4.58271236e+16</value>
          </length>
        </pAxis>
        <nAxis>
          <azimuth>
            <value>290.395343</value>
          </azimuth>
          <plunge>
            <value>75.5727149</value>
          </plunge>
          <length>
            <value>1.462120541e+16</value>
          </length>
        </nAxis>
      </principalAxes>
      <azimuthalGap>142.1660848</azimuthalGap>
      <misfit>0.04449995559</misfit>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T05:47:58.637516Z</creationTime>
      </creationInfo>
      <momentTensor publicID="MomentTensor/20210430054758.638114.613274">
        <derivedOriginID>Origin/20210430054758.637903.613272</derivedOriginID>
        <momentMagnitudeID>Magnitude/20210430054758.638028.613273</momentMagnitudeID>
        <scalarMoment>
          <value>4.054450786e+16</value>
        </scalarMoment>
        <tensor>
          <Mrr>
            <value>1.494946669e+16</value>
          </Mrr>
          <Mtt>
            <value>-1.071722352e+16</value>
          </Mtt>
          <Mpp>
            <value>-4.232243171e+15</value>
          </Mpp>
          <Mrt>
            <value>-6.808587328e+15</value>
          </Mrt>
          <Mrp>
            <value>1.170150535e+15</value>
          </Mrp>
          <Mtp>
            <value>3.765637191e+16</value>
          </Mtp>
        </tensor>
        <varianceReduction>0.9555000444</varianceReduction>
        <doubleCouple>0.3618973104</doubleCouple>
        <clvd>0.6381026896</clvd>
        <greensFunctionID>sc3gf1d:/qseis_ak135f_continental</greensFunctionID>
        <filterID>BP 40s-100s</filterID>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T05:47:58.637516Z</creationTime>
        </creationInfo>
        <dataUsed>
          <waveType>body waves</waveType>
          <stationCount>8</stationCount>
          <componentCount>8</componentCount>
        </dataUsed>
        <dataUsed>
          <waveType>surface waves</waveType>
          <stationCount>1</stationCount>
          <componentCount>1</componentCount>
        </dataUsed>
        <phaseSetting code="L" lowerPeriod="60" upperPeriod="150" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="P" lowerPeriod="40" upperPeriod="100" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="R" lowerPeriod="60" upperPeriod="150" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="S" lowerPeriod="40" upperPeriod="100" minimumSNR="1" maximumTimeShift="10"/>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054758.638236.613275" active="true" weight="1">
          <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH">
            <resourceURI>G.SANVU</resourceURI>
          </waveformID>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-4</timeShift>
            <dataTimeWindow>54.01234519 155.3729706</dataTimeWindow>
            <misfit>0.04565635681</misfit>
            <snr>27.23679924</snr>
          </component>
          <component phaseCode="R" component="0" active="true">
            <weight>1</weight>
            <timeShift>-4</timeShift>
            <dataTimeWindow>95.90205638 234.3056838</dataTimeWindow>
            <misfit>0.02109039307</misfit>
            <snr>7.709475517</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.123312071</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>96.43622093 234.3056838</dataTimeWindow>
            <misfit>0.8464826965</misfit>
            <snr>2.305141687</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.2406651974</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>96.43622093 234.3056838</dataTimeWindow>
            <misfit>1</misfit>
            <snr>4.024114132</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054758.638322.613276" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BH">
            <resourceURI>IU.TARA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.007092492655</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>326.4520581 872.8422378</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.073470592</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>6</timeShift>
            <dataTimeWindow>205.7445822 306.9925842</dataTimeWindow>
            <misfit>0.4964299774</misfit>
            <snr>1.363570571</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>388.6334025 872.8422378</dataTimeWindow>
            <misfit>0.8863717079</misfit>
            <snr>4.783790112</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.003497791942</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>388.6334025 872.8422378</dataTimeWindow>
            <misfit>0.9628971267</misfit>
            <snr>2.31493187</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.004891633056</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>368.4220656 568.4220656</dataTimeWindow>
            <misfit>0.7426436996</misfit>
            <snr>0.6371799707</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.009299323894</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>368.4220656 568.4220656</dataTimeWindow>
            <misfit>1</misfit>
            <snr>0.82813555</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054758.638383.613277" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="EIDS" channelCode="BH">
            <resourceURI>AU.EIDS</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.1044915617</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>437.9514809 1154.246532</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.215333939</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>5</timeShift>
            <dataTimeWindow>266.9266902 369.2104797</dataTimeWindow>
            <misfit>0.238970108</misfit>
            <snr>4.692846775</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>521.3708106 1154.246532</dataTimeWindow>
            <misfit>0.7552874374</misfit>
            <snr>1.796907663</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.08761250228</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>521.3708106 1154.246532</dataTimeWindow>
            <misfit>0.763740406</misfit>
            <snr>2.604088306</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.03381812572</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>488.3866849 688.3866849</dataTimeWindow>
            <misfit>0.3428308105</misfit>
            <snr>1.600562096</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.03597045317</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>488.3866849 688.3866849</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.383267522</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054758.63844.613278" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BH">
            <resourceURI>IU.CTAO</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.02272730693</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>465.9110441 1222.790913</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.926164627</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>1</timeShift>
            <dataTimeWindow>280.6353776 382.8960571</dataTimeWindow>
            <misfit>0.5432221222</misfit>
            <snr>7.60748148</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>554.6560049 1222.790913</dataTimeWindow>
            <misfit>0.5438781357</misfit>
            <snr>1.595830441</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.031402722</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>554.6560049 1222.790913</dataTimeWindow>
            <misfit>0.8834739971</misfit>
            <snr>1.921049237</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.01518514752</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>513.5637303 713.5637303</dataTimeWindow>
            <misfit>0.9644511843</misfit>
            <snr>0.9940682054</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.01513855904</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>513.5637303 713.5637303</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.104261518</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054758.638519.613279" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="ARMA" channelCode="BH">
            <resourceURI>AU.ARMA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.09928009659</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>509.9773866 1330.301247</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.778090715</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>1</timeShift>
            <dataTimeWindow>301.7205975 404.132782</dataTimeWindow>
            <misfit>0.3361422729</misfit>
            <snr>1.406597495</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.1159364 1330.301247</dataTimeWindow>
            <misfit>0.9172219753</misfit>
            <snr>2.54085803</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.04847697541</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.1159364 1330.301247</dataTimeWindow>
            <misfit>0.7893798637</misfit>
            <snr>2.103213787</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.02705101855</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>550.9146855 750.9146855</dataTimeWindow>
            <misfit>0.9281681299</misfit>
            <snr>2.0030725</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.04330698028</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>550.9146855 750.9146855</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.200439453</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054758.638569.613280" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="CMSA" channelCode="BH">
            <resourceURI>AU.CMSA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.2612291873</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>606.998931 1560.886236</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.590667725</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-2</timeShift>
            <dataTimeWindow>342.0220828 444.8430176</dataTimeWindow>
            <misfit>0.3897646713</misfit>
            <snr>2.734752893</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>722.617775 1560.886236</dataTimeWindow>
            <misfit>0.6833830261</misfit>
            <snr>1.75257659</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.1859548837</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>722.617775 1560.886236</dataTimeWindow>
            <misfit>0.5847386932</misfit>
            <snr>2.04881382</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.09062115848</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>620.6214221 820.6214221</dataTimeWindow>
            <misfit>0.6709676361</misfit>
            <snr>1.990436196</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.112135753</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>620.6214221 820.6214221</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.806781292</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054758.638618.613281" active="true" weight="1">
          <waveformID networkCode="G" stationCode="CAN" locationCode="00" channelCode="BH">
            <resourceURI>G.CAN</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>625.4319812 1604.400325</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.338611841</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-2</timeShift>
            <dataTimeWindow>349.3843523 452.2696228</dataTimeWindow>
            <misfit>0.1499567413</misfit>
            <snr>1.981680036</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>744.5618824 1604.400325</dataTimeWindow>
            <misfit>0.8057865715</misfit>
            <snr>1.965375543</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>744.5618824 1604.400325</dataTimeWindow>
            <misfit>0.8435315895</misfit>
            <snr>1.055261493</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>633.5868385 833.5868385</dataTimeWindow>
            <misfit>0.5141334915</misfit>
            <snr>1.668983817</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>633.5868385 833.5868385</dataTimeWindow>
            <misfit>1</misfit>
            <snr>4.945716381</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054758.638662.613282" active="true" weight="1">
          <waveformID networkCode="NZ" stationCode="BFZ" locationCode="10" channelCode="HH">
            <resourceURI>NZ.BFZ</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.1881081164</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>666.0476493 1700.198689</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.286252737</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>1</timeShift>
            <dataTimeWindow>365.5252363 468.5096436</dataTimeWindow>
            <misfit>0.4459382248</misfit>
            <snr>1.030604124</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>792.9138682 1700.198689</dataTimeWindow>
            <misfit>0.9931901526</misfit>
            <snr>1.620030284</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.1233444288</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>792.9138682 1700.198689</dataTimeWindow>
            <misfit>0.9617488289</misfit>
            <snr>3.375401735</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.06228311732</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>662.1432886 862.1432886</dataTimeWindow>
            <misfit>0.6241755295</misfit>
            <snr>1.309775591</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.0721578002</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>662.1432886 862.1432886</dataTimeWindow>
            <misfit>1</misfit>
            <snr>0.5607553124</snr>
          </component>
        </stationMomentTensorContribution>
      </momentTensor>
    </focalMechanism>
    <focalMechanism publicID="FocalMechanism/20210430054838.925691.613371">
      <triggeringOriginID>Origin/20210430053829.67918.86662</triggeringOriginID>
      <nodalPlanes>
        <nodalPlane1>
          <strike>
            <value>177.2639358</value>
          </strike>
          <dip>
            <value>76.65512867</value>
          </dip>
          <rake>
            <value>174.342645</value>
          </rake>
        </nodalPlane1>
        <nodalPlane2>
          <strike>
            <value>268.5737521</value>
          </strike>
          <dip>
            <value>84.49588082</value>
          </dip>
          <rake>
            <value>13.40783625</value>
          </rake>
        </nodalPlane2>
      </nodalPlanes>
      <principalAxes>
        <tAxis>
          <azimuth>
            <value>133.5856399</value>
          </azimuth>
          <plunge>
            <value>13.35786003</value>
          </plunge>
          <length>
            <value>3.116494748e+16</value>
          </length>
        </tAxis>
        <pAxis>
          <azimuth>
            <value>42.28184833</value>
          </azimuth>
          <plunge>
            <value>5.473468684</value>
          </plunge>
          <length>
            <value>-4.578250102e+16</value>
          </length>
        </pAxis>
        <nAxis>
          <azimuth>
            <value>290.4923551</value>
          </azimuth>
          <plunge>
            <value>75.5255386</value>
          </plunge>
          <length>
            <value>1.461755355e+16</value>
          </length>
        </nAxis>
      </principalAxes>
      <azimuthalGap>142.1660848</azimuthalGap>
      <misfit>0.04444464526</misfit>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T05:48:38.925529Z</creationTime>
      </creationInfo>
      <momentTensor publicID="MomentTensor/20210430054838.926004.613374">
        <derivedOriginID>Origin/20210430054838.925868.613372</derivedOriginID>
        <momentMagnitudeID>Magnitude/20210430054838.925934.613373</momentMagnitudeID>
        <scalarMoment>
          <value>4.050286547e+16</value>
        </scalarMoment>
        <tensor>
          <Mrr>
            <value>1.495125103e+16</value>
          </Mrr>
          <Mtt>
            <value>-1.069723406e+16</value>
          </Mtt>
          <Mpp>
            <value>-4.254016978e+15</value>
          </Mpp>
          <Mrt>
            <value>-6.807395796e+15</value>
          </Mrt>
          <Mrp>
            <value>1.164127571e+15</value>
          </Mrp>
          <Mtp>
            <value>3.7613197e+16</value>
          </Mtp>
        </tensor>
        <varianceReduction>0.9555553547</varianceReduction>
        <doubleCouple>0.3614349054</doubleCouple>
        <clvd>0.6385650946</clvd>
        <greensFunctionID>sc3gf1d:/qseis_ak135f_continental</greensFunctionID>
        <filterID>BP 40s-100s</filterID>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T05:48:38.925529Z</creationTime>
        </creationInfo>
        <dataUsed>
          <waveType>body waves</waveType>
          <stationCount>7</stationCount>
          <componentCount>7</componentCount>
        </dataUsed>
        <dataUsed>
          <waveType>surface waves</waveType>
          <stationCount>1</stationCount>
          <componentCount>1</componentCount>
        </dataUsed>
        <phaseSetting code="L" lowerPeriod="60" upperPeriod="150" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="P" lowerPeriod="40" upperPeriod="100" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="R" lowerPeriod="60" upperPeriod="150" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="S" lowerPeriod="40" upperPeriod="100" minimumSNR="1" maximumTimeShift="10"/>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054838.926108.613375" active="true" weight="1">
          <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH">
            <resourceURI>G.SANVU</resourceURI>
          </waveformID>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-4</timeShift>
            <dataTimeWindow>54.01234519 155.3729706</dataTimeWindow>
            <misfit>0.04565406799</misfit>
            <snr>27.23679924</snr>
          </component>
          <component phaseCode="R" component="0" active="true">
            <weight>1</weight>
            <timeShift>-4</timeShift>
            <dataTimeWindow>95.90205638 234.3056838</dataTimeWindow>
            <misfit>0.02108886719</misfit>
            <snr>7.709475517</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.123312071</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>96.43622093 234.3056838</dataTimeWindow>
            <misfit>0.8464826965</misfit>
            <snr>2.305141687</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.2406651974</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>96.43622093 234.3056838</dataTimeWindow>
            <misfit>1</misfit>
            <snr>4.024114132</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054838.926196.613376" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BH">
            <resourceURI>IU.TARA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.007092492655</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>326.4520581 872.8422378</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.073470592</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>6</timeShift>
            <dataTimeWindow>205.7445822 306.9925842</dataTimeWindow>
            <misfit>0.4963869095</misfit>
            <snr>1.363570571</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>388.6334025 872.8422378</dataTimeWindow>
            <misfit>0.8863717079</misfit>
            <snr>4.783790112</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.003497791942</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>388.6334025 872.8422378</dataTimeWindow>
            <misfit>0.9628971267</misfit>
            <snr>2.31493187</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.004891633056</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>368.4220656 568.4220656</dataTimeWindow>
            <misfit>0.7426436996</misfit>
            <snr>0.6371799707</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.009299323894</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>368.4220656 568.4220656</dataTimeWindow>
            <misfit>1</misfit>
            <snr>0.82813555</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054838.926249.613377" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="EIDS" channelCode="BH">
            <resourceURI>AU.EIDS</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.1044915617</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>437.9514809 1154.246532</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.215333939</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>5</timeShift>
            <dataTimeWindow>266.9266902 369.2104797</dataTimeWindow>
            <misfit>0.2391711426</misfit>
            <snr>4.692846775</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>521.3708106 1154.246532</dataTimeWindow>
            <misfit>0.7552874374</misfit>
            <snr>1.796907663</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.08761250228</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>521.3708106 1154.246532</dataTimeWindow>
            <misfit>0.763740406</misfit>
            <snr>2.604088306</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.03381812572</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>488.3866849 688.3866849</dataTimeWindow>
            <misfit>0.3428308105</misfit>
            <snr>1.600562096</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.03597045317</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>488.3866849 688.3866849</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.383267522</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054838.926287.613378" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BH">
            <resourceURI>IU.CTAO</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.02272730693</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>465.9110441 1222.790913</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.926164627</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>1</timeShift>
            <dataTimeWindow>280.6353776 382.8960571</dataTimeWindow>
            <misfit>0.5432171631</misfit>
            <snr>7.60748148</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>554.6560049 1222.790913</dataTimeWindow>
            <misfit>0.5438781357</misfit>
            <snr>1.595830441</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.031402722</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>554.6560049 1222.790913</dataTimeWindow>
            <misfit>0.8834739971</misfit>
            <snr>1.921049237</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.01518514752</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>513.5637303 713.5637303</dataTimeWindow>
            <misfit>0.9644511843</misfit>
            <snr>0.9940682054</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.01513855904</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>513.5637303 713.5637303</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.104261518</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054838.926323.613379" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="ARMA" channelCode="BH">
            <resourceURI>AU.ARMA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.09928009659</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>509.9773866 1330.301247</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.778090715</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>1</timeShift>
            <dataTimeWindow>301.7205975 404.132782</dataTimeWindow>
            <misfit>0.3360371399</misfit>
            <snr>1.406597495</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.1159364 1330.301247</dataTimeWindow>
            <misfit>0.9172219753</misfit>
            <snr>2.54085803</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.04847697541</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.1159364 1330.301247</dataTimeWindow>
            <misfit>0.7893798637</misfit>
            <snr>2.103213787</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.02705101855</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>550.9146855 750.9146855</dataTimeWindow>
            <misfit>0.9281681299</misfit>
            <snr>2.0030725</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.04330698028</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>550.9146855 750.9146855</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.200439453</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054838.926373.613380" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="CMSA" channelCode="BH">
            <resourceURI>AU.CMSA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.2612291873</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>606.998931 1560.886236</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.590667725</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-2</timeShift>
            <dataTimeWindow>342.0220828 444.8430176</dataTimeWindow>
            <misfit>0.389816246</misfit>
            <snr>2.734752893</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>722.617775 1560.886236</dataTimeWindow>
            <misfit>0.6833830261</misfit>
            <snr>1.75257659</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.1859548837</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>722.617775 1560.886236</dataTimeWindow>
            <misfit>0.5847386932</misfit>
            <snr>2.04881382</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.09062115848</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>620.6214221 820.6214221</dataTimeWindow>
            <misfit>0.6709676361</misfit>
            <snr>1.990436196</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.112135753</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>620.6214221 820.6214221</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.806781292</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054838.926416.613381" active="true" weight="1">
          <waveformID networkCode="G" stationCode="CAN" locationCode="00" channelCode="BH">
            <resourceURI>G.CAN</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>625.4319812 1604.400325</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.338611841</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-2</timeShift>
            <dataTimeWindow>349.3843523 452.2696228</dataTimeWindow>
            <misfit>0.1501287842</misfit>
            <snr>1.981680036</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>744.5618824 1604.400325</dataTimeWindow>
            <misfit>0.8057865715</misfit>
            <snr>1.965375543</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>744.5618824 1604.400325</dataTimeWindow>
            <misfit>0.8435315895</misfit>
            <snr>1.055261493</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>633.5868385 833.5868385</dataTimeWindow>
            <misfit>0.5141334915</misfit>
            <snr>1.668983817</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>633.5868385 833.5868385</dataTimeWindow>
            <misfit>1</misfit>
            <snr>4.945716381</snr>
          </component>
        </stationMomentTensorContribution>
      </momentTensor>
    </focalMechanism>
    <focalMechanism publicID="FocalMechanism/20210430054919.510665.613512">
      <triggeringOriginID>Origin/20210430053829.67918.86662</triggeringOriginID>
      <nodalPlanes>
        <nodalPlane1>
          <strike>
            <value>177.2639358</value>
          </strike>
          <dip>
            <value>76.65512867</value>
          </dip>
          <rake>
            <value>174.342645</value>
          </rake>
        </nodalPlane1>
        <nodalPlane2>
          <strike>
            <value>268.5737521</value>
          </strike>
          <dip>
            <value>84.49588082</value>
          </dip>
          <rake>
            <value>13.40783625</value>
          </rake>
        </nodalPlane2>
      </nodalPlanes>
      <principalAxes>
        <tAxis>
          <azimuth>
            <value>133.5856399</value>
          </azimuth>
          <plunge>
            <value>13.35786003</value>
          </plunge>
          <length>
            <value>3.116494748e+16</value>
          </length>
        </tAxis>
        <pAxis>
          <azimuth>
            <value>42.28184833</value>
          </azimuth>
          <plunge>
            <value>5.473468684</value>
          </plunge>
          <length>
            <value>-4.578250102e+16</value>
          </length>
        </pAxis>
        <nAxis>
          <azimuth>
            <value>290.4923551</value>
          </azimuth>
          <plunge>
            <value>75.5255386</value>
          </plunge>
          <length>
            <value>1.461755355e+16</value>
          </length>
        </nAxis>
      </principalAxes>
      <azimuthalGap>142.1660848</azimuthalGap>
      <misfit>0.04444464526</misfit>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T05:49:19.510573Z</creationTime>
      </creationInfo>
      <momentTensor publicID="MomentTensor/20210430054919.510952.613515">
        <derivedOriginID>Origin/20210430054919.510829.613513</derivedOriginID>
        <momentMagnitudeID>Magnitude/20210430054919.510879.613514</momentMagnitudeID>
        <scalarMoment>
          <value>4.050286547e+16</value>
        </scalarMoment>
        <tensor>
          <Mrr>
            <value>1.495125103e+16</value>
          </Mrr>
          <Mtt>
            <value>-1.069723406e+16</value>
          </Mtt>
          <Mpp>
            <value>-4.254016978e+15</value>
          </Mpp>
          <Mrt>
            <value>-6.807395796e+15</value>
          </Mrt>
          <Mrp>
            <value>1.164127571e+15</value>
          </Mrp>
          <Mtp>
            <value>3.7613197e+16</value>
          </Mtp>
        </tensor>
        <varianceReduction>0.9555553547</varianceReduction>
        <doubleCouple>0.3614349054</doubleCouple>
        <clvd>0.6385650946</clvd>
        <greensFunctionID>sc3gf1d:/qseis_ak135f_continental</greensFunctionID>
        <filterID>BP 40s-100s</filterID>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T05:49:19.510573Z</creationTime>
        </creationInfo>
        <dataUsed>
          <waveType>body waves</waveType>
          <stationCount>7</stationCount>
          <componentCount>7</componentCount>
        </dataUsed>
        <dataUsed>
          <waveType>surface waves</waveType>
          <stationCount>1</stationCount>
          <componentCount>1</componentCount>
        </dataUsed>
        <phaseSetting code="L" lowerPeriod="60" upperPeriod="150" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="P" lowerPeriod="40" upperPeriod="100" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="R" lowerPeriod="60" upperPeriod="150" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="S" lowerPeriod="40" upperPeriod="100" minimumSNR="1" maximumTimeShift="10"/>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054919.511047.613516" active="true" weight="1">
          <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH">
            <resourceURI>G.SANVU</resourceURI>
          </waveformID>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-4</timeShift>
            <dataTimeWindow>54.01234519 155.3729706</dataTimeWindow>
            <misfit>0.04565406799</misfit>
            <snr>27.23679924</snr>
          </component>
          <component phaseCode="R" component="0" active="true">
            <weight>1</weight>
            <timeShift>-4</timeShift>
            <dataTimeWindow>95.90205638 234.3056838</dataTimeWindow>
            <misfit>0.02108886719</misfit>
            <snr>7.709475517</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.123312071</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>96.43622093 234.3056838</dataTimeWindow>
            <misfit>0.8464826965</misfit>
            <snr>2.305141687</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.2406651974</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>96.43622093 234.3056838</dataTimeWindow>
            <misfit>1</misfit>
            <snr>4.024114132</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054919.511125.613517" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BH">
            <resourceURI>IU.TARA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.007092492655</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>326.4520581 872.8422378</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.073470592</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>6</timeShift>
            <dataTimeWindow>205.7445822 306.9925842</dataTimeWindow>
            <misfit>0.4963869095</misfit>
            <snr>1.363570571</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>388.6334025 872.8422378</dataTimeWindow>
            <misfit>0.8863717079</misfit>
            <snr>4.783790112</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.003497791942</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>388.6334025 872.8422378</dataTimeWindow>
            <misfit>0.9628971267</misfit>
            <snr>2.31493187</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.004891633056</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>368.4220656 568.4220656</dataTimeWindow>
            <misfit>0.7426436996</misfit>
            <snr>0.6371799707</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.009299323894</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>368.4220656 568.4220656</dataTimeWindow>
            <misfit>1</misfit>
            <snr>0.82813555</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054919.511167.613518" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="EIDS" channelCode="BH">
            <resourceURI>AU.EIDS</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.1044915617</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>437.9514809 1154.246532</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.215333939</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>5</timeShift>
            <dataTimeWindow>266.9266902 369.2104797</dataTimeWindow>
            <misfit>0.2391711426</misfit>
            <snr>4.692846775</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>521.3708106 1154.246532</dataTimeWindow>
            <misfit>0.7552874374</misfit>
            <snr>1.796907663</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.08761250228</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>521.3708106 1154.246532</dataTimeWindow>
            <misfit>0.763740406</misfit>
            <snr>2.604088306</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.03381812572</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>488.3866849 688.3866849</dataTimeWindow>
            <misfit>0.3428308105</misfit>
            <snr>1.600562096</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.03597045317</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>488.3866849 688.3866849</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.383267522</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054919.511211.613519" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BH">
            <resourceURI>IU.CTAO</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.02272730693</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>465.9110441 1222.790913</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.926164627</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>1</timeShift>
            <dataTimeWindow>280.6353776 382.8960571</dataTimeWindow>
            <misfit>0.5432171631</misfit>
            <snr>7.60748148</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>554.6560049 1222.790913</dataTimeWindow>
            <misfit>0.5438781357</misfit>
            <snr>1.595830441</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.031402722</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>554.6560049 1222.790913</dataTimeWindow>
            <misfit>0.8834739971</misfit>
            <snr>1.921049237</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.01518514752</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>513.5637303 713.5637303</dataTimeWindow>
            <misfit>0.9644511843</misfit>
            <snr>0.9940682054</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.01513855904</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>513.5637303 713.5637303</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.104261518</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054919.511259.613520" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="ARMA" channelCode="BH">
            <resourceURI>AU.ARMA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.09928009659</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>509.9773866 1330.301247</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.778090715</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>1</timeShift>
            <dataTimeWindow>301.7205975 404.132782</dataTimeWindow>
            <misfit>0.3360371399</misfit>
            <snr>1.406597495</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.1159364 1330.301247</dataTimeWindow>
            <misfit>0.9172219753</misfit>
            <snr>2.54085803</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.04847697541</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.1159364 1330.301247</dataTimeWindow>
            <misfit>0.7893798637</misfit>
            <snr>2.103213787</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.02705101855</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>550.9146855 750.9146855</dataTimeWindow>
            <misfit>0.9281681299</misfit>
            <snr>2.0030725</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.04330698028</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>550.9146855 750.9146855</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.200439453</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054919.511293.613521" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="CMSA" channelCode="BH">
            <resourceURI>AU.CMSA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.2612291873</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>606.998931 1560.886236</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.590667725</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-2</timeShift>
            <dataTimeWindow>342.0220828 444.8430176</dataTimeWindow>
            <misfit>0.389816246</misfit>
            <snr>2.734752893</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>722.617775 1560.886236</dataTimeWindow>
            <misfit>0.6833830261</misfit>
            <snr>1.75257659</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.1859548837</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>722.617775 1560.886236</dataTimeWindow>
            <misfit>0.5847386932</misfit>
            <snr>2.04881382</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.09062115848</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>620.6214221 820.6214221</dataTimeWindow>
            <misfit>0.6709676361</misfit>
            <snr>1.990436196</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.112135753</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>620.6214221 820.6214221</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.806781292</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430054919.511325.613522" active="true" weight="1">
          <waveformID networkCode="G" stationCode="CAN" locationCode="00" channelCode="BH">
            <resourceURI>G.CAN</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>625.4319812 1604.400325</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.338611841</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-2</timeShift>
            <dataTimeWindow>349.3843523 452.2696228</dataTimeWindow>
            <misfit>0.1501287842</misfit>
            <snr>1.981680036</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>744.5618824 1604.400325</dataTimeWindow>
            <misfit>0.8057865715</misfit>
            <snr>1.965375543</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>744.5618824 1604.400325</dataTimeWindow>
            <misfit>0.8435315895</misfit>
            <snr>1.055261493</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>633.5868385 833.5868385</dataTimeWindow>
            <misfit>0.5141334915</misfit>
            <snr>1.668983817</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>633.5868385 833.5868385</dataTimeWindow>
            <misfit>1</misfit>
            <snr>4.945716381</snr>
          </component>
        </stationMomentTensorContribution>
      </momentTensor>
    </focalMechanism>
    <focalMechanism publicID="FocalMechanism/20210430055022.509599.613623">
      <triggeringOriginID>Origin/20210430053829.67918.86662</triggeringOriginID>
      <nodalPlanes>
        <nodalPlane1>
          <strike>
            <value>177.2639358</value>
          </strike>
          <dip>
            <value>76.65512867</value>
          </dip>
          <rake>
            <value>174.342645</value>
          </rake>
        </nodalPlane1>
        <nodalPlane2>
          <strike>
            <value>268.5737521</value>
          </strike>
          <dip>
            <value>84.49588082</value>
          </dip>
          <rake>
            <value>13.40783625</value>
          </rake>
        </nodalPlane2>
      </nodalPlanes>
      <principalAxes>
        <tAxis>
          <azimuth>
            <value>133.5856399</value>
          </azimuth>
          <plunge>
            <value>13.35786003</value>
          </plunge>
          <length>
            <value>3.116494748e+16</value>
          </length>
        </tAxis>
        <pAxis>
          <azimuth>
            <value>42.28184833</value>
          </azimuth>
          <plunge>
            <value>5.473468684</value>
          </plunge>
          <length>
            <value>-4.578250102e+16</value>
          </length>
        </pAxis>
        <nAxis>
          <azimuth>
            <value>290.4923551</value>
          </azimuth>
          <plunge>
            <value>75.5255386</value>
          </plunge>
          <length>
            <value>1.461755355e+16</value>
          </length>
        </nAxis>
      </principalAxes>
      <azimuthalGap>142.1660848</azimuthalGap>
      <misfit>0.04444464526</misfit>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T05:50:22.509494Z</creationTime>
      </creationInfo>
      <momentTensor publicID="MomentTensor/20210430055022.509883.613626">
        <derivedOriginID>Origin/20210430055022.509769.613624</derivedOriginID>
        <momentMagnitudeID>Magnitude/20210430055022.509822.613625</momentMagnitudeID>
        <scalarMoment>
          <value>4.050286547e+16</value>
        </scalarMoment>
        <tensor>
          <Mrr>
            <value>1.495125103e+16</value>
          </Mrr>
          <Mtt>
            <value>-1.069723406e+16</value>
          </Mtt>
          <Mpp>
            <value>-4.254016978e+15</value>
          </Mpp>
          <Mrt>
            <value>-6.807395796e+15</value>
          </Mrt>
          <Mrp>
            <value>1.164127571e+15</value>
          </Mrp>
          <Mtp>
            <value>3.7613197e+16</value>
          </Mtp>
        </tensor>
        <varianceReduction>0.9555553547</varianceReduction>
        <doubleCouple>0.3614349054</doubleCouple>
        <clvd>0.6385650946</clvd>
        <greensFunctionID>sc3gf1d:/qseis_ak135f_continental</greensFunctionID>
        <filterID>BP 40s-100s</filterID>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T05:50:22.509494Z</creationTime>
        </creationInfo>
        <dataUsed>
          <waveType>body waves</waveType>
          <stationCount>7</stationCount>
          <componentCount>7</componentCount>
        </dataUsed>
        <dataUsed>
          <waveType>surface waves</waveType>
          <stationCount>1</stationCount>
          <componentCount>1</componentCount>
        </dataUsed>
        <phaseSetting code="L" lowerPeriod="60" upperPeriod="150" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="P" lowerPeriod="40" upperPeriod="100" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="R" lowerPeriod="60" upperPeriod="150" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="S" lowerPeriod="40" upperPeriod="100" minimumSNR="1" maximumTimeShift="10"/>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055022.509973.613627" active="true" weight="1">
          <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH">
            <resourceURI>G.SANVU</resourceURI>
          </waveformID>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-4</timeShift>
            <dataTimeWindow>54.01234519 155.3729706</dataTimeWindow>
            <misfit>0.04565406799</misfit>
            <snr>27.23679924</snr>
          </component>
          <component phaseCode="R" component="0" active="true">
            <weight>1</weight>
            <timeShift>-4</timeShift>
            <dataTimeWindow>95.90205638 234.3056838</dataTimeWindow>
            <misfit>0.02108886719</misfit>
            <snr>7.709475517</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.123312071</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>96.43622093 234.3056838</dataTimeWindow>
            <misfit>0.8464826965</misfit>
            <snr>2.305141687</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.2406651974</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>96.43622093 234.3056838</dataTimeWindow>
            <misfit>1</misfit>
            <snr>4.024114132</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055022.510054.613628" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BH">
            <resourceURI>IU.TARA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.007092492655</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>326.4520581 872.8422378</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.073470592</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>6</timeShift>
            <dataTimeWindow>205.7445822 306.9925842</dataTimeWindow>
            <misfit>0.4963869095</misfit>
            <snr>1.363570571</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>388.6334025 872.8422378</dataTimeWindow>
            <misfit>0.8863717079</misfit>
            <snr>4.783790112</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.003497791942</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>388.6334025 872.8422378</dataTimeWindow>
            <misfit>0.9628971267</misfit>
            <snr>2.31493187</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.004891633056</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>368.4220656 568.4220656</dataTimeWindow>
            <misfit>0.7426436996</misfit>
            <snr>0.6371799707</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.009299323894</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>368.4220656 568.4220656</dataTimeWindow>
            <misfit>1</misfit>
            <snr>0.82813555</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055022.510104.613629" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="EIDS" channelCode="BH">
            <resourceURI>AU.EIDS</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.1044915617</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>437.9514809 1154.246532</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.215333939</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>5</timeShift>
            <dataTimeWindow>266.9266902 369.2104797</dataTimeWindow>
            <misfit>0.2391711426</misfit>
            <snr>4.692846775</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>521.3708106 1154.246532</dataTimeWindow>
            <misfit>0.7552874374</misfit>
            <snr>1.796907663</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.08761250228</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>521.3708106 1154.246532</dataTimeWindow>
            <misfit>0.763740406</misfit>
            <snr>2.604088306</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.03381812572</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>488.3866849 688.3866849</dataTimeWindow>
            <misfit>0.3428308105</misfit>
            <snr>1.600562096</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.03597045317</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>488.3866849 688.3866849</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.383267522</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055022.510147.613630" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BH">
            <resourceURI>IU.CTAO</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.02272730693</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>465.9110441 1222.790913</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.926164627</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>1</timeShift>
            <dataTimeWindow>280.6353776 382.8960571</dataTimeWindow>
            <misfit>0.5432171631</misfit>
            <snr>7.60748148</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>554.6560049 1222.790913</dataTimeWindow>
            <misfit>0.5438781357</misfit>
            <snr>1.595830441</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.031402722</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>554.6560049 1222.790913</dataTimeWindow>
            <misfit>0.8834739971</misfit>
            <snr>1.921049237</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.01518514752</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>513.5637303 713.5637303</dataTimeWindow>
            <misfit>0.9644511843</misfit>
            <snr>0.9940682054</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.01513855904</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>513.5637303 713.5637303</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.104261518</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055022.510185.613631" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="ARMA" channelCode="BH">
            <resourceURI>AU.ARMA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.09928009659</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>509.9773866 1330.301247</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.778090715</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>1</timeShift>
            <dataTimeWindow>301.7205975 404.132782</dataTimeWindow>
            <misfit>0.3360371399</misfit>
            <snr>1.406597495</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.1159364 1330.301247</dataTimeWindow>
            <misfit>0.9172219753</misfit>
            <snr>2.54085803</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.04847697541</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.1159364 1330.301247</dataTimeWindow>
            <misfit>0.7893798637</misfit>
            <snr>2.103213787</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.02705101855</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>550.9146855 750.9146855</dataTimeWindow>
            <misfit>0.9281681299</misfit>
            <snr>2.0030725</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.04330698028</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>550.9146855 750.9146855</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.200439453</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055022.510229.613632" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="CMSA" channelCode="BH">
            <resourceURI>AU.CMSA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.2612291873</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>606.998931 1560.886236</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.590667725</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-2</timeShift>
            <dataTimeWindow>342.0220828 444.8430176</dataTimeWindow>
            <misfit>0.389816246</misfit>
            <snr>2.734752893</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>722.617775 1560.886236</dataTimeWindow>
            <misfit>0.6833830261</misfit>
            <snr>1.75257659</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.1859548837</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>722.617775 1560.886236</dataTimeWindow>
            <misfit>0.5847386932</misfit>
            <snr>2.04881382</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.09062115848</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>620.6214221 820.6214221</dataTimeWindow>
            <misfit>0.6709676361</misfit>
            <snr>1.990436196</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.112135753</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>620.6214221 820.6214221</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.806781292</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055022.510273.613633" active="true" weight="1">
          <waveformID networkCode="G" stationCode="CAN" locationCode="00" channelCode="BH">
            <resourceURI>G.CAN</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>625.4319812 1604.400325</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.338611841</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-2</timeShift>
            <dataTimeWindow>349.3843523 452.2696228</dataTimeWindow>
            <misfit>0.1501287842</misfit>
            <snr>1.981680036</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>744.5618824 1604.400325</dataTimeWindow>
            <misfit>0.8057865715</misfit>
            <snr>1.965375543</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>744.5618824 1604.400325</dataTimeWindow>
            <misfit>0.8435315895</misfit>
            <snr>1.055261493</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>633.5868385 833.5868385</dataTimeWindow>
            <misfit>0.5141334915</misfit>
            <snr>1.668983817</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>633.5868385 833.5868385</dataTimeWindow>
            <misfit>1</misfit>
            <snr>4.945716381</snr>
          </component>
        </stationMomentTensorContribution>
      </momentTensor>
    </focalMechanism>
    <focalMechanism publicID="FocalMechanism/20210430055105.174854.613660">
      <triggeringOriginID>Origin/20210430053829.67918.86662</triggeringOriginID>
      <nodalPlanes>
        <nodalPlane1>
          <strike>
            <value>177.2639358</value>
          </strike>
          <dip>
            <value>76.65512867</value>
          </dip>
          <rake>
            <value>174.342645</value>
          </rake>
        </nodalPlane1>
        <nodalPlane2>
          <strike>
            <value>268.5737521</value>
          </strike>
          <dip>
            <value>84.49588082</value>
          </dip>
          <rake>
            <value>13.40783625</value>
          </rake>
        </nodalPlane2>
      </nodalPlanes>
      <principalAxes>
        <tAxis>
          <azimuth>
            <value>133.5856399</value>
          </azimuth>
          <plunge>
            <value>13.35786003</value>
          </plunge>
          <length>
            <value>3.116494748e+16</value>
          </length>
        </tAxis>
        <pAxis>
          <azimuth>
            <value>42.28184833</value>
          </azimuth>
          <plunge>
            <value>5.473468684</value>
          </plunge>
          <length>
            <value>-4.578250102e+16</value>
          </length>
        </pAxis>
        <nAxis>
          <azimuth>
            <value>290.4923551</value>
          </azimuth>
          <plunge>
            <value>75.5255386</value>
          </plunge>
          <length>
            <value>1.461755355e+16</value>
          </length>
        </nAxis>
      </principalAxes>
      <azimuthalGap>142.1660848</azimuthalGap>
      <misfit>0.04444464526</misfit>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T05:51:05.174557Z</creationTime>
      </creationInfo>
      <momentTensor publicID="MomentTensor/20210430055105.1753.613663">
        <derivedOriginID>Origin/20210430055105.175114.613661</derivedOriginID>
        <momentMagnitudeID>Magnitude/20210430055105.175223.613662</momentMagnitudeID>
        <scalarMoment>
          <value>4.050286547e+16</value>
        </scalarMoment>
        <tensor>
          <Mrr>
            <value>1.495125103e+16</value>
          </Mrr>
          <Mtt>
            <value>-1.069723406e+16</value>
          </Mtt>
          <Mpp>
            <value>-4.254016978e+15</value>
          </Mpp>
          <Mrt>
            <value>-6.807395796e+15</value>
          </Mrt>
          <Mrp>
            <value>1.164127571e+15</value>
          </Mrp>
          <Mtp>
            <value>3.7613197e+16</value>
          </Mtp>
        </tensor>
        <varianceReduction>0.9555553547</varianceReduction>
        <doubleCouple>0.3614349054</doubleCouple>
        <clvd>0.6385650946</clvd>
        <greensFunctionID>sc3gf1d:/qseis_ak135f_continental</greensFunctionID>
        <filterID>BP 40s-100s</filterID>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T05:51:05.174557Z</creationTime>
        </creationInfo>
        <dataUsed>
          <waveType>body waves</waveType>
          <stationCount>7</stationCount>
          <componentCount>7</componentCount>
        </dataUsed>
        <dataUsed>
          <waveType>surface waves</waveType>
          <stationCount>1</stationCount>
          <componentCount>1</componentCount>
        </dataUsed>
        <phaseSetting code="L" lowerPeriod="60" upperPeriod="150" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="P" lowerPeriod="40" upperPeriod="100" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="R" lowerPeriod="60" upperPeriod="150" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="S" lowerPeriod="40" upperPeriod="100" minimumSNR="1" maximumTimeShift="10"/>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055105.175412.613664" active="true" weight="1">
          <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH">
            <resourceURI>G.SANVU</resourceURI>
          </waveformID>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-4</timeShift>
            <dataTimeWindow>54.01234519 155.3729706</dataTimeWindow>
            <misfit>0.04565406799</misfit>
            <snr>27.23679924</snr>
          </component>
          <component phaseCode="R" component="0" active="true">
            <weight>1</weight>
            <timeShift>-4</timeShift>
            <dataTimeWindow>95.90205638 234.3056838</dataTimeWindow>
            <misfit>0.02108886719</misfit>
            <snr>7.709475517</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.123312071</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>96.43622093 234.3056838</dataTimeWindow>
            <misfit>0.8464826965</misfit>
            <snr>2.305141687</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.2406651974</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>96.43622093 234.3056838</dataTimeWindow>
            <misfit>1</misfit>
            <snr>4.024114132</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055105.175506.613665" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BH">
            <resourceURI>IU.TARA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.007092492655</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>326.4520581 872.8422378</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.073470592</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>6</timeShift>
            <dataTimeWindow>205.7445822 306.9925842</dataTimeWindow>
            <misfit>0.4963869095</misfit>
            <snr>1.363570571</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>388.6334025 872.8422378</dataTimeWindow>
            <misfit>0.8863717079</misfit>
            <snr>4.783790112</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.003497791942</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>388.6334025 872.8422378</dataTimeWindow>
            <misfit>0.9628971267</misfit>
            <snr>2.31493187</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.004891633056</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>368.4220656 568.4220656</dataTimeWindow>
            <misfit>0.7426436996</misfit>
            <snr>0.6371799707</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.009299323894</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>368.4220656 568.4220656</dataTimeWindow>
            <misfit>1</misfit>
            <snr>0.82813555</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055105.17556.613666" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="EIDS" channelCode="BH">
            <resourceURI>AU.EIDS</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.1044915617</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>437.9514809 1154.246532</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.215333939</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>5</timeShift>
            <dataTimeWindow>266.9266902 369.2104797</dataTimeWindow>
            <misfit>0.2391711426</misfit>
            <snr>4.692846775</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>521.3708106 1154.246532</dataTimeWindow>
            <misfit>0.7552874374</misfit>
            <snr>1.796907663</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.08761250228</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>521.3708106 1154.246532</dataTimeWindow>
            <misfit>0.763740406</misfit>
            <snr>2.604088306</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.03381812572</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>488.3866849 688.3866849</dataTimeWindow>
            <misfit>0.3428308105</misfit>
            <snr>1.600562096</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.03597045317</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>488.3866849 688.3866849</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.383267522</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055105.175608.613667" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BH">
            <resourceURI>IU.CTAO</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.02272730693</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>465.9110441 1222.790913</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.926164627</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>1</timeShift>
            <dataTimeWindow>280.6353776 382.8960571</dataTimeWindow>
            <misfit>0.5432171631</misfit>
            <snr>7.60748148</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>554.6560049 1222.790913</dataTimeWindow>
            <misfit>0.5438781357</misfit>
            <snr>1.595830441</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.031402722</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>554.6560049 1222.790913</dataTimeWindow>
            <misfit>0.8834739971</misfit>
            <snr>1.921049237</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.01518514752</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>513.5637303 713.5637303</dataTimeWindow>
            <misfit>0.9644511843</misfit>
            <snr>0.9940682054</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.01513855904</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>513.5637303 713.5637303</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.104261518</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055105.175645.613668" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="ARMA" channelCode="BH">
            <resourceURI>AU.ARMA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.09928009659</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>509.9773866 1330.301247</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.778090715</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>1</timeShift>
            <dataTimeWindow>301.7205975 404.132782</dataTimeWindow>
            <misfit>0.3360371399</misfit>
            <snr>1.406597495</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.1159364 1330.301247</dataTimeWindow>
            <misfit>0.9172219753</misfit>
            <snr>2.54085803</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.04847697541</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.1159364 1330.301247</dataTimeWindow>
            <misfit>0.7893798637</misfit>
            <snr>2.103213787</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.02705101855</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>550.9146855 750.9146855</dataTimeWindow>
            <misfit>0.9281681299</misfit>
            <snr>2.0030725</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.04330698028</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>550.9146855 750.9146855</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.200439453</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055105.175694.613669" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="CMSA" channelCode="BH">
            <resourceURI>AU.CMSA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.2612291873</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>606.998931 1560.886236</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.590667725</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-2</timeShift>
            <dataTimeWindow>342.0220828 444.8430176</dataTimeWindow>
            <misfit>0.389816246</misfit>
            <snr>2.734752893</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>722.617775 1560.886236</dataTimeWindow>
            <misfit>0.6833830261</misfit>
            <snr>1.75257659</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.1859548837</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>722.617775 1560.886236</dataTimeWindow>
            <misfit>0.5847386932</misfit>
            <snr>2.04881382</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.09062115848</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>620.6214221 820.6214221</dataTimeWindow>
            <misfit>0.6709676361</misfit>
            <snr>1.990436196</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.112135753</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>620.6214221 820.6214221</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.806781292</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055105.175728.613670" active="true" weight="1">
          <waveformID networkCode="G" stationCode="CAN" locationCode="00" channelCode="BH">
            <resourceURI>G.CAN</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>625.4319812 1604.400325</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.338611841</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-2</timeShift>
            <dataTimeWindow>349.3843523 452.2696228</dataTimeWindow>
            <misfit>0.1501287842</misfit>
            <snr>1.981680036</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>744.5618824 1604.400325</dataTimeWindow>
            <misfit>0.8057865715</misfit>
            <snr>1.965375543</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>744.5618824 1604.400325</dataTimeWindow>
            <misfit>0.8435315895</misfit>
            <snr>1.055261493</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>633.5868385 833.5868385</dataTimeWindow>
            <misfit>0.5141334915</misfit>
            <snr>1.668983817</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>633.5868385 833.5868385</dataTimeWindow>
            <misfit>1</misfit>
            <snr>4.945716381</snr>
          </component>
        </stationMomentTensorContribution>
      </momentTensor>
    </focalMechanism>
    <focalMechanism publicID="FocalMechanism/20210430055144.024719.613676">
      <triggeringOriginID>Origin/20210430053829.67918.86662</triggeringOriginID>
      <nodalPlanes>
        <nodalPlane1>
          <strike>
            <value>177.2639358</value>
          </strike>
          <dip>
            <value>76.65512867</value>
          </dip>
          <rake>
            <value>174.342645</value>
          </rake>
        </nodalPlane1>
        <nodalPlane2>
          <strike>
            <value>268.5737521</value>
          </strike>
          <dip>
            <value>84.49588082</value>
          </dip>
          <rake>
            <value>13.40783625</value>
          </rake>
        </nodalPlane2>
      </nodalPlanes>
      <principalAxes>
        <tAxis>
          <azimuth>
            <value>133.5856399</value>
          </azimuth>
          <plunge>
            <value>13.35786003</value>
          </plunge>
          <length>
            <value>3.116494748e+16</value>
          </length>
        </tAxis>
        <pAxis>
          <azimuth>
            <value>42.28184833</value>
          </azimuth>
          <plunge>
            <value>5.473468684</value>
          </plunge>
          <length>
            <value>-4.578250102e+16</value>
          </length>
        </pAxis>
        <nAxis>
          <azimuth>
            <value>290.4923551</value>
          </azimuth>
          <plunge>
            <value>75.5255386</value>
          </plunge>
          <length>
            <value>1.461755355e+16</value>
          </length>
        </nAxis>
      </principalAxes>
      <azimuthalGap>142.1660848</azimuthalGap>
      <misfit>0.04444464526</misfit>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T05:51:44.024554Z</creationTime>
      </creationInfo>
      <momentTensor publicID="MomentTensor/20210430055144.025093.613679">
        <derivedOriginID>Origin/20210430055144.024927.613677</derivedOriginID>
        <momentMagnitudeID>Magnitude/20210430055144.02503.613678</momentMagnitudeID>
        <scalarMoment>
          <value>4.050286547e+16</value>
        </scalarMoment>
        <tensor>
          <Mrr>
            <value>1.495125103e+16</value>
          </Mrr>
          <Mtt>
            <value>-1.069723406e+16</value>
          </Mtt>
          <Mpp>
            <value>-4.254016978e+15</value>
          </Mpp>
          <Mrt>
            <value>-6.807395796e+15</value>
          </Mrt>
          <Mrp>
            <value>1.164127571e+15</value>
          </Mrp>
          <Mtp>
            <value>3.7613197e+16</value>
          </Mtp>
        </tensor>
        <varianceReduction>0.9555553547</varianceReduction>
        <doubleCouple>0.3614349054</doubleCouple>
        <clvd>0.6385650946</clvd>
        <greensFunctionID>sc3gf1d:/qseis_ak135f_continental</greensFunctionID>
        <filterID>BP 40s-100s</filterID>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T05:51:44.024554Z</creationTime>
        </creationInfo>
        <dataUsed>
          <waveType>body waves</waveType>
          <stationCount>7</stationCount>
          <componentCount>7</componentCount>
        </dataUsed>
        <dataUsed>
          <waveType>surface waves</waveType>
          <stationCount>1</stationCount>
          <componentCount>1</componentCount>
        </dataUsed>
        <phaseSetting code="L" lowerPeriod="60" upperPeriod="150" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="P" lowerPeriod="40" upperPeriod="100" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="R" lowerPeriod="60" upperPeriod="150" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="S" lowerPeriod="40" upperPeriod="100" minimumSNR="1" maximumTimeShift="10"/>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055144.025194.613680" active="true" weight="1">
          <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH">
            <resourceURI>G.SANVU</resourceURI>
          </waveformID>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-4</timeShift>
            <dataTimeWindow>54.01234519 155.3729706</dataTimeWindow>
            <misfit>0.04565406799</misfit>
            <snr>27.23679924</snr>
          </component>
          <component phaseCode="R" component="0" active="true">
            <weight>1</weight>
            <timeShift>-4</timeShift>
            <dataTimeWindow>95.90205638 234.3056838</dataTimeWindow>
            <misfit>0.02108886719</misfit>
            <snr>7.709475517</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.123312071</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>96.43622093 234.3056838</dataTimeWindow>
            <misfit>0.8464826965</misfit>
            <snr>2.305141687</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.2406651974</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>96.43622093 234.3056838</dataTimeWindow>
            <misfit>1</misfit>
            <snr>4.024114132</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055144.025273.613681" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BH">
            <resourceURI>IU.TARA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.007092492655</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>326.4520581 872.8422378</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.073470592</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>6</timeShift>
            <dataTimeWindow>205.7445822 306.9925842</dataTimeWindow>
            <misfit>0.4963869095</misfit>
            <snr>1.363570571</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>388.6334025 872.8422378</dataTimeWindow>
            <misfit>0.8863717079</misfit>
            <snr>4.783790112</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.003497791942</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>388.6334025 872.8422378</dataTimeWindow>
            <misfit>0.9628971267</misfit>
            <snr>2.31493187</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.004891633056</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>368.4220656 568.4220656</dataTimeWindow>
            <misfit>0.7426436996</misfit>
            <snr>0.6371799707</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.009299323894</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>368.4220656 568.4220656</dataTimeWindow>
            <misfit>1</misfit>
            <snr>0.82813555</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055144.025322.613682" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="EIDS" channelCode="BH">
            <resourceURI>AU.EIDS</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.1044915617</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>437.9514809 1154.246532</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.215333939</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>5</timeShift>
            <dataTimeWindow>266.9266902 369.2104797</dataTimeWindow>
            <misfit>0.2391711426</misfit>
            <snr>4.692846775</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>521.3708106 1154.246532</dataTimeWindow>
            <misfit>0.7552874374</misfit>
            <snr>1.796907663</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.08761250228</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>521.3708106 1154.246532</dataTimeWindow>
            <misfit>0.763740406</misfit>
            <snr>2.604088306</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.03381812572</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>488.3866849 688.3866849</dataTimeWindow>
            <misfit>0.3428308105</misfit>
            <snr>1.600562096</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.03597045317</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>488.3866849 688.3866849</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.383267522</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055144.025366.613683" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BH">
            <resourceURI>IU.CTAO</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.02272730693</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>465.9110441 1222.790913</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.926164627</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>1</timeShift>
            <dataTimeWindow>280.6353776 382.8960571</dataTimeWindow>
            <misfit>0.5432171631</misfit>
            <snr>7.60748148</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>554.6560049 1222.790913</dataTimeWindow>
            <misfit>0.5438781357</misfit>
            <snr>1.595830441</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.031402722</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>554.6560049 1222.790913</dataTimeWindow>
            <misfit>0.8834739971</misfit>
            <snr>1.921049237</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.01518514752</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>513.5637303 713.5637303</dataTimeWindow>
            <misfit>0.9644511843</misfit>
            <snr>0.9940682054</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.01513855904</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>513.5637303 713.5637303</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.104261518</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055144.02541.613684" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="ARMA" channelCode="BH">
            <resourceURI>AU.ARMA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.09928009659</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>509.9773866 1330.301247</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.778090715</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>1</timeShift>
            <dataTimeWindow>301.7205975 404.132782</dataTimeWindow>
            <misfit>0.3360371399</misfit>
            <snr>1.406597495</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.1159364 1330.301247</dataTimeWindow>
            <misfit>0.9172219753</misfit>
            <snr>2.54085803</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.04847697541</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.1159364 1330.301247</dataTimeWindow>
            <misfit>0.7893798637</misfit>
            <snr>2.103213787</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.02705101855</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>550.9146855 750.9146855</dataTimeWindow>
            <misfit>0.9281681299</misfit>
            <snr>2.0030725</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.04330698028</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>550.9146855 750.9146855</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.200439453</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055144.025456.613685" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="CMSA" channelCode="BH">
            <resourceURI>AU.CMSA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.2612291873</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>606.998931 1560.886236</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.590667725</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-2</timeShift>
            <dataTimeWindow>342.0220828 444.8430176</dataTimeWindow>
            <misfit>0.389816246</misfit>
            <snr>2.734752893</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>722.617775 1560.886236</dataTimeWindow>
            <misfit>0.6833830261</misfit>
            <snr>1.75257659</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.1859548837</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>722.617775 1560.886236</dataTimeWindow>
            <misfit>0.5847386932</misfit>
            <snr>2.04881382</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.09062115848</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>620.6214221 820.6214221</dataTimeWindow>
            <misfit>0.6709676361</misfit>
            <snr>1.990436196</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.112135753</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>620.6214221 820.6214221</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.806781292</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055144.025511.613686" active="true" weight="1">
          <waveformID networkCode="G" stationCode="CAN" locationCode="00" channelCode="BH">
            <resourceURI>G.CAN</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>625.4319812 1604.400325</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.338611841</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-2</timeShift>
            <dataTimeWindow>349.3843523 452.2696228</dataTimeWindow>
            <misfit>0.1501287842</misfit>
            <snr>1.981680036</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>744.5618824 1604.400325</dataTimeWindow>
            <misfit>0.8057865715</misfit>
            <snr>1.965375543</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>744.5618824 1604.400325</dataTimeWindow>
            <misfit>0.8435315895</misfit>
            <snr>1.055261493</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>633.5868385 833.5868385</dataTimeWindow>
            <misfit>0.5141334915</misfit>
            <snr>1.668983817</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>633.5868385 833.5868385</dataTimeWindow>
            <misfit>1</misfit>
            <snr>4.945716381</snr>
          </component>
        </stationMomentTensorContribution>
      </momentTensor>
    </focalMechanism>
    <focalMechanism publicID="FocalMechanism/20210430055224.677725.613689">
      <triggeringOriginID>Origin/20210430053829.67918.86662</triggeringOriginID>
      <nodalPlanes>
        <nodalPlane1>
          <strike>
            <value>177.2639358</value>
          </strike>
          <dip>
            <value>76.65512867</value>
          </dip>
          <rake>
            <value>174.342645</value>
          </rake>
        </nodalPlane1>
        <nodalPlane2>
          <strike>
            <value>268.5737521</value>
          </strike>
          <dip>
            <value>84.49588082</value>
          </dip>
          <rake>
            <value>13.40783625</value>
          </rake>
        </nodalPlane2>
      </nodalPlanes>
      <principalAxes>
        <tAxis>
          <azimuth>
            <value>133.5856399</value>
          </azimuth>
          <plunge>
            <value>13.35786003</value>
          </plunge>
          <length>
            <value>3.116494748e+16</value>
          </length>
        </tAxis>
        <pAxis>
          <azimuth>
            <value>42.28184833</value>
          </azimuth>
          <plunge>
            <value>5.473468684</value>
          </plunge>
          <length>
            <value>-4.578250102e+16</value>
          </length>
        </pAxis>
        <nAxis>
          <azimuth>
            <value>290.4923551</value>
          </azimuth>
          <plunge>
            <value>75.5255386</value>
          </plunge>
          <length>
            <value>1.461755355e+16</value>
          </length>
        </nAxis>
      </principalAxes>
      <azimuthalGap>142.1660848</azimuthalGap>
      <misfit>0.04444464526</misfit>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T05:52:24.677544Z</creationTime>
      </creationInfo>
      <momentTensor publicID="MomentTensor/20210430055224.678121.613692">
        <derivedOriginID>Origin/20210430055224.677935.613690</derivedOriginID>
        <momentMagnitudeID>Magnitude/20210430055224.678039.613691</momentMagnitudeID>
        <scalarMoment>
          <value>4.050286547e+16</value>
        </scalarMoment>
        <tensor>
          <Mrr>
            <value>1.495125103e+16</value>
          </Mrr>
          <Mtt>
            <value>-1.069723406e+16</value>
          </Mtt>
          <Mpp>
            <value>-4.254016978e+15</value>
          </Mpp>
          <Mrt>
            <value>-6.807395796e+15</value>
          </Mrt>
          <Mrp>
            <value>1.164127571e+15</value>
          </Mrp>
          <Mtp>
            <value>3.7613197e+16</value>
          </Mtp>
        </tensor>
        <varianceReduction>0.9555553547</varianceReduction>
        <doubleCouple>0.3614349054</doubleCouple>
        <clvd>0.6385650946</clvd>
        <greensFunctionID>sc3gf1d:/qseis_ak135f_continental</greensFunctionID>
        <filterID>BP 40s-100s</filterID>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T05:52:24.677544Z</creationTime>
        </creationInfo>
        <dataUsed>
          <waveType>body waves</waveType>
          <stationCount>7</stationCount>
          <componentCount>7</componentCount>
        </dataUsed>
        <dataUsed>
          <waveType>surface waves</waveType>
          <stationCount>1</stationCount>
          <componentCount>1</componentCount>
        </dataUsed>
        <phaseSetting code="L" lowerPeriod="60" upperPeriod="150" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="P" lowerPeriod="40" upperPeriod="100" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="R" lowerPeriod="60" upperPeriod="150" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="S" lowerPeriod="40" upperPeriod="100" minimumSNR="1" maximumTimeShift="10"/>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055224.67821.613693" active="true" weight="1">
          <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH">
            <resourceURI>G.SANVU</resourceURI>
          </waveformID>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-4</timeShift>
            <dataTimeWindow>54.01234519 155.3729706</dataTimeWindow>
            <misfit>0.04565406799</misfit>
            <snr>27.23679924</snr>
          </component>
          <component phaseCode="R" component="0" active="true">
            <weight>1</weight>
            <timeShift>-4</timeShift>
            <dataTimeWindow>95.90205638 234.3056838</dataTimeWindow>
            <misfit>0.02108886719</misfit>
            <snr>7.709475517</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.123312071</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>96.43622093 234.3056838</dataTimeWindow>
            <misfit>0.8464826965</misfit>
            <snr>2.305141687</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.2406651974</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>96.43622093 234.3056838</dataTimeWindow>
            <misfit>1</misfit>
            <snr>4.024114132</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055224.678289.613694" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BH">
            <resourceURI>IU.TARA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.007092492655</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>326.4520581 872.8422378</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.073470592</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>6</timeShift>
            <dataTimeWindow>205.7445822 306.9925842</dataTimeWindow>
            <misfit>0.4963869095</misfit>
            <snr>1.363570571</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>388.6334025 872.8422378</dataTimeWindow>
            <misfit>0.8863717079</misfit>
            <snr>4.783790112</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.003497791942</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>388.6334025 872.8422378</dataTimeWindow>
            <misfit>0.9628971267</misfit>
            <snr>2.31493187</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.004891633056</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>368.4220656 568.4220656</dataTimeWindow>
            <misfit>0.7426436996</misfit>
            <snr>0.6371799707</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.009299323894</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>368.4220656 568.4220656</dataTimeWindow>
            <misfit>1</misfit>
            <snr>0.82813555</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055224.678331.613695" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="EIDS" channelCode="BH">
            <resourceURI>AU.EIDS</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.1044915617</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>437.9514809 1154.246532</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.215333939</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>5</timeShift>
            <dataTimeWindow>266.9266902 369.2104797</dataTimeWindow>
            <misfit>0.2391711426</misfit>
            <snr>4.692846775</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>521.3708106 1154.246532</dataTimeWindow>
            <misfit>0.7552874374</misfit>
            <snr>1.796907663</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.08761250228</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>521.3708106 1154.246532</dataTimeWindow>
            <misfit>0.763740406</misfit>
            <snr>2.604088306</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.03381812572</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>488.3866849 688.3866849</dataTimeWindow>
            <misfit>0.3428308105</misfit>
            <snr>1.600562096</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.03597045317</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>488.3866849 688.3866849</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.383267522</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055224.678371.613696" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BH">
            <resourceURI>IU.CTAO</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.02272730693</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>465.9110441 1222.790913</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.926164627</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>1</timeShift>
            <dataTimeWindow>280.6353776 382.8960571</dataTimeWindow>
            <misfit>0.5432171631</misfit>
            <snr>7.60748148</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>554.6560049 1222.790913</dataTimeWindow>
            <misfit>0.5438781357</misfit>
            <snr>1.595830441</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.031402722</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>554.6560049 1222.790913</dataTimeWindow>
            <misfit>0.8834739971</misfit>
            <snr>1.921049237</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.01518514752</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>513.5637303 713.5637303</dataTimeWindow>
            <misfit>0.9644511843</misfit>
            <snr>0.9940682054</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.01513855904</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>513.5637303 713.5637303</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.104261518</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055224.678409.613697" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="ARMA" channelCode="BH">
            <resourceURI>AU.ARMA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.09928009659</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>509.9773866 1330.301247</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.778090715</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>1</timeShift>
            <dataTimeWindow>301.7205975 404.132782</dataTimeWindow>
            <misfit>0.3360371399</misfit>
            <snr>1.406597495</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.1159364 1330.301247</dataTimeWindow>
            <misfit>0.9172219753</misfit>
            <snr>2.54085803</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.04847697541</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.1159364 1330.301247</dataTimeWindow>
            <misfit>0.7893798637</misfit>
            <snr>2.103213787</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.02705101855</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>550.9146855 750.9146855</dataTimeWindow>
            <misfit>0.9281681299</misfit>
            <snr>2.0030725</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.04330698028</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>550.9146855 750.9146855</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.200439453</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055224.678449.613698" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="CMSA" channelCode="BH">
            <resourceURI>AU.CMSA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.2612291873</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>606.998931 1560.886236</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.590667725</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-2</timeShift>
            <dataTimeWindow>342.0220828 444.8430176</dataTimeWindow>
            <misfit>0.389816246</misfit>
            <snr>2.734752893</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>722.617775 1560.886236</dataTimeWindow>
            <misfit>0.6833830261</misfit>
            <snr>1.75257659</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.1859548837</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>722.617775 1560.886236</dataTimeWindow>
            <misfit>0.5847386932</misfit>
            <snr>2.04881382</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.09062115848</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>620.6214221 820.6214221</dataTimeWindow>
            <misfit>0.6709676361</misfit>
            <snr>1.990436196</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.112135753</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>620.6214221 820.6214221</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.806781292</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055224.678539.613699" active="true" weight="1">
          <waveformID networkCode="G" stationCode="CAN" locationCode="00" channelCode="BH">
            <resourceURI>G.CAN</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>625.4319812 1604.400325</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.338611841</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-2</timeShift>
            <dataTimeWindow>349.3843523 452.2696228</dataTimeWindow>
            <misfit>0.1501287842</misfit>
            <snr>1.981680036</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>744.5618824 1604.400325</dataTimeWindow>
            <misfit>0.8057865715</misfit>
            <snr>1.965375543</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>744.5618824 1604.400325</dataTimeWindow>
            <misfit>0.8435315895</misfit>
            <snr>1.055261493</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>633.5868385 833.5868385</dataTimeWindow>
            <misfit>0.5141334915</misfit>
            <snr>1.668983817</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>633.5868385 833.5868385</dataTimeWindow>
            <misfit>1</misfit>
            <snr>4.945716381</snr>
          </component>
        </stationMomentTensorContribution>
      </momentTensor>
    </focalMechanism>
    <focalMechanism publicID="FocalMechanism/20210430055237.798688.613702">
      <triggeringOriginID>Origin/20210430053829.67918.86662</triggeringOriginID>
      <nodalPlanes>
        <nodalPlane1>
          <strike>
            <value>164.8829495</value>
          </strike>
          <dip>
            <value>70.3692766</value>
          </dip>
          <rake>
            <value>-10.82922764</value>
          </rake>
        </nodalPlane1>
        <nodalPlane2>
          <strike>
            <value>258.5599946</value>
          </strike>
          <dip>
            <value>79.80713553</value>
          </dip>
          <rake>
            <value>-160.0412282</value>
          </rake>
        </nodalPlane2>
      </nodalPlanes>
      <principalAxes>
        <tAxis>
          <azimuth>
            <value>30.54048323</value>
          </azimuth>
          <plunge>
            <value>6.455188247</value>
          </plunge>
          <length>
            <value>4.139941267e+16</value>
          </length>
        </tAxis>
        <pAxis>
          <azimuth>
            <value>123.0642651</value>
          </azimuth>
          <plunge>
            <value>21.26539152</value>
          </plunge>
          <length>
            <value>-2.401444327e+16</value>
          </length>
        </pAxis>
        <nAxis>
          <azimuth>
            <value>284.5395181</value>
          </azimuth>
          <plunge>
            <value>67.68405004</value>
          </plunge>
          <length>
            <value>-1.738496941e+16</value>
          </length>
        </nAxis>
      </principalAxes>
      <azimuthalGap>239.9937659</azimuthalGap>
      <misfit>0.05122932626</misfit>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T05:52:37.798509Z</creationTime>
      </creationInfo>
      <momentTensor publicID="MomentTensor/20210430055237.798925.613705">
        <derivedOriginID>Origin/20210430055237.798824.613703</derivedOriginID>
        <momentMagnitudeID>Magnitude/20210430055237.798878.613704</momentMagnitudeID>
        <scalarMoment>
          <value>3.600584686e+16</value>
        </scalarMoment>
        <tensor>
          <Mrr>
            <value>-1.751401679e+16</value>
          </Mrr>
          <Mtt>
            <value>2.395560652e+16</value>
          </Mtt>
          <Mpp>
            <value>-6.441589734e+15</value>
          </Mpp>
          <Mrt>
            <value>6.878449846e+15</value>
          </Mrt>
          <Mrp>
            <value>-1.459147913e+15</value>
          </Mrp>
          <Mtp>
            <value>-2.803436909e+16</value>
          </Mtp>
        </tensor>
        <varianceReduction>0.9487706737</varianceReduction>
        <doubleCouple>0.1601344906</doubleCouple>
        <clvd>0.8398655094</clvd>
        <greensFunctionID>sc3gf1d:/qseis_ak135f_continental</greensFunctionID>
        <filterID>BP 40s-100s</filterID>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T05:52:37.798509Z</creationTime>
        </creationInfo>
        <dataUsed>
          <waveType>body waves</waveType>
          <stationCount>6</stationCount>
          <componentCount>6</componentCount>
        </dataUsed>
        <dataUsed>
          <waveType>surface waves</waveType>
          <stationCount>2</stationCount>
          <componentCount>2</componentCount>
        </dataUsed>
        <phaseSetting code="L" lowerPeriod="60" upperPeriod="150" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="P" lowerPeriod="40" upperPeriod="100" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="R" lowerPeriod="60" upperPeriod="150" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="S" lowerPeriod="40" upperPeriod="100" minimumSNR="1" maximumTimeShift="10"/>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055237.798996.613706" active="true" weight="1">
          <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH">
            <resourceURI>G.SANVU</resourceURI>
          </waveformID>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>13</timeShift>
            <dataTimeWindow>53.63027949 156.0533524</dataTimeWindow>
            <misfit>0.0538092804</misfit>
            <snr>27.76552963</snr>
          </component>
          <component phaseCode="R" component="0" active="true">
            <weight>1</weight>
            <timeShift>13</timeShift>
            <dataTimeWindow>95.7357546 233.6470627</dataTimeWindow>
            <misfit>0.0135030365</misfit>
            <snr>7.810316086</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.1225382313</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>95.80669276 233.6470627</dataTimeWindow>
            <misfit>0.8503775787</misfit>
            <snr>2.28793931</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.2404360026</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>95.80669276 233.6470627</dataTimeWindow>
            <misfit>1</misfit>
            <snr>4.052101612</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055237.799053.613707" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BH">
            <resourceURI>IU.CTAO</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.02286983654</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>466.4858417 1223.809662</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.931627631</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>23</timeShift>
            <dataTimeWindow>280.5268052 383.9821472</dataTimeWindow>
            <misfit>0.3305758667</misfit>
            <snr>7.685530186</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>555.3402877 1223.809662</dataTimeWindow>
            <misfit>0.5222902298</misfit>
            <snr>1.587236524</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.03153970093</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>555.3402877 1223.809662</dataTimeWindow>
            <misfit>0.8752791119</misfit>
            <snr>1.927176356</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.01516006142</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>513.4310566 713.4310566</dataTimeWindow>
            <misfit>0.9602469277</misfit>
            <snr>0.9928926229</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.01511287596</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>513.4310566 713.4310566</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.096615553</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055237.799095.613708" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="ARMA" channelCode="BH">
            <resourceURI>AU.ARMA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.09926290065</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>510.33621 1330.781912</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.781404734</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>22</timeShift>
            <dataTimeWindow>301.4975206 405.1126709</dataTimeWindow>
            <misfit>0.3349632263</misfit>
            <snr>1.401822686</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.5431071 1330.781912</dataTimeWindow>
            <misfit>0.9152205563</misfit>
            <snr>2.543514967</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.04821261019</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.5431071 1330.781912</dataTimeWindow>
            <misfit>0.7728412819</misfit>
            <snr>2.092487812</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.02695878781</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>550.4549714 750.4549714</dataTimeWindow>
            <misfit>0.9222802162</misfit>
            <snr>2.002534866</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.04319990426</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>550.4549714 750.4549714</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.201293588</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055237.799131.613709" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="CMSA" channelCode="BH">
            <resourceURI>AU.CMSA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.2611875236</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.4171059 1561.449098</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.602042913</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>19</timeShift>
            <dataTimeWindow>341.7647989 445.8375549</dataTimeWindow>
            <misfit>0.2742035675</misfit>
            <snr>2.716827869</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>723.1156022 1561.449098</dataTimeWindow>
            <misfit>0.7192238808</misfit>
            <snr>1.7530936</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.1848328114</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>723.1156022 1561.449098</dataTimeWindow>
            <misfit>0.5486158752</misfit>
            <snr>2.034671307</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.09039121866</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>620.1837002 820.1837002</dataTimeWindow>
            <misfit>0.7016073608</misfit>
            <snr>1.9897753</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.1117968857</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>620.1837002 820.1837002</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.807088137</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055237.799163.613710" active="true" weight="1">
          <waveformID networkCode="GE" stationCode="GENI" channelCode="BH">
            <resourceURI>GE.GENI</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.002601774875</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>613.0224029 1574.907032</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.51305449</snr>
          </component>
          <component phaseCode="P" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>344.2293443 448.1009216</dataTimeWindow>
            <misfit>0.3895554733</misfit>
            <snr>1.140642762</snr>
          </component>
          <component phaseCode="R" component="0" active="true">
            <weight>1</weight>
            <timeShift>3</timeShift>
            <dataTimeWindow>729.7885749 1574.907032</dataTimeWindow>
            <misfit>0.6598792267</misfit>
            <snr>1.3100878</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.003157971893</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>729.7885749 1574.907032</dataTimeWindow>
            <misfit>0.6976543236</misfit>
            <snr>0.671446979</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.002202074742</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>624.530621 824.530621</dataTimeWindow>
            <misfit>0.6486729813</misfit>
            <snr>1.743083</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.001638681977</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>624.530621 824.530621</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.213575363</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055237.799196.613711" active="true" weight="1">
          <waveformID networkCode="G" stationCode="CAN" locationCode="00" channelCode="BH">
            <resourceURI>G.CAN</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>625.7614442 1604.751708</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.337115765</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>22</timeShift>
            <dataTimeWindow>349.0895754 453.2279968</dataTimeWindow>
            <misfit>0.3680528641</misfit>
            <snr>1.971666813</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>744.9541002 1604.751708</dataTimeWindow>
            <misfit>0.7885295105</misfit>
            <snr>1.954316378</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>744.9541002 1604.751708</dataTimeWindow>
            <misfit>0.8367526436</misfit>
            <snr>1.057021737</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>633.0850964 833.0850964</dataTimeWindow>
            <misfit>0.505587616</misfit>
            <snr>1.669407845</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>633.0850964 833.0850964</dataTimeWindow>
            <misfit>1</misfit>
            <snr>4.944037437</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055237.799238.613712" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="BBOO" channelCode="BH">
            <resourceURI>AU.BBOO</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.07827074081</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>771.8995178 1949.167137</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.339173675</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>11</timeShift>
            <dataTimeWindow>406.8917076 511.0207214</dataTimeWindow>
            <misfit>0.6939381599</misfit>
            <snr>3.792047501</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>918.9279973 1949.167137</dataTimeWindow>
            <misfit>0.646211586</misfit>
            <snr>1.682954192</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.1121883839</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>918.9279973 1949.167137</dataTimeWindow>
            <misfit>0.9730479693</misfit>
            <snr>1.311385036</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.09564966708</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>735.6475006 935.6475006</dataTimeWindow>
            <misfit>0.9303808403</misfit>
            <snr>2.019122839</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.0449138172</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>735.6475006 935.6475006</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.079048872</snr>
          </component>
        </stationMomentTensorContribution>
      </momentTensor>
    </focalMechanism>
    <focalMechanism publicID="FocalMechanism/20210430055304.911836.613715">
      <triggeringOriginID>Origin/20210430053829.67918.86662</triggeringOriginID>
      <nodalPlanes>
        <nodalPlane1>
          <strike>
            <value>177.2639358</value>
          </strike>
          <dip>
            <value>76.65512867</value>
          </dip>
          <rake>
            <value>174.342645</value>
          </rake>
        </nodalPlane1>
        <nodalPlane2>
          <strike>
            <value>268.5737521</value>
          </strike>
          <dip>
            <value>84.49588082</value>
          </dip>
          <rake>
            <value>13.40783625</value>
          </rake>
        </nodalPlane2>
      </nodalPlanes>
      <principalAxes>
        <tAxis>
          <azimuth>
            <value>133.5856399</value>
          </azimuth>
          <plunge>
            <value>13.35786003</value>
          </plunge>
          <length>
            <value>3.116494748e+16</value>
          </length>
        </tAxis>
        <pAxis>
          <azimuth>
            <value>42.28184833</value>
          </azimuth>
          <plunge>
            <value>5.473468684</value>
          </plunge>
          <length>
            <value>-4.578250102e+16</value>
          </length>
        </pAxis>
        <nAxis>
          <azimuth>
            <value>290.4923551</value>
          </azimuth>
          <plunge>
            <value>75.5255386</value>
          </plunge>
          <length>
            <value>1.461755355e+16</value>
          </length>
        </nAxis>
      </principalAxes>
      <azimuthalGap>142.1660848</azimuthalGap>
      <misfit>0.04444464526</misfit>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T05:53:04.911698Z</creationTime>
      </creationInfo>
      <momentTensor publicID="MomentTensor/20210430055304.912189.613718">
        <derivedOriginID>Origin/20210430055304.912021.613716</derivedOriginID>
        <momentMagnitudeID>Magnitude/20210430055304.912111.613717</momentMagnitudeID>
        <scalarMoment>
          <value>4.050286547e+16</value>
        </scalarMoment>
        <tensor>
          <Mrr>
            <value>1.495125103e+16</value>
          </Mrr>
          <Mtt>
            <value>-1.069723406e+16</value>
          </Mtt>
          <Mpp>
            <value>-4.254016978e+15</value>
          </Mpp>
          <Mrt>
            <value>-6.807395796e+15</value>
          </Mrt>
          <Mrp>
            <value>1.164127571e+15</value>
          </Mrp>
          <Mtp>
            <value>3.7613197e+16</value>
          </Mtp>
        </tensor>
        <varianceReduction>0.9555553547</varianceReduction>
        <doubleCouple>0.3614349054</doubleCouple>
        <clvd>0.6385650946</clvd>
        <greensFunctionID>sc3gf1d:/qseis_ak135f_continental</greensFunctionID>
        <filterID>BP 40s-100s</filterID>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T05:53:04.911698Z</creationTime>
        </creationInfo>
        <dataUsed>
          <waveType>body waves</waveType>
          <stationCount>7</stationCount>
          <componentCount>7</componentCount>
        </dataUsed>
        <dataUsed>
          <waveType>surface waves</waveType>
          <stationCount>1</stationCount>
          <componentCount>1</componentCount>
        </dataUsed>
        <phaseSetting code="L" lowerPeriod="60" upperPeriod="150" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="P" lowerPeriod="40" upperPeriod="100" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="R" lowerPeriod="60" upperPeriod="150" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="S" lowerPeriod="40" upperPeriod="100" minimumSNR="1" maximumTimeShift="10"/>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055304.912278.613719" active="true" weight="1">
          <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH">
            <resourceURI>G.SANVU</resourceURI>
          </waveformID>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-4</timeShift>
            <dataTimeWindow>54.01234519 155.3729706</dataTimeWindow>
            <misfit>0.04565406799</misfit>
            <snr>27.23679924</snr>
          </component>
          <component phaseCode="R" component="0" active="true">
            <weight>1</weight>
            <timeShift>-4</timeShift>
            <dataTimeWindow>95.90205638 234.3056838</dataTimeWindow>
            <misfit>0.02108886719</misfit>
            <snr>7.709475517</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.123312071</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>96.43622093 234.3056838</dataTimeWindow>
            <misfit>0.8464826965</misfit>
            <snr>2.305141687</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.2406651974</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>96.43622093 234.3056838</dataTimeWindow>
            <misfit>1</misfit>
            <snr>4.024114132</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055304.912352.613720" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="TARA" locationCode="00" channelCode="BH">
            <resourceURI>IU.TARA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.007092492655</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>326.4520581 872.8422378</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.073470592</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>6</timeShift>
            <dataTimeWindow>205.7445822 306.9925842</dataTimeWindow>
            <misfit>0.4963869095</misfit>
            <snr>1.363570571</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>388.6334025 872.8422378</dataTimeWindow>
            <misfit>0.8863717079</misfit>
            <snr>4.783790112</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.003497791942</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>388.6334025 872.8422378</dataTimeWindow>
            <misfit>0.9628971267</misfit>
            <snr>2.31493187</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.004891633056</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>368.4220656 568.4220656</dataTimeWindow>
            <misfit>0.7426436996</misfit>
            <snr>0.6371799707</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.009299323894</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>368.4220656 568.4220656</dataTimeWindow>
            <misfit>1</misfit>
            <snr>0.82813555</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055304.912397.613721" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="EIDS" channelCode="BH">
            <resourceURI>AU.EIDS</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.1044915617</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>437.9514809 1154.246532</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.215333939</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>5</timeShift>
            <dataTimeWindow>266.9266902 369.2104797</dataTimeWindow>
            <misfit>0.2391711426</misfit>
            <snr>4.692846775</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>521.3708106 1154.246532</dataTimeWindow>
            <misfit>0.7552874374</misfit>
            <snr>1.796907663</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.08761250228</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>521.3708106 1154.246532</dataTimeWindow>
            <misfit>0.763740406</misfit>
            <snr>2.604088306</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.03381812572</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>488.3866849 688.3866849</dataTimeWindow>
            <misfit>0.3428308105</misfit>
            <snr>1.600562096</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.03597045317</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>488.3866849 688.3866849</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.383267522</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055304.91244.613722" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BH">
            <resourceURI>IU.CTAO</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.02272730693</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>465.9110441 1222.790913</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.926164627</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>1</timeShift>
            <dataTimeWindow>280.6353776 382.8960571</dataTimeWindow>
            <misfit>0.5432171631</misfit>
            <snr>7.60748148</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>554.6560049 1222.790913</dataTimeWindow>
            <misfit>0.5438781357</misfit>
            <snr>1.595830441</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.031402722</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>554.6560049 1222.790913</dataTimeWindow>
            <misfit>0.8834739971</misfit>
            <snr>1.921049237</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.01518514752</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>513.5637303 713.5637303</dataTimeWindow>
            <misfit>0.9644511843</misfit>
            <snr>0.9940682054</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.01513855904</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>513.5637303 713.5637303</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.104261518</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055304.9125.613723" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="ARMA" channelCode="BH">
            <resourceURI>AU.ARMA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.09928009659</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>509.9773866 1330.301247</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.778090715</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>1</timeShift>
            <dataTimeWindow>301.7205975 404.132782</dataTimeWindow>
            <misfit>0.3360371399</misfit>
            <snr>1.406597495</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.1159364 1330.301247</dataTimeWindow>
            <misfit>0.9172219753</misfit>
            <snr>2.54085803</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.04847697541</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.1159364 1330.301247</dataTimeWindow>
            <misfit>0.7893798637</misfit>
            <snr>2.103213787</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.02705101855</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>550.9146855 750.9146855</dataTimeWindow>
            <misfit>0.9281681299</misfit>
            <snr>2.0030725</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.04330698028</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>550.9146855 750.9146855</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.200439453</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055304.912546.613724" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="CMSA" channelCode="BH">
            <resourceURI>AU.CMSA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.2612291873</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>606.998931 1560.886236</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.590667725</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-2</timeShift>
            <dataTimeWindow>342.0220828 444.8430176</dataTimeWindow>
            <misfit>0.389816246</misfit>
            <snr>2.734752893</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>722.617775 1560.886236</dataTimeWindow>
            <misfit>0.6833830261</misfit>
            <snr>1.75257659</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.1859548837</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>722.617775 1560.886236</dataTimeWindow>
            <misfit>0.5847386932</misfit>
            <snr>2.04881382</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.09062115848</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>620.6214221 820.6214221</dataTimeWindow>
            <misfit>0.6709676361</misfit>
            <snr>1.990436196</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.112135753</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>620.6214221 820.6214221</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.806781292</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055304.912579.613725" active="true" weight="1">
          <waveformID networkCode="G" stationCode="CAN" locationCode="00" channelCode="BH">
            <resourceURI>G.CAN</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>625.4319812 1604.400325</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.338611841</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-2</timeShift>
            <dataTimeWindow>349.3843523 452.2696228</dataTimeWindow>
            <misfit>0.1501287842</misfit>
            <snr>1.981680036</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>744.5618824 1604.400325</dataTimeWindow>
            <misfit>0.8057865715</misfit>
            <snr>1.965375543</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>744.5618824 1604.400325</dataTimeWindow>
            <misfit>0.8435315895</misfit>
            <snr>1.055261493</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>633.5868385 833.5868385</dataTimeWindow>
            <misfit>0.5141334915</misfit>
            <snr>1.668983817</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>633.5868385 833.5868385</dataTimeWindow>
            <misfit>1</misfit>
            <snr>4.945716381</snr>
          </component>
        </stationMomentTensorContribution>
      </momentTensor>
    </focalMechanism>
    <focalMechanism publicID="FocalMechanism/20210430055315.679219.613728">
      <triggeringOriginID>Origin/20210430053829.67918.86662</triggeringOriginID>
      <nodalPlanes>
        <nodalPlane1>
          <strike>
            <value>164.8935705</value>
          </strike>
          <dip>
            <value>70.42045954</value>
          </dip>
          <rake>
            <value>-10.77901599</value>
          </rake>
        </nodalPlane1>
        <nodalPlane2>
          <strike>
            <value>258.5440618</value>
          </strike>
          <dip>
            <value>79.85106896</value>
          </dip>
          <rake>
            <value>-160.0961928</value>
          </rake>
        </nodalPlane2>
      </nodalPlanes>
      <principalAxes>
        <tAxis>
          <azimuth>
            <value>30.54212928</value>
          </azimuth>
          <plunge>
            <value>6.451649599</value>
          </plunge>
          <length>
            <value>4.142454241e+16</value>
          </length>
        </tAxis>
        <pAxis>
          <azimuth>
            <value>123.0554767</value>
          </azimuth>
          <plunge>
            <value>21.19601158</value>
          </plunge>
          <length>
            <value>-2.403728267e+16</value>
          </length>
        </pAxis>
        <nAxis>
          <azimuth>
            <value>284.4947026</value>
          </azimuth>
          <plunge>
            <value>67.75192514</value>
          </plunge>
          <length>
            <value>-1.738725974e+16</value>
          </length>
        </nAxis>
      </principalAxes>
      <azimuthalGap>239.9937659</azimuthalGap>
      <misfit>0.05123870399</misfit>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T05:53:15.679051Z</creationTime>
      </creationInfo>
      <momentTensor publicID="MomentTensor/20210430055315.67945.613731">
        <derivedOriginID>Origin/20210430055315.679349.613729</derivedOriginID>
        <momentMagnitudeID>Magnitude/20210430055315.679402.613730</momentMagnitudeID>
        <scalarMoment>
          <value>3.602846425e+16</value>
        </scalarMoment>
        <tensor>
          <Mrr>
            <value>-1.751403872e+16</value>
          </Mrr>
          <Mtt>
            <value>2.396642117e+16</value>
          </Mtt>
          <Mpp>
            <value>-6.452382453e+15</value>
          </Mpp>
          <Mrt>
            <value>6.878202897e+15</value>
          </Mrt>
          <Mrp>
            <value>-1.458038176e+15</value>
          </Mrp>
          <Mtp>
            <value>-2.805766595e+16</value>
          </Mtp>
        </tensor>
        <varianceReduction>0.948761296</varianceReduction>
        <doubleCouple>0.1605334071</doubleCouple>
        <clvd>0.8394665929</clvd>
        <greensFunctionID>sc3gf1d:/qseis_ak135f_continental</greensFunctionID>
        <filterID>BP 40s-100s</filterID>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T05:53:15.679051Z</creationTime>
        </creationInfo>
        <dataUsed>
          <waveType>body waves</waveType>
          <stationCount>6</stationCount>
          <componentCount>6</componentCount>
        </dataUsed>
        <dataUsed>
          <waveType>surface waves</waveType>
          <stationCount>2</stationCount>
          <componentCount>2</componentCount>
        </dataUsed>
        <phaseSetting code="L" lowerPeriod="60" upperPeriod="150" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="P" lowerPeriod="40" upperPeriod="100" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="R" lowerPeriod="60" upperPeriod="150" minimumSNR="1" maximumTimeShift="10"/>
        <phaseSetting code="S" lowerPeriod="40" upperPeriod="100" minimumSNR="1" maximumTimeShift="10"/>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055315.679541.613732" active="true" weight="1">
          <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH">
            <resourceURI>G.SANVU</resourceURI>
          </waveformID>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>13</timeShift>
            <dataTimeWindow>53.63027949 156.0533524</dataTimeWindow>
            <misfit>0.0538092804</misfit>
            <snr>27.76552963</snr>
          </component>
          <component phaseCode="R" component="0" active="true">
            <weight>1</weight>
            <timeShift>13</timeShift>
            <dataTimeWindow>95.7357546 233.6470627</dataTimeWindow>
            <misfit>0.0135030365</misfit>
            <snr>7.810316086</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.1225382313</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>95.80669276 233.6470627</dataTimeWindow>
            <misfit>0.8503775787</misfit>
            <snr>2.28793931</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.2404360026</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>95.80669276 233.6470627</dataTimeWindow>
            <misfit>1</misfit>
            <snr>4.052101612</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055315.679607.613733" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BH">
            <resourceURI>IU.CTAO</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.02286983654</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>466.4858417 1223.809662</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.931627631</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>23</timeShift>
            <dataTimeWindow>280.5268052 383.9821472</dataTimeWindow>
            <misfit>0.3305445099</misfit>
            <snr>7.685530186</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>555.3402877 1223.809662</dataTimeWindow>
            <misfit>0.5222902298</misfit>
            <snr>1.587236524</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.03153970093</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>555.3402877 1223.809662</dataTimeWindow>
            <misfit>0.8752791119</misfit>
            <snr>1.927176356</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.01516006142</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>513.4310566 713.4310566</dataTimeWindow>
            <misfit>0.9602469277</misfit>
            <snr>0.9928926229</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.01511287596</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>513.4310566 713.4310566</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.096615553</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055315.679643.613734" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="ARMA" channelCode="BH">
            <resourceURI>AU.ARMA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.09926290065</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>510.33621 1330.781912</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.781404734</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>22</timeShift>
            <dataTimeWindow>301.4975206 405.1126709</dataTimeWindow>
            <misfit>0.3349866486</misfit>
            <snr>1.401822686</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.5431071 1330.781912</dataTimeWindow>
            <misfit>0.9152205563</misfit>
            <snr>2.543514967</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.04821261019</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.5431071 1330.781912</dataTimeWindow>
            <misfit>0.7728412819</misfit>
            <snr>2.092487812</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.02695878781</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>550.4549714 750.4549714</dataTimeWindow>
            <misfit>0.9222802162</misfit>
            <snr>2.002534866</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.04319990426</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>550.4549714 750.4549714</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.201293588</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055315.679677.613735" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="CMSA" channelCode="BH">
            <resourceURI>AU.CMSA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.2611875236</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.4171059 1561.449098</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.602042913</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>19</timeShift>
            <dataTimeWindow>341.7647989 445.8375549</dataTimeWindow>
            <misfit>0.2741293335</misfit>
            <snr>2.716827869</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>723.1156022 1561.449098</dataTimeWindow>
            <misfit>0.7192238808</misfit>
            <snr>1.7530936</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.1848328114</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>723.1156022 1561.449098</dataTimeWindow>
            <misfit>0.5486158752</misfit>
            <snr>2.034671307</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.09039121866</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>620.1837002 820.1837002</dataTimeWindow>
            <misfit>0.7016073608</misfit>
            <snr>1.9897753</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.1117968857</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>620.1837002 820.1837002</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.807088137</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055315.679708.613736" active="true" weight="1">
          <waveformID networkCode="GE" stationCode="GENI" channelCode="BH">
            <resourceURI>GE.GENI</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.002601774875</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>613.0224029 1574.907032</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.51305449</snr>
          </component>
          <component phaseCode="P" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>344.2293443 448.1009216</dataTimeWindow>
            <misfit>0.3895554733</misfit>
            <snr>1.140642762</snr>
          </component>
          <component phaseCode="R" component="0" active="true">
            <weight>1</weight>
            <timeShift>3</timeShift>
            <dataTimeWindow>729.7885749 1574.907032</dataTimeWindow>
            <misfit>0.6595460892</misfit>
            <snr>1.3100878</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.003157971893</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>729.7885749 1574.907032</dataTimeWindow>
            <misfit>0.6976543236</misfit>
            <snr>0.671446979</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.002202074742</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>624.530621 824.530621</dataTimeWindow>
            <misfit>0.6486729813</misfit>
            <snr>1.743083</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.001638681977</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>624.530621 824.530621</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.213575363</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055315.67974.613737" active="true" weight="1">
          <waveformID networkCode="G" stationCode="CAN" locationCode="00" channelCode="BH">
            <resourceURI>G.CAN</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>625.7614442 1604.751708</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.337115765</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>22</timeShift>
            <dataTimeWindow>349.0895754 453.2279968</dataTimeWindow>
            <misfit>0.3680213928</misfit>
            <snr>1.971666813</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>744.9541002 1604.751708</dataTimeWindow>
            <misfit>0.7885295105</misfit>
            <snr>1.954316378</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>744.9541002 1604.751708</dataTimeWindow>
            <misfit>0.8367526436</misfit>
            <snr>1.057021737</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>633.0850964 833.0850964</dataTimeWindow>
            <misfit>0.505587616</misfit>
            <snr>1.669407845</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>633.0850964 833.0850964</dataTimeWindow>
            <misfit>1</misfit>
            <snr>4.944037437</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430055315.679778.613738" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="BBOO" channelCode="BH">
            <resourceURI>AU.BBOO</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.0782699585</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>771.8995178 1949.167137</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.341569304</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>11</timeShift>
            <dataTimeWindow>406.8917076 511.0207214</dataTimeWindow>
            <misfit>0.6938347435</misfit>
            <snr>3.792047739</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>918.9279973 1949.167137</dataTimeWindow>
            <misfit>0.6461832809</misfit>
            <snr>1.682917714</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.1121884212</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>918.9279973 1949.167137</dataTimeWindow>
            <misfit>0.9730709481</misfit>
            <snr>1.311584949</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.09564973414</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>735.6475006 935.6475006</dataTimeWindow>
            <misfit>0.9303810072</misfit>
            <snr>2.019127131</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.04491367191</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>735.6475006 935.6475006</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.079047799</snr>
          </component>
        </stationMomentTensorContribution>
      </momentTensor>
    </focalMechanism>
    <focalMechanism publicID="FocalMechanism/20210430060422.075728.613830">
      <triggeringOriginID>Origin/20210430060321.912081.87745</triggeringOriginID>
      <nodalPlanes>
        <nodalPlane1>
          <strike>
            <value>96.41691357</value>
          </strike>
          <dip>
            <value>63.35056623</value>
          </dip>
          <rake>
            <value>120.5114546</value>
          </rake>
        </nodalPlane1>
        <nodalPlane2>
          <strike>
            <value>223.6918761</value>
          </strike>
          <dip>
            <value>39.64561625</value>
          </dip>
          <rake>
            <value>44.66695599</value>
          </rake>
        </nodalPlane2>
      </nodalPlanes>
      <principalAxes>
        <tAxis>
          <azimuth>
            <value>51.43748025</value>
          </azimuth>
          <plunge>
            <value>59.5006584</value>
          </plunge>
          <length>
            <value>2.9040125e+16</value>
          </length>
        </tAxis>
        <pAxis>
          <azimuth>
            <value>164.784113</value>
          </azimuth>
          <plunge>
            <value>13.13917798</value>
          </plunge>
          <length>
            <value>-2.252057593e+16</value>
          </length>
        </pAxis>
        <nAxis>
          <azimuth>
            <value>261.610809</value>
          </azimuth>
          <plunge>
            <value>26.9861616</value>
          </plunge>
          <length>
            <value>-6.519549072e+15</value>
          </length>
        </nAxis>
      </principalAxes>
      <azimuthalGap>275.0755894</azimuthalGap>
      <misfit>0.05462560336</misfit>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T06:04:22.075526Z</creationTime>
      </creationInfo>
      <momentTensor publicID="MomentTensor/20210430060422.076096.613833">
        <derivedOriginID>Origin/20210430060422.075945.613831</derivedOriginID>
        <momentMagnitudeID>Magnitude/20210430060422.076007.613832</momentMagnitudeID>
        <scalarMoment>
          <value>2.639137852e+16</value>
        </scalarMoment>
        <tensor>
          <Mrr>
            <value>1.905365826e+16</value>
          </Mrr>
          <Mtt>
            <value>-1.708917273e+16</value>
          </Mtt>
          <Mpp>
            <value>-1.964485535e+15</value>
          </Mpp>
          <Mrt>
            <value>1.311153896e+16</value>
          </Mrt>
          <Mrp>
            <value>-1.122965319e+16</value>
          </Mrp>
          <Mtp>
            <value>-8.307637138e+15</value>
          </Mtp>
        </tensor>
        <varianceReduction>0.9453743966</varianceReduction>
        <doubleCouple>0.5509971757</doubleCouple>
        <clvd>0.4490028243</clvd>
        <greensFunctionID>sc3gf1d:/qseis_ak135f_continental</greensFunctionID>
        <filterID>BP 20s-50s</filterID>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T06:04:22.075526Z</creationTime>
        </creationInfo>
        <dataUsed>
          <waveType>body waves</waveType>
          <stationCount>6</stationCount>
          <componentCount>10</componentCount>
        </dataUsed>
        <dataUsed>
          <waveType>surface waves</waveType>
          <stationCount>1</stationCount>
          <componentCount>1</componentCount>
        </dataUsed>
        <phaseSetting code="L" lowerPeriod="20" upperPeriod="50" minimumSNR="1" maximumTimeShift="5"/>
        <phaseSetting code="P" lowerPeriod="20" upperPeriod="50" minimumSNR="1" maximumTimeShift="5"/>
        <phaseSetting code="R" lowerPeriod="20" upperPeriod="50" minimumSNR="1" maximumTimeShift="5"/>
        <phaseSetting code="S" lowerPeriod="20" upperPeriod="50" minimumSNR="1" maximumTimeShift="5"/>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430060422.076227.613834" active="true" weight="1">
          <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH">
            <resourceURI>G.SANVU</resourceURI>
          </waveformID>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>3.5</timeShift>
            <dataTimeWindow>53.77600984 163.5579224</dataTimeWindow>
            <misfit>0.02182151794</misfit>
            <snr>81.24188995</snr>
          </component>
          <component phaseCode="R" component="0" active="true">
            <weight>1</weight>
            <timeShift>3.5</timeShift>
            <dataTimeWindow>95.92970023 234.1153194</dataTimeWindow>
            <misfit>0.03021896362</misfit>
            <snr>15.39611244</snr>
          </component>
          <component phaseCode="S" component="1" active="true">
            <weight>0.09312412143</weight>
            <timeShift>2.5</timeShift>
            <dataTimeWindow>95.99128559 234.1153194</dataTimeWindow>
            <misfit>0.2349594879</misfit>
            <snr>6.608595371</snr>
          </component>
          <component phaseCode="S" component="2" active="true">
            <weight>0.5</weight>
            <timeShift>2.5</timeShift>
            <dataTimeWindow>95.99128559 234.1153194</dataTimeWindow>
            <misfit>0.3769834518</misfit>
            <snr>18.67375755</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430060422.076321.613835" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BH">
            <resourceURI>IU.CTAO</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.4853368402</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>465.9328979 1219.357786</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.787621975</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>3.5</timeShift>
            <dataTimeWindow>277.1593897 391.7224121</dataTimeWindow>
            <misfit>0.6821413612</misfit>
            <snr>8.672035217</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>554.6820213 1219.357786</dataTimeWindow>
            <misfit>0.9713567424</misfit>
            <snr>3.029930115</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>554.6820213 1219.357786</dataTimeWindow>
            <misfit>0.9881583071</misfit>
            <snr>2.284801483</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.06190500408</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>507.550815 707.550815</dataTimeWindow>
            <misfit>0.9165032578</misfit>
            <snr>2.589633226</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.05962682888</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>507.550815 707.550815</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.614733696</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430060422.076378.613836" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="CMSA" channelCode="BH">
            <resourceURI>AU.CMSA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.0265274 1556.74541</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.923905849</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>8.5</timeShift>
            <dataTimeWindow>337.8271335 453.9801025</dataTimeWindow>
            <misfit>0.6506649017</misfit>
            <snr>2.731471539</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>722.6506279 1556.74541</dataTimeWindow>
            <misfit>0.9888920009</misfit>
            <snr>3.88241744</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>722.6506279 1556.74541</dataTimeWindow>
            <misfit>0.968524828</misfit>
            <snr>1.972385168</snr>
          </component>
          <component phaseCode="S" component="1" active="true">
            <weight>0.25</weight>
            <timeShift>5.5</timeShift>
            <dataTimeWindow>613.0229443 813.0229443</dataTimeWindow>
            <misfit>0.6467418289</misfit>
            <snr>3.392154217</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.2793957889</weight>
            <timeShift>6.5</timeShift>
            <dataTimeWindow>613.0229443 813.0229443</dataTimeWindow>
            <misfit>0.980320816</misfit>
            <snr>5.216963768</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430060422.076425.613837" active="true" weight="1">
          <waveformID networkCode="II" stationCode="WRAB" locationCode="00" channelCode="BH">
            <resourceURI>II.WRAB</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>705.8035534 1789.973952</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.138683558</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-0.5</timeShift>
            <dataTimeWindow>377.3292325 493.4380798</dataTimeWindow>
            <misfit>0.508277359</misfit>
            <snr>3.309036016</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>840.2423255 1789.973952</dataTimeWindow>
            <misfit>0.9704398942</misfit>
            <snr>4.131324768</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>840.2423255 1789.973952</dataTimeWindow>
            <misfit>0.89189538</misfit>
            <snr>1.623862982</snr>
          </component>
          <component phaseCode="S" component="1" active="true">
            <weight>0.25</weight>
            <timeShift>6.5</timeShift>
            <dataTimeWindow>682.9333838 882.9333838</dataTimeWindow>
            <misfit>0.3490604401</misfit>
            <snr>8.451694489</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>5.5</timeShift>
            <dataTimeWindow>682.9333838 882.9333838</dataTimeWindow>
            <misfit>1.220899811</misfit>
            <snr>1.997008562</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430060422.076487.613838" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="BBOO" channelCode="BH">
            <resourceURI>AU.BBOO</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.4272579551</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>771.4508899 1944.233551</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.862731576</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-1.5</timeShift>
            <dataTimeWindow>402.837993 519.1951294</dataTimeWindow>
            <misfit>0.6156885529</misfit>
            <snr>1.7967695</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>918.3939165 1944.233551</dataTimeWindow>
            <misfit>0.8503261852</misfit>
            <snr>3.336666822</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>918.3939165 1944.233551</dataTimeWindow>
            <misfit>0.8996747875</misfit>
            <snr>1.496700287</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.19868204</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>728.3444788 928.3444788</dataTimeWindow>
            <misfit>0.7465877914</misfit>
            <snr>2.075638771</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.1614352167</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>728.3444788 928.3444788</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.058134913</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430060422.07653.613839" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="MBWA" locationCode="00" channelCode="BH">
            <resourceURI>IU.MBWA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>1011.463538 2499.892509</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.522829533</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-1.5</timeShift>
            <dataTimeWindow>492.3050743 608.8818359</dataTimeWindow>
            <misfit>0.5596050262</misfit>
            <snr>3.198191166</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>1204.12326 2499.892509</dataTimeWindow>
            <misfit>0.856917181</misfit>
            <snr>2.64085865</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>1204.12326 2499.892509</dataTimeWindow>
            <misfit>0.889964571</misfit>
            <snr>1.938743472</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>889.626503 1089.626503</dataTimeWindow>
            <misfit>0.5830461121</misfit>
            <snr>6.722157001</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>889.626503 1089.626503</dataTimeWindow>
            <misfit>1</misfit>
            <snr>5.879295826</snr>
          </component>
        </stationMomentTensorContribution>
      </momentTensor>
    </focalMechanism>
    <focalMechanism publicID="FocalMechanism/20210430060441.05961.613897">
      <triggeringOriginID>Origin/20210430060321.912081.87745</triggeringOriginID>
      <nodalPlanes>
        <nodalPlane1>
          <strike>
            <value>110.0692888</value>
          </strike>
          <dip>
            <value>73.50908694</value>
          </dip>
          <rake>
            <value>138.7826539</value>
          </rake>
        </nodalPlane1>
        <nodalPlane2>
          <strike>
            <value>214.0329987</value>
          </strike>
          <dip>
            <value>50.81602697</value>
          </dip>
          <rake>
            <value>21.48255344</value>
          </rake>
        </nodalPlane2>
      </nodalPlanes>
      <principalAxes>
        <tAxis>
          <azimuth>
            <value>64.3322132</value>
          </azimuth>
          <plunge>
            <value>40.3519148</value>
          </plunge>
          <length>
            <value>3.139951398e+16</value>
          </length>
        </tAxis>
        <pAxis>
          <azimuth>
            <value>166.7869266</value>
          </azimuth>
          <plunge>
            <value>14.24314096</value>
          </plunge>
          <length>
            <value>-3.262246268e+16</value>
          </length>
        </pAxis>
        <nAxis>
          <azimuth>
            <value>272.11404</value>
          </azimuth>
          <plunge>
            <value>46.15967363</value>
          </plunge>
          <length>
            <value>1.222948701e+15</value>
          </length>
        </nAxis>
      </principalAxes>
      <azimuthalGap>181.5093784</azimuthalGap>
      <misfit>0.04381576429</misfit>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T06:04:41.059503Z</creationTime>
      </creationInfo>
      <momentTensor publicID="MomentTensor/20210430060441.059829.613900">
        <derivedOriginID>Origin/20210430060441.059734.613898</derivedOriginID>
        <momentMagnitudeID>Magnitude/20210430060441.059771.613899</momentMagnitudeID>
        <scalarMoment>
          <value>3.202850413e+16</value>
        </scalarMoment>
        <tensor>
          <Mrr>
            <value>1.1825093e+16</value>
          </Mrr>
          <Mtt>
            <value>-2.562424714e+16</value>
          </Mtt>
          <Mpp>
            <value>1.379915414e+16</value>
          </Mpp>
          <Mrt>
            <value>1.430727583e+16</value>
          </Mrt>
          <Mrp>
            <value>-1.157590277e+16</value>
          </Mrp>
          <Mtp>
            <value>-1.391760216e+16</value>
          </Mtp>
        </tensor>
        <varianceReduction>0.9561842357</varianceReduction>
        <doubleCouple>0.9250241337</doubleCouple>
        <clvd>0.07497586631</clvd>
        <greensFunctionID>sc3gf1d:/qseis_ak135f_continental</greensFunctionID>
        <filterID>BP 20s-50s</filterID>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T06:04:41.059503Z</creationTime>
        </creationInfo>
        <dataUsed>
          <waveType>body waves</waveType>
          <stationCount>5</stationCount>
          <componentCount>7</componentCount>
        </dataUsed>
        <dataUsed>
          <waveType>surface waves</waveType>
          <stationCount>2</stationCount>
          <componentCount>2</componentCount>
        </dataUsed>
        <phaseSetting code="L" lowerPeriod="20" upperPeriod="50" minimumSNR="1" maximumTimeShift="5"/>
        <phaseSetting code="P" lowerPeriod="20" upperPeriod="50" minimumSNR="1" maximumTimeShift="5"/>
        <phaseSetting code="R" lowerPeriod="20" upperPeriod="50" minimumSNR="1" maximumTimeShift="5"/>
        <phaseSetting code="S" lowerPeriod="20" upperPeriod="50" minimumSNR="1" maximumTimeShift="5"/>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430060441.059932.613901" active="true" weight="1">
          <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH">
            <resourceURI>G.SANVU</resourceURI>
          </waveformID>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>0.5</timeShift>
            <dataTimeWindow>53.76731414 164.0293198</dataTimeWindow>
            <misfit>0.0190965271</misfit>
            <snr>81.98966217</snr>
          </component>
          <component phaseCode="R" component="0" active="true">
            <weight>1</weight>
            <timeShift>1.5</timeShift>
            <dataTimeWindow>95.85862155 233.9884218</dataTimeWindow>
            <misfit>0.02704902649</misfit>
            <snr>15.24331284</snr>
          </component>
          <component phaseCode="S" component="1" active="true">
            <weight>0.09086188674</weight>
            <timeShift>0.5</timeShift>
            <dataTimeWindow>95.96922529 233.9884218</dataTimeWindow>
            <misfit>0.2259015656</misfit>
            <snr>6.442846775</snr>
          </component>
          <component phaseCode="S" component="2" active="true">
            <weight>0.4096038342</weight>
            <timeShift>0.5</timeShift>
            <dataTimeWindow>95.96922529 233.9884218</dataTimeWindow>
            <misfit>0.4080250931</misfit>
            <snr>14.96203709</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430060441.059998.613902" active="true" weight="1">
          <waveformID networkCode="II" stationCode="WRAB" locationCode="00" channelCode="BH">
            <resourceURI>II.WRAB</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>706.0589554 1790.312408</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.065047264</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-2.5</timeShift>
            <dataTimeWindow>377.166782 494.1123657</dataTimeWindow>
            <misfit>0.371787262</misfit>
            <snr>3.308938503</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>840.5463755 1790.312408</dataTimeWindow>
            <misfit>0.967227385</misfit>
            <snr>4.026244164</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>840.5463755 1790.312408</dataTimeWindow>
            <misfit>0.8744926262</misfit>
            <snr>1.625114799</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>682.6369155 882.6369155</dataTimeWindow>
            <misfit>0.4962975693</misfit>
            <snr>8.449304581</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>682.6369155 882.6369155</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.024760485</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430060441.060062.613903" active="true" weight="1">
          <waveformID networkCode="IA" stationCode="KMPI" channelCode="BH">
            <resourceURI>IA.KMPI</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.03312719613</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>741.4189548 1873.626194</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.905547023</snr>
          </component>
          <component phaseCode="P" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>391.130769 508.0405579</dataTimeWindow>
            <misfit>0.8348035049</misfit>
            <snr>2.297007322</snr>
          </component>
          <component phaseCode="R" component="0" active="true">
            <weight>1</weight>
            <timeShift>-4.5</timeShift>
            <dataTimeWindow>882.6416129 1873.626194</dataTimeWindow>
            <misfit>0.5073026276</misfit>
            <snr>4.23784399</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.0428089872</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>882.6416129 1873.626194</dataTimeWindow>
            <misfit>0.9541045094</misfit>
            <snr>1.972735047</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.02086803317</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>707.4655453 907.4655453</dataTimeWindow>
            <misfit>0.609950943</misfit>
            <snr>1.210440397</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.01540092099</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>707.4655453 907.4655453</dataTimeWindow>
            <misfit>1</misfit>
            <snr>0.8067843318</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430060441.060107.613904" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="BBOO" channelCode="BH">
            <resourceURI>AU.BBOO</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.4326652288</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>771.6520856 1944.439646</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.86477375</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-3.5</timeShift>
            <dataTimeWindow>402.6494928 519.848053</dataTimeWindow>
            <misfit>0.6301008224</misfit>
            <snr>1.79516077</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>918.6334352 1944.439646</dataTimeWindow>
            <misfit>0.8573006725</misfit>
            <snr>3.296366453</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>918.6334352 1944.439646</dataTimeWindow>
            <misfit>0.8613654709</misfit>
            <snr>1.497937322</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.1984621137</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>728.0038974 928.0038974</dataTimeWindow>
            <misfit>0.6841908455</misfit>
            <snr>2.084967136</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.1613917053</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>728.0038974 928.0038974</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.066167235</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430060441.060148.613905" active="true" weight="1">
          <waveformID networkCode="G" stationCode="PPTF" locationCode="00" channelCode="BH">
            <resourceURI>G.PPTF</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>953.7078669 2366.901478</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.597336411</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>3.5</timeShift>
            <dataTimeWindow>471.2878506 588.5927429</dataTimeWindow>
            <misfit>0.6413870621</misfit>
            <snr>1.536093593</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>1135.366508 2366.901478</dataTimeWindow>
            <misfit>0.9601058197</misfit>
            <snr>2.841041565</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>1135.366508 2366.901478</dataTimeWindow>
            <misfit>0.9757282209</misfit>
            <snr>1.516458511</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.2066085637</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>851.440928 1051.440928</dataTimeWindow>
            <misfit>0.9399758673</misfit>
            <snr>1.585787773</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.2421266735</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>851.440928 1051.440928</dataTimeWindow>
            <misfit>1</misfit>
            <snr>0.737841785</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430060441.060186.613906" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="MBWA" locationCode="00" channelCode="BH">
            <resourceURI>IU.MBWA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>1011.719963 2500.200937</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.525402665</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-4.5</timeShift>
            <dataTimeWindow>492.1163584 609.5544434</dataTimeWindow>
            <misfit>0.6073803329</misfit>
            <snr>3.196275711</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>1204.428527 2500.200937</dataTimeWindow>
            <misfit>0.8652792168</misfit>
            <snr>2.642363787</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>1204.428527 2500.200937</dataTimeWindow>
            <misfit>0.9156316757</misfit>
            <snr>1.935712814</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>889.2943949 1089.294395</dataTimeWindow>
            <misfit>0.5793464279</misfit>
            <snr>6.734830379</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>889.2943949 1089.294395</dataTimeWindow>
            <misfit>1</misfit>
            <snr>5.897432327</snr>
          </component>
        </stationMomentTensorContribution>
      </momentTensor>
    </focalMechanism>
    <focalMechanism publicID="FocalMechanism/20210430060540.614651.614026">
      <triggeringOriginID>Origin/20210430060321.912081.87745</triggeringOriginID>
      <nodalPlanes>
        <nodalPlane1>
          <strike>
            <value>96.41691357</value>
          </strike>
          <dip>
            <value>63.35056623</value>
          </dip>
          <rake>
            <value>120.5114546</value>
          </rake>
        </nodalPlane1>
        <nodalPlane2>
          <strike>
            <value>223.6918761</value>
          </strike>
          <dip>
            <value>39.64561625</value>
          </dip>
          <rake>
            <value>44.66695599</value>
          </rake>
        </nodalPlane2>
      </nodalPlanes>
      <principalAxes>
        <tAxis>
          <azimuth>
            <value>51.43748025</value>
          </azimuth>
          <plunge>
            <value>59.5006584</value>
          </plunge>
          <length>
            <value>2.9040125e+16</value>
          </length>
        </tAxis>
        <pAxis>
          <azimuth>
            <value>164.784113</value>
          </azimuth>
          <plunge>
            <value>13.13917798</value>
          </plunge>
          <length>
            <value>-2.252057593e+16</value>
          </length>
        </pAxis>
        <nAxis>
          <azimuth>
            <value>261.610809</value>
          </azimuth>
          <plunge>
            <value>26.9861616</value>
          </plunge>
          <length>
            <value>-6.519549072e+15</value>
          </length>
        </nAxis>
      </principalAxes>
      <azimuthalGap>275.0755894</azimuthalGap>
      <misfit>0.05462560336</misfit>
      <evaluationMode>automatic</evaluationMode>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scautomt@gempa-proc</author>
        <creationTime>2021-04-30T06:05:40.614507Z</creationTime>
      </creationInfo>
      <momentTensor publicID="MomentTensor/20210430060540.614999.614029">
        <derivedOriginID>Origin/20210430060540.614867.614027</derivedOriginID>
        <momentMagnitudeID>Magnitude/20210430060540.614927.614028</momentMagnitudeID>
        <scalarMoment>
          <value>2.639137852e+16</value>
        </scalarMoment>
        <tensor>
          <Mrr>
            <value>1.905365826e+16</value>
          </Mrr>
          <Mtt>
            <value>-1.708917273e+16</value>
          </Mtt>
          <Mpp>
            <value>-1.964485535e+15</value>
          </Mpp>
          <Mrt>
            <value>1.311153896e+16</value>
          </Mrt>
          <Mrp>
            <value>-1.122965319e+16</value>
          </Mrp>
          <Mtp>
            <value>-8.307637138e+15</value>
          </Mtp>
        </tensor>
        <varianceReduction>0.9453743966</varianceReduction>
        <doubleCouple>0.5509971757</doubleCouple>
        <clvd>0.4490028243</clvd>
        <greensFunctionID>sc3gf1d:/qseis_ak135f_continental</greensFunctionID>
        <filterID>BP 20s-50s</filterID>
        <creationInfo>
          <agencyID>GEMPA</agencyID>
          <author>scautomt@gempa-proc</author>
          <creationTime>2021-04-30T06:05:40.614507Z</creationTime>
        </creationInfo>
        <dataUsed>
          <waveType>body waves</waveType>
          <stationCount>6</stationCount>
          <componentCount>10</componentCount>
        </dataUsed>
        <dataUsed>
          <waveType>surface waves</waveType>
          <stationCount>1</stationCount>
          <componentCount>1</componentCount>
        </dataUsed>
        <phaseSetting code="L" lowerPeriod="20" upperPeriod="50" minimumSNR="1" maximumTimeShift="5"/>
        <phaseSetting code="P" lowerPeriod="20" upperPeriod="50" minimumSNR="1" maximumTimeShift="5"/>
        <phaseSetting code="R" lowerPeriod="20" upperPeriod="50" minimumSNR="1" maximumTimeShift="5"/>
        <phaseSetting code="S" lowerPeriod="20" upperPeriod="50" minimumSNR="1" maximumTimeShift="5"/>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430060540.615105.614030" active="true" weight="1">
          <waveformID networkCode="G" stationCode="SANVU" locationCode="00" channelCode="BH">
            <resourceURI>G.SANVU</resourceURI>
          </waveformID>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>3.5</timeShift>
            <dataTimeWindow>53.77600984 163.5579224</dataTimeWindow>
            <misfit>0.02182151794</misfit>
            <snr>81.24188995</snr>
          </component>
          <component phaseCode="R" component="0" active="true">
            <weight>1</weight>
            <timeShift>3.5</timeShift>
            <dataTimeWindow>95.92970023 234.1153194</dataTimeWindow>
            <misfit>0.03021896362</misfit>
            <snr>15.39611244</snr>
          </component>
          <component phaseCode="S" component="1" active="true">
            <weight>0.09312412143</weight>
            <timeShift>2.5</timeShift>
            <dataTimeWindow>95.99128559 234.1153194</dataTimeWindow>
            <misfit>0.2349594879</misfit>
            <snr>6.608595371</snr>
          </component>
          <component phaseCode="S" component="2" active="true">
            <weight>0.5</weight>
            <timeShift>2.5</timeShift>
            <dataTimeWindow>95.99128559 234.1153194</dataTimeWindow>
            <misfit>0.3769834518</misfit>
            <snr>18.67375755</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430060540.615185.614031" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="CTAO" locationCode="00" channelCode="BH">
            <resourceURI>IU.CTAO</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.4853368402</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>465.9328979 1219.357786</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.787621975</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>3.5</timeShift>
            <dataTimeWindow>277.1593897 391.7224121</dataTimeWindow>
            <misfit>0.6821413612</misfit>
            <snr>8.672035217</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>554.6820213 1219.357786</dataTimeWindow>
            <misfit>0.9713567424</misfit>
            <snr>3.029930115</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>554.6820213 1219.357786</dataTimeWindow>
            <misfit>0.9881583071</misfit>
            <snr>2.284801483</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.06190500408</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>507.550815 707.550815</dataTimeWindow>
            <misfit>0.9165032578</misfit>
            <snr>2.589633226</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.05962682888</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>507.550815 707.550815</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.614733696</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430060540.615243.614032" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="CMSA" channelCode="BH">
            <resourceURI>AU.CMSA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>607.0265274 1556.74541</dataTimeWindow>
            <misfit>1</misfit>
            <snr>2.923905849</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>8.5</timeShift>
            <dataTimeWindow>337.8271335 453.9801025</dataTimeWindow>
            <misfit>0.6506649017</misfit>
            <snr>2.731471539</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>722.6506279 1556.74541</dataTimeWindow>
            <misfit>0.9888920009</misfit>
            <snr>3.88241744</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>722.6506279 1556.74541</dataTimeWindow>
            <misfit>0.968524828</misfit>
            <snr>1.972385168</snr>
          </component>
          <component phaseCode="S" component="1" active="true">
            <weight>0.25</weight>
            <timeShift>5.5</timeShift>
            <dataTimeWindow>613.0229443 813.0229443</dataTimeWindow>
            <misfit>0.6467418289</misfit>
            <snr>3.392154217</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.2793957889</weight>
            <timeShift>6.5</timeShift>
            <dataTimeWindow>613.0229443 813.0229443</dataTimeWindow>
            <misfit>0.9748835778</misfit>
            <snr>5.216963768</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430060540.615301.614033" active="true" weight="1">
          <waveformID networkCode="II" stationCode="WRAB" locationCode="00" channelCode="BH">
            <resourceURI>II.WRAB</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>705.8035534 1789.973952</dataTimeWindow>
            <misfit>1</misfit>
            <snr>3.138683558</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-0.5</timeShift>
            <dataTimeWindow>377.3292325 493.4380798</dataTimeWindow>
            <misfit>0.508277359</misfit>
            <snr>3.309036016</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>840.2423255 1789.973952</dataTimeWindow>
            <misfit>0.9704398942</misfit>
            <snr>4.131324768</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>840.2423255 1789.973952</dataTimeWindow>
            <misfit>0.89189538</misfit>
            <snr>1.623862982</snr>
          </component>
          <component phaseCode="S" component="1" active="true">
            <weight>0.25</weight>
            <timeShift>6.5</timeShift>
            <dataTimeWindow>682.9333838 882.9333838</dataTimeWindow>
            <misfit>0.3490604401</misfit>
            <snr>8.451694489</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>5.5</timeShift>
            <dataTimeWindow>682.9333838 882.9333838</dataTimeWindow>
            <misfit>1.221195889</misfit>
            <snr>1.997008562</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430060540.615354.614034" active="true" weight="1">
          <waveformID networkCode="AU" stationCode="BBOO" channelCode="BH">
            <resourceURI>AU.BBOO</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.4272579551</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>771.4508899 1944.233551</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.862731576</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-1.5</timeShift>
            <dataTimeWindow>402.837993 519.1951294</dataTimeWindow>
            <misfit>0.6156885529</misfit>
            <snr>1.7967695</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>918.3939165 1944.233551</dataTimeWindow>
            <misfit>0.8503261852</misfit>
            <snr>3.336666822</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>918.3939165 1944.233551</dataTimeWindow>
            <misfit>0.8996747875</misfit>
            <snr>1.496700287</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.19868204</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>728.3444788 928.3444788</dataTimeWindow>
            <misfit>0.7465877914</misfit>
            <snr>2.075638771</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.1614352167</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>728.3444788 928.3444788</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.058134913</snr>
          </component>
        </stationMomentTensorContribution>
        <stationMomentTensorContribution publicID="MomentTensorStationContribution/20210430060540.615416.614035" active="true" weight="1">
          <waveformID networkCode="IU" stationCode="MBWA" locationCode="00" channelCode="BH">
            <resourceURI>IU.MBWA</resourceURI>
          </waveformID>
          <component phaseCode="L" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>1011.463538 2499.892509</dataTimeWindow>
            <misfit>1</misfit>
            <snr>1.522829533</snr>
          </component>
          <component phaseCode="P" component="0" active="true">
            <weight>1</weight>
            <timeShift>-1.5</timeShift>
            <dataTimeWindow>492.3050743 608.8818359</dataTimeWindow>
            <misfit>0.5596050262</misfit>
            <snr>3.198191166</snr>
          </component>
          <component phaseCode="R" component="0" active="false">
            <weight>1</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>1204.12326 2499.892509</dataTimeWindow>
            <misfit>0.856917181</misfit>
            <snr>2.64085865</snr>
          </component>
          <component phaseCode="R" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>1204.12326 2499.892509</dataTimeWindow>
            <misfit>0.889964571</misfit>
            <snr>1.938743472</snr>
          </component>
          <component phaseCode="S" component="1" active="false">
            <weight>0.25</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>889.626503 1089.626503</dataTimeWindow>
            <misfit>0.5830461121</misfit>
            <snr>6.722157001</snr>
          </component>
          <component phaseCode="S" component="2" active="false">
            <weight>0.5</weight>
            <timeShift>0</timeShift>
            <dataTimeWindow>889.626503 1089.626503</dataTimeWindow>
            <misfit>1</misfit>
            <snr>5.879295826</snr>
          </component>
        </stationMomentTensorContribution>
      </momentTensor>
    </focalMechanism>
    <event publicID="gempa2021ijvk">
      <preferredOriginID>Origin/20210430094923.406014.99645</preferredOriginID>
      <preferredMagnitudeID>Origin/20210430094923.406014.99645/netMag/M</preferredMagnitudeID>
      <preferredFocalMechanismID>FocalMechanism/20210430060540.614651.614026</preferredFocalMechanismID>
      <creationInfo>
        <agencyID>GEMPA</agencyID>
        <author>scevent@gempa-proc</author>
        <creationTime>2021-04-30T05:27:35.164944Z</creationTime>
        <modificationTime>2021-04-30T09:49:24.259856Z</modificationTime>
      </creationInfo>
      <description>
        <text>Santa Cruz Islands</text>
        <type>region name</type>
      </description>
      <originReference>Origin/20210430052734.985523.85085</originReference>
      <originReference>Origin/20210430052852.335811.85253</originReference>
      <originReference>Origin/20210430052906.647836.85374</originReference>
      <originReference>Origin/20210430052956.353827.85503</originReference>
      <originReference>Origin/20210430053040.704638.85662</originReference>
      <originReference>Origin/20210430053139.929603.85951</originReference>
      <originReference>Origin/20210430053158.499233.85979</originReference>
      <originReference>Origin/20210430053829.67918.86662</originReference>
      <originReference>Origin/20210430060321.912081.87745</originReference>
      <originReference>Origin/20210430094923.406014.99645</originReference>
      <focalMechanismReference>FocalMechanism/20210430054646.973651.613243</focalMechanismReference>
      <focalMechanismReference>FocalMechanism/20210430054719.938764.613258</focalMechanismReference>
      <focalMechanismReference>FocalMechanism/20210430054758.63766.613271</focalMechanismReference>
      <focalMechanismReference>FocalMechanism/20210430054838.925691.613371</focalMechanismReference>
      <focalMechanismReference>FocalMechanism/20210430054919.510665.613512</focalMechanismReference>
      <focalMechanismReference>FocalMechanism/20210430055022.509599.613623</focalMechanismReference>
      <focalMechanismReference>FocalMechanism/20210430055105.174854.613660</focalMechanismReference>
      <focalMechanismReference>FocalMechanism/20210430055144.024719.613676</focalMechanismReference>
      <focalMechanismReference>FocalMechanism/20210430055224.677725.613689</focalMechanismReference>
      <focalMechanismReference>FocalMechanism/20210430055237.798688.613702</focalMechanismReference>
      <focalMechanismReference>FocalMechanism/20210430055304.911836.613715</focalMechanismReference>
      <focalMechanismReference>FocalMechanism/20210430055315.679219.613728</focalMechanismReference>
      <focalMechanismReference>FocalMechanism/20210430060422.075728.613830</focalMechanismReference>
      <focalMechanismReference>FocalMechanism/20210430060441.05961.613897</focalMechanismReference>
      <focalMechanismReference>FocalMechanism/20210430060540.614651.614026</focalMechanismReference>
    </event>
  </EventParameters>
</seiscomp>
)EVENTXML";

#endif
