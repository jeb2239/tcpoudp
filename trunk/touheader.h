/***************************************************


				TOU HEADER
				

***************************************************/



#include <stdint.h>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bimap.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/timer.hpp>
#include <boost/system/system_error.hpp>

/******************************************************
 * Type define here
 * ***************************************************/
//Types
typedef	unsigned long 	u_long;
typedef	unsigned short	u_short;
typedef unsigned char	u_char;

//Timers
#define TOUT_TIMERS		4
#define TOUT_REXMIT		0
#define TOUT_PERSIST	1
#define TOUT_KEEP		2
#define TOUT_2MSL		3

//<<<<<<< .mine
#define TOU_MSS			1400 
//=======
//>>>>>>> .r12
//States
#define TOUS_CLOSED			0
#define TOUS_LISTEN			1
#define TOUS_SYN_SENT		2
#define TOUS_SYN_RECEIVED	3
#define TOUS_ESTABLISHED	4
#define TOUS_CLOSE_WAIT		5
#define TOUS_FIN_WAIT_1		6
#define TOUS_CLOSING		7
#define TOUS_LAST_ACK		8
#define TOUS_FIN_WAIT_2		9
#define TOUS_TIME_WAIT		10

/******************************************************
 * ToU header
 * ***************************************************/
class touHeader {
  public:
    u_long		seq;
    u_long		mag;
    u_long		ack_seq;
    
    u_short doff:4;
	u_short res1:4;
	u_short res2:2;
	u_short urg:1;
	u_short ack:1;
	u_short psh:1;
	u_short rst:1;
	u_short syn:1;
	u_short fin:1;

	u_short window;
	u_short check;
	
	
};

/******************************************************
 * ToU application layer: ToU hdr + Data
 * ***************************************************/
class touPkg {
  public:
    touHeader		t;
    //touHeader		t2;
    char 			buf[TOU_MSS];
};

/******************************************************
 * ToU control block
 * ***************************************************/
class touCb {
  public:
    short		t_state;		//11 connection states
    short		t_timer[TOUT_TIMERS];	//4 timers
    u_short		t_flags;		//pending
/*
 * WND & SEQ control. See RFC 783
 */
    u_long		snd_una;		//send unacked
    u_long		snd_nxt;		//send next
    u_long		snd_w11;		//send wnd update seg seq #
    u_long		snd_w12;		//send wnd update seg ack #
    u_long		iss;			//initial send seq #
    u_long		snd_wnd;		//sender's windoe

    u_long		rcv_wnd;		//rcv window
    u_long		rcv_nxt;		//rcv next
    u_long		irs;			//initial rcv seq #

/*
 * Additional var for this implementation
 */
    u_long		snd_cwnd;		//congestion-controlled wnd
    u_long		snd_awnd;		//sender's advertisec window from recver    u_long		snd_ssthresh;	//snd_cwnd size threshold for slow start
    						//exponential to linear switch
};

/******************************************************
 * socket table
 * ***************************************************/
class sockTb {
  public:
    touCb		*tc;			//tcp control block

    int			sockd;			//socket file descriptor
    u_short 		sport;
    u_short 		dport;
    string  		sip;
    string  		dip;
    

};

/******************************************************
 * tou main class
 * ***************************************************/
class tou {
  public :
    unsigned long 	test;
    touHeader 		t;
    int tou_socket(void);
    int tou_accept();
    int tou_bind();
    int tou_listen();
    int tou_send();
    int tou_recv();
    int tou_close();
};	

