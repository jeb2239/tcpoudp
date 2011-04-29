/******************************************************************************
 * processtou.cpp
 * Processtou is a class that responsible for handling most of the I/O
 * operations. It's  sending/receiving via circular buffers.
 * ***************************************************************************/

#include "timer.h"
#include "timestamp.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>

using namespace std;


/**
 * pendPacketLen
 * Return the size of next pending datagram in bytes. 
 * Return zero on no datagrams are pending.
 */
static inline int pendPacketLen(int sockfd) {
	int recvb;
	if (ioctl(sockfd, SIOCINQ, &recvb) < 0) {
		perror("Error in SIOCINQ");
		exit(1);
	}
	return recvb;
}


/**
 * calculate number of xmit for congestion avoidance
 */
int processTou::cal_num_caxmit(int snd_sz) {
	int xmit_cnt = snd_sz/TOU_SMSS;

	if (xmit_cnt%2 == 1) {
		if (snd_sz%TOU_SMSS != 0)
			xmit_cnt++;
		else 
			xmit_cnt--;
	}
	
	return xmit_cnt;
}
	
/**
 * proc_ack:
 * Retrun 1: tp's ack seq bigger than unacked seq. 
 * Return 0: tp's ack seq equal to unacked seq.
 * Return -1: tp's ack seq smaller than unacked seq.
 */
int processTou::proc_ack(int sockfd, sockTb *stb, touPkg *tp) {

	boost::mutex::scoped_lock lock(stb->stb_mtx);

	if (tp->t.ack_seq > stb->tc.snd_una) {
		socktb->sc->addwnd(); //will get sscamutex
		socktb->tc.snd_una = tp->t.ack_seq; // update snd_una

		return ACK_BIGTHAN_UNA;
	}else if (tp->t.ack_seq == stb->tc.snd_una) {

		return ACK_EQUALTO_UNA;
	}else {
		socktb->sc->addwnd(); //will get sscamutex

		return ACK_SMLTHAN_UNA;
	}
}

/**
 * Receiving/sending of packets through circular buffer. Using a non-blocking
 * recvfrom functiothat. Return positive on getting a valid data; Zero on fail
 * to get run_t_mtx lock, and negtive number on no incomming data available.
 */
