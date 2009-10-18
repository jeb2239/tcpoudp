#include <stdint.h>
#include <iostream>

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

#define TOU_MSS			1400
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
/*
#define	TOU_FIN		1
#define	TOU_SYN		2
#define	TOU_RST		4
#define	TOU_PSH		8
#define	TOU_ACK		10
#define	TOU_RSV		20
#define	TOU_ECE		40
#define	TOU_CWR		80
#define	TOU_FLAGS (TOU_FIN|TOU_SYN|TOU_RST|TOU_PSH|TOU_ACK|TOU_RSV|TOU_ECE|TOU_CWR)
/******************************************************
 * ToU header
 * ***************************************************/
class touheader {
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
	u_short urg_ptr;
    u_short	wnd:16;
};

/******************************************************
 * ToU application layer: ToU hdr + Data
 * ***************************************************/
class toupkg {
  public:
    touheader		*touhdr;
    char			*payload;
};

/******************************************************
 * ToU control block
 * ***************************************************/
class toucb {
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
    u_long		snd_wnd;		//send window

    u_long		rcv_wnd;		//rcv window
    u_long		rcv_nxt;		//rcv next
    u_long		irs;			//initial rcv seq #

/*
 * Additional var for this implementation
 */
    u_long		snd_cwnd;		//congestion-controlled wnd
    u_long		snd_ssthresh;		//snd_cwnd size threshold for slow start
    						//exponential to linear switch
};

/******************************************************
 * socket table
 * ***************************************************/
class socktb {
  public:
    toucb		*tc;			//tcp control block

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
    touheader 		t;
    int tou_socket(void);
    int tou_accept();
    int tou_bind();
    int tou_listen();
    int tou_send();
    int tou_recv();
    int tou_close();
};	

socktb socktable;
