/********************************************************
 * tou.cpp 
 * This is a main ToU API where most of the functions used
 * by application are resided here.
 * ******************************************************/

#include "tou.h"

#define RECVBUFSIZE 2000
using namespace std;

boost::mutex socktabmutex1;	//used by touClose function
processTou	*ptou;

/*
 * touSocket:
 * Create Socket
 * @result -1 if the creation is not successful.
 * Last modified: Feb 24, 2010
 */
int touMain::touSocket(int domain, int type, int protocol) {

	if ((domain != AF_INET) || (type != SOCK_DGRAM) || (protocol != 0 )) {
		cout << "ERROR CREATING SOCKET" ;
		return -1 ;
	}
	sd = socket(domain,type,0);
	sm.addSocketTable(sd);
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, NULL, sizeof(int)); 

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
	//sm.setSocketTable((sockaddr_in *)my_addr,sockfd);
	sm.setSocketTable(sockfd, (sockaddr_in *)my_addr, NULL, SOCKTB_SET_SRC_TUPLE);

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
int touMain::touConnect(int sd, struct sockaddr_in *sock_dst, int addrlen) {

	int			rv, rcvbuf;
	sockaddr_in		sockaddrs;
	socklen_t		len = sizeof(sockaddr);
	size_t			addrlens = sizeof(sockaddr);
	sockTb			*s = sm.getSocketTable(sd);
	touPkg			*tp_sa;
	char			recvcont[RECVBUFSIZE] = "";

	/* Init the socktb */
	//sm.setSocketTableD(sock_dst,sd);		
	sm.setSocketTable(sd, NULL, sock_dst, SOCKTB_SET_DST_TUPLE);
	touPkg ackpkt(0);
	ackpkt.clean();

	srand((unsigned)time(NULL)); 
	//ackpkt.putHeaderSeq(rand()%(u_long)65535, (u_long)0);  
	//For test, assigned seq #
	ackpkt.putHeaderSeq((u_long)0, (u_long)0);
	ackpkt.t.syn = FLAGON;

	sm.setSocketState(TOUS_SYN_SENT,sd);
	sm.setTCBState(TOU_CC_SS, sd);
	sm.setTCB(ackpkt.getSeq()+1, ackpkt.getSeq(),sd);
	sm.setTCBRcv(0, sd);
	sm.setTCBCwnd((TOU_MSS*2), sd);
	rcvbuf = getSocketRcvBuf(sd);
	assert(rcvbuf > 0);
	sm.setTCBAwnd((int)(rcvbuf-300000), sd);

	/* sm.setTCBAwnd((int)(rcvbuf/2.1), sd); */
	s->log();

	/* Send the SYN */
	//cout<<"Sending SYN to  :  " << inet_ntoa(sock_dst->sin_addr)<< "  " <<htons(sock_dst->sin_port) << endl;
	string tempdata = ackpkt.toString();
	if( 0 >  (rv = sendto(sd, tempdata.data(), tempdata.size(), 0,(struct sockaddr*)sock_dst,sizeof(struct sockaddr_in))))
		perror("Send the SYN");
  //cout << tempdata;
	//cout << "[SYN] size of content: "<< tempdata.size() << " rv: "<< rv << "sizeof: "<< sizeof(tempdata) << endl;

	setSocketBlock(sd);

	/*Prepare for receiving */
	/*Receive SYN ACK */
	if (rv = recvfrom(sd, recvcont, RECVBUFSIZE, 0, (struct sockaddr*)&sockaddrs, &len)){
		string tempdata(recvcont, rv);
		tp_sa = new touPkg(tempdata);
		sockTb *s = sm.getSocketTable(sd);
		sm.setTCBRcv(tp_sa->t.seq,sd);
		//cout << "[SYN ACK] size of content: "<< tempdata.size() << " rv: "<< rv << "sizeof: "<< sizeof(tempdata) << endl;

		if((s->sockstate) == TOUS_CLOSE_WAIT)
			cout <<"Other end shutdown !! "<< endl;
		
	}else if( -1 == rv ){
		perror("Recv SYN ACK: ");
	}

	//for the rest of the recvfrom operate in non block mode
	setSocketNonblock(sd);

	/* update the pkt.seq to local tcp.rvc_nxt */
	sm.setTCBRcv(tp_sa->getSeq()+1, sd); // cli's recv seq
	sm.setTCB(s->tc.snd_nxt, tp_sa->getAckseq(), sd); //cli's send seq(here just update ackseq)

	/* set up the fianl 3-way pkt */
	ackpkt.clean();
	ackpkt.putHeaderSeq(s->tc.snd_nxt, s->tc.rcv_nxt);
	ackpkt.t.syn = FLAGOFF;
	ackpkt.t.ack = FLAGON;
	
	s->log();
	string tempdata_ack = ackpkt.toString();
	/*send ACK - final 4way handshake */
	if( -1 >= (rv = sendto(sd, tempdata_ack.data(), tempdata_ack.size() , 0, (struct sockaddr*)sock_dst, addrlens)))
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

	u_long			randnum;
	touPkg			ackpkt(0);
	int			rv, control=0, flagforsyn = 1, rcvbuf;
	struct sockaddr_in	sockaddrs;

	socklen_t len = sizeof(sockaddrs);
	sockTb *s = sm.getSocketTable(sd);
	char recvcontent[RECVBUFSIZE];
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
	sm.setTCBCwnd((TOU_MSS*2), sd);
	rcvbuf = getSocketRcvBuf(sd);
	assert(rcvbuf > 0);
	sm.setTCBAwnd((int)(rcvbuf-300000), sd);

	/* sm.setTCBAwnd((int)(rcvbuf/2.1), sd); */
	cerr << "Default Accept buffer size ... : " << rcvbuf << endl;
	assignaddr(&sockaddrs,AF_INET,s->dip,s->dport);

	if(s->sockstate == TOUS_SYN_RECEIVED) {
		cerr<< "Recv the first SYN \n\n";

		/* send SYN ACK: set up the ack pkt */
		ackpkt.clean();
		ackpkt.putHeaderSeq(s->tc.snd_nxt, s->tc.rcv_nxt);
		ackpkt.t.syn = FLAGON;
		ackpkt.t.ack = FLAGON;
		
		/* update the local TCB, and ready to send */
		sm.setTCB(s->tc.snd_nxt+1, s->tc.snd_una, sd);
		//cout <<"Sending SYN ACK: Destination :  " << s->dip << "  " << s->dport << endl;
		string sendcontent = ackpkt.toString();
		if( -1 >= (rv = sendto(sd, sendcontent.data(), sendcontent.size(), 0, (struct sockaddr *)&sockaddrs, len)))
			perror("send : ");

		/*recv final(ACK) handshake */
		setSocketBlock(sd);
		rv = recvfrom(sd, recvcontent, RECVBUFSIZE, 0, (struct sockaddr *)socket2, addrlen);
		setSocketNonblock(sd);

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
		s->log();
		return true;
	}

	return false;
}

