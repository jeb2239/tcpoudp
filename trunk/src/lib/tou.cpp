#include "tou.h"
std::vector<sockTb*> SS;
timerMng tm1;
boost::mutex socktabmutex1;

FILE *_fptrace = fopen("../debug.txt", "w");
using namespace std;

/*
 *Byte stream functions
 *@result //converts to byteorder	
 */
void touMain::convertFromByteOrder(touPkg &tp) {

tp.t.seq = ntohl(tp.t.seq);
tp.t.mag = ntohl(tp.t.seq);
tp.t.ack_seq = ntohl(tp.t.ack_seq);
tp.t.syn = ntohs(tp.t.syn);
tp.t.ack = ntohs(tp.t.ack);
}

/*
 *Byte stream functions
 *@result //converts to byteorder	
 */
void touMain::convertToByteOrder(touPkg &tp) {
	
tp.t.seq = htonl(tp.t.seq);
tp.t.mag = htonl(tp.t.mag);	
tp.t.ack_seq = htonl(tp.t.ack_seq);
tp.t.syn = htons(tp.t.syn);
tp.t.ack = htons(tp.t.ack);
}
	
/*
 *Create Socket
 *@result 1 if successful
 */
int touMain::touSocket(int domain, int type, int protocol) {
	
if ((domain != AF_INET) || (type != SOCK_DGRAM) || (protocol != 0 )) {
  cout << "ERROR CREATING SOCKET" ;
	return -1 ;
	}
sd = socket(domain,type,0);
sm.setSocketTable(sd);
setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); 
cout << " Socket returns : " << sd << endl;
return sd;
}

/*
 *Bind Socket
 *return 0 if succesful
 */

int touMain::touBind(int sockfd, struct sockaddr *my_addr, int addrlen) {

int rv;
rv = bind(sockfd,my_addr,addrlen);
sm.setSocketTable((sockaddr_in *)my_addr,sockfd);
cout << "Bind returns  : " << rv <<endl;
return rv;
}
	
/* 
 * Listen()
 * @result socket state listening  
 */
int touMain::touListen(int sd, int backlog) {

sm.setSocketState(TOUS_LISTEN,sd);
}
	
/* 
 * Connect()
 * @result return 1 on successful (lee rewrite&modify version@Nov 23) 
 */
int touMain::touConnect(int sd, struct sockaddr_in *socket1, int addrlen) {
  cout << " Inside Connect " << endl;
  int rv;
  sockaddr_in sockaddrs;
  size_t len = sizeof(sockaddr);
	size_t addrlens = sizeof(sockaddr);
  sockTb *s = sm.getSocketTable(sd);
  sm.setSocketTableD(socket1,sd);		
  touPkg ackpkt;
  
  ackpkt.putHeaderSeq(rand()%(u_long)65535, (u_long)0);  /* NOT correct */
  ackpkt.t.syn = FLAGON;
  /* Connection Initialization (IMPOERTANT)*/
  sm.setSocketState(TOUS_SYN_SENT,sd);
  sm.setTCBState(TOU_CC_SS, sd);
  sm.setTCB((ackpkt.getSeq()+1), ackpkt.getSeq(),sd);
  sm.setTCBRcv(0, sd);
  sm.setTCBCwnd(2024, sd);
  sm.setTCBAwnd(2024, sd);
  
  ackpkt.printall();
  s->printall();
  
  /* Send the SYN */
  cout<<"Sending to  :  " << inet_ntoa(socket1->sin_addr)<< "  " <<htons(socket1->sin_port) << endl;
  if( -1 <=  (rv = sendto(sd, &ackpkt, sizeof(ackpkt), 0,(struct sockaddr*)socket1,sizeof(struct sockaddr_in))))
		perror("send : ");
  
  /*Prepare for receiving */
  /*Receive SYN ACK */
  if (0 == (rv = recvfrom(sd, &tp, sizeof tp, 0, (struct sockaddr*)&sockaddrs, &len))){
		cout <<"Other end shutdown !! "<< endl;
	}else if( -1 == rv ){
		perror("talker: sendto");
	}
  
  /* How do you know this pkt is the one you want? u neither ck seq # nor ck flags...!? 
   * how about add some check mechanism
   * ?*/
  if(s->tc.snd_nxt == tp.getAckseq()) 
		cout << "SYN ACK Received properly SEQ; "<<tp.getSeq()<<" ACK_SEQ: "<<tp.getAckseq()<<  endl;
  tp.printall();
	
	/* update the pkt.seq to local tcp.rvc_nxt */
  sm.setTCBRcv(tp.getSeq()+1, sd);

	/* set up the fianl 3-way pkt */
  ackpkt.putHeaderSeq(s->tc.snd_nxt, s->tc.rcv_nxt);
  ackpkt.t.syn = FLAGOFF;
  ackpkt.t.ack = FLAGON;
  ackpkt.printall();
  s->printall();
  
  /*send ACK - final 3way handshake */
  if( -1 <= (rv = sendto(sd, &ackpkt, sizeof(ackpkt), 0, (struct sockaddr*)socket1, addrlens)))
		perror("send : ");

  if(rv>1) {
    sm.setSocketState(TOUS_ESTABLISHED,sd);
    return true;
    }

	return false;
}/* End of Connect */



