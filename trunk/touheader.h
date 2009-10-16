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
#define TOUT_PERSIST		1
#define TOUT_KEEP		2
#define TOUT_2MSL		3

//States
#define TOUS_CLOSED		0
#define TOUS_LISTEN		1
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
class touheader {
  public:
    u_long		seq;
    u_long		mag;
    u_long		ack_seq;
    u_short		doff:4,
			rsv:4;
    u_char		flags;
#define	TOU_FIN		0x01
#define	TOU_SYN		0x02
#define	TOU_RST		0x04
#define	TOU_PSH		0x08
#define	TOU_ACK		0x10
#define	TOU_RSV		0x20
#define	TOU_ECE		0x40
#define	TOU_CWR		0x80
#define	TOU_FLAGS (TOU_FIN|TOU_SYN|TOU_RST|TOU_PSH|TOU_ACK|TOU_RSV|TOU_ECE|TOU_CWR)
    u_short		wnd:16;
};

/******************************************************
 * ToU application layer: ToU hdr + Data
 * ***************************************************/
class toupkg {
  public:
    touheader		*touhdr;
    char		*payload;
};

/******************************************************
 * ToU control block
 * ***************************************************/
class toucb {
  public:
    short		t_state;		//11 connection states
    short		t_timer[TOU_TIMERS];	//4 timers
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
    u_long  		sip;
    u_long  		dip;

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

