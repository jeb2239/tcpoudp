/******************************************************
 * touSockTable.h
 * This is ToU socket table & its talbe management
 *****************************************************/
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
#include "circularBuffer.h"     //circular buffer
#include "touHeader.h"

//boost::mutex soctabmutex;
//boost::mutex socktabmutex1;

/* compare the pkt by it's sequence number */
class heapPkgComp {
	public:
		bool operator() (const touPkg& lhs, const touPkg& rhs) const {
			return (lhs.t.seq >= rhs.t.seq);
		}
};

typedef std::priority_queue<touPkg, std::vector<touPkg>, heapPkgComp> minPkgHeapType;

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
		bool ckHpRecvBuf(const touPkg &pkt);//ch if there's duplicate pkt in HpRecvBuf

		void printall() { /* TEST */
			std::cout<<"*** SOCKET TABLE RESULT ***\n";
			std::cout<<"sockd: "<<sockd<<" sport: "<<sport<<" dport: "<<dport <<std::endl;
			//cout<<"sip    : "<<sip<<" dip : "<<dip <<endl;
			std::cout<<"cc_state: "<<tc.cc_state<<std::endl;
			std::cout<<"snd_una : "<<tc.snd_una<<std::endl;
			std::cout<<"snd_nxt : "<<tc.snd_nxt<<std::endl;
			std::cout<<"rcv_nxt : "<<tc.rcv_nxt<<std::endl;
			std::cout<<"snd_cwnd:"<<tc.snd_cwnd<<std::endl;
			std::cout<<"snd_awnd:"<<tc.snd_awnd<<std::endl;
			std::cout<<"snd_ssthresh:"<<tc.snd_ssthresh<<std::endl<<std::endl;
		}
};

/* SS is Socket Table for storing socket informaiton */
extern std::vector<sockTb*> SS;

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
