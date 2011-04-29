/**
 * tou_sock_table.h
 * ToU socket table, and its table management class
 *
 * Copyright 2009 by Columbia University; all rights reserved
 * Jan 10, 2010
 */

#include "tou_congestion.h"
#include "circ_buf.h"
#include "tou_header.h"

//socket table queue (stbq) mutex, a mutex for stbq
extern boost::mutex stbq_mutex;

#define SOCKTB_SET_SRC_TUPLE	0	//A flag for setting src ip, port
#define SOCKTB_SET_DST_TUPLE	1	//A flag for setting dst ip, port
#define SOCKTB_SET_SRCDST_TUPLE	2	//A flag for setting src/dst ip, port

/**
 * heap_pkt_comp:
 * compare packet by there sequence numbers
 */
class heap_pkt_comp {

  public:
	  bool operator() (const touPkg& lhs, const touPkg& rhs) const {
		  return (lhs.t.seq >= rhs.t.seq);
	  }
};

/* define the buffer(a priority queue) for timer nodes */
typedef std::priority_queue<touPkg, std::vector<touPkg>, heap_pkt_comp> MinPktHeapType;

/**
 * sockTb: 
 * ToU socket table where stores the information that is essential for management
 * of ToU connections.
 */
class sockTb {

	private:
		//touPkg	duppkt;				//temporary pkt used with minhp_recv

	public:
		boost::mutex stb_mtx;		//socket table mutex
		boost::mutex stb_tc_ack_mtx;//tc's DELACK state mutex
		boost::mutex stb_cb_send_mtx;//circular buffer sned mutex
		boost::mutex stb_cb_recv_mtx;//circular buffer recv mutex
		boost::mutex stb_minhp_recv_mtx;//for minhp_recv

		/*
		 	protected by stb_mtx. *sc has its own sscamutex
		 */
		int		sockd;				//socket file descriptor
		int		sockstate;  		//SOCK_CREATED, BIND, LISTEN,
	   								//CONNECT, ESTABLISHED, TERMINATING
		int		runstate;			//processtou's run func.	
		u_short	sport;				//souce port number
		u_short dport;      		//destination port number
		std::string  sip;			//source ip address
		std::string  dip;			//destination ip address
		ssca *sc;					//congestion control management
		tou_cb tc;					//ToU control block management	

		/*
		 	protected by stb_cb_send_mtx, stb_cb_recv_mtx, stb_minhp_recv_mtx
		 */
		CircularBuffer 	cb_send;	//circular buffer for sending
		CircularBuffer 	cb_recv;	//circular buffer for recving
		MinPktHeapType	minhp_recv;	//buffer for out-of-order packets


		sockTb();
		~sockTb() {delete sc;}

		void operator=(sockTb &src);

		/* 
			get the state of the current delack state

		*/

		/*
			check whether there's any duplicate pkt in minhp_recv
			return true on yes, false on no
		*/
		/*
		bool ckHpRecvBuf(const touPkg &pkt);
		*/

		/*
			push a duplicate data pkt of peer host into minhp_recv 
		*/
		void pushHpRecvBuf(const touPkg &pkt);

		/* 
			check the number of the accumulation of duplicate akcs is exceed
			certain number. in Reno: the number is three
		*/
		bool ck_num_dupack();

		/* 
			logging mechanism. it prints all regarding the socket tables 
		*/
		void log();
		void log(unsigned short logflag);
};

/**
 * sockMng:
 * socket management which is responsible for the menagement of ToU socket
 * table.
 */
class sockMng {

	public :
		/*
			initiate a new socket table
			note: need to push this new socket table pointer into stbq once
			get the socket id
		*/
		sockMng() {}
		~sockMng() {}

		/*
			return a pointer of socket table with specified socket file descriptor
		*/
		sockTb* getSocketTable(int sockfd);

		/*
			add a new socket table into stbq (socket table queue)
		*/
		void addSocketTable(int sockfd);

		/*
			delete a socket table from stbq
		*/
		void delSocketTable(int sockfd);

		/*
			setup parameters of a socket table
			for flag options:
			SOCKTB_SET_SRC_TUPLE: for source ip and port
			SOCKTB_SET_DST_TUPLE: for destination ip and port
			SOCKTB_SET_SRCDST_TUPLE: for both src/dst ip and port
		*/
		void setSocketTable(int sockfd, struct sockaddr_in *srcsock,
							struct sockaddr_in *dstsock, int flag);

		/*
			setup the socket state
		*/
		void setSocketState(int state , int sockfd);

		void setTCBState(int, int);										/* set tcb's congestion state */
		void setTCB(unsigned long ,unsigned long ,  int);				/* set snd_nxt, snd_una */
		void setTCBCwnd(unsigned long, int);							/* set snd_cwnd */
		void setTCBAwnd(unsigned long, int);							/* set snd_awnd */
		void setTCBRcv(unsigned long, int);								/* set rcv_nxt in socket table */

		/* insert data, data's len */ 
		int setCbData(const char *, int, int);

		/* get circular buffer's send available size. */
		unsigned long getCirSndBuf();
	private:
		sockTb	*stbp;								//temporary socket table pointer
		std::vector<sockTb*>::iterator stbiter;		//iterator for socket table
													//vector	
};

extern std::vector<sockTb*> stbq;					//for storing socket tb info

