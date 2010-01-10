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
		boost::mutex soctabmutex;
		touPkg	duppkt;

	public:
		touCb		tc;					//tcp control block
		int			sockd;			//socket file descriptor
		int			sockstate;  //SOCK_CREATED, BIND, LISTEN, CONNECT, ESTABLISHED, TERMINATING
		u_short	sport;
		u_short dport;      //destination port
		std::string  sip;
		std::string  dip;		//destination ip 
		int			cid;				//connection id. probably dont need it
		int     tcpstate;   //state in which the connection is   
		CircularBuffer 	CbSendBuf;
		CircularBuffer 	CbRecvBuf;
		minPkgHeapType	HpRecvBuf;
		ssca	*sc;					//congestion control class
		int pfd[2];
		sockTb();
		~sockTb() {delete sc;}
		bool ckHpRecvBuf(const touPkg &pkt);//ch if there's dup pkt in HpRecvBuf
		void printall();
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
		struct sockTb* getSocketTable(int);
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
		int setCbData(char *, int, int);
	private:
		sockTb *s;
		std::vector<sockTb*>::iterator stbiter;
		boost::mutex soctabmutex;

		/* for test */
		void setSocketTable(struct sockaddr_in *, int sockfd, char * ip, unsigned short port);
};
