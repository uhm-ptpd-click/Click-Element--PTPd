#ifndef DATATYPES_H_
#define DATATYPES_H_


#include <stdio.h> 
#include <sys/param.h>
#include <stdlib.h>
#include <stdbool.h>
/*Struct defined in spec*/


/**
* brief Main structures used in ptpdv2
* This header file defines structures defined by the spec,
* main program data structure, and all messages structures
*/

/* brief The TimeInterval type represents time intervals */
typedef struct {
	Integer64 scaledNanoseconds;
} TimeInterval;


/* brief The PTP_Timestamp type represents a positive time with respect to the epoch */
// THE CLICK ROUTER ALREADY INCLUDES A PTP_Timestamp TYPE

typedef struct  {
	UInteger48 secondsField;
	UInteger32 nanosecondsField;
} PTP_Timestamp;



/* brief The ClockIdentity type identifies a clock */
typedef Octet ClockIdentity[CLOCK_IDENTITY_LENGTH];


/* brief The PortIdentity identifies a PTP port. */
typedef struct {
	ClockIdentity clockIdentity;
	UInteger16 portNumber;
} PortIdentity;


/* brief The PortAdress type represents the protocol address of a PTP port */
typedef struct {
	Enumeration16 networkProtocol;
	UInteger16 adressLength;
	Octet* adressField;
} PortAdress;


/* brief The ClockQuality represents the quality of a clock */
typedef struct {
	UInteger8 clockClass;
	Enumeration8 clockAccuracy;
	UInteger16 offsetScaledLogVariance;
} ClockQuality;


/* brief The TLV type represents TLV extension fields */
typedef struct {
	Enumeration16 tlvType;
	UInteger16 lengthField;
	Octet* valueField;
} TLV;


/* brief The PTPText data type is used to represent textual material in PTP messages */
typedef struct {
	UInteger8 lengthField;
	Octet* textField;
} PTPText;


/* brief The FaultRecord type is used to construct fault logs */
typedef struct {
	UInteger16 faultRecordLength;
	PTP_Timestamp faultTime;
	Enumeration8 severityCode;
	PTPText faultName;
	PTPText faultValue;
	PTPText faultDescription;
} FaultRecord;


/* brief The common header for all PTP messages (Table 18 of the spec) */
/* Message header */
typedef struct {
 	Nibble transportSpecific;
 	Enumeration4 messageType;
 	UInteger4 versionPTP;
 	UInteger16 messageLength;
 	UInteger8 domainNumber;
 	Octet flagField[2];
 	Integer64 correctionfield;
	PortIdentity sourcePortIdentity;
 	UInteger16 sequenceId;
 	UInteger8 controlField;
 	Integer8 logMessageInterval;
} MsgHeader;


/* brief Announce message fields (Table 25 of the spec) */
/*Announce Message */
typedef struct {
	PTP_Timestamp originPTP_Timestamp;
	Integer16 currentUtcOffset;
	UInteger8 grandmasterPriority1;
	ClockQuality grandmasterClockQuality;
	UInteger8 grandmasterPriority2;
	ClockIdentity grandmasterIdentity;
	UInteger16 stepsRemoved;
	Enumeration8 timeSource;
}MsgAnnounce;


/* brief Sync message fields (Table 26 of the spec) */
/*Sync Message */
typedef struct {
	PTP_Timestamp originPTP_Timestamp;
}MsgSync;


/* brief DelayReq message fields (Table 26 of the spec) */
/*DelayReq Message */
typedef struct {
	PTP_Timestamp originPTP_Timestamp;
}MsgDelayReq;


/* brief DelayResp message fields (Table 30 of the spec) */
/*delayResp Message*/
typedef struct {
	PTP_Timestamp receivePTP_Timestamp;
	PortIdentity requestingPortIdentity;
}MsgDelayResp;


