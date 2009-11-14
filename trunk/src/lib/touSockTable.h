/******************************************************
 * This is ToU socket table & its talbe management
 *****************************************************/
 
 #include "touControlBlock.h"
 #include "touCongestion.h"
 #include "circularbuffer.h"     //circular buffer
 
 /* SS is Socket Table for storing socket informaiton */
 vector<sockTb*> SS;
 boost::mutex soctabmutex;
 
class sockTb {
  public:
    touCb	tc;			//tcp control block
    int			sockd;		//socket file descriptor
		int			sockstate;  //SOCK_CREATED, BIND, LISTEN, CONNECT, ESTABLISHED, TERMINATING
    u_short	sport;
    u_short dport;      //destination port
    string  sip;
    string  dip;        //destination ip 
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
	}

	~sockTb() {
	  /*
	  free(sip);
	  free(dip);
	  */
	  delete sc;
	}
	void printall() {
	cout<<"sockd  : "<<sockd<<" sport :"<<sport<<" dport :"<<dport <<endl;
	cout<<"sip    : "<<sip<<" dip : "<<dip <<endl;
	cout<<"cc_state: "<<tc.cc_state<<endl;
	cout<<"snd_una: "<<tc.snd_una<<endl;
	cout<<"snd_nxt: "<<tc.snd_nxt<<endl;
	cout<<"snd_cwnd:"<<tc.snd_cwnd<<endl;
	cout<<"snd_awnd:"<<tc.snd_awnd<<endl;
	cout<<"snd_ssthresh:"<<tc.snd_ssthresh<<endl<<endl;
	}

};

class sockMng {
  public:
    struct sockTb* getSocketTable(int);
    void setSocketTable(struct sockaddr_in *, int);
    void setSocketTableD(struct sockaddr_in *, int); 
    void setSocketTable(int );
    void delSocketTable(int );
	
	/* for test */
	void setSocketTable(struct sockaddr_in *, int sockfd, char * ip, unsigned short port);
  private:
    vector<sockTb*>::iterator stbiter;
    sockTb *s;
};