int processTou::run(int sockfd) {

	boost::try_mutex::scoped_lock lock(run_t_mtx);
	if (!lock.owns_lock()) //didn't grape the lock
		return 0;

	touPkg	*tp; //packet for receiving data
	socktb = sm->getSocketTable(sockfd);
	int rv = 0, state = socktb->runstate;

	while(state != PROCESS_END){ //FSM
		switch(state){
			 /*
			  * Get a new packet.
			  */
		  case PROCESS_GET_PKT:
			  if (0 >= (rv = run_recvfrom(sockfd, &tp)) )
				  return (rv = -1);

			  lg.logData("[PROCESSTOU MSG] ****** PROCESSTOU START ******", TOULOG_ALL);
			  state = processGetPktState(socktb, tp);
			  break;

			  /*
			   * Clean(free) previous states and get a new packet if possible.
			   */
		  case PROCESS_CLEAR_GET_PKT:
			  delete tp;
			  if (0 >= (rv = run_recvfrom(sockfd, &tp)) )
				  return (rv = -1);

			  state = processGetPktState(socktb, tp);
			  break;

			  /*
			   * Check if the connection is in established state.
			   */
		  case PROCESS_SYN:
			  //if(!(socktb->sockstate == TOUS_ESTABLISHED))
			  sm->setSocketTable(sockfd, NULL, &sockaddrs, SOCKTB_SET_DST_TUPLE);
			  sm->setSocketState(TOUS_SYN_RECEIVED,sockfd);
			  /* Connection Initailization */
			  sm->setTCBRcv(tp->getSeq()+1, sockfd);

			  state = PROCESS_END;
			  break;

			  /*
			   * Check for FIN flag (Close).
			   * Connection Control: Chinmay add here.
			   */
		  case PROCESS_FIN: 
			  sm->setSocketState(TOUS_CLOSE_WAIT,sockfd);

			  state = PROCESS_END;
			  break;

			  /* 
			   * Receive an ACK from peer.
			   */
		  case PROCESS_ACK_WITHOUT_DATA:
			  lg.logData("[PROCESSTOU ACK_WITHOUT_DATA] GET_ACK: " +
							   lg.c2s(tp->t.ack_seq), TOULOG_ALL);

			  /* proc_ack updates and returns corresponding results */
			  switch (proc_ack(sockfd, socktb, tp)) {
				case ACK_BIGTHAN_UNA:
					/* after updating, clear useless timer as well.
					 * note that tc.snd_una has been updated to tp->ack_seq */
					tmng.delete_timer(sockfd,TIMER_ID_SNDPKT,tp->t.ack_seq);
				case ACK_SMLTHAN_UNA:
					break;

				case ACK_EQUALTO_UNA:
					lg.logData("[PROCESSTOU ACK_WITHOUT_DATA] DUP_ACK: " + 
							   lg.c2s(tp->t.ack_seq) + " Duplicate ACKs count #: " + 
							   lg.c2s(socktb->tc.dupackcount), TOULOG_ALL);

					/* duplicate ack (already receive one) */
					socktb->sc->setdwnd(tp->t.ack_seq);

					/* if dup acks are exceed number of three. init "fast retransmit" 
					 * algorithm - rexmitting the data pkt immediately. */
					if(socktb->ck_num_dupack())
						tmng.rexmit_for_dup_ack(sockfd, TIMER_ID_SNDPKT, tp->t.ack_seq);

					break;
				default:
					break;
			  }

			  lg.logData("[PROCESSTOU ACK_WITHOUT_DATA] Try to send data left in circ buff",
						 TOULOG_ALL);

			  /* window size has updated, try to send data left in circ buf */
			  send(sockfd);

			  state = PROCESS_END;
			  break;

			  /* 
			   * recover loss pkts from minhp_recv heap.
			   */
		  case PROCESS_PKT_RECOVERY:
			  if (0 == proc_pkt_rec(socktb, sockfd)) {
				  /* if recovery successfully, set init runstate to "GET_PKT" */
				  socktb->runstate = PROCESS_GET_PKT;
				  state = PROCESS_ACK_DATARECSUCC_SENDBACK_ACK;

			  }else {
				  socktb->runstate = PROCESS_PKT_RECOVERY;
				  state = PROCESS_ACK_DATARECSUCC_NO_SENDBACK_ACK;

			  }
			  break;

			  /* 
			   * A packet containing data.Pry about the out-of-order buffer(HpRecvBuf) and 
			   * see whether recovery is possible. If possible, it will use a loop to recover
			   * data possible until the mismatch occurs.
			   */
		  case PROCESS_ACK_WITH_DATA_MATCH_EXPECTED_SEQ:
			  /*
			  lg.logData("[PROCESSTOU MSG] PROCESS_ACK_WITH_DATA_SEQ_MATCH", 
						 TOULOG_ALL);
			  */

			  sm->setTCBRcv(tp->t.seq + tp->buf->size(), sockfd);
			  putcircbuf(socktb, sockfd, tp->buf, tp->buf->size());

			  state = PROCESS_PKT_RECOVERY; //try if recovery is possible
			  break;

		  case PROCESS_ACK_WITH_DATA_LESS_EXPECTED_SEQ:
			  lg.logData("[PROCESSTOU MSG] PROCESS_ACK_WITH_DATA_SEQ_LESS: PKT SEQ#(" + 
						 lg.c2s(tp->t.seq) + ")", TOULOG_ALL|TOULOG_PTSRN);
			  lg.logData("[PROCESSTOU MSG] stb->tc.rcv_nxt: " + lg.c2s(socktb->tc.rcv_nxt) +
						 " )", TOULOG_ALL|TOULOG_PTSRN);

			  if (pendPacketLen(sockfd) > 0 && 
				  socktb->cb_recv.getAvSize() >= RECVBUFSIZE) {
				  state = PROCESS_CLEAR_GET_PKT;	
			  }else {
				  state = PROCESS_END;
			  }
			  break;

		  case PROCESS_ACK_WITH_DATA_MORE_EXPECTED_SEQ:
			  lg.logData("[PROCESSTOU MSG] PROCESS_ACK_WITH_DATA_SEQ_MORE: PKT SEQ#(" + 
						 lg.c2s(tp->t.seq) + ")", TOULOG_ALL|TOULOG_PTSRN);

			  socktb->pushHpRecvBuf(*tp);
			  /* Making immediate xmit of ACK, set the state to 
			   * TOUS_DELACK_IMMED_UPDATE__XMIT */
			  set_ack_state(socktb, TOUS_DELACK_IMMED_UPDATE_XMIT);

			  lg.logData(" >>> Push into HpRecvBuf. PKT SEQ#(" + lg.c2s(tp->t.seq) +
						 ") Size of HpRecvBuf is: " + lg.c2s(socktb->minhp_recv.size()) , 
						 TOULOG_ALL);

			  state = PROCESS_ACK_DATARECSUCC_SENDBACK_ACK;
			  break;

		  case PROCESS_ACK_DATARECSUCC_SENDBACK_ACK:
			  lg.logData("[PROCESSTOU MSG] PROCESS_ACK_DATARECSUCC_SENDBACK_ACK", 
						 TOULOG_ALL);
			  /*
			  lg.logData("[PROCESSTOU MSG] pending len(" + lg.c2s(pendPacketLen(sockfd)) +
						 ") available circBuf (" + lg.c2s(socktb->cb_recv.getAvSize()) + " )", 
						 TOULOG_ALL);
						 */

			  proc_delack(socktb);

		  case PROCESS_ACK_DATARECSUCC_NO_SENDBACK_ACK:
			  if (pendPacketLen(sockfd) > 0 && 
				  socktb->cb_recv.getAvSize() >= RECVBUFSIZE &&
				  socktb->runstate == PROCESS_GET_PKT) {
				  state = PROCESS_CLEAR_GET_PKT;	

			  }else {
				  state = PROCESS_END;	
			  }
			  break;

		  default:
			  cerr << "Error in PROCESS SWITCH STATE" << endl;
			  state = PROCESS_END;
			  break;

		}/* END OF SWITCH */
	}/* END OF WHILE(STATE) LOOP */

	socktb->log(TOULOG_ALL);
	lg.logData("[PROCESSTOU MSG] ****** PROCESSTOU END ******", TOULOG_ALL);

	/* delete received tou pakcet */
	delete tp;

	return rv;
}/* END of processtou */


