#include "tou.h"
boost::mutex recvqmutex;
boost::mutex sndqmutex;
#define RECVBUFSIZE	2000
using namespace std;

/* PKTLOSTTEST test */
int pktlosttest_int = 1;
static int counter = 0;
/* END PKTLOSETEST test */
static int cwnd_counter = 0;
/* congestion window counter */
// for midterm demo
extern FILE *ofile;
extern FILE *ofile_cwnd;
//


/**
 * processTou
 * responsible for receiving/sending of packets through circular buffer
 */
void processTou::run(int sockfd) {
	int			rv;					//bytes recved from other side(not payload size)
	int			rvpl;				//byte recved form other side, payload size
	int			rvplq;			//byte stored in HpRecvBuf, payload size
	int			lenofcb;		//size of circular buffer
	int			state;			//FSM for processTou
	size_t	len = sizeof(sockaddrs);//sockaddr's size
	char		recvbuf[RECVBUFSIZE];
	char		recvcontent[RECVBUFSIZE];
	touPkg	*tp;				//packet for receiving data
	touPkg	pkgack(0);	//packet for sending ACK
	string	pkgackcontent;
	socktb = sm->getSocketTable(sockfd);

	lg.logData("[PROCESSTOU MSG] ****** PROCESSTOU START ******", TOULOG_ALL);

	/* Waiting for incoming data */
	memset(recvcontent, NULL, RECVBUFSIZE);
	rv = recvfrom(sockfd, recvcontent, RECVBUFSIZE, 0, 
			(struct sockaddr*)&sockaddrs,&len);

	/* Convert char* to packet struct */
	string content(recvcontent, rv);
	tp = new touPkg(content); 
	tp->log(TOULOG_ALL);

	/* Base on the packet received, decide what's the current state.
	 * Then pass it into a FSM for subsequent processing. */
	state = processGetPktState(tp);
	while( state != PROCESS_END ){
	switch(state){
		case PROCESS_SYN: /* Connection Control: Chinmay add here.*/
			//Check if the connection is in estabished state
			//cout << "[PROCESSTOU MSG] PROCESS_SYN: SYN Received "  << endl;
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
		
		case PROCESS_FIN: /* Connection Control: Chinmay add here. */
			//cout<< "[PROCESSTOU MSG] PROCESS_FIN: Received Fin " << endl ;
			/* Check for FIN flag (Close) : */
			//cout<<"Closing Connection... " << endl;        
			sm->setSocketState(TOUS_CLOSE_WAIT,sockfd);
			state = PROCESS_END;
		break;
		

		/* While receiving the ack from the peer host, it firstly checks whether a
		 * packet is duplicate by looking into the del_timer_queue(buf). If it's
		 * duplicate packet, then I need to check whether the number of duplicate
		 * packet is exceed three. If yes, perform fast retransmit. */
		case PROCESS_ACK_WITHOUT_DATA:
			lg.logData("[PROCESSTOU MSG] PROCESS_ACK_WITHOUT_DATA: new ACK", 
					TOULOG_ALL);

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
						lg.c2s(socktb->tc.dupackcount), TOULOG_ALL);
			}

			/* Fired pkt handling,
			 * while receiving pkg with seq # <= t_timeout_seq, it means that the
			 * fired pkt is received. Then we should decrease t_timerout_postpone
			 * by one. */
			if (tp->t.ack_seq <= socktb->tc.t_timeout_seq && 
					0 < socktb->tc.t_timeout_postpone)
				socktb->tc.t_timeout_postpone--;

			lg.logData("[PROCESS_ACK_WITHOUT_DATA] Try to send data left in circ buff",
					TOULOG_ALL);

			/* Because window size is updated, try to send data left in circ buf */
			send(sockfd);

			state = PROCESS_END;
		break;
		
		/* When there's a packet containing data coming, it will pry about the
		 * out-of-order buffer(HpRecvBuf) and see if the recovery is possible.
		 * If the recovery is possible, it will use a while loop to recover the
		 * out-of-order data until the mismatch occurs.*/
		case PROCESS_ACK_WITH_DATA_MATCH_EXPECTED_SEQ:
			lg.logData("[PROCESSTOU MSG] PROCESS_ACK_WITH_DATA_MATCH_EXPECTED_SEQ", 
					TOULOG_ALL);
			rvpl = tp->buf->size(); //get pkt payload size

			// ******************************throughput test
			if (calThroughput()) {
				cout << "[PROCESS_ACK_WITH_DATA_MATCH_EXPECTED_SEQ] goto PKTLOSTTEST;" << endl;
				socktb->log(TOULOG_ALL|TOULOG_PTSRN);
				goto PKTLOSTTEST;
			}
			// ******************************End of throughput test

			/* Try recovery from out-of-order buffer(HpRecvBuf). If the HpRecvBuf is
			 * not empty, plus the seq number is matched. Start recovery */
			if (!socktb->HpRecvBuf.empty()) {
				lg.logData(" >>> Processtou Recovery Start >>> Recovering from seq#: " +
						lg.c2s(socktb->tc.rcv_nxt), TOULOG_ALL|TOULOG_PTSRN);

				sm->setTCBRcv(tp->t.seq + rvpl, sockfd);
				lenofcb = putcircbuf(socktb, sockfd, tp->buf, rvpl);

				/* Recover form the HpRecvBuf: Try to put the out-of-order data back into
				 * circular buffer until the matching is failure. */
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
				/* Handle in-order pkt (no recovery needed) */
				/* in-order pkt: expecting rcv_nxt number */
				sm->setTCBRcv(tp->t.seq + rvpl, sockfd);
				lenofcb = putcircbuf(socktb, sockfd, tp->buf, rvpl);
			}

			state = PROCESS_ACK_DATARECSUCC_SENDBACK_ACK;
		break;
		
		case PROCESS_ACK_WITH_DATA_LESS_EXPECTED_SEQ:
			lg.logData("[PROCESSTOU MSG] PROCESS_ACK_WITH_DATA_LESS_EXPECTED_SEQ", 
					TOULOG_ALL);

			lg.logData(">> Discard it. Packet sequence number: " + lg.c2s(tp->t.seq)
					, TOULOG_ALL);

			/* throughput test */
			if (calThroughput()) ;

			state = PROCESS_ACK_DATARECSUCC_SENDBACK_ACK;
		break;
		
		case PROCESS_ACK_WITH_DATA_MORE_EXPECTED_SEQ:
			lg.logData("[PROCESSTOU MSG] PROCESS_ACK_WITH_DATA_MORE_EXPECTED_SEQ", 
					TOULOG_ALL);

			//througput test
			if (calThroughput()) 
				goto PKTLOSTTEST;
			// End of throughput test

			socktb->pushHpRecvBuf(*tp);
			lg.logData(">> Push into HpRecvBuf. Packet seq #: " + lg.c2s(tp->t.seq) +
					" Size of HpRecvBuf is: " + lg.c2s(socktb->HpRecvBuf.size()) , 
					TOULOG_ALL);

			state = PROCESS_ACK_DATARECSUCC_SENDBACK_ACK;
		break;
		
		case PROCESS_ACK_DATARECSUCC_SENDBACK_ACK:
			lg.logData("[PROCESSTOU MSG] PROCESS_ACK_DATARECSUCC_SENDBACK_ACK", 
					TOULOG_ALL);

			/* Sending ACK back to sender */
			pkgack.clean();
			assignaddr((struct sockaddr_in *)&sockaddrs, AF_INET, socktb->dip, 
					socktb->dport);
			pkgack.putHeaderSeq(socktb->tc.snd_nxt, socktb->tc.rcv_nxt);
			pkgack.t.ack = FLAGON;
			//pkgack.log();
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

	PKTLOSTTEST: ;/* for test */
  socktb->log(TOULOG_ALL);	
	lg.logData("[PROCESSTOU MSG] ****** PROCESSTOU END ******", TOULOG_ALL);

	/* delete received tou pakcet */
	delete tp;

	///////////////////////////////////////////////////////////////////////////////midterm test throughput ///
	fprintf(ofile, "%d\n", socktb->tc.snd_nxt);

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
	socktb->log(TOULOG_PTSRN);
	tp->log(TOULOG_PTSRN);
	return PROCESS_END;
}