/* brief FollowUp message fields (Table 27 of the spec) */
/*Follow-up Message*/
typedef struct {
	PTP_Timestamp preciseOriginPTP_Timestamp;
}MsgFollowUp;


/* brief PDelayReq message fields (Table 29 of the spec) */
/*PdelayReq Message*/
typedef struct {
	PTP_Timestamp originPTP_Timestamp;

}MsgPDelayReq;


/* brief PDelayResp message fields (Table 30 of the spec) */
/*PdelayResp Message*/
typedef struct {
	PTP_Timestamp requestReceiptPTP_Timestamp;
	PortIdentity requestingPortIdentity;
}MsgPDelayResp;


/* brief PDelayRespFollowUp message fields (Table 31 of the spec) */
/*PdelayRespFollowUp Message*/
typedef struct {
	PTP_Timestamp responseOriginPTP_Timestamp;
	PortIdentity requestingPortIdentity;
}MsgPDelayRespFollowUp;


/* brief Signaling message fields (Table 33 of the spec) */
/*Signaling Message*/
typedef struct {
	PortIdentity targetPortIdentity;
	char* tlv;
}MsgSignaling;


/* brief Management message fields (Table 37 of the spec) */
/*management Message*/
typedef struct {
	PortIdentity targetPortIdentity;
	UInteger8 startingBoundaryHops;
	UInteger8 boundaryHops;
	Enumeration4 actionField;
	char* tlv;
}MsgManagement;



/* brief Time structure to handle Linux time information */
typedef struct {
  Integer32 seconds;
  Integer32 nanoseconds;
} TimeInternal;


/* brief Structure used as a timer */
typedef struct {
  float interval;
  float left;
  bool expire;
} IntervalTimer;


/* brief ForeignMasterRecord is used to manage foreign masters */
typedef struct
{
  PortIdentity foreignMasterPortIdentity;
  UInteger16 foreignMasterAnnounceMessages;

  //This one is not in the spec
  MsgAnnounce  announce;
  MsgHeader    header;

} ForeignMasterRecord;



/**struct PtpClock
 * brief Main program data structure */

