/**
 * @file   ptpd.c
 * @date   Wed Jun 23 10:13:38 2010
 *
 * @brief  The main() function for the PTP daemon
 *
 * This file contains very little code, as should be obvious,
 * and only serves to tie together the rest of the daemon.
 * All of the default options are set here, but command line
 * arguments are processed in the ptpdStartup() routine called
 * below.
 */

/*
#include "ptpd.hh"


RunTimeOpts rtOpts;			//statically allocated run-time
					// configuration data

int
main(int argc, char **argv)
{
	PtpClock *ptpClock;
	Integer16 ret;

	// initialize run-time options to default values



	memset(&rtOpts, 0, sizeof(rtOpts));
	rtOpts.announceInterval = DEFAULT_ANNOUNCE_INTERVAL;
	rtOpts.syncInterval = DEFAULT_SYNC_INTERVAL;
	rtOpts.clockQuality.clockAccuracy = DEFAULT_CLOCK_ACCURACY;
	rtOpts.clockQuality.clockClass = DEFAULT_CLOCK_CLASS;
	rtOpts.clockQuality.offsetScaledLogVariance = DEFAULT_CLOCK_VARIANCE;
	rtOpts.priority1 = DEFAULT_PRIORITY1;
	rtOpts.priority2 = DEFAULT_PRIORITY2;
	rtOpts.domainNumber = DEFAULT_DOMAIN_NUMBER;
	// rtOpts.slaveOnly = FALSE;
	rtOpts.currentUtcOffset = DEFAULT_UTC_OFFSET;
	// rtOpts.ifaceName
	rtOpts.noAdjust  = NO_ADJUST;
	rtOpts.maxAdjust = DEFAULT_CLOCK_ADJUST_LIMIT;
	rtOpts.maxStep   = DEFAULT_CLOCK_STEP_LIMIT;
	rtOpts.maxDelay  = DEFAULT_DELAY_LIMIT;
	// rtOpts.displayStats = FALSE;
	// rtOpts.csvStats = FALSE;
	rtOpts.displayPackets = FALSE;
	// rtOpts.unicastAddress
	rtOpts.ap = DEFAULT_AP;
	rtOpts.ai = DEFAULT_AI;
	rtOpts.s = DEFAULT_DELAY_S;
	rtOpts.inboundLatency.nanoseconds = DEFAULT_INBOUND_LATENCY;
	rtOpts.outboundLatency.nanoseconds = DEFAULT_OUTBOUND_LATENCY;
	rtOpts.max_foreign_records = DEFAULT_MAX_FOREIGN_RECORDS;
	// rtOpts.ethernet_mode = FALSE;
	// rtOpts.E2E_mode = FALSE;
	// rtOpts.offset_first_updated = FALSE;
	// rtOpts.file[0] = 0;
	rtOpts.logFd = -1;
	rtOpts.recordFP = NULL;
	rtOpts.useSysLog = FALSE;
	rtOpts.ttl = 1;

	rtOpts.probe = FALSE;
	rtOpts.quickPoll = 0;

	// Initialize run time options with command line arguments
	if (!(ptpClock = ptpdStartup(argc, argv, &ret, &rtOpts)))
		return ret;

	
        if (rtOpts.probe) {
                // not yet implemented
        }
        else {
		NOTIFY("ptpd %s started\n", VERSION_STRING);

		//do the protocol engine 
		protocol(&rtOpts, ptpClock);
		//forever loop..
	}

        ptpdShutdown();

	NOTIFY("self shutdown, probably due to an error\n");

	return 1;
}*/
