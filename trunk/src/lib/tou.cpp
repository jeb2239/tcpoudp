#include "tou.h"
#define RECVBUFSIZE 2000
std::vector<sockTb*> SS;
timerMng tm1;
boost::mutex socktabmutex1;

FILE *_fptrace = fopen("../debug.txt", "a");
using namespace std;

void closer::dothis(int sd) {
	std::cout << "Inside do this .... " << std::endl;
	sockTb *s = sm.getSocketTable(sd);
	{
		sockaddr_in sockaddrs,socket1;
		size_t len = sizeof(sockaddr);
		//tp.t.seq = s->tc.snd_nxt;
		u_short port = (s->dport);
		assignaddr(&sockaddrs,AF_INET,s->dip,port);
		if(s->sockstate == TOUS_CLOSE_WAIT) 
		{
			cout << " Inside server close " << endl;
			boost::mutex::scoped_lock lock(socktabmutex1);
			cout << "Sending FIN ACK " << endl;
			//tp.clean();
			tp.t.fin = 1;
			tp.t.ack = 1; 
			tp.t.seq = 2;
			s->printall();
			int rv = sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)&sockaddrs , sizeof(struct sockaddr_in)); 
			tp.printall();
			//tp.clean();
			fd_set socks;
			struct timeval tim;
			FD_ZERO(&socks);
			FD_SET(sd, &socks);
			tim.tv_sec = 5; 
			cout << "waiting for ACK" << endl;
			size_t  len = sizeof(sockaddrs);
			//while(1)
			{    
				//if (select(sd+1, &socks, NULL, NULL, &tim)) 
				{
					cout << " Waiting 2 MSL and ack" << endl ;
					recvfrom(sd,&tp, sizeof(tp),0,(struct sockaddr*)&sockaddrs , &len);
					tp.printall();
					cout << "after recv from" << endl;
					if(tp.t.ack == 1)
					{
						close(sd);
						//return 1;
					}
				}
				//else 
				{
					cout << " 2 MSL Over "  << endl;
					close(sd);
					//return 1;
				}
			}
			
	
		}
		else
		{
			boost::mutex::scoped_lock lock(socktabmutex1);
			cout << " Inside client close " << endl;
			sockaddr_in sockaddrs,socket1;
			size_t len = sizeof(sockaddr);
			//tp.t.seq = s->tc.snd_nxt;
			sm.setSocketState(TOUS_FIN_WAIT_1,sd);
			cout << " Receiving Fin ACK " << endl;
			recvfrom(sd,&tp,sizeof(tp),0,(struct sockaddr *)&socket1,&len);
			cout <<" after recv from" << endl;
			tp.printall();
			if(tp.t.fin ==1 && tp.t.ack ==1) {
				sm.setSocketState(TOUS_TIME_WAIT,sd);
				cout << " Received FIN ACK" << endl;
			}
			tp.clean();
			tp.t.ack = 1;
			tp.t.seq = 3;
			cout << " Sending ACK " << endl;
			s->printall();
			sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)&sockaddrs , sizeof(struct sockaddr_in));
			tp.printall();
			sm.delSocketTable(sd);
			close(sd); 
			
		}
	}
	cout << "Work done by the close thread" << endl;
	cout << "Exiting close thread" << endl;
}

/* 
 * Close()
 * @param socket descriptor
 * @result calls the close thread to handle tear down 
 */

int touMain::touClose(int sd) {
	sockaddr_in sockaddrs;
	sockTb *s;
	//sm.setTCB((tp.getSeq()+1), tp.getSeq(),sd);
	//sm.setTCBRcv(0, sd);
	s = sm.getSocketTable(sd);
	cout << " Inside close  " << endl;
	if(s->sockstate == TOUS_CLOSE_WAIT) {
		cout << "Server side baby " << endl;
		closer *cl = new closer(sd);
		cl->m_thread1.join();
	}
	else
	{
		tp.clean();
		//tp.t.seq = s->tc.snd_nxt;
		u_short port = (s->dport);
		assignaddr(&sockaddrs,AF_INET,s->dip,port);
		tp.clean();
		tp.t.seq = 0;
		tp.t.fin = FLAGON;
		tp.t.ack_seq = tp.t.seq + 1;
		cout << "Sending Fin " << endl;
		s->printall();
		int rv = sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)&sockaddrs , sizeof(struct sockaddr_in));
		tp.printall();
		/*Client Side */
		cout << "Client side baby " << endl;
		closer *cl = new closer(sd);
		cl->m_thread1.join();	
	}
	
	return 1;


}

/*
 *Byte stream functions
 *@result converts from byteorder	
 */
/*
void touMain::convertFromByteOrder(touPkg &tp) {

	tp.t.seq = ntohl(tp.t.seq);
	tp.t.mag = ntohl(tp.t.seq);
	tp.t.ack_seq = ntohl(tp.t.ack_seq);
	tp.t.syn = ntohs(tp.t.syn);
	tp.t.ack = ntohs(tp.t.ack);
}
*/
/*
 *Byte stream functions
 *@result //converts to byteorder	
 */
