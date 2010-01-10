/**
 * ssca
 * Slow start algorithm, congestion avoidance and fast retransmit.
 * States of congestion control algorithm are implemented here.
 *
 * Copyright 2009 by Columbia University; all rights reserved
 * C, Lee, Oct 18, 2009
 */
#include "touCongestion.h"

/**
 * ssca(touCb *ptb):
 * Initialization of congestion control states.
 */
ssca::ssca(touCb *ptb)
			:tb(ptb) {
  tb->cc_state = TOU_CC_SS;
  tb->snd_cwnd = TOU_SMSS;
  tb->dupackcount = 0;
  tb->snd_ssthresh = TOU_MAX_SSTHRESH;
}

/**
 * addwnd():
 * Adding values(e.g. MSS) to window size base on current control control state
 * This function should be called only when processtou.run() gets "new ack"
 * return 0, if success.
 * retrun 1, if failure.
 */
int ssca::addwnd() {
  int err = 0;
  switch(tb->cc_state) {
    case TOU_CC_ERR:
  	  /* some tou_congestion_control err msg here */
		  err = 1;
		  break;

		case TOU_CC_SS:
			/* while cwnd less than ssthresh, we adopt slow start.*/
			if( tb->snd_cwnd < tb->snd_ssthresh ){
				tb->snd_cwnd += TOU_SMSS;
				tb->snd_cwnd = ckSize(tb->snd_cwnd);
			}else{
			//go to CA
				tb->cc_state = TOU_CC_CA;
				tb->snd_cwnd += (TOU_SMSS*TOU_SMSS/tb->snd_cwnd);
				tb->snd_cwnd = ckSize(tb->snd_cwnd);
			}
			tb->dupackcount = 0;
			break;

		case TOU_CC_CA:
			tb->snd_cwnd += (TOU_SMSS*TOU_SMSS/tb->snd_cwnd);
			tb->snd_cwnd = ckSize(tb->snd_cwnd);
			tb->dupackcount = 0;
			break;

		case TOU_CC_FR:
			//go to CA
			tb->cc_state = TOU_CC_CA;
			tb->snd_cwnd = tb->snd_ssthresh;
			tb->snd_cwnd = ckSize(tb->snd_cwnd);
			tb->dupackcount = 0;
			break;

		default:
			/* some err msg here */
			err = 1;
			break;
	}
	return err;
}/* end of ssca::addwnd() */

u_long ssca::getwnd() {
	tb->snd_wnd = std::min(tb->snd_cwnd, tb->snd_awnd);
	tb->snd_wnd = ckSize(tb->snd_wnd);
	return tb->snd_wnd;
}

/**
 * settwnd():
 * set wnd while time out occurs.
 * return 1 if failure.
 * return 0 if success
 */
int ssca::settwnd() {
	int err = 0;
  
	switch(tb->cc_state) {
		case TOU_CC_ERR:
			/* some err msg here */
			err = 1;
			break;

		case TOU_CC_SS:
			tb->snd_ssthresh = std::max(tb->snd_cwnd/2, (unsigned long)2*TOU_SMSS);
			tb->snd_cwnd = TOU_SMSS;
			tb->dupackcount = 0;
			break;

		case TOU_CC_CA:
		case TOU_CC_FR:
			/* go to SS */
			tb->cc_state = TOU_CC_SS;
			tb->snd_ssthresh = std::max(tb->snd_cwnd/2, (unsigned long)2*TOU_SMSS);
			tb->snd_cwnd = TOU_SMSS;
			tb->dupackcount = 0;
			break;

		default:
			/* some err msg here */
			err = 1;
			break;
	}
	return err;
}

/**
 * setdwnd();
 * set wnd while receives duplicate ack
 * return 1 if failure.
 * return 0 if success.	     
 */
int ssca::setdwnd() {
	int err = 0;
  
	switch(tb->cc_state) {
		case TOU_CC_ERR:
			/* some err msg here */
			err = 1;
			break;

		case TOU_CC_SS:
		case TOU_CC_CA:
			tb->dupackcount++;

			if(tb->dupackcount >= 3) {
				/* 3dup ack should go to FR */
				tb->cc_state = TOU_CC_FR;
				tb->snd_ssthresh = tb->snd_cwnd / 2;
				tb->snd_cwnd = tb->snd_ssthresh + ( 3 * TOU_SMSS );
			}
			break;

		case TOU_CC_FR:
			tb->dupackcount++;
			tb->snd_cwnd += TOU_SMSS;
			tb->snd_cwnd = ckSize(tb->snd_cwnd);
			break;

		default:
			/* some err msg here */
			err = 1;
			break;
	}
	return err;
}/* end of int ssca::setdwnd() */

