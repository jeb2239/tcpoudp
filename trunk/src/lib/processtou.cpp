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
	char		recvcontent[RECVBUFSIZE];
	touPkg	*tp;
	memset(recvcontent, 0, RECVBUFSIZE);
	string	pkgackcontent;

	socktb = sm->getSocketTable(sockfd);
	touPkg	pkgack(0);	//packet for sending ACK

	cout << "*** processTou Start Processtou now Waiting for data from client *** "  << endl;
	/* Waiting for incoming data */
	rv = recvfrom(sockfd, recvcontent, RECVBUFSIZE, 0, (struct sockaddr*)&sockaddrs,&len);

	string content(recvcontent, rv);
	tp = new touPkg(content); 

	/* test print it out */
  //cout << "size of content: "<<content.size() << " size of recvsize: "<<rv <<endl; //<<recvcontent<<endl<<endl<<content<<endl;
	tp->printall();
	cout << "Received pkt size: "<<rv <<" from : " << inet_ntoa(sockaddrs.sin_addr) <<  " " << htons(sockaddrs.sin_port) << endl; 
	/* end of test */
	
	state = processGetPktState(tp);
	while( state != PROCESS_END ){
	switch(state){
		case PROCESS_SYN: /* Connection Control */
			//Check if the connection is in estabished state
			cout << "[PROCESSTOU MSG] PROCESS_SYN: SYN Received "  << endl;
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
			cout<< "[PROCESSTOU MSG] PROCESS_FIN: Received Fin " << endl ;
			tp->printall();
			/* Check for FIN flag (Close) : */
			cout<<"Closing Connection... " << endl;        
			sm->setSocketState(TOUS_CLOSE_WAIT,sockfd);
			state = PROCESS_END;
		break;
		
		case PROCESS_ACK_WITHOUT_DATA:
			lg.logData("[PROCESSTOU MSG] PROCESS_ACK_WITHOUT_DATA: new ACK", 
					TOULOG_ALL|TOULOG_PTSRN);

			/* Checking whether this pkt is existed in del timer queue(buf) */
			if (!tm1.ck_del_timer(sockfd, 88, tp->t.ack_seq)) {
				/* No, push it into del timer queue */
				tm1.delete_timer(sockfd,88,tp->t.ack_seq);
				socktb->sc->addwnd();
				sm->setTCB(socktb->tc.snd_nxt, tp->t.ack_seq, sockfd); // update snd_una
			}else{
				/* duplicate ack (already have one in delqueue) */
				socktb->sc->setdwnd();

				/* dup acks are exceed number of three. 
				 * init "fast retransmit" algorithm */
				if(socktb->ck_dupack_3()) // while it eq to 3, rexmit
					tm1.rexmit_for_dup_ack(sockfd, 88, tp->t.ack_seq);

				lg.logData("[PROCESS_ACK_WITHOUT_DATA] DUPLICATE ACK: " + 
						lg.c2s(tp->t.ack_seq) + " Duplicate ACKs count #: " + 
						lg.c2s(socktb->tc.dupackcount), TOULOG_ALL|TOULOG_PTSRN);
			}
			socktb->printall();
			lg.logData("[PROCESS_ACK_WITHOUT_DATA] Try to send data left in circ buff",
					TOULOG_ALL|TOULOG_PTSRN);

			/* Because window size is updated, try to send data left in circ buf */
			send(sockfd);

			state = PROCESS_END;
		break;
		
		case PROCESS_ACK_WITH_DATA_MATCH_EXPECTED_SEQ:
			lg.logData("[PROCESSTOU MSG] PROCESS_ACK_WITH_DATA_MATCH_EXPECTED_SEQ", 
					TOULOG_ALL|TOULOG_PTSRN);
			rvpl = tp->buf->size(); //get pkt payload size

			/* test sent pkt lost */
			if(tp->t.seq == 18417 && pktlosttest_int ){
			std::cout << "\n *** pkg 18417 lost test: no ack once PKTLOSTTEST ****\n";
			std::cout << " *** pkg 18417 lost test: no ack once PKTLOSTTEST ****\n\n";
			/* FOR TEST */goto PKTLOSTTEST;
			}

			/* try recovery from duplicate pkt first
			 * HpRecvBuf is not empty, and get the correct seq. Start recovery */
			if (!socktb->HpRecvBuf.empty()) {
				lg.logData(" >>> Processtou Recovery Start >>> Recovering from seq#: " +
						lg.c2s(socktb->tc.rcv_nxt), TOULOG_ALL|TOULOG_PTSRN);

				sm->setTCBRcv(tp->t.seq + rvpl, sockfd);
				lenofcb = putcircbuf(socktb, sockfd, tp->buf, rvpl);

				/* recovery form HpRecvBuf
				 * out of order pkt: looply ck pkt in HpRecvBuf, and try to put pkt back
				 * to circular buf. */
				while ((socktb->HpRecvBuf.top().t.seq) == (socktb->tc.rcv_nxt)) {
					lg.logData(" >>> Processtou Recovery >>> seq#: " + 
							lg.c2s(socktb->HpRecvBuf.top().t.seq) + " elm#: " + 
							lg.c2s(socktb->HpRecvBuf.size()), TOULOG_ALL|TOULOG_PTSRN);

					rvplq = socktb->HpRecvBuf.top().buf->size();

					if(socktb->ckHpRecvBuf(socktb->HpRecvBuf.top())){
						/* dup pkt in heap(buf), just pop it without doing nothing */
					}else{
						/* no dup, put this pkt back in file: 
						 * 1. update rcv_nxt. 
						 * 2. move data to circular buf. */
						sm->setTCBRcv(socktb->HpRecvBuf.top().t.seq + rvplq, sockfd);
						std::string str(socktb->HpRecvBuf.top().buf->c_str(), rvplq);
						lenofcb = putcircbuf(socktb, sockfd, &str, rvplq);
					}

					/* top pkt in queue(buf) is back in file now, then pop it */
					socktb->HpRecvBuf.pop();

				}/* End of while */

			}else{
				/* Handle in-order pkt (not recovery needed) */
				/* in-order pkt: expecting rcv_nxt number */
				sm->setTCBRcv(tp->t.seq + rvpl, sockfd);
				lenofcb = putcircbuf(socktb, sockfd, tp->buf, rvpl);
			}

			state = PROCESS_ACK_DATARECSUCC_SENDBACK_ACK;
		break;
		
		case PROCESS_ACK_WITH_DATA_LESS_EXPECTED_SEQ:
			lg.logData("[PROCESSTOU MSG] PROCESS_ACK_WITH_DATA_LESS_EXPECTED_SEQ", 
					TOULOG_ALL|TOULOG_PTSRN);

			lg.logData(">> Discard it. Packet sequence number: " + lg.c2s(tp->t.seq)
					, TOULOG_ALL|TOULOG_PTSRN);

			state = PROCESS_ACK_DATARECSUCC_SENDBACK_ACK;
		break;
		
		case PROCESS_ACK_WITH_DATA_MORE_EXPECTED_SEQ:
			lg.logData("[PROCESSTOU MSG] PROCESS_ACK_WITH_DATA_MORE_EXPECTED_SEQ", 
					TOULOG_ALL|TOULOG_PTSRN);

			/*
			// test sent pkt lost 
			if(tp->t.seq == 19873 && pktlosttest_int ){
			std::cout << "\n *** pkg 19873 lost test: no ack once PKTLOSTTEST ****\n";
			std::cout << " *** pkg 19873 lost test: no ack once PKTLOSTTEST ****\n\n";
			// FOR TEST 
			goto PKTLOSTTEST;
			}
			*/

			/* test sent pkt lost */
			if(tp->t.seq == 21329 && pktlosttest_int ){
			std::cout << "\n *** pkg 21329 lost test: no ack once PKTLOSTTEST ****\n";
			std::cout << " *** pkg 21329 lost test: no ack once PKTLOSTTEST ****\n\n";
			goto PKTLOSTTEST;
			}

			/* test sent pkt lost */
			if(tp->t.seq == 24241 && pktlosttest_int ){
			std::cout << "\n *** pkg 24241 lost test: no ack once PKTLOSTTEST ****\n";
			std::cout << " *** pkg 24241 lost test: no ack once PKTLOSTTEST ****\n\n";
			pktlosttest_int = 0;
			/* FOR TEST */goto PKTLOSTTEST;
			}

			socktb->pushHpRecvBuf(*tp);
			lg.logData(">> Push into HpRecvBuf. Packet seq #: " + lg.c2s(tp->t.seq) +
					" Size of HpRecvBuf is: " + lg.c2s(socktb->HpRecvBuf.size()) , 
					TOULOG_ALL|TOULOG_PTSRN);

			state = PROCESS_ACK_DATARECSUCC_SENDBACK_ACK;
		break;
		
		case PROCESS_ACK_DATARECSUCC_SENDBACK_ACK:
			lg.logData("[PROCESSTOU MSG] PROCESS_ACK_DATARECSUCC_SENDBACK_ACK", 
					TOULOG_ALL|TOULOG_PTSRN);

			/* Sending ACK back to sender */
			pkgack.clean();
			assignaddr((struct sockaddr_in *)&sockaddrs, AF_INET, socktb->dip, 
					socktb->dport);
			pkgack.putHeaderSeq(socktb->tc.snd_nxt, socktb->tc.rcv_nxt);
			pkgack.t.ack = FLAGON;
			pkgack.printall();
			pkgackcontent = pkgack.toString();
			sendto(sockfd, pkgackcontent.data(), pkgackcontent.size(), 0, 
					(struct sockaddr *)&sockaddrs, sizeof(sockaddrs));

			state = PROCESS_END;
		break;
		
		default:
			std::cerr << "Error in PROCESS SWITCH STATE" << std::endl;
			state = PROCESS_END;
		break;
		
	}/* END OF SWITCH */
	}/* END OF WHILE(STATE) LOOP */

	PKTLOSTTEST: /* for test */
	socktb->printall();
	cout << " *** Leaving process tou *** " << endl;
	/* delete received tou pakcet */
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
			tp->buf->size() == 0 &&
			socktb->tc.snd_nxt >= tp->t.ack_seq)
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

  cerr << "[PROCESSTOU] state error : PROCESS_END \n";
	socktb->printall();
	tp->printall();
	exit(1);
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
int processTou::assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, 
		string ip, unsigned short port){

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
	unsigned long	sndsize = 0;
	sockaddr_in		sockaddrs ;

	/* set up recv's info */
	assignaddr(&sockaddrs, AF_INET, socktb->dip, socktb->dport);

	lg.logData("[PROCESSTOU SEND] # of bytes in the circ buff: " +
			lg.c2s(socktb->CbSendBuf.getTotalElements()), TOULOG_ALL|TOULOG_PTSRN);

	/* get the current window size */
	sndsize = getSendSize(socktb);

	socktb->printall();//TEST
	lg.logData("[PROCESSTOU SEND] # of bytes can be sent base on window size: " +
			lg.c2s(sndsize), TOULOG_ALL|TOULOG_PTSRN);

	while ((0 < socktb->CbSendBuf.getTotalElements() ) && 
			(0 < (sndsize = getSendSize(socktb)) )) {

		/* Deciding how many bytes can be sent in this packet.
		 * >= available size is bigger than MSS, so send # of size of SMSS
		 * else wnd available for sending is less than SMSS, send sndsize*/
		len = (sndsize >= TOU_SMSS)? popsndq(socktb, buf, TOU_SMSS): popsndq(socktb, 
				buf, sndsize);

		toupkg = new touPkg( buf, len); 
		toupkg->putHeaderSeq(socktb->tc.snd_nxt, socktb->tc.rcv_nxt);
		toupkg->t.ack = FLAGON;

		// snd_nxt move on
		socktb->tc.snd_nxt += len;

		// sending
		lg.logData("[PROCESSTOU SEND] >>> [SEND] data length sent: " + lg.c2s(len) +
				", and original sndsize is: " + lg.c2s(sndsize), TOULOG_ALL|TOULOG_PTSRN);
		toupkg->printall();
		string sendcontent = toupkg->toString();
		sendto(sockfd, sendcontent.data(), sendcontent.size(), 0, 
				(struct sockaddr *)&sockaddrs, sizeof(sockaddr));

		// add timer
		tm1.add(sockfd, 88, toupkg->t.seq + len, socktb, toupkg->buf);

	}//End of while

	cout << " *** LEAVE: processtou::send " <<endl;
}

/**
 * getSendSize(sockTb &socktb):
 * return current send window size available
 */
unsigned long processTou::getSendSize(sockTb *socktb) {
	unsigned long sndsize = 0;
	unsigned long curwnd; //current wnd size

	curwnd = socktb->sc->getwnd();
	if ((socktb->tc.snd_una+curwnd) >= socktb->tc.snd_nxt){ 
		sndsize=((socktb->tc.snd_una + curwnd)-socktb->tc.snd_nxt);
	}

	return sndsize;
}

