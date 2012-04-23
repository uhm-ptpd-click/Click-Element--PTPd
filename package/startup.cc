/**
 * @file   startup.c
 * @date   Wed Jun 23 09:33:27 2010
 * 
 * @brief  Code to handle daemon startup, including command line args
 * 
 * The function in this file are called when the daemon starts up
 * and include the getopt() command line argument parsing.
 */

#include "ptpd.hh"

PtpClock *ptpClock;

void 
catch_close(int sig)
{
	ptpdShutdown();

	switch (sig) {
	case SIGINT:
		NOTIFY("shutdown on interrupt signal\n");
		break;
	case SIGTERM:
		NOTIFY("shutdown on terminate signal\n");
		break;
	default:
		NOTIFY("shutdown on ? signal\n");
	}
	exit(0);
}

/** 
 * Signal handler for HUP which tells us to swap the log file.
 * 
 * @param sig 
 */
void 
catch_sighup(int sig)
{
	if(!logToFile())
		NOTIFY("SIGHUP logToFile failed\n");
	if(!recordToFile())
		NOTIFY("SIGHUP recordToFile failed\n");

	NOTIFY("I've been SIGHUP'd\n");
}

/** 
 * Log output to a file
 * 
 * 
 * @return True if success, False if failure
 */
int 
logToFile()
{
	extern RunTimeOpts rtOpts;
	if(rtOpts.logFd != -1)
		close(rtOpts.logFd);
	
	if((rtOpts.logFd = creat(rtOpts.logFile, 0444)) != -1) {
		dup2(rtOpts.logFd, STDOUT_FILENO);
		dup2(rtOpts.logFd, STDERR_FILENO);
	}
	return rtOpts.logFd != -1;
}

/** 
 * Record quality data for later correlation
 * 
 * 
 * @return True if success, False if failure
 */
int
recordToFile()
{
	extern RunTimeOpts rtOpts;

	if (rtOpts.recordFP != NULL)
		fclose(rtOpts.recordFP);

	if ((rtOpts.recordFP = fopen(rtOpts.recordFile, "w")) == NULL)
		PERROR("could not open sync recording file");
	else
		setlinebuf(rtOpts.recordFP);
	return (rtOpts.recordFP != NULL);
}

void 
ptpdShutdown()
{
	netShutdown(&ptpClock->netPath);

	free(ptpClock->foreign);
	free(ptpClock);
}

PtpClock *
ptpdStartup(int argc, char **argv, Integer16 * ret, RunTimeOpts * rtOpts)
{
	int c, nondaemon = 1; //SET TO ONE BY KENO
	int noclose = 0;

	printf("Here: Ptpdstartup\n");
	
	
	// Run in command line (non-daemon) mode
	// case 'c': nondaemon = 1;
		
	// Specify the sync interval in 2^NUMBER seconds
	// case 'y': rtOpts->syncInterval = strtol(optarg, 0, 0);
	

	ptpClock = (PtpClock *) calloc(1, sizeof(PtpClock));
	if (!ptpClock) {
		PERROR("failed to allocate memory for protocol engine data");
		*ret = 2;
		return 0;
	} else {
		DBG("allocated %d bytes for protocol engine data\n", 
		    (int)sizeof(PtpClock));
		ptpClock->foreign = (ForeignMasterRecord *)
			calloc(rtOpts->max_foreign_records, 
			       sizeof(ForeignMasterRecord));
		if (!ptpClock->foreign) {
			PERROR("failed to allocate memory for foreign "
			       "master data");
			*ret = 2;
			free(ptpClock);
			return 0;
		} else {
			DBG("allocated %d bytes for foreign master data\n", 
			    (int)(rtOpts->max_foreign_records * 
				  sizeof(ForeignMasterRecord)));
		}
	}

	/* Init to 0 net buffer */
	memset(ptpClock->msgIbuf, 0, PACKET_SIZE);
	memset(ptpClock->msgObuf, 0, PACKET_SIZE);

	ptpClock->observed_drift = 0;

	signal(SIGINT, catch_close);
	signal(SIGTERM, catch_close);
	signal(SIGHUP, catch_sighup);

	*ret = 0;

	return ptpClock;
}