/* main program data structure */
typedef struct {
	/* Default data set */

	/*Static members*/
	bool twoStepFlag;
	ClockIdentity clockIdentity;
	UInteger16 numberPorts;

	/*Dynamic members*/
	ClockQuality clockQuality;

	/*Configurable members*/
	UInteger8 priority1;
	UInteger8 priority2;
	UInteger8 domainNumber;
	bool slaveOnly;

	/* Current data set */

	/*Dynamic members*/
	UInteger16 stepsRemoved;
	TimeInternal offsetFromMaster;
	TimeInternal meanPathDelay;

	/* Parent data set */

	/*Dynamic members*/
	PortIdentity parentPortIdentity;
	bool parentStats;
	UInteger16 observedParentOffsetScaledLogVariance;
	Integer32 observedParentClockPhaseChangeRate;
	ClockIdentity grandmasterIdentity;
	ClockQuality grandmasterClockQuality;
	UInteger8 grandmasterPriority1;
	UInteger8 grandmasterPriority2;

	/* Global time properties data set */

	/*Dynamic members*/
	Integer16 currentUtcOffset;
	bool currentUtcOffsetValid;
	bool leap59;
	bool leap61;
	bool timeTraceable;
	bool frequencyTraceable;
	bool ptpTimescale;
	Enumeration8 timeSource;

	/* Port configuration data set */

	/*Static members*/
	PortIdentity portIdentity;

	/*Dynamic members*/
	Enumeration8 portState;
	Integer8 logMinDelayReqInterval;
	TimeInternal peerMeanPathDelay;
 
	/*Configurable members*/
	Integer8 logAnnounceInterval;
	UInteger8 announceReceiptTimeout;
	Integer8 logSyncInterval;
	Enumeration8 delayMechanism;
	Integer8 logMinPdelayReqInterval;
	UInteger4 versionNumber;


	/* Foreign master data set */
	ForeignMasterRecord *foreign;

	/* Other things we need for the protocol */
	UInteger16 number_foreign_records;
	Integer16  max_foreign_records;
	Integer16  foreign_record_i;
	Integer16  foreign_record_best;
	bool  record_update;


	MsgHeader msgTmpHeader;
	
	//union {
		MsgSync  sync;
		MsgFollowUp  follow;
		MsgDelayReq  req;
		MsgDelayResp resp;
		MsgPDelayReq  preq;
		MsgPDelayResp  presp;
		MsgPDelayRespFollowUp  prespfollow;
		MsgManagement  manage;
		MsgAnnounce  announce;
		MsgSignaling signaling;
	//} msgTmp;


	Octet msgObuf[PACKET_SIZE];
	Octet msgIbuf[PACKET_SIZE];

	TimeInternal  master_to_slave_delay;
	TimeInternal  slave_to_master_delay;
	Integer32     observed_drift;

	TimeInternal  pdelay_req_receive_time;
	TimeInternal  pdelay_req_send_time;
	TimeInternal  pdelay_resp_receive_time;
	TimeInternal  pdelay_resp_send_time;
	TimeInternal  sync_receive_time;
	TimeInternal  delay_req_send_time;
	TimeInternal  delay_req_receive_time;
	MsgHeader	PdelayReqHeader;
	MsgHeader 	delayReqHeader;
	TimeInternal	pdelayMS;
	TimeInternal	pdelaySM;
	TimeInternal  delayMS;
	TimeInternal	delaySM;
	TimeInternal  lastSyncCorrectionField;
	TimeInternal  lastPdelayRespCorrectionField;


	double  R;

	bool  sentPDelayReq;
	UInteger16  sentPDelayReqSequenceId;
	UInteger16  sentDelayReqSequenceId;
	UInteger16  sentSyncSequenceId;
	UInteger16  sentAnnounceSequenceId;
	UInteger16  recvPDelayReqSequenceId;
	UInteger16  recvSyncSequenceId;
	bool  waitingForFollow;

	offset_from_master_filter  ofm_filt;
	one_way_delay_filter  owd_filt;

	bool message_activity;

	IntervalTimer  itimer[TIMER_ARRAY_SIZE];

	NetPath netPath;

	/*Usefull to init network stuff*/
	UInteger8 port_communication_technology;
	Octet port_uuid_field[PTP_UUID_LENGTH];

} PtpClock;


/**struct RunTimeOpts
 * brief Program options set at run-time*/

/* program options set at run-time */
typedef struct {
	Integer8 announceInterval;
	Integer8 syncInterval;
	ClockQuality clockQuality;
	UInteger8 priority1;
	UInteger8 priority2;
	UInteger8 domainNumber;
	bool slaveOnly;
	Integer16 currentUtcOffset;
	Octet ifaceName[IFACE_NAME_LENGTH];
	bool noAdjust;
	Integer32 maxAdjust; /* Max number of ns off, past which we no longer adjust the clock */
	Integer32 maxStep;   /* Max number of ns to slew-only, past which we will step the clock */
	Integer32 maxDelay;  /* Max number of ns of delay, past which we discard the measurement */
	bool displayStats;
	bool csvStats;
	bool displayPackets;
	Octet unicastAddress[MAXHOSTNAMELEN];
	Integer16 ap, ai;
	Integer16 s;
	TimeInternal inboundLatency, outboundLatency;
	Integer16 max_foreign_records;
	bool ethernet_mode;
	bool E2E_mode;
	bool offset_first_updated;
	char logFile[PATH_MAX];
	int logFd;
	bool useSysLog;
	int ttl;
	char recordFile[PATH_MAX];
	FILE *recordFP;

	bool probe;      // Management probes not implemented yet
        bool quickPoll;  // Management probes not implemented yet

} RunTimeOpts;

#endif /*DATATYPES_H_*/
