/**
 * @file   ptpd_dep.h
 * 
 * @brief  External definitions for inclusion elsewhere.
 * 
 * 
 */

#ifndef PTPD_DEP_H_
#define PTPD_DEP_H_

 /* name System messages */
#define ERROR(x, ...)   message(LOG_ERR, x, ##__VA_ARGS__)
#define PERROR(x, ...)  message(LOG_ERR, x ": %m\n", ##__VA_ARGS__)
#define WARNING(x, ...) message(LOG_WARNING, x, ##__VA_ARGS__)
#define NOTIFY(x, ...)  message(LOG_NOTICE, x, ##__VA_ARGS__)
#define INFO(x, ...)    message(LOG_INFO, x, ##__VA_ARGS__)



/* name Debug messages */
#ifdef PTPD_DBGV
#ifndef PTPD_DBG
#define PTPD_DBG
#endif
#define DBGV(x, ...) message(LOG_DEBUG, x, ##__VA_ARGS__)
#else
#define DBGV(x, ...)
#endif

#ifdef PTPD_DBG
#define DBG(x, ...) message(LOG_DEBUG, x, ##__VA_ARGS__)
#else
#define DBG(x, ...)
#endif



/* name Endian corrections */
#if defined(PTPD_MSBF)
#define shift8(x,y)   ( (x) << ((3-y)<<3) )
#define shift16(x,y)  ( (x) << ((1-y)<<4) )
#elif defined(PTPD_LSBF)
#define shift8(x,y)   ( (x) << ((y)<<3) )
#define shift16(x,y)  ( (x) << ((y)<<4) )
#endif

#define flip16(x) htons(x)
#define flip32(x) htonl(x)

/* i don't know any target platforms that do not have htons and htonl,
   but here are generic funtions just in case */
/*
#if defined(PTPD_MSBF)
#define flip16(x) (x)
#define flip32(x) (x)
#elif defined(PTPD_LSBF)
static inline Integer16 flip16(Integer16 x)
{
   return (((x) >> 8) & 0x00ff) | (((x) << 8) & 0xff00);
}

static inline Integer32 flip32(x)
{
  return (((x) >> 24) & 0x000000ff) | (((x) >> 8 ) & 0x0000ff00) |
         (((x) << 8 ) & 0x00ff0000) | (((x) << 24) & 0xff000000);
}
#endif
*/




/* name Bit array manipulations */
#define getFlag(x,y)  !!( *(UInteger8*)((x)+((y)<8?1:0)) &   (1<<((y)<8?(y):(y)-8)) )
#define setFlag(x,y)    ( *(UInteger8*)((x)+((y)<8?1:0)) |=   1<<((y)<8?(y):(y)-8)  )
#define clearFlag(x,y)  ( *(UInteger8*)((x)+((y)<8?1:0)) &= ~(1<<((y)<8?(y):(y)-8)) )




/** \name msg.c
 *-Pack and unpack PTP messages */
void msgUnpackHeader(char*,MsgHeader*);
void msgUnpackAnnounce (char*,MsgAnnounce*);
void msgUnpackSync(char*,MsgSync*);
void msgUnpackFollowUp(char*,MsgFollowUp*);
void msgUnpackPDelayReq(char*,MsgPDelayReq*);
void msgUnpackPDelayResp(char*,MsgPDelayResp*);
void msgUnpackPDelayRespFollowUp(char*,MsgPDelayRespFollowUp*);
void msgUnpackManagement(char*,MsgManagement*);
UInteger8 msgUnloadManagement(char*,MsgManagement*,PtpClock*,RunTimeOpts*);
void msgUnpackManagementPayload(char *buf, MsgManagement *manage);
void msgPackHeader(char*,PtpClock*);
void msgPackAnnounce(char*,PtpClock*);
void msgPackSync(char*,PTP_Timestamp*,PtpClock*);
void msgPackFollowUp(char*,PTP_Timestamp*,PtpClock*);
void msgPackPDelayReq(char*,PTP_Timestamp*,PtpClock*);
void msgPackPDelayResp(char*,MsgHeader*,PTP_Timestamp*,PtpClock*);
void msgPackPDelayRespFollowUp(char*,MsgHeader*,PTP_Timestamp*,PtpClock*);
UInteger16 msgPackManagement(char*,MsgManagement*,PtpClock*);
UInteger16 msgPackManagementResponse(char*,MsgHeader*,MsgManagement*,PtpClock*);

void msgDump(PtpClock *ptpClock);
void msgDebugHeader(MsgHeader *header);
void msgDebugSync(MsgSync *sync);
void msgDebugAnnounce(MsgAnnounce *announce);
void msgDebugDelayReq(MsgDelayReq *req);
void msgDebugFollowUp(MsgFollowUp *follow);
void msgDebugDelayResp(MsgDelayResp *resp);
void msgDebugManagement(MsgManagement *manage);




/** \name net.c (Unix API dependent)
 * -Init network stuff, send and receive datas*/
bool netInit(NetPath*,RunTimeOpts*,PtpClock*);
bool netShutdown(NetPath*);
int netSelect(TimeInternal*,NetPath*);
ssize_t netRecvEvent(Octet*,TimeInternal*,NetPath*);
ssize_t netRecvGeneral(Octet*,TimeInternal*,NetPath*);
ssize_t netSendEvent(Octet*,UInteger16,NetPath*);
ssize_t netSendGeneral(Octet*,UInteger16,NetPath*);
ssize_t netSendPeerGeneral(Octet*,UInteger16,NetPath*);
ssize_t netSendPeerEvent(Octet*,UInteger16,NetPath*);




/** \name servo.c
 * -Clock servo*/
void initClock(RunTimeOpts*,PtpClock*);
void updatePeerDelay (one_way_delay_filter*, RunTimeOpts*,PtpClock*,TimeInternal*,bool);
void updateDelay (one_way_delay_filter*, RunTimeOpts*, PtpClock*,TimeInternal*);
void updateOffset(TimeInternal*,TimeInternal*,
  offset_from_master_filter*,RunTimeOpts*,PtpClock*,TimeInternal*);
void updateClock(RunTimeOpts*,PtpClock*);



/** \name startup.c (Unix API dependent)
 * -Handle with runtime options*/
int logToFile(void);
int recordToFile(void);
PtpClock * ptpdStartup(int,char**,Integer16*,RunTimeOpts*);
void ptpdShutdown(void);




/** \name sys.c (Unix API dependent)
 * -Manage timing system API*/

void message(int priority, const char *format, ...);
void displayStats(RunTimeOpts *rtOpts, PtpClock *ptpClock);
bool nanoSleep(TimeInternal*);
void getTime(TimeInternal*);
void setTime(TimeInternal*);
double getRand(void);
bool adjFreq(Integer32);




/** \name timer.c (Unix API dependent)
 * -Handle with timers*/
void initTimer(void);
void timerUpdate(IntervalTimer*);
void timerStop(UInteger16,IntervalTimer*);
//void timerStart(UInteger16,UInteger16,IntervalTimer*);
void timerStart(UInteger16,float,IntervalTimer*);
bool timerExpired(UInteger16,IntervalTimer*);



/*Test functions*/


#endif /*PTPD_DEP_H_*/
