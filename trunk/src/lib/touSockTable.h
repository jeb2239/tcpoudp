/**
 * touSockTable.h
 * Including ToU socket table, and its talbe management
 *
 * Copyright 2009 by Columbia University; all rights reserved
 * Jan 10, 2010
 */
#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <queue>
#include <deque>								
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/system/system_error.hpp>
#include <boost/thread.hpp>

#include "touCongestion.h"
#include "circularBuffer.h"
#include "touHeader.h"

//boost::mutex soctabmutex;
//boost::mutex socktabmutex1;

/**
 * heapPkgComp:
 * compare the pkt by its sequence number
 */
class heapPkgComp {
	public:
		bool operator() (const touPkg& lhs, const touPkg& rhs) const {
			return (lhs.t.seq >= rhs.t.seq);
		}
};

/* define the buffer(queue) for timer nodes */
typedef std::priority_queue<touPkg, std::vector<touPkg>, heapPkgComp> minPkgHeapType;

/**
 * sockTb: 
 * ToU socket table where stores the information that essential for management
 * of ToU connections.
 */
class sockTb {
	private:
		boost::mutex stmutex;
		touPkg	duppkt;

	public:
		int			sockd;			//socket file descriptor
		int			sockstate;  //SOCK_CREATED, BIND, LISTEN, CONNECT, ESTABLISHED, TERMINATING
		int     tcpstate;   //state in which the connection is   
		u_short	sport;			//souce port number
		u_short dport;      //destination port number
		std::string  sip;		//source ip address
		std::string  dip;		//destination ip address
		CircularBuffer 	CbSendBuf;	//circular buffer for sending
		CircularBuffer 	CbRecvBuf;	//circular buffer for recving
		minPkgHeapType	HpRecvBuf;
		ssca		*sc;				//congestion control management 
		touCb		tc;					//tcp control block management
		int pfd[2];

		sockTb();

		~sockTb() {delete sc;}

		bool ckHpRecvBuf(const touPkg &pkt);//ch if there's dup pkt in HpRecvBuf

		void pushHpRecvBuf(const touPkg &pkt);

		/* ck_dupack_3: checking the accumulation of duplicate acks */
		bool ck_dupack_3();

		/* logging mechanism. it prints all regarding the socket tables */
		void log();
		void log(unsigned short logflag);
};

/* SS is Socket Table for storing socket informaiton */
extern std::vector<sockTb*> SS;

/**
 * sockMng:
 * socket management which is responsible for the menagement of ToU socket
 * table.
 */
class sockMng {
	public :
		sockMng() {s = new sockTb();}
		sockTb* getSocketTable(int);
		void setSocketTable(struct sockaddr_in *, int);
		void setSocketTableD(struct sockaddr_in *, int); 
		void setSocketTable(int );
		void delSocketTable(int );
		void setSocketState(int , int);

		void setTCBState(int, int);												/* set tcb's congestion state */
		void setTCB(unsigned long ,unsigned long ,  int);	/* set snd_nxt, snd_una */
		void setTCBCwnd(unsigned long, int);							/* set snd_cwnd */
		void setTCBAwnd(unsigned long, int);							/* set snd_awnd */
		void setTCBRcv(unsigned long, int);								/* set rcv_nxt in socket table */

		/* insert data, data's len */ 
		int setCbData(const char *, int, int);
	//private:
		sockTb *s;
		
	private: // should change back
		std::vector<sockTb*>::iterator stbiter;
		boost::mutex soctabmutex;

		/* for test */
		void setSocketTable(struct sockaddr_in *, int sockfd, char * ip, unsigned short port);
};