/*
void touMain::convertToByteOrder(touPkg &tp) {

	tp.t.seq = htonl(tp.t.seq);
	tp.t.mag = htonl(tp.t.mag);	
	tp.t.ack_seq = htonl(tp.t.ack_seq);
	tp.t.syn = htons(tp.t.syn);
	tp.t.ack = htons(tp.t.ack);
}
*/
/*
 *Create Socket
 *@result 1 if successful
 */
int touMain::touSocket(int domain, int type, int protocol) {

	int yes;
	//int g = 9;
	//TRACE(5,"KadPeer::OnTimer...transaction map:  LObject map: %d\n", g);

	if ((domain != AF_INET) || (type != SOCK_DGRAM) || (protocol != 0 )) {
		cout << "ERROR CREATING SOCKET" ;
		return -1 ;
	}
	sd = socket(domain,type,0);
	sm.setSocketTable(sd);
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); 
	/* init processTou */
	proTou(sd);
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
	int					rv;
	sockaddr_in	sockaddrs;
	size_t			len = sizeof(sockaddr);
	size_t			addrlens = sizeof(sockaddr);
	sockTb			*s = sm.getSocketTable(sd);
	touPkg			*tp_sa;
	char				recvcont[RECVBUFSIZE] = "";

	/* Init the socktb */
	sm.setSocketTableD(socket1,sd);		
	touPkg ackpkt(0);
	ackpkt.clean();
	srand((unsigned)time(NULL)); 
	ackpkt.putHeaderSeq(rand()%(u_long)65535, (u_long)0);  
	ackpkt.t.syn = FLAGON;
	sm.setSocketState(TOUS_SYN_SENT,sd);
	sm.setTCBState(TOU_CC_SS, sd);
	sm.setTCB((ackpkt.getSeq()+1), ackpkt.getSeq(),sd);
	sm.setTCBRcv(0, sd);
	sm.setTCBCwnd(2024, sd);
	sm.setTCBAwnd(65535, sd);
	s->printall();

	/* Send the SYN */
	cout<<"Sending SYN to  :  " << inet_ntoa(socket1->sin_addr)<< "  " <<htons(socket1->sin_port) << endl;
	string tempdata = ackpkt.toString();
	ackpkt.printall();
	if( -1 <=  (rv = sendto(sd, tempdata.data(), tempdata.size(), 0,(struct sockaddr*)socket1,sizeof(struct sockaddr_in))))
		perror("send : ");
	cout << tempdata;
	cout << "[SYN] size of content: "<< tempdata.size() << " rv: "<< rv << "sizeof: "<< sizeof(tempdata) << endl;


	/*Prepare for receiving */
	/*Receive SYN ACK */
	if (rv = recvfrom(sd, recvcont, RECVBUFSIZE, 0, (struct sockaddr*)&sockaddrs, &len)){
		string tempdata(recvcont, rv);
		tp_sa = new touPkg(tempdata);
		sockTb *s = sm.getSocketTable(sd);
		sm.setTCBRcv(tp_sa->t.seq,sd);
		cout << "[SYN ACK] size of content: "<< tempdata.size() << " rv: "<< rv << "sizeof: "<< sizeof(tempdata) << endl;

		if((s->sockstate) == TOUS_CLOSE_WAIT)
			cout <<"Other end shutdown !! "<< endl;
		
	}else if( -1 == rv ){
		perror("Recv SYN ACK: ");
	}

	/*check for Recovery from Old Duplicate SYN */
	// /////////////BUG ???//////////////////////// How to make sure it's final 3 way hsk pkt?
	// how about dup the second time!? third time!?
	/*
	if(s->tc.snd_nxt == tp.getAckseq()) 
		cout << "SYN ACK Received properly SEQ; "<<tp.getSeq()<<" ACK_SEQ: "<<tp.getAckseq()<<  endl;
	else {
		// Send reset
		ackpkt.t.rst = FLAGON;
		ackpkt.t.ack_seq = tp.t.seq;
		ackpkt.t.seq = s->tc.snd_nxt;
		tempdata = ackpkt.toString();
		rv = sendto(sd, tempdata.data() , tempdata.size(), 0, (struct sockaddr*)socket1, addrlens);
	}
	*/

	tp_sa->printall();
	/* update the pkt.seq to local tcp.rvc_nxt */
	sm.setTCBRcv(tp_sa->getSeq()+1, sd); // cli's recv seq
	sm.setTCB(s->tc.snd_nxt, tp_sa->getAckseq(), sd); //cli's send seq(here just update ackseq)

	/* set up the fianl 3-way pkt */
	ackpkt.clean();
	ackpkt.putHeaderSeq(s->tc.snd_nxt, s->tc.rcv_nxt);
	ackpkt.t.syn = FLAGOFF;
	ackpkt.t.ack = FLAGON;
	
	s->printall();
	string tempdata_ack = ackpkt.toString();
	/*send ACK - final 4way handshake */
	if( -1 >= (rv = sendto(sd, tempdata_ack.data(), tempdata_ack.size() , 0, (struct sockaddr*)socket1, addrlens)))
		perror("send : ");
	ackpkt.printall();

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
	u_long		randnum;
	touPkg		ackpkt(0);
	int				rv, control=0, flagforsyn = 1;
	struct sockaddr_in sockaddrs;
	socklen_t len = sizeof(sockaddrs);
	sockTb *s = sm.getSocketTable(sd);
	char			recvcontent[RECVBUFSIZE];
	memset(recvcontent, 0, RECVBUFSIZE);

	/* If Received FIN */
	if(s->sockstate == TOUS_CLOSE_WAIT) {
		cout << "Server side baby " << endl;
		//  write(s->pfd[1], (ss), sizeof(ss));
		closer *cl = new closer(sd);
		//	m_thread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&touMain::dothis, this, sd)));
		cl->m_thread1.join();
	}

	/* Connection Initialization */
	sm.setTCBState(TOU_CC_SS, sd);
	srand((unsigned)time(NULL)); 
	randnum = rand()%(u_long)65535;
	sm.setTCB(randnum, randnum,sd); // it's own set.
	sm.setTCBCwnd(2024, sd);
	sm.setTCBAwnd(65535, sd);
	assignaddr(&sockaddrs,AF_INET,s->dip,s->dport);

	if(s->sockstate == TOUS_SYN_RECEIVED) {
		cerr<< "Recv the first SYN \n\n";
		s->printall();

		/* send SYN ACK: set up the ack pkt */
		ackpkt.clean();
		ackpkt.putHeaderSeq(s->tc.snd_nxt, s->tc.rcv_nxt);
		ackpkt.t.syn = FLAGON;
		ackpkt.t.ack = FLAGON;
		
		/* update the local TCB, and ready to send */
		sm.setTCB(s->tc.snd_nxt+1, s->tc.snd_una, sd);
		cout <<"Sending SYN ACK: Destination :  " << s->dip << "  " << s->dport << endl;
		string sendcontent = ackpkt.toString();
		if( -1 >= (rv = sendto(sd, sendcontent.data(), sendcontent.size(), 0, (struct sockaddr *)&sockaddrs, len)))
			perror("send : ");
		ackpkt.printall();
		s->printall();

		/*recv final(ACK) handshake */
		rv = recvfrom(sd, recvcontent, RECVBUFSIZE, 0, (struct sockaddr *)socket2, addrlen);
		string recvdata(recvcontent, rv);
		touPkg tp_ack(recvdata);

		/*If received FIN*/
		if(s->sockstate == TOUS_CLOSE_WAIT) {
			cout << "Server side baby " << endl;
			//  write(s->pfd[1], (ss), sizeof(ss));
			closer *cl = new closer(sd);
			//	m_thread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&touMain::dothis, this, sd)));
			cl->m_thread1.join();
		}

		cerr<< "Recving the final ACK \n\n";
		tp_ack.printall();

		if ((s->tc.snd_una+1) == tp_ack.getAckseq()){
			/* Connection ControlTabe Update */
			sm.setTCB(s->tc.snd_nxt,s->tc.snd_una+1, sd);
			cout<<"1. Accept received final 3-way handshake ACK. Connection ESTABLISHED !!!" << endl;
		}
		if ((s->tc.rcv_nxt) == tp_ack.getSeq()){
			/* Connection ControlTabe Update */
			sm.setTCBRcv(tp_ack.getSeq(), sd);
			cout<<"2. Accpet received final 3-way handshake ACK. Connection ESTABLISHED !!!" << endl;
		}

		sm.setSocketState(TOUS_ESTABLISHED,sd);
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
	ptou->send(sd);
	std::cout << "LEAVE touSend function, data push into circular buff\n";\
	return lenr;
}

/* 
 * Receive()
 * Simply fetch data from circular buffer
 * @result return no of bytes successfully received
 * revised @	1. Nov 24, 2009
 *						2. Jan 07, 2010 
 */
int touMain::touRecv(int sd, char *recvBuffer, int bufferLength, int flags) {
	int end;
	int readsize = 0 ;
	sockTb *s = sm.getSocketTable(sd);

	readsize = s->CbRecvBuf.getAt(recvBuffer,bufferLength,end);
	s->CbRecvBuf.remove(readsize);

	/* following is for test */
	if(readsize == bufferLength)
		std::cout << "[touRecv Test] READ complete file\n";
	else
		std::cout << "[touRecv Test] READ partial file XXX\n";
	/* end of test */

	if(s->sockstate == TOUS_CLOSE_WAIT){
		cout << " TOU Recv has received close from the remote side ..." << endl;
		return 0;
	}
	return readsize;
}/* End of touRecv */


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

int closer::assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, string ip, u_short port) {

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
		an = socktb->CbSendBuf.insert(sendbuf, len);
	}
	return an;
}
