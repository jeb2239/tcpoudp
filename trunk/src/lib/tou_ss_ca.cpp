/*************************************************************
 * Slow start algorithm and Congestion control algorithm
 * C, Lee, Oct 18, 2009
 * **********************************************************/

#include "toucong.h"

ssca::ssca(toucb *ptb):tb(ptb) {
  tb->snd_cwnd = TOU_SMSS;
  tb->snd_ssthresh = TOU_MAX_SSTHRESH;
}
/* Add value to window size depends on slow start or congestion
 * control mechanism.
 * While cwnd equal to ssthresh, we adopt slow start.*/
void ssca::addwnd() {
  if( tb->snd_cwnd <= tb->snd_ssthresh ){
    // Slow start
    tb->snd_cwnd += TOU_SMSS;
  }else{
    tb->snd_cwnd += TOU_SMSS*TOU_SMSS/tb->snd_cwnd;
  }
}

u_long ssca::getwnd() {
  u_long curwnd = std::min(tb->snd_cwnd, tb->snd_awnd);
  return curwnd;
}

void ssca::settwnd() {
  u_long curwnd = std::min(tb->snd_cwnd, tb->snd_awnd);
  tb->snd_ssthresh = std::max(curwnd/2, (unsigned long)2*TOU_SMSS);
  tb->snd_cwnd = TOU_SMSS;
}

void ssca::setdwnd() {
  u_long curwnd = std::min(tb->snd_cwnd, tb->snd_awnd);
  if( curwnd >= 2*TOU_SMSS )
      tb->snd_ssthresh = curwnd/2;
}

