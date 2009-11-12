/*************************************************************
 * Congestion control header 
 * Slow start, Congestion avoidance, Fast ReXmit
 *
 * **********************************************************/

/* segment size definition */
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include "touControlBlock.h"

/* TOU_SMSS should be exactly same as TOU_MSS */
#define TOU_SMSS                100 
#define TOU_RMSS                536
#define TOU_MAX_SSTHRESH        1000	//1K
#define TOU_MAX_CIRCULAR_BUF		40960	//40K

/* Congestion Control Window States */
#define TOU_CC_ERR	0
#define TOU_CC_SS		1
#define	TOU_CC_CA		2
#define	TOU_CC_FR		3

class ssca {
  private:
		inline unsigned long ckSize(unsigned long wnd){
			return ((wnd>(TOU_MAX_CIRCULAR_BUF/2)) ? (unsigned long)(TOU_MAX_CIRCULAR_BUF/2) : wnd);
		}

    touCb		*tb;  

  public:
    ssca(){}
    /* init cwnd to one segment
     * init ssthresh to default */
    ssca(touCb *);
    ~ssca(){}

    /* while xmitting successfully(i.e. get new ACK)
     * return 0, if success.
     * return 1, if failure.			 */
    int addwnd();
    unsigned long getwnd();

    /* pkgloss: while timeout occring */
    int settwnd();

    /* pkgloss: while duplicate ACKs */
    int setdwnd();

};