/* 
 * touSend():
 * It simply puts the data onto circular buffer available, and returns the
 * number of bytes that been put successfully.
 * @result return number of bytes successfully sent  
 */
int touMain::touSend(int sd, char *sendBufer, int len, int flags) { 

	int lenr = -1;
	lenr = pushsndq(sd , sendBufer,  len);
	ptou->send(sd);

	return lenr;
}

/* 
 * Receive()
 * Simply fetch data from circular buffer
 * @return number of bytes on successfully read
 * @return -1 on error
 * @return 0 on connection close
 */
int touMain::touRecv(int sd, char *recvBuffer, int bufferLength, int flags) {

	int end, readsize = 0, clearrs = 0;
	sockTb *s = sm.getSocketTable(sd);

	if (s != NULL ) {
		readsize = s->cb_recv.getAt(recvBuffer,bufferLength,end);
		clearrs = s->cb_recv.remove(readsize);
		if (clearrs != readsize) 
			readsize = -1;
	}

	if(s->sockstate == TOUS_CLOSE_WAIT){ /* To chinmay: is this the only condition?*/
		readsize = 0;
	}

	return readsize;
}/* End of touRecv */


/* 
 *get the sockaddr_in infomation 
 */
int touMain::assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, 
		string ip, u_short port) {

	bzero(sockaddr, sizeof(*sockaddr));
	sockaddr->sin_family = sa_family;
	sockaddr->sin_port = htons((short)(port));
	if( 1 != inet_pton(sa_family, ip.c_str(), &sockaddr->sin_addr) )
		return 0;
	return 1;
}

int closer::assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, 
		string ip, u_short port) {

	bzero(sockaddr, sizeof(*sockaddr));
	sockaddr->sin_family = sa_family;
	sockaddr->sin_port = htons((short)(port));
	if( 1 != inet_pton(sa_family, ip.c_str(), &sockaddr->sin_addr) )
		return 0;
	return 1;
}

/**
 * pushsndq:
 * It performs the task of putting data into circular buffer. The operation
 * should be cooperated with thread lock/unlock.
 * @return number of bytes been put into circular buffer.
 */
