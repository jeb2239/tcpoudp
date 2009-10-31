/*************************************************************
 * Congestion control header 
 * **********************************************************/
#include "touheader.h"

//segment size definition
#define TOU_SMSS                1464
#define TOU_RMSS                536
#define TOU_MAX_SSTHRESH        65535

class ssca {
  public:
    ssca();
    /* init cwnd to one segment
     * init ssthresh to default */
    ssca(toucb *ptb);
    ~ssca();

    /* while xmitting successfully */
    void addwnd();
    u_long getwnd();

    /* pkgloss: while timeout occring */
    void settwnd();

    /* pkgloss: while duplicate ACKs */
    void setdwnd();

  private:
    toucb		*tb;

};
