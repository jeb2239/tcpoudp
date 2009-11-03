/*************************************************************
 * Congestion control header 
 * **********************************************************/

//segment size definition
#include<stdlib.h>
#include <iostream>
#include <algorithm>
#include "touControlBlock.h"
#define TOU_SMSS                1464
#define TOU_RMSS                536
#define TOU_MAX_SSTHRESH        65535
#define u_long  unsigned long
class ssca {
private:
    touCb		*tb;  

	public:
    ssca(){}
    /* init cwnd to one segment
     * init ssthresh to default */
    ssca(touCb *);
    ~ssca(){}

    /* while xmitting successfully */
    void addwnd();
    unsigned long getwnd();

    /* pkgloss: while timeout occring */
    void settwnd();

    /* pkgloss: while duplicate ACKs */
    void setdwnd();

  

};