/* 
 * Accept()
 * @result return 1 on successful (Lee rewrite & modify @Nov 23)
 */
int touMain::touAccept(int sd, struct sockaddr_in *socket2, socklen_t *addrlen) {
  cout << " Inside Accept " << endl;	
  u_long	randnum;
  touPkg	ackpkt;
  int rv, control=0, flagforsyn = 1;
  struct sockaddr_in sockaddrs;
  socklen_t len = sizeof(sockaddrs);
  sockTb *s = sm.getSocketTable(sd);
  
  /* Connection Initialization (IMPOERTANT)*/
  sm.setTCBState(TOU_CC_SS, sd);
  randnum = rand()%(u_long)65535; /* NOT Correct */
  sm.setTCB(randnum, randnum,sd);
  sm.setTCBRcv((u_long)0, sd);
  sm.setTCBCwnd(2024, sd);
  sm.setTCBAwnd(2024, sd);
  
  u_short port = (s->dport);
  cout <<"Destination :  " << s->dip << "  " << s->dport << endl;
  assignaddr(&sockaddrs,AF_INET,s->dip,port);

  if(s->sockstate == TOUS_SYN_RECEIVED) {  
  	/* Connection Initailization */
  	sm.setTCBRcv(tp.getSeq()+1, sd);
  	/* End of Connection Initialization */

    /* send SYN ACK */
  	/* why use recved pkt directly without clean the data first? what if there're uncleaned flags... 
  	 * ... I...... what to say...  be careful with coding... 
  	 * it may not cause any problem here, but it's important to take care of ur code well.
  	 *
  	 * BTW, random # is wrong... it should go with "seed" maybe u can ck out the usage menu.
  	 */

		cerr<< "Recv the first SYN \n\n";
		tp.printall();
		s->printall();
  	
		/* set up the ack pkt */
  	ackpkt.putHeaderSeq(s->tc.snd_nxt, s->tc.rcv_nxt);
  	ackpkt.t.syn = FLAGON;
  	ackpkt.t.ack = FLAGON;

		/* update the local TCB, and ready to send */
		sm.setTCB(s->tc.snd_nxt+1, s->tc.snd_una, sd);

  
    //Code for testing begin
  	if( -1 == (rv = sendto(sd, &ackpkt, sizeof(ackpkt), 0, (struct sockaddr *)&sockaddrs, len)))
			perror("send : ");

		cerr<< "Sending the first  SYN ACK \n\n";
		ackpkt.printall();
		s->printall();

  	/*recv final(ACK) handshake */
  	rv = recvfrom(sd, &tp, sizeof tp, 0, (struct sockaddr *)socket2, addrlen);
		cerr<< "Recving the final ACK \n\n";
		tp.printall();

  	if ((s->tc.snd_una+1) == tp.getAckseq()){
  		/* Connection ControlTabe Update */
			sm.setTCB(s->tc.snd_nxt,s->tc.snd_una+1, sd);
			cout<<"1. Accpet received final 3-way handshake ACK. Connection ESTABLISHED !!!" << endl;
	  }
		if ((s->tc.rcv_nxt) == tp.getSeq()){
			/* Connection ControlTabe Update */
  		sm.setTCBRcv(tp.getSeq(), sd);
      cout<<"2. Accpet received final 3-way handshake ACK. Connection ESTABLISHED !!!" << endl;
  	}


    sm.setSocketState(TOUS_ESTABLISHED,sd);
		tp.printall();
		s->printall();
    return true;
    }

  return false;
}

/* 
 * Send()
 * @result return no of bytes successfully sent  
 */
int touMain::touSend(int sd, char *sendBufer, int len1, int flags) { 

	std::cout << "INSIDE touSend function ..... " << endl;
	int len = len1; //length of sendBufer
	int lenr = pushsndq(sd , sendBufer,  len);
	std::cout << "LEAVE touSend function, data push into circular buff\n";\
  return lenr;
}

/* 
 * Receive()
 * @result return no of bytes successfully received  
 */
