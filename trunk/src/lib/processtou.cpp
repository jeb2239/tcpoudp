#include "tou.h"
boost::mutex recvqmutex;
boost::mutex sndqmutex;
#define RECVBUFSIZE	2000

/* PKTLOSTTEST test */
int pktlosttest_int = 1;

/**
 * processTou
 * responsible for receiving/sending of packets from circular buffer
 */
void processTou::run(int sockfd) {
	int			rv;					//bytes recved from other side, (NOT payload size)
	int			rvpl;				//byte recved form other side, payload size
	int			rvplq;			//byte stored in HpRecvBuf, payload size
	int			lenofcb;		//size of circular buffer
	int			state;			//FSM for processTou
	size_t	len = sizeof(sockaddrs);//sockaddr's size
	char		recvbuf[RECVBUFSIZE];
	char		recvcontent[RECVBUFSIZE] = "";
	string	pkgackcontent;

	socktb = sm->getSocketTable(sockfd);
	touPkg	pkgack(0);	//packet for sending ACK

	cout << "*** processTou Start Processtou now Waiting for data from client *** "  << endl;
	/* Waiting for incoming data */
	rv = recvfrom(sockfd, recvcontent, RECVBUFSIZE, 0, (struct sockaddr*)&sockaddrs,&len);
	string content(recvcontent, rv);
	tp = new touPkg(content);

	/* test print it out */
  cout << "size of content: "<<content.size() << " size of recvsize: "<<rv <<endl; //<<recvcontent<<endl<<endl<<content<<endl;
	tp->printall();
	cout << "Received pkt size: "<<rv <<" from : " << inet_ntoa(sockaddrs.sin_addr) <<  " " << htons(sockaddrs.sin_port) << endl; 
	/* end of test */
	
	state = processGetPktState(tp);
	while( state != PROCESS_END ){
	switch(state){
		case PROCESS_SYN: /* Connection Control */
			//Check if the connection is in estabished state
			cout << "SYN Received "  << endl;
			//if(socktb->sockstate == TOUS_ESTABLISHED)
			{
				//Do nothing	
			}
			//else
			{
				sm->setSocketTableD(&sockaddrs,sockfd);
				sm->setSocketState(TOUS_SYN_RECEIVED,sockfd);
				/* Connection Initailization */
				sm->setTCBRcv(tp->getSeq()+1, sockfd);
			}
			state = PROCESS_END;
		break;
		
		case PROCESS_FIN: /* Connection Control */
			cout<< "Received Fin " << endl ;
			tp->printall();
			/* Check for FIN flag (Close) : */
			cout<<"Closing Connection... " << endl;        
			sm->setSocketState(TOUS_CLOSE_WAIT,sockfd);
			state = PROCESS_END;
		break;
		
		case PROCESS_ACK_WITHOUT_DATA:
			std::cout<< "Get a (new ACK) in TOUS_ESTABLISHED state\n";
			if(!tm1.ck_del_timer(sockfd, 88, tp->t.ack_seq)){//ck if already have an ACK
				tm1.delete_timer(sockfd,88,tp->t.ack_seq);
				socktb->sc->addwnd();
				sm->setTCB(socktb->tc.snd_nxt, tp->t.ack_seq, sockfd); // set up snd_una only
			}else{//duplicate ack (already have one in delqueue)
				socktb->sc->setdwnd();
				std::cout << "duplicate ACK: "<< tp->t.ack_seq << " it's #: "<< socktb->tc.dupackcount<<std::endl;
			}
			tp->printall();
			socktb->printall();
			// wnd size goes up, got chance to send another pkt
			cout<< "processTou->SEND: Try to send data left in circular buff..."<<endl;
			send(sockfd);
			state = PROCESS_END;
		break;
		
		case PROCESS_ACK_WITH_DATA_MATCH_EXPECTED_SEQ:
			rvpl = tp->buf->size(); //get pkt payload size
			if(rvpl > TOU_MSS) rvpl = TOU_MSS; /* err: it'll read 1464, some dirty bits in the rear */
			
			/* For test */
			std::cout << "Get DATA in TOUS_ESTABLISHED state\n";
			std::cout << "@@@ DATA GET @@@ Data size: "<<rvpl <<std::endl;
			/* test sent pkt lost */
			if(tp->t.seq == 53715 && pktlosttest_int ){
			std::cout << "\n *** pkg 53715 lost test: no ack once PKTLOSTTEST *********\n";
			std::cout << " *** pkg 53715 lost test: no ack once PKTLOSTTEST *********\n\n";
			pktlosttest_int = 0;
			/* FOR TEST */goto PKTLOSTTEST;
			}
			
			/* try recovery from duplicate pkt first
			 * HpRecvBuf is not empty, and get the correct seq. Start recovery */
			if(!socktb->HpRecvBuf.empty()){
				std::cout<< " >>>>***>>> Inside recovery. match the current tp. solved.  seq#: " <<  
					socktb->tc.rcv_nxt << "number of elm in HpRecvBuf: " << socktb->HpRecvBuf.size() << "\n";
				sm->setTCBRcv(tp->t.seq + rvpl, sockfd);
				lenofcb = putcircbuf(socktb, sockfd, tp->buf, rvpl);
				std::cerr<< "socktb->HpRecvBuf.top().t.seq) == (socktb->tc.rcv_nxt " << 
					socktb->HpRecvBuf.top().t.seq << " " <<socktb->tc.rcv_nxt <<std::endl;
				/* recovery form HpRecvBuf
				 * out of order pkt: looply ck pkt in HpRecvBuf, and try to place this.tp into circ buf */
				while( (socktb->HpRecvBuf.top().t.seq) == (socktb->tc.rcv_nxt) ){
					std::cout<< " >>>>***>>> Inside matching while seq#: " <<  socktb->HpRecvBuf.top().t.seq << 
						"number of elm in HpRecvBuf: " << socktb->HpRecvBuf.size()<<"\n";
					rvplq = socktb->HpRecvBuf.top().buf->size();
					if(socktb->ckHpRecvBuf(socktb->HpRecvBuf.top())){
						/* dup pkt in heap, just pop it. so do nothing */
					}else{
						/* not dup, let this pkt in file */
						sm->setTCBRcv( socktb->HpRecvBuf.top().t.seq + rvplq, sockfd); /* set up rcv_nxt */
						std::string str(socktb->HpRecvBuf.top().buf->c_str(), rvplq);
						lenofcb = putcircbuf(socktb, sockfd, &str, rvplq);
					}
					/* this top pkt in q is back in file, pop it */
					socktb->HpRecvBuf.pop();
				}/* End of while */
				recovery = true;
			}
			if( recovery == false ){
				/* in-order pkt: expecting rcv_nxt number */
				sm->setTCBRcv(tp->t.seq + rvpl, sockfd); /* set up rcv_nxt */
				/* Try to put data into circular buff */
				lenofcb = putcircbuf(socktb, sockfd, tp->buf, rvpl);
				std::cout<< " >>>>***>>> Seqnumber matched \n";
			}
			recovery = false;
			state = PROCESS_ACK_DATARECSUCC_SENDBACK_ACK;
		break;
		
		case PROCESS_ACK_WITH_DATA_LESS_EXPECTED_SEQ:
			/* test don't care */
			std::cout<< "\n >>>>***>>> tp->t.seq < socktb->tc.rcv_nxt DONT CARE" 
			<<  tp->t.seq  << "number of elm in HpRecvBuf: " 
			<< socktb->HpRecvBuf.size() << "\n";
			state = PROCESS_ACK_DATARECSUCC_SENDBACK_ACK;
		break;
		
		case PROCESS_ACK_WITH_DATA_MORE_EXPECTED_SEQ:
			socktb->HpRecvBuf.push(*tp);
			std::cout<< "\n >>>>***>>> Inside push into the HpRecvBuf: "
			<<tp->t.seq  << "number of elm in HpRecvBuf: " 
			<< socktb->HpRecvBuf.size() << "\n";
			state = PROCESS_ACK_DATARECSUCC_SENDBACK_ACK;
		break;
		
		case PROCESS_ACK_DATARECSUCC_SENDBACK_ACK:
			/* send ACK back to sender */
			pkgack.clean();
			assignaddr((struct sockaddr_in *)&sockaddrs, AF_INET, socktb->dip, socktb->dport);
			pkgack.putHeaderSeq(socktb->tc.snd_nxt, socktb->tc.rcv_nxt);
			pkgack.t.ack = FLAGON;
			std::cout << " >>> [SEND ACK] data sent <<< " <<std::endl;
			pkgack.printall();
			pkgackcontent = pkgack.toString();
			sendto(sockfd, pkgackcontent.data(), pkgackcontent.size(), 0, (struct sockaddr *)&sockaddrs, sizeof(sockaddrs));
			socktb->printall();
			state = PROCESS_END;
		break;
		
		default:
			std::cerr << "Error in PROCESS SWITCH STATE\n";
			state = PROCESS_END;
		break;
		
	}/* END OF SWITCH */
	}/* END OF WHILE(STATE) LOOP */

	PKTLOSTTEST: /* for test */
	cout << " *** Leaving process tou *** " << endl;
	/* delete received tou pakcet */
	delete tp;
}/* END of processtou */

