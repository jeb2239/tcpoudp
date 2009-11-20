/******************************************************
 *  * ToU control block
 *   * ***************************************************/
#define TOUT_TIMERS	4
class touCb {
    public:
      short             t_state;                //11 connection states
      short							cc_state;								//3 congestion states
      short             t_timer[TOUT_TIMERS];   //4 timers
			unsigned long			t_timeout;							//current timeout(ms)
      u_short           t_flags;                //pending
  /*
   *  * WND & SEQ control. See RFC 783
   *   */
      u_long            iss;                    //initial send seq #
      u_long            irs;                    //initial rcv seq #
      u_long            snd_una;                //send # unacked
      u_long            snd_nxt;                //send # next

      u_long						snd_ack;								//send's ack #

      u_long            snd_w11;                //send wnd update seg seq #
      u_long            snd_w12;                //send wnd update seg ack #
      u_long            snd_wnd;                //sender's window

      u_long            rcv_wnd;                //rcv window
      u_long            rcv_nxt;                //rcv next

      /*
       *  * Additional var for this implementation
       *   */
      short				dupackcount;			//duplicate ack(should count to three)

      u_long            snd_cwnd;               //congestion-controlled wnd
      u_long            snd_awnd;               //sender's advertised window from recver    
      u_long            snd_ssthresh;   		//snd_cwnd size threshold for slow start
     //exponential to linear switch


};