int touMain::touRecv(int sd, char *recvBuffer, int bufferLength, int flags) {
    
int rv, control=0, flagforsyn = 1;
size_t len = sizeof(sockaddr);
int end = 0;
struct sockaddr_in sockaddrs;
char buffer[bufferLength];

sockTb *s = sm.getSocketTable(sd);
s->CbRecvBuf.getAt(buffer,bufferLength,end);
s->CbRecvBuf.remove(bufferLength);
cout << "Received data " << buffer << endl;
//convertToByteOrder(tp);
tp.t.seq = s->tc.snd_nxt;
cout << "Seq no recv : " << s->tc.snd_nxt <<"   " <<  tp.t.seq << endl;
u_short port = (s->dport);
assignaddr(&sockaddrs,AF_INET,s->dip,port);
//send syn ack
tp.t.ack_seq = tp.t.seq + strlen(buffer);
	//  tp.t.seq = rand()%(u_long)65530;
tp.t.seq = tp.t.seq;  	
tp.t.syn = 0;
tp.t.ack = 1;
cout << "Sent ack = " << tp.t.ack_seq << endl;
	//Code for testing begin
rv = sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr *)&sockaddrs, sizeof(struct sockaddr_in));
perror("send : ");
if(s->sockstate == TOUS_CLOSE_WAIT)
{
  cout << " TOU Recv has received close from the remote side ..." << endl;
  return 0;
}
return bufferLength;
}

/* 
 * Close()
 */

int touMain::touClose(int sd) {

cout << " Inside close  " << endl;
sockTb *s;
sockaddr_in sockaddrs,socket1;
s = sm.getSocketTable(sd);
size_t len = sizeof(sockaddr);
tp.t.seq = s->tc.snd_nxt;
u_short port = (s->dport);
assignaddr(&sockaddrs,AF_INET,s->dip,port);
if(s->sockstate == TOUS_CLOSE_WAIT) {
  cout << "Sending FIN ACK " << endl;
  tp.t.fin = 1;
  tp.t.ack = 1; 
  int rv = sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)&sockaddrs , sizeof(struct sockaddr_in)); 

  fd_set socks;
 	struct timeval tim;
 	FD_ZERO(&socks);
 	FD_SET(sd, &socks);
 	tim.tv_sec = 30; 
  size_t  len = sizeof(sockaddrs);
  while(1){    
			if (select(sd+1, &socks, NULL, NULL, &tim)) {
        cout << " Waiting 2 MSL " << endl ;
        recvfrom(sd,&tp, sizeof(tp),0,(struct sockaddr*)&sockaddrs , &len);
        if(tp.t.ack == 1)
        {
          close(sd);
          return 1;
         }
    }
    else {
        cout << " 2 MSL Over "  << endl;
         close(sd);
          return 1;
    }
}
 /* sm.setSocketState(TOUS_LAST_ACK,sd); 
  
  sm.delSocketTable(sd);
  close(sd);
  return 1;*/
 }
/*Client Side */
tp.t.fin = 1;
tp.t.ack_seq = tp.t.seq + 1;
cout << "sending fin " << endl;
int rv = sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)&sockaddrs , sizeof(struct sockaddr_in));
sm.setSocketState(TOUS_FIN_WAIT_1,sd);
cout << " Receiving FIn ACK " << endl;
recvfrom(sd,&tp,sizeof(tp),0,(struct sockaddr *)&socket1,&len);
if(tp.t.fin ==1 && tp.t.ack ==1) {
  sm.setSocketState(TOUS_TIME_WAIT,sd);
  }
tp.t.ack == 1;
cout << " Sending ACK " << endl;
//sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)&sockaddrs , sizeof(struct sockaddr_in));
sm.delSocketTable(sd);
close(sd);  
}
  
/* >0 if fd is okay
 * ==0 if timeout
 * <0 if error occur 
 */
int touMain::timero(int fd, int usec) {
	
fd_set                timeo_rset;
struct timeval        tv;
FD_ZERO(&timeo_rset);
FD_SET(fd, &timeo_rset);
tv.tv_sec = 0;
tv.tv_usec = usec;
return (select(fd+1, &timeo_rset, NULL, NULL, &tv));
}


/* 
 *get the sockaddr_in infomation 
 */
int touMain::assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, string ip, u_short port) {
	
bzero(sockaddr, sizeof(*sockaddr));
sockaddr->sin_family = sa_family;
sockaddr->sin_port = htons((short)(port));
if( 1 != inet_pton(sa_family, ip.c_str(), &sockaddr->sin_addr) )
	return 0;
return 1;
}

/*
 *called during tou_bind()
 */
int touMain::pushsndq(int sockfd, char *sendbuf, int &len) {
int an = 0; //actual number of bytes been sent
sockTb *socktb;
socktb = sm.getSocketTable(sockfd);
  /* insert the data into circular buf */
 if( 0 < socktb->CbSendBuf.getAvSize()){
   //an = sm.setCbData(sendbuf,sockfd, len);
	an = socktb->CbSendBuf.insert(sendbuf, len);
 }
return an;
}
