/**
 * ssca
 * Slow start algorithm, congestion avoidance and fast retransmit.
 * States of congestion control algorithm are implemented here.
 *
 * Copyright 2009 by Columbia University; all rights reserved
 * C, Lee, Oct 18, 2009
 */
#include "tou_congestion.h"

/**
 * ssca(tou_cb *ptc):
 * Initialization of congestion control states.
 */
ssca::ssca(tou_cb *ptc)
			:tc(ptc) {
  tc->cc_state = TOU_CC_SS;
  tc->snd_cwnd = TOU_SMSS;
  tc->snd_awnd = TOU_SMSS;
  tc->snd_wnd = TOU_SMSS;
  tc->rcv_wnd = TOU_SMSS;
  tc->dupackcount = 0;
  tc->snd_ssthresh = TOU_MAX_SSTHRESH;

}

/**
 * addwnd():
 * Adding values(e.g. MSS) to window size base on current control control state
 * This function should be called only when processtou.run() gets "new ack"
 * return 0, if success.
 * retrun 1, if failure.
 */
int ssca::addwnd() {

  boost::mutex::scoped_lock lock(sscamutex);

  int err = 0;
  switch(tc->cc_state) {
    case TOU_CC_ERR:
  	  /* some tou_congestion_control err msg here */
		  err = 1;
		  break;

		case TOU_CC_SS:
			/* while cwnd less than ssthresh, we adopt slow start.*/
			if( tc->snd_cwnd < tc->snd_ssthresh ){
				tc->snd_cwnd += TOU_SMSS;
				tc->snd_cwnd = ckSize(tc->snd_cwnd);
			}else{
			//go to CA
				tc->cc_state = TOU_CC_CA;
				tc->snd_cwnd += (TOU_SMSS*TOU_SMSS/tc->snd_cwnd);
				tc->snd_cwnd = ckSize(tc->snd_cwnd);
			}
			tc->dupackcount = 0;
			break;

		case TOU_CC_CA:
			tc->snd_cwnd += (TOU_SMSS*TOU_SMSS/tc->snd_cwnd);
			tc->snd_cwnd = ckSize(tc->snd_cwnd);
			tc->dupackcount = 0;
			break;

		case TOU_CC_FR:
			//go to CA
			tc->cc_state = TOU_CC_CA;
			tc->snd_cwnd = tc->snd_ssthresh;
			tc->snd_cwnd = ckSize(tc->snd_cwnd);
			tc->dupackcount = 0;
			break;

		default:
			/* some err msg here */
			err = 1;
			break;
	}
	return err;
}/* end of ssca::addwnd() */

/**
 * get current window
 */
u_long ssca::getwnd() {

	boost::mutex::scoped_lock lock(sscamutex);

	tc->snd_wnd = std::min(tc->snd_cwnd, tc->snd_awnd);
	tc->snd_wnd = ckSize(tc->snd_wnd);
	return tc->snd_wnd;
}

/**
 * settwnd():
 * set wnd while time out occurs.
 * return 1 if failure.
 * return 0 if success
 */
int ssca::settwnd() {

	boost::mutex::scoped_lock lock(sscamutex);

	int err = 0;
	switch(tc->cc_state) {
		case TOU_CC_ERR:
			/* some err msg here */
			err = 1;
			break;

		case TOU_CC_SS:
			tc->snd_ssthresh = std::max(tc->snd_cwnd/2, (unsigned long)2*TOU_SMSS);
			tc->snd_cwnd = TOU_SMSS;
			tc->dupackcount = 0;
			break;

		case TOU_CC_CA:
		case TOU_CC_FR:
			/* go to SS */
			tc->cc_state = TOU_CC_SS;
			tc->snd_ssthresh = std::max(tc->snd_cwnd/2, (unsigned long)2*TOU_SMSS);
			tc->snd_cwnd = TOU_SMSS;
			tc->dupackcount = 0;
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
int ssca::setdwnd(u_long ack_seq) {

	boost::mutex::scoped_lock lock(sscamutex);

	int err = 0;
	switch(tc->cc_state) {
		case TOU_CC_ERR:
			/* some err msg here */
			err = 1;
			break;

		case TOU_CC_SS:
		case TOU_CC_CA:
			/*
			 * dup on != snd_una can only happend when re-ordering
			 * or reproduction of network. Either case can we omit
			 * the effect of dup ACKs.
			 */

			if (ack_seq == tc->snd_una) {
				tc->dupackcount++;
			}else {
				//not continuous ACK seq
				tc->dupackcount = 1;
			}

			if(tc->dupackcount >= TCPREXMTTHRESH) {
				/* 3dup ack should go to FR */
				tc->cc_state = TOU_CC_FR;
				tc->snd_ssthresh = std::max(tc->snd_cwnd/2, (unsigned long)2*TOU_SMSS);
				tc->snd_cwnd = tc->snd_ssthresh + (3 * TOU_SMSS);
			}
			break;

		case TOU_CC_FR:
			if (ack_seq == tc->snd_una) {
				tc->dupackcount++;
			} else {
				//not continuous ACK seq
				tc->dupackcount = 1;
			}

			tc->snd_cwnd += TOU_SMSS;
			tc->snd_cwnd = ckSize(tc->snd_cwnd);
			break;

		default:
			/* some err msg here */
			err = 1;
			break;
	}
	return err;
}/* end of int ssca::setdwnd() */