/**
 * popsndq:
 * return number of bytes popped.
 * return 0 on nothing to pop.
 * return -1 on an error occurs.
 */
int processTou::popsndq(sockTb *socktb, char *sendbuf, int len) {
	int	bread = 0, dread = 0, end;
	memset(sendbuf, NULL, TOU_MSS);

	/* try to get data form circular buffer */
	if ( 0 < (bread = socktb->CbSendBuf.getAt(sendbuf, len, end))){
		dread = socktb->CbSendBuf.remove(bread);

		if (bread != dread)
			bread = -1;
	}

	return bread;
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


	counter += rvpl;
	cerr << "rvpl: " << rvpl << " count: " << counter << endl;

	/* Try to put data into circular buf */
	if( rvpl <= (lenofcb = (socktb->CbRecvBuf.getAvSize())) ){
		// insert all of buf into cb
		lenofcb = sm->setCbData(str->data(), rvpl, sockfd);
	}else{
		// just insert size of available cb
		lenofcb = sm->setCbData(str->data(), lenofcb, sockfd);
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
			lg.c2s(socktb->CbSendBuf.getTotalElements()), TOULOG_ALL);

  /* TEST: For testing throughputrecord time when data in circular buf has been sent out */
	if (socktb->CbSendBuf.getTotalElements()==0) gettimeofday(&tend, NULL);

	/* get the current window size */
	sndsize = getSendSize(socktb);

	lg.logData("[PROCESSTOU SEND] # of bytes can be sent base on window size: " +
			lg.c2s(sndsize), TOULOG_ALL);

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

		// for midterm demo
		fprintf(ofile_cwnd, "%d, %d\n", cwnd_counter++, (int)(socktb->tc.snd_cwnd/TOU_SMSS));

		// sending
		lg.logData("[PROCESSTOU SEND] >>> [SEND] data length sent: " + lg.c2s(len) +
				", and original sndsize is: " + lg.c2s(sndsize), TOULOG_ALL);
		toupkg->log();

		string sendcontent = toupkg->toString();
		sendto(sockfd, sendcontent.data(), sendcontent.size(), 0, 
				(struct sockaddr *)&sockaddrs, sizeof(sockaddr));

		// add timer
		tm1.add(sockfd, 88, toupkg->t.seq + len, socktb, toupkg->buf);

	}//End of while
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


/**
 * setThroughput(int lossnum, int totalnum):
 * test function
 */
int* processTou::setThroughput(int lossnum, int totalnum) {
	int randnum;
	this->totalnum = totalnum;
	pktiterator = 0;

	pktnum =  new int[totalnum];
	for(int i=0; i<totalnum; i++)
		pktnum[i] = 0;

	srand((unsigned) time(0));
	for(int i=0; i<lossnum; i++){
		randnum = (rand()%totalnum);
		if (pktnum[randnum] == 1) i--;
		pktnum[randnum] = 1;
	}

	return pktnum;
}


/**
 * calThroughput():
 * return true in meeting the jmped pkt
 * retrun false in normal pkt
 */
bool processTou::calThroughput() {
	bool retval = false;
	if (pktiterator <= totalnum) {
		if ((pktnum[pktiterator++] == 1) && (totalnum > 0)) {
			cout<<"pktiterator : "<< (pktiterator-1) << " totalnum: "<<totalnum <<endl;
			retval = true;
		}
	}

	return retval;
}

