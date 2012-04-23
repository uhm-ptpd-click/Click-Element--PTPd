/*
 * sampleelt.{cc,hh} -- sample package element
 * Eddie Kohler
 *
 * Copyright (c) 2000 Massachusetts Institute of Technology
 * Copyright (c) 2000 Mazu Networks, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 */

// ALWAYS INCLUDE <click/config.h> FIRST
#include <click/config.h>

#include "ptpd2pack.hh"
#include "ptpd.hh"
#include "datatypes.hh"
CLICK_DECLS

PTPd2PackageElement::PTPd2PackageElement()
{
}

PTPd2PackageElement::~PTPd2PackageElement()
{
}


int
PTPd2PackageElement::initialize(ErrorHandler *errh)
{
	printf("Successfully linked with PTPd2 package!");
    	printf("KENN !!!");
    	printf("PTPd2 !!!");
	
	RunTimeOpts rtOpts;
	PtpClock *ptpClock;
	Integer16 ret;	
	int argc;
	char **argv;	

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
	rtOpts.currentUtcOffset = DEFAULT_UTC_OFFSET;
	rtOpts.noAdjust  = NO_ADJUST;  // false
	rtOpts.maxAdjust = DEFAULT_CLOCK_ADJUST_LIMIT;
	rtOpts.maxStep   = DEFAULT_CLOCK_STEP_LIMIT;
	rtOpts.maxDelay  = DEFAULT_DELAY_LIMIT;
	rtOpts.displayPackets = FALSE;
	rtOpts.ap = DEFAULT_AP;
	rtOpts.ai = DEFAULT_AI;
	rtOpts.s = DEFAULT_DELAY_S;
	rtOpts.inboundLatency.nanoseconds = DEFAULT_INBOUND_LATENCY;
	rtOpts.outboundLatency.nanoseconds = DEFAULT_OUTBOUND_LATENCY;
	rtOpts.max_foreign_records = DEFAULT_MAX_FOREIGN_RECORDS;
	rtOpts.logFd = -1;
	rtOpts.recordFP = NULL;
	rtOpts.useSysLog = FALSE;
	rtOpts.ttl = 1;

	rtOpts.probe = FALSE;
	rtOpts.quickPoll = 0;

	// Initialize run time options with command line arguments
	if (!(ptpClock = ptpdStartup(argc, argv, &ret, &rtOpts)))
		return ret;

       	NOTIFY("ptpd %s started\n", VERSION_STRING);
	// do the protocol engine
	protocol(&rtOpts, ptpClock);
	// forever loop..
	
        ptpdShutdown();
	NOTIFY("self shutdown, probably due to an error\n");
	
	return 0;

}

CLICK_ENDDECLS
EXPORT_ELEMENT(PTPd2PackageElement)