int touMain::pushsndq(int sockfd, char *sendbuf, int &len) {

	int an = 0; //actual number of bytes been sent
	sockTb *stb = sm.getSocketTable(sockfd);

	/* insert the data into circular buf */
	if (stb != NULL) 
		an = stb->cb_send.insert(sendbuf, len);

	return an;
}

/**
 * proTou:
 * Initiate processtou
 */
int touMain::proTou(int sockfd){

	//ptou was declared at processtou.h
	ptou = new processTou(sockfd, &sm);

	return 0;
}

/*
 * getCirSndBuf 
 * get circular buffer's send available size.
 */
unsigned long touMain::getCirSndBuf() {

	return sm.getCirSndBuf();
}

/**
 * setSocketNonblock
 * manually set the socket to nonblock mode
 */
void touMain::setSocketNonblock(int sockfd) {

	int flags = fcntl(sockfd, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(sockfd, F_SETFL, flags);

}

/**
 * setSocketBlock
 * manually set the socket to block mode
 */
void touMain::setSocketBlock(int sockfd) {

	int flags = fcntl(sockfd, F_GETFL);
	flags &= ~O_NONBLOCK;
	fcntl(sockfd, F_SETFL, flags);

}

/** 
 * getSocketBuffer
 * SO_RCVBUF is telling what the actual socket buffer size is.
 * (has nothing to do with how much data is being held at the moment) 
 * return size of buffer in byte on success. -1 on error
 *
 */
int touMain::getSocketRcvBuf(int sockfd) {

	int retval;
	socklen_t retlen;
	if (getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &retval, &retlen) != 0) {
		if (errno == EBADF) {
			cout << "The parameter s is not a valid descriptor." << endl;
		}else if (errno == ENOPROTOOPT) {
			cout << "The option is unknown at the level indicated." << endl;
		}else if (errno == ENOTSOCK) {
			cout << "The parameter s is a file, not a socket." << endl;
		}else{
			cout << "getsockopt unknown error. "<< retval << endl;
		}
		retval = -1;
	}
	return retval;
}

/**
 * run the processtou run function
 */
void touMain::run(int sockfd) {
	ptou->run(sockfd);

}






void closer::dothis(int sd) {
	std::cout << "Inside do this .... " << std::endl;
	sockTb *s = sm.getSocketTable(sd);
	{
		sockaddr_in sockaddrs,socket1;
		socklen_t len = sizeof(sockaddr);
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
			//s->printall();
			int rv = sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)&sockaddrs , sizeof(struct sockaddr_in));
			//tp.printall();
			//tp.clean();
			fd_set socks;
			struct timeval tim;
			FD_ZERO(&socks);
			FD_SET(sd, &socks);
			tim.tv_sec = 5;
			cout << "waiting for ACK" << endl;
			socklen_t  len = sizeof(sockaddrs);
			//while(1)
			{
				//if (select(sd+1, &socks, NULL, NULL, &tim))
				{
					cout << " Waiting 2 MSL and ack" << endl ;
					recvfrom(sd,&tp, sizeof(tp),0,(struct sockaddr*)&sockaddrs , &len);
					//tp.printall();
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
			socklen_t len = sizeof(sockaddr);
			//tp.t.seq = s->tc.snd_nxt;
			sm.setSocketState(TOUS_FIN_WAIT_1,sd);
			cout << " Receiving Fin ACK " << endl;
			recvfrom(sd,&tp,sizeof(tp),0,(struct sockaddr *)&socket1,&len);
			cout <<" after recv from" << endl;
			//tp.printall();
			if(tp.t.fin ==1 && tp.t.ack ==1) {
				sm.setSocketState(TOUS_TIME_WAIT,sd);
				cout << " Received FIN ACK" << endl;
			}
			tp.clean();
			tp.t.ack = 1;
			tp.t.seq = 3;
			cout << " Sending ACK " << endl;
			//s->printall();
			sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)&sockaddrs , sizeof(struct sockaddr_in));
			//tp.printall();
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
	touPkg tp(0);
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
		u_short port = (s->dport);
		assignaddr(&sockaddrs,AF_INET,s->dip,port);
		tp.clean();
		tp.t.seq = 0;
		tp.t.fin = FLAGON;
		tp.t.ack_seq = tp.t.seq + 1;
		cout << "Sending Fin " << endl;
		//s->printall();
		int rv = sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)&sockaddrs , sizeof(struct sockaddr_in));
		//tp.printall();
		/*Client Side */
		cout << "Client side baby " << endl;
		closer *cl = new closer(sd);
		cl->m_thread1.join();
	}

	return 1;


}
