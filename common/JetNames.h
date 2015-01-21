/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 * Copyright (C) 2007 Hottinger Baldwin Messtechnik GmbH
 * Im Tiefen See 45
 * 64293 Darmstadt
 * Germany
 * http://www.hbm.com
 * All rights reserved
 *
 * The copyright to the computer program(s) herein is the property of
 * Hottinger Baldwin Messtechnik GmbH (HBM), Germany. The program(s)
 * may be used and/or copied only with the written permission of HBM
 * or in accordance with the terms and conditions stipulated in the
 * agreement/contract under which the program(s) have been supplied.
 * This copyright notice must not be removed.
 *
 * This Software is licenced by the
 * "General supply and license conditions for software"
 * which is part of the standard terms and conditions of sale from HBM.
*/

/// @file


#ifndef _HBM__JET__JETNAMES_H
#define _HBM__JET__JETNAMES_H
namespace hbm {
	namespace jet {
		static const char DS404_PATH[] = "ds404";

		static const char SYSTEM_PATH[] = "system";
		static const char UUID[] = "uuid";
		static const char MODULE_TYPE[] = "moduleType";
		static const char MODULE_LABEL[] = "moduleLabel";
		static const char FIRMWARE_VERSION[] = "firmwareVersion";
		static const char HOUSING[] = "housing";
		static const char STARTED[] = "started";
		static const char PORT[] = "port";
		static const char HARDWARE_REVISION[] = "hardwareRevision";
		static const char MODULE_REVISION[] = "moduleRevision";
		static const char PLATFORM_ID[] = "platformId";

		static const char ANALOGINPATH[] = "fb/analogIn";
		static const char CANDECODERPATH[] = "fb/canDecoder";
		static const char PROPS[] = "props";
		static const char OUTPUTUNIT[] = "outputUnit";
		static const char OUTPUTRATE[] = "outputRate";
		static const char TIMESHIFT[] = "timeShift";
		static const char SIGNALFORMAT[] = "signalFormat";
		static const char PHYSUNIT[] = "physUnit";
		static const char RTAVAILABLE[] = "rtAvailable";

		static const char ID[] = "id";
		static const char DESCRIPTION[] = "description";

		static const char TYPE[] = "type";
		static const char TEXT[] = "text";
        static const char ERR[] = "error";
		static const char INFO[] = "info";

		static const char SCALTYPE[] = "scalType";
		static const char SCALGAIN[] = "gain";
		static const char SCALLINTAB[] = "lintable";
		static const char SCALZEROSPAN[] = "zerospan";
		static const char ELECZERO[] = "elecZero";
		static const char ELECSPAN[] = "elecSpan";
		static const char PHYSSPAN[] = "physSpan";
		static const char SCALSTRAINGAGE[] = "straingage";
		static const char GAGEFACTOR[] = "gageFactor";
		static const char BRIDGEFACTOR[] = "bridgeFactor";
		static const char SENSORTYPE[] = "sensorType";

		static const char VOLTAGE[] = "voltage";
		static const char CURRENT[] = "current";
		static const char BRIDGE[] = "bridge";
		static const char THERMOCOUPLE[] = "thermocouple";
		static const char INDUCTIVEBRIDGE[] = "indBridge";
		static const char POTI[] = "potentiometer";
		static const char PRESSURE[] = "pressure";
		static const char WIRING[] = "wiring";

		static const char SPAN[] = "span";
		static const char FACTOR[] = "factor";
		static const char TARGET[] = "target";
		static const char OFFSET[] = "offset";
		static const char MIN[] = "min";
		static const char MAX[] = "max";
		static const char UNKNOWN[] = "unknown";
		static const char MAXELECTRICAL[] = "maxElectrical";
		static const char NEGATIVE[] = "negative";
		static const char POSITIVE[] = "positive";

		static const char FILTERTYPE[] = "filterType";
		static const char LOWPASS[] = "lowpass";
		static const char HIGHPASS[] = "highpass";
		static const char FREQUENCY[] = "frequency";

		static const char OUTS[] = "outs";
		static const char SIG[] = "Signal"; // Capital letter in order to be compatibel with signal reference

		static const char SIGREF[] = "ref";
		static const char SIGID[] = "sigid";
		static const char ACTIVE[] = "active";

		static const char MODE[] = "mode";
		static const char CONNECTOR[] = "connector";
		static const char TESTSIGNAL[] = "testSignal";
		static const char PERIODIC[] = "periodic";
		static const char WAVEFORM[] = "waveform";
		static const char AMPLITUDE[] = "amplitude";

