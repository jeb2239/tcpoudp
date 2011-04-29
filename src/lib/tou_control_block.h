/******************************************************
 * ToU control block
 *****************************************************/
#define TOUT_TIMERS	4

class tou_cb {

    public:
      short		t_state;                //11 connection states
      short		cc_state;				//3 congestion states
	  short		t_delack_state;			//2 delayed ack states

      u_long	t_rto;					//current timeout(ms): RTO(i)
      u_short	t_rto_beta;				//RTO(i) beta (default std: 2)
      u_long	t_rtt;					//round time trip
      float		t_rtt_alpha;			//RTT alpha (default std: 0.125)


      //u_long	t_timeout_seq;			//which current seq. causing timeout
      //u_short	t_flags;				//pending
	  //short	t_timer[TOUT_TIMERS];	//4 timers

      /*
       * WND & SEQ control. See RFC 783
       */
      u_long	iss;					//initial send seq #
	  u_long	irs;					//initial rcv seq #
      u_long	snd_una;				//send # unacked
      u_long	snd_nxt;				//send # next
      u_long	rcv_nxt;				//rcv  # next

      /*
       * Additional var for this implementation
       */
      short		dupackcount;			//duplicate ack(should count to three)
      short 	TcpMaxConnectRetransmissions;
      short		TcpMaxDataRetransmissions;

      u_long	snd_wnd;				//sender's window
      u_long	rcv_wnd;				//rcv window
      u_long	snd_cwnd;				//congestion-controlled wnd
      u_long	snd_awnd;				//sender's advertised window from recver    
      u_long	snd_ssthresh;			//snd_cwnd size threshold for slow start
										//exponential to linear switch
};