/**
 * processGetPktState
 * return state by analyzing newly received packet
 * @result return the state of process state
 */
int processTou::processGetPktState(touPkg *tp){
	if(tp->t.syn == FLAGON)
		return PROCESS_SYN;

	else if(tp->t.fin == FLAGON)
		return PROCESS_FIN;
	
	if(tp->t.ack == FLAGON &&
			socktb->sockstate == TOUS_ESTABLISHED && 
			socktb->tc.snd_nxt == tp->t.seq)
		return PROCESS_ACK_WITHOUT_DATA;

	/* receiving data */
	if(tp->t.ack == FLAGON && 
			0 < tp->buf->size() && 
			tp->t.seq == socktb->tc.rcv_nxt &&
			socktb->sockstate == TOUS_ESTABLISHED)
		return PROCESS_ACK_WITH_DATA_MATCH_EXPECTED_SEQ;

	else if(tp->t.ack == FLAGON &&
			0 < tp->buf->size() &&
			tp->t.seq < socktb->tc.rcv_nxt &&
			socktb->sockstate == TOUS_ESTABLISHED)
		return PROCESS_ACK_WITH_DATA_LESS_EXPECTED_SEQ;

	else if(tp->t.ack == FLAGON &&
			0 < tp->buf->size() &&
			tp->t.seq > socktb->tc.rcv_nxt &&
			socktb->sockstate == TOUS_ESTABLISHED)
		return PROCESS_ACK_WITH_DATA_MORE_EXPECTED_SEQ;

	return PROCESS_END;
}