/**
 * processGetPktState
 * return state by analyzing newly received packet
 * @result return the state of process state
 */
int processTou::processGetPktState(sockTb *stb, touPkg *tp){

	boost::mutex::scoped_lock lock_a(stb->stb_mtx);
	boost::mutex::scoped_lock lock_k(stb->stb_tc_ack_mtx);

	if(tp->t.syn == FLAGON) 
		return PROCESS_SYN;

	else if(tp->t.fin == FLAGON) 
		return PROCESS_FIN;

	if(tp->t.ack == FLAGON &&
	   stb->sockstate == TOUS_ESTABLISHED && 
	   tp->buf->size() == 0 &&
	   stb->tc.snd_nxt >= tp->t.ack_seq) {
		return PROCESS_ACK_WITHOUT_DATA;
	}

	/* receiving data */
	if(tp->t.ack == FLAGON && 0 < tp->buf->size() && 
	   tp->t.seq == stb->tc.rcv_nxt &&
	   stb->sockstate == TOUS_ESTABLISHED){
		return PROCESS_ACK_WITH_DATA_MATCH_EXPECTED_SEQ;

	}else if(tp->t.ack == FLAGON && 0 < tp->buf->size() &&
			tp->t.seq < stb->tc.rcv_nxt &&
			stb->sockstate == TOUS_ESTABLISHED){
		return PROCESS_ACK_WITH_DATA_LESS_EXPECTED_SEQ;

	}else if(tp->t.ack == FLAGON && 0 < tp->buf->size() &&
			tp->t.seq > stb->tc.rcv_nxt &&
			stb->sockstate == TOUS_ESTABLISHED){
		return PROCESS_ACK_WITH_DATA_MORE_EXPECTED_SEQ;
	}

	cerr << "[PROCESSTOU] state error : PROCESS_END \n";
	stb->log(TOULOG_PTSRN);
	tp->log(TOULOG_PTSRN);
	exit(1);
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
	if ( 0 < (bread = socktb->cb_send.getAt(sendbuf, len, end))){
		dread = socktb->cb_send.remove(bread);

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

/** 
 * push data into circular buf 
 * Return number of bytes been saved in circbuf. 
 * 0 if have no enough space. 
 */
int processTou::putcircbuf(sockTb *socktb, int sockfd, string *str, int len){
	int lenofcb;

	/* Try to put data into circular buf */
	/* 
	if( rvpl <= (lenofcb = (socktb->cb_recv.getAvSize())) ){
		// insert all of buf into cb
		lenofcb = sm->setCbData(str->data(), rvpl, sockfd);
	}else{
		// just insert size of available cb
		lenofcb = sm->setCbData(str->data(), lenofcb, sockfd);
	}
	*/
	lenofcb = sm->setCbData(str->data(), len, sockfd);
	if (lenofcb == 0) {
		cerr << " ### ### ### *** ### ### ### putcircbuf has no enough space: lenofcb/len " << lenofcb << "/"
			<< len << endl;
	}

	return lenofcb;
}

void processTou::set_ack_state(sockTb *stb, int state) {

	boost::mutex::scoped_lock lock(stb->stb_tc_ack_mtx);
	stb->tc.t_delack_state = state;
}


/**
 * for data packet
 * note: the pointer "pkt" will be modified.
 */
string processTou::gen_pkt(sockTb *stb, touPkg **pkt, char *buf, int len) {

	boost::mutex::scoped_lock lock(stb->stb_mtx);

	*pkt = new touPkg(buf, len);
	assert(*pkt != NULL);
	(*pkt)->putHeaderSeq(stb->tc.snd_nxt, stb->tc.rcv_nxt);
	(*pkt)->t.ack = FLAGON;
	(*pkt)->log(TOULOG_PKT|TOULOG_PKTSENT);

	return (*pkt)->toString();
}

/** 
 * for ack packet
 */
string processTou::gen_pkt(sockTb *stb, struct sockaddr_in *sa) {

	boost::mutex::scoped_lock lock(stb->stb_mtx);

	touPkg tou_pkt(0);
	tou_pkt.clean();
	assignaddr(sa,AF_INET,stb->dip,stb->dport);
	tou_pkt.putHeaderSeq(stb->tc.snd_nxt, stb->tc.rcv_nxt);
	tou_pkt.t.ack = FLAGON;

	return tou_pkt.toString();
}

/* 
 * send
 * Sending pkt to end host. It will try to send with max payload. If the available
 * wnd size is detected, send will abort, and waits for next change to xmit data.
 */
void processTou::send(int sockfd) {

	boost::mutex::scoped_lock lock(send_mtx);

	int		len;
	sockaddr_in	sockaddrs;
	socktb = sm->getSocketTable(sockfd);
	int sndsize = getSendSize(socktb), xmit_cnt;//get cur wnd size

	/* set up recv's info */
	assignaddr(&sockaddrs, AF_INET, socktb->dip, socktb->dport);

	lg.logData("[PROCESSTOU SEND] circ buff("+lg.c2s(socktb->cb_send.getTotalElements())+
			   ") # of bytes can be sent base on window size: "+lg.c2s(sndsize),TOULOG_ALL);

	xmit_cnt = cal_num_caxmit(sndsize);

	while (0 < socktb->cb_send.getTotalElements()) {
		sndsize = getSendSize(socktb);

		if (socktb->tc.cc_state == TOU_CC_CA) {
			if (xmit_cnt-- <= 0)
				break;
		}else {
			if (MIN_SEND_SIZE > sndsize)
				break;
		}

		touPkg		*pkt_p;
		if (sndsize >= TOU_SMSS) { //snd how many bytes
			len = popsndq(socktb, run_buf, TOU_SMSS);
		}else {
			len = popsndq(socktb, run_buf, sndsize);
		}

		string strpkt = gen_pkt(socktb, &pkt_p, run_buf, len);
		socktb->tc.snd_nxt += len; //snd_nxt move forward

		// sending
		lg.logData((int)(socktb->tc.snd_cwnd/TOU_SMSS), socktb->tc.snd_ssthresh, 
				   (int)(socktb->tc.snd_awnd/TOU_SMSS), socktb->tc.snd_nxt, 
				   socktb->tc.cc_state, TOULOG_PROBE);
		lg.logData("[PROCESSTOU SEND] >>> [SEND] data length sent: " + lg.c2s(len) +
				", and original sndsize is: " + lg.c2s(sndsize), TOULOG_ALL);

		sendto(sockfd, strpkt.data(), strpkt.size(), 0, 
			   (struct sockaddr *)&sockaddrs, sizeof(sockaddr));

		// add timer
		tmng.add(sockfd, TIMER_ID_SNDPKT, pkt_p->t.seq + len, socktb, pkt_p->buf);
		delete pkt_p;
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
 * processTou::proc_delack(sockTb *stb):
 * process the delayed ack. 
 * TOUS_DELACK_XMIT: 
 * TOUS_DELACK_QUE:
 */
void processTou::proc_delack(sockTb *stb) {
	
	/* lock DELACK states */
	boost::mutex::scoped_lock a_lock(ack_mtx);
	boost::mutex::scoped_lock lock(stb->stb_tc_ack_mtx);

	if (stb->tc.t_delack_state == TOUS_DELACK_IMMED_UPDATE_XMIT ||
		stb->tc.t_delack_state == TOUS_DELACK_QUEXMIT ||
		stb->tc.t_delack_state == TOUS_DELACK_EXPXMIT ||
		stb->tc.t_delack_state == TOUS_DELACK_XMIT) {
		lg.logData("[PROCESSTOU] TOUS_DELACK_XMIT: xmit an ack("+
				lg.c2s(stb->tc.rcv_nxt)+")", TOULOG_ALL);

		struct sockaddr_in sockaddrs;
		string strpkt = gen_pkt(stb, &sockaddrs);
		sendto(stb->sockd, strpkt.data(), strpkt.size(), 0, 
			   (struct sockaddr *)&sockaddrs, sizeof(sockaddrs));
		tmng.update_ack(stb->sockd, TIMER_ID_DELACK, TOUS_DELACK_XMIT, stb);
		stb->tc.t_delack_state = TOUS_DELACK_QUE;

	}else if (stb->tc.t_delack_state == TOUS_DELACK_QUE) {
		lg.logData("[PROCESSTOU] TOUS_DELACK_QUE: queue ack("+
				lg.c2s(stb->tc.rcv_nxt)+")", TOULOG_ALL);

		tmng.update_ack(stb->sockd, TIMER_ID_DELACK, TOUS_DELACK_QUE, stb);
		stb->tc.t_delack_state = TOUS_DELACK_XMIT;

	}else {
		cerr << "Error in processTou::proc_delack()" << endl;
		exit(1);

	}

}


/* 
 * out-of-order buffer(minhp_recv) not empty implies recovery is possible.
 * Recover form the HpRecvBuf: Try to put the out-of-order data back into
 * circular buffer until the matching is fail. 
 * Return 0 on success recovery, else -1
 */
int processTou::proc_pkt_rec(sockTb *stb, int sockfd) {

	int retval = 0;
	boost::mutex::scoped_lock lock(socktb->stb_minhp_recv_mtx);

	if (stb->minhp_recv.empty())
		return retval;

	lg.logData(" >>> Processtou Recovery Start: Recovering from seq#: " +
			   lg.c2s(socktb->tc.rcv_nxt)+ " cur minhp_recv(" + 
			   lg.c2s(socktb->minhp_recv.top().t.seq)+")", TOULOG_ALL|TOULOG_PTSRN);

	retval = do_proc_pkt_rec(socktb, sockfd);

	/* Making immediate xmit of ACK, set the state to 
	 * TOUS_DELACK_IMMED_UPDATE_XMIT. The reason we don't
	 * use TOUS_DELACK_XMIT is because flagging ack_state
	 * to avoid TIMER time-out it!*/
	set_ack_state(socktb, TOUS_DELACK_IMMED_UPDATE_XMIT);
	return retval;

}

/* 
 * do out-of-order buffer(minhp_recv) not empty implies recovery is possible.
 * Recover form the HpRecvBuf: Try to put the out-of-order data back into
 * circular buffer until the matching is fail. 
 * note: stb_minhp_recv_mtx must be graped before calling.
 * 0 if success
 * -1 if have no enough space in circbuf
 */
int processTou::do_proc_pkt_rec(sockTb *stb, int sockfd) {
	
	int datasz;		//byte stored in HpRecvBuf, payload size

	while (!stb->minhp_recv.empty()) {
		assert(stb->minhp_recv.top().t.seq >= stb->tc.rcv_nxt);

		if (stb->minhp_recv.top().t.seq == stb->tc.rcv_nxt) {
			lg.logData(" >>> Processtou Recovery >>> seq#: " +
					   lg.c2s(stb->minhp_recv.top().t.seq) + " elm#: " +
					   lg.c2s(stb->minhp_recv.size()), TOULOG_ALL|TOULOG_PTSRN);

			datasz = stb->minhp_recv.top().buf->size();
			string str(stb->minhp_recv.top().buf->c_str(), datasz);

			if (putcircbuf(stb, sockfd, &str, datasz) > 0) {
				sm->setTCBRcv(stb->minhp_recv.top().t.seq + datasz, sockfd);
			}else {
				lg.logData(" >>> Processtou Recovery BREAK ON >>> NO_MEMORY! >>> seq#: " +
						   lg.c2s(stb->minhp_recv.top().t.seq) + " elm#: " +
						   lg.c2s(stb->minhp_recv.size()), TOULOG_ALL|TOULOG_PTSRN);

				/* ==0 circbuf doesn't have enough space */
				return -1;
			}

		} else if (stb->minhp_recv.top().t.seq > stb->tc.rcv_nxt) {
			lg.logData(" >>> Processtou Recovery BREAK >>> >>> seq#: " +
					   lg.c2s(stb->minhp_recv.top().t.seq) + " elm#: " +
					   lg.c2s(stb->minhp_recv.size()), TOULOG_ALL|TOULOG_PTSRN);

			break;
		}

		/* top pkt in queue(buf) is back in file now, then pop it */
		stb->minhp_recv.pop();
	}/* End of while */
	return 0;

}

int processTou::run_recvfrom(int sockfd, touPkg** tpp) {

	int rv = -1;
	socklen_t l = sizeof(sockaddrs);
	/*
	   Non-blocking recvfrom which will return on receiving nothing. If it does
	   receive packet, packet will be converted from char* to "touPkg" 
	 */
	memset(recv_cnt, NULL, RECVBUFSIZE);
	rv = recvfrom(sockfd, recv_cnt, RECVBUFSIZE, 0, (struct sockaddr*)&sockaddrs, &l); 

	if (rv <= 0) {
		if (rv < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
			/*
			lg.logData("[PROCESSTOU MSG] ****** PROCESSTOU END ****** rv("+lg.c2s(rv)+
					   ") recvfrom: no data available", TOULOG_ALL);
			*/
		}

		return rv;
	}
	*tpp = new touPkg(*(new string(recv_cnt, rv)));
	(*tpp)->log(TOULOG_PKT | TOULOG_PKTRECV);

	return rv;
}