		static const char DUTYCYCLE[] = "dutyCycle";
		static const char PHASE[] = "phase";
		static const char DELTA[] = "delta";

		/// ECAT
		static const char ECAT_UPDATERATE[] = "ethercat/updateRate";
		static const char ECAT_STATE[] = "ethercat/state";
		static const char ECAT_STATE_CURRENT[] = "current";
		static const char ECAT_STATE_REQUESTET[] = "requested";
		static const char ECAT_MODE[] = "ethercat/mode";
		static const char ECAT_MODE_AUTO[] = "auto";
		static const char ECAT_MODE_CONFIG[] = "config";

		/// PTP
		static const char PTP_INFO_PATH[] = "ptp/info";
		static const char PTP_GET_OFFSET_MTHPATH[] = "ptp/getOffset";
		static const char PTP_GRANDMASTER[] = "grandmaster";
		static const char PTP_MODE[] = "ptm_mode";
		static const char PTP_MASTER_OFFSET[] = "master_offset";
		static const char PTP_UTC_OFFSET[] = "utc_offset";
		static const char PTP_UTC_VALID_TRACE[] = "utc_valid_trace";
		static const char PTP_FLAGS[] = "ptp_flags";

		/// SYNC
		static const char TSD_METHOD[] = "sync/method";
		static const char TSD_PARAM[] = "sync/param";
		static const char PARAM_TIMESOURCE[] = "timesource";

		static const char PARAM_THRESHOLD[] = "threshold";
		static const char PARAM_NTPSERVER_IP[] = "ntpserver";
		static const char PARAM_PTP4L_CFG[] = "ptp4l";

		static const char TSD_FW_MODULES[] = "sync/fw";
		static const char TSD_FW_BACK[] = "back";
		static const char TSD_FW_FRONT[] = "front";
		static const char TSD_ERROR_PATH[] = "sync/errors";

		static const char APP_PARAM[] = "sync/app";
		static const char APP_MODE_BACK[] = "mode_back";
		static const char APP_MODE_FRONT[] = "mode_front";
		static const char APP_MASTER_UUID_BACK[] = "master_uuid_back";
		static const char APP_MASTER_UUID_FRONT[] = "master_uuid_front";
		static const char APP_MASTER_THRESHOLD_BACK[] = "master_threshold_back";
		static const char APP_MASTER_THRESHOLD_FRONT[] = "master_threshold_front";

		static const char SYNC_STATUS_PATH[] = "sync/status";
		static const char SYNC_STATUS_SYNC[] = "sync";
		static const char SYNC_STATUS_SOURCE[] = "source";
		static const char SYNC_STATUS_MODE_FRONT[] = "frontFW";
		static const char SYNC_STATUS_MODE_BACK[] = "backFW";
		static const char SYNC_SOURCE_INTERNAL[] = "internal";
		static const char SYNC_SOURCE_BACKFW[] = "back_fw";
		static const char SYNC_SOURCE_FRONTFW[] = "front_fw";
		static const char SYNC_SOURCE_NTP[] = "ntp";
		static const char SYNC_SOURCE_IRIG[] = "irig";
		static const char SYNC_SOURCE_PTP[] = "ptp";
		static const char SYNC_SOURCE_ECAT[] = "ecat";
		static const char SYNC_MODE_SINGLE[] = "single";
		static const char SYNC_MODE_SLAVE[] = "slave";
		static const char SYNC_MODE_MASTER[] = "master";

		/// Demux
		static const char DEMUX_BASE_PATH[] = "demux";
		static const char DEMUX_KEEPALIVE[] = "keepalive";
		static const char DEMUX_CANTIMEOUT[] = "cantimeout";
		static const char DEMUX_DEFAULT_VALUE[] =  "1";

		/// methods
		static const char SETINTERNALTESTMODE[] = "setInternalTestMode";
		static const char SETPLLVOLTAGE[] = "setPllVoltage";
		static const char SETPLATFORMSERIALS[] = "setPlatformSerials";
		static const char SETMODULECONFIG[] = "setModuleConfig";
		static const char RESTART[] = "restart";
		static const char SETZERO[] = "setZero";
		static const char RESETZERO[] = "resetZero";
		static const char RESTOREFACTORYDEFAULTS[] = "restoreFactoryDefaults";
		static const char SETLEDMODE[] = "setLedMode";

	}
}
#endif // _JETNAMES_H
