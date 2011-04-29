/*************************************************************
 * Congestion control header 
 * Slow start, Congestion avoidance, Fast ReXmit
 *
 * **********************************************************/

/* segment size definition */
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include "tou_control_block.h"
#include "tou_boost.h"

//Ethernet 1500-20-24
#ifndef TOU_MSS
#define TOU_MSS				1456
#endif
/* TOU_SMSS should be exactly same as TOU_MSS */
#define TOU_SMSS			1456
#define TOU_RMSS			1456
//#define TOU_MAX_SSTHRESH	65535
#define TOU_MAX_SSTHRESH	2147483647

#define TOU_MAX_TIMEO		64000		//64 SECs
#define TOU_INIT_TIMEO		3000		//RFC 1122 [Bra89]
#define TOU_INIT_RTO_BETA	2			//TCP recommends a value of b=2 as a reasonable balance
#define TOU_INIT_TIMEO_PP	0			//initial pkt time out postpone coefficient
#define TOU_INIT_ALPHA		0.125		

#define TCPREXMTTHRESH		3			//threshold of duplicate ACKs

/* #define TOU_MAX_CIRCULAR_BUF	8388608	//TOU's circular buffer size: 8M */
#define TOU_MAX_CIRCULAR_BUF	6291456	//TOU's circular buffer size: 6M */
/* #define TOU_MAX_CIRCULAR_BUF	2097152	//TOU's circular buffer size: 2M */
/* #define TOU_MAX_CIRCULAR_BUF	65536	//TOU's circular buffer size: 64k */

/* Congestion Control Window States */
#define TOU_CC_ERR		0			//congestion control: error state
#define TOU_CC_SS		1			//congestion control: slow start
#define	TOU_CC_CA		2			//congestion control: congestion avoidance
#define	TOU_CC_FR		3			//congestion control: fast retransmit

class ssca {
  private:
	inline unsigned long ckSize(unsigned long wnd){
		return ((wnd>(TOU_MAX_CIRCULAR_BUF/2)) ? (unsigned long)
				(TOU_MAX_CIRCULAR_BUF/2) : wnd);
	}

	tou_cb		*tc;  
	boost::mutex	sscamutex;

  public:
    ssca(){}
    /* init cwnd to one segment
     * init ssthresh to default */
    ssca(tou_cb *);
    ~ssca(){}

    /* while xmitting successfully(i.e. get new ACK)
     * return 0, if success.
     * return 1, if failure.	*/
    int addwnd();
    unsigned long getwnd();

    /* pkgloss: while timeout occring */
    int settwnd();

    /* pkgloss: while duplicate ACKs */
    int setdwnd(u_long ackseq);

};
