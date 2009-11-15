/******************************************************
 * touSockTable.h
 * This is ToU socket table & its talbe management
 *****************************************************/
 
#include <iostream>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/system/system_error.hpp>
#include <boost/thread.hpp>


#include "touCongestion.h"
#include "circularBuffer.h"     //circular buffer
 
//boost::mutex soctabmutex;
//boost::mutex socktabmutex1;
 
class sockTb {
	private:
		boost::mutex soctabmutex;
  public:
    touCb	tc;			//tcp control block
    int			sockd;		//socket file descriptor
		int			sockstate;  //SOCK_CREATED, BIND, LISTEN, CONNECT, ESTABLISHED, TERMINATING
    u_short	sport;
    u_short dport;      //destination port
		std::string  sip;
		std::string  dip;        //destination ip 
    int 	cid;        //connection id. probably dont need it
    int     tcpstate;   //state in which the connection is   
    CircularBuffer 	CbSendBuf;
    CircularBuffer 	CbRecvBuf;
		ssca	*sc;		//congestion control class

	/* TEST */
	int		ackcount;/* FOR TEST ONLY */

    sockTb() {
      /*
	  sip = (char*)malloc(sizeof(char)*30);
	  dip = (char*)malloc(sizeof(char)*30);
	  */
      CbSendBuf.setSize(TOU_MAX_CIRCULAR_BUF);
			CbRecvBuf.setSize(TOU_MAX_CIRCULAR_BUF);
			sc = new ssca(&tc); 
			tc.t_timeout = 3000;//3000ms
	}

	~sockTb() {
	  /*
	  free(sip);
	  free(dip);
	  */
	  delete sc;
	}
	void printall() {
		std::cout<<"sockd  : "<<sockd<<" sport :"<<sport<<" dport :"<<dport <<std::endl;
		//cout<<"sip    : "<<sip<<" dip : "<<dip <<endl;
		std::cout<<"cc_state: "<<tc.cc_state<<std::endl;
		std::cout<<"snd_una: "<<tc.snd_una<<std::endl;
		std::cout<<"snd_nxt: "<<tc.snd_nxt<<std::endl;
		std::cout<<"snd_cwnd:"<<tc.snd_cwnd<<std::endl;
		std::cout<<"snd_awnd:"<<tc.snd_awnd<<std::endl;
		std::cout<<"snd_ssthresh:"<<tc.snd_ssthresh<<std::endl<<std::endl;
	}

};

/* SS is Socket Table for storing socket informaiton */
extern std::vector<sockTb*> SS;

class sockMng {
		public :
			sockMng() {
				s = new sockTb();
			}
			struct sockTb* getSocketTable(int);
			void setSocketTable(struct sockaddr_in *, int);
			void setSocketTableD(struct sockaddr_in *, int); 
			void setSocketTable(int );
			void delSocketTable(int );
			void setSocketState(int , int);
			void setTCB(long unsigned int, int);
			int setCbData(char *, int, int);
			sockTb *s;
		private:
			std::vector<sockTb*>::iterator stbiter;
			boost::mutex soctabmutex;

			/* for test */
			void setSocketTable(struct sockaddr_in *, int sockfd, char * ip, unsigned short port);
};