int processTou::popsndq(sockTb *socktb, char *sendbuf, int len) {
	//boost::mutex::scoped_lock lock(sndqmutex);
	int	bread, end;
	memset(sendbuf, 0, TOU_MSS);

	/* try to get data form circular buffer */
	if( 0 < (bread = socktb->CbSendBuf.getAt(sendbuf, len, end))) {
		socktb->CbSendBuf.remove(bread);
		return bread;
	}
	return 0;
}

/* get the sockaddr_in infomation */
int processTou::assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, string ip, unsigned short port){
	bzero(sockaddr, sizeof(*sockaddr));
	sockaddr->sin_family = sa_family;
	sockaddr->sin_port = htons(port);
	if( 1 != inet_pton(sa_family, ip.c_str(), &sockaddr->sin_addr) )
		return 0;
	return 1; 
}

/* push data into circular buf */
int processTou::putcircbuf(sockTb *socktb, int sockfd, std::string *str, int rvpl){
	int lenofcb;
	char buf[str->size()];
	strncpy(buf, str->c_str(), str->size());

	/* Try to put data into circular buff */
	if( rvpl <= (lenofcb = (socktb->CbRecvBuf.getAvSize())) ){
		// insert all of buf into cb
		lenofcb = sm->setCbData(buf, rvpl, sockfd);
	}else{
		// just insert size of available cb
		lenofcb = sm->setCbData(buf, lenofcb, sockfd);
	}
	return lenofcb;
}

/* sending pkt to other end */
void processTou::send(int sockfd) {
	socktb = sm->getSocketTable(sockfd);
	char					buf[TOU_SMSS];
	touPkg				*toupkg;
	int						totalelm;
	int						len;
	unsigned long	curwnd; //current wnd size
	unsigned long	sndsize = 0;
	sockaddr_in		sockaddrs ;

	/* set up recv's info */
	assignaddr(&sockaddrs, AF_INET, socktb->dip, socktb->dport);

	//while( 0 < (totalelm = socktb->CbSendBuf.getTotalElements()))
	{
		// there're some data in the buf need to be send 
		cout << " *** Inside: processtou::send number of bytes in the circular buff: "<< socktb->CbSendBuf.getTotalElements() <<endl;

		/* get the current window size */
		curwnd = socktb->sc->getwnd();
		sndsize=((socktb->tc.snd_una + curwnd)-socktb->tc.snd_nxt);

		socktb->printall();//TEST
		cout <<"how much i can snd base on current window size: "<< sndsize<<std::endl;

		while( (0 < socktb->CbSendBuf.getTotalElements() ) && (0 < ( sndsize=((socktb->tc.snd_una + curwnd)- socktb->tc.snd_nxt)))) {
			if( sndsize >= TOU_SMSS ){
				//available size is bigger than MSS, so send size of SMSS at most
				len = popsndq(socktb, buf, TOU_SMSS);
				cout << "Client send (>= TOU_SMSS)"<<endl;
			}else{
				//wnd available for sending is less than SMSS
				len = popsndq(socktb, buf, sndsize);
				cout << "Client send (< TOU_SMSS)"<<endl;
			}
			toupkg = new touPkg( buf, len); 
			toupkg->putHeaderSeq(socktb->tc.snd_nxt, socktb->tc.rcv_nxt);
			toupkg->t.ack = FLAGON;

			//snd_nxt moves on
			socktb->tc.snd_nxt += len;

			// send
			std::cout << " >>> [SEND] data length sent: "<<len<<", and origianl sndsize is "<<sndsize<<std::endl;
			toupkg->printall();
			string sendcontent = toupkg->toString();
			sendto(sockfd, sendcontent.data(), sendcontent.size(), 0, (struct sockaddr *)&sockaddrs, sizeof(sockaddr));

			// add timer
			tm1.add(sockfd, 88, toupkg->t.seq + len, socktb, toupkg->buf);
			//delete toupkg;
		}//end of  while( 0 < ( sndsize=((socktb->tc.snd_una + curwnd)-socktb->tc.snd_nxt)))

		cout << " *** LEAVE: processtou::sendi\n " <<endl;
	}//end of  while( socktb->CbSendBf.getTotalElements() > 0)
}

