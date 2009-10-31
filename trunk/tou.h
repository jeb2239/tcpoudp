/***************************************************
 * This is ToU library header file. Programs which 
 * want to use ToU library functions should include
 * this header file.
 **************************************************/
 
/***************************************************
 * Include from STD library
 **************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <sstream>
/***************************************************
 * Include from BOOST library
 **************************************************/


/***************************************************
 * Include from self-define header
 **************************************************/
#include "timer.h"						//timer library
#include "touheader.h"					//touheader
#include "toucong.h"					//congestion


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

//<<<<<<< .chinmay
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


//TRACE(level, params, etc);

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
	
	//circular buf send 
	//circular buf recv
	//mutex
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

