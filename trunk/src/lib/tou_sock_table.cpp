/**
 * tou_sock_table.cpp:
 * Implementation of socket table and socket table management
 *
 * Copyright 2009 by Columbia University; all rights reserved
 * Jan 10, 2010
 */

#include "tou.h"
#include <assert.h>
using namespace std;

vector<sockTb*> stbq; //for storing socket tb info
boost::mutex stbq_mutex; //socket table queue (stbq) mutex, a mutex for stbq

/**
 * sockTb():
 * constructor
 */
sockTb::sockTb() {

	runstate = PROCESS_GET_PKT;
	cb_send.setSize(TOU_MAX_CIRCULAR_BUF);
	cb_recv.setSize(TOU_MAX_CIRCULAR_BUF);
	sc = new ssca(&tc);

	tc.t_delack_state = TOUS_DELACK_XMIT;
	tc.t_rto = TOU_INIT_TIMEO;
    tc.t_rto_beta = TOU_INIT_RTO_BETA;
    tc.t_rtt = TOU_INIT_TIMEO;
    tc.t_rtt_alpha = TOU_INIT_ALPHA;
	tc.dupackcount = 0;

	tc.iss = 0;                 //initial send seq #
	tc.irs = 0;                 //initial rcv seq #
}


/**
 * getSocketTable(int sockfd):
 * return socktable ptr if matchs with sockfd 
 * return NULL if failure 
 */
sockTb* sockMng::getSocketTable(int sockfd) {

	//lock the stbq
	boost::mutex::scoped_lock lock(stbq_mutex);

	for(stbiter=stbq.begin(); stbiter!=stbq.end(); stbiter++){
		if((*stbiter)->sockd == sockfd)
			return (*stbiter);
	}
	return NULL;
}


/**
 * addSocketTable(int sd):
 * called during tou_socket() 
 */
void sockMng::addSocketTable(int sockfd) {

	boost::mutex::scoped_lock lock(stbq_mutex);

	stbp = new sockTb;
	stbp->sockd = sockfd;
	stbq.push_back(stbp);
}

/**
 * delSocketTable(int sd):
 * called during tou_close() 
 */
void sockMng::delSocketTable(int sd) {

	boost::mutex::scoped_lock lock(stbq_mutex);

	for(stbiter=stbq.begin(); stbiter!=stbq.end(); stbiter++) {
		if((*stbiter)->sockd == sd) {
			stbq.erase(stbiter);
			break;
		}
	}//End of for
}

/**
 * setSocketTable
 * set the four tuples
 * SOCKTB_SET_SRC_TUPLE: for source ip and port
 * SOCKTB_SET_DST_TUPLE: for destination ip and port
 * SOCKTB_SET_SRCDST_TUPLE: for both src/dst ip and port
 */
void sockMng::setSocketTable(int sockfd, struct sockaddr_in *srcsock,
							struct sockaddr_in *dstsock, int flag) {

	sockTb	*stbp = getSocketTable(sockfd);
	assert(stbp != NULL);

	boost::mutex::scoped_lock lock(stbp->stb_mtx);
	
	switch(flag) {
		case SOCKTB_SET_SRC_TUPLE:

			assert(srcsock != NULL);

			stbp->sport = ntohs(srcsock->sin_port);
			stbp->sip = inet_ntoa(srcsock->sin_addr);
			break;
			
		case SOCKTB_SET_DST_TUPLE:

			assert(dstsock != NULL);

			stbp->dport = ntohs(dstsock->sin_port);
			stbp->dip = inet_ntoa(dstsock->sin_addr);
			break;

		case SOCKTB_SET_SRCDST_TUPLE:

			assert(srcsock != NULL);
			assert(dstsock != NULL);

			stbp->sport = ntohs(srcsock->sin_port);
			stbp->sip = inet_ntoa(srcsock->sin_addr);
			stbp->dport = ntohs(dstsock->sin_port);
			stbp->dip = inet_ntoa(dstsock->sin_addr);
			break;
		default:
			std::cerr << "[Error] sockMng::setSocketTable\n";
			break;
	}//End of switch

}

/**
 * setSocketState(int state, int sd):
 */
void sockMng::setSocketState(int state, int sockfd) {

	sockTb *stbp = getSocketTable(sockfd);
	assert(stbp != NULL);

	boost::mutex::scoped_lock lock(stbp->stb_mtx);

	stbp->sockstate = state;
}


/**
 * setTCB(u_long seq, u_long seq1, int sd):
 */
void sockMng::setTCB(u_long snd_nxt, u_long snd_una, int sockfd) {

	sockTb *stbp = getSocketTable(sockfd);

	boost::mutex::scoped_lock lock(stbp->stb_mtx);

	stbp->tc.snd_nxt = snd_nxt;
	stbp->tc.snd_una = snd_una;
}

/**
 * setTCBRcv(unsigned long seq , int sd):
 */
void sockMng::setTCBRcv(unsigned long seq , int sockfd)  {

	sockTb *stb = getSocketTable(sockfd);

	boost::mutex::scoped_lock lock(stb->stb_mtx);

	stb->tc.rcv_nxt = seq;
}


/**
 * setTCBCwnd(u_long winsize, int sd):
 */
void sockMng::setTCBCwnd(u_long winsize, int sockfd) {

	sockTb *stbp = getSocketTable(sockfd);

	boost::mutex::scoped_lock lock(stbp->stb_mtx);

	stbp->tc.snd_cwnd = winsize;
}

/**
 * setTCBAwnd(u_long winsize, int sd):
 */
void sockMng::setTCBAwnd(u_long winsize, int sockfd) {

	sockTb *stbp = getSocketTable(sockfd);

	boost::mutex::scoped_lock lock(stbp->stb_mtx);

	stbp->tc.snd_awnd = winsize;
}

/**
 * setTCBState(int state , int sd):
 */
void sockMng::setTCBState(int state , int sockfd)  {

	sockTb *stbp = getSocketTable(sockfd);

	boost::mutex::scoped_lock lock(stbp->stb_mtx);

	stbp->tc.cc_state = state;
}

/**
 * setCbData(char *buf,int len,int sd):
 */
int sockMng::setCbData(const char *buf, int len, int sockfd) { 

	sockTb *stbp = getSocketTable(sockfd);

	//lock the circular buffer of recv
	boost::mutex::scoped_lock lock(stbp->stb_cb_recv_mtx);

	int len1 = stbp->cb_recv.insert(buf,len);
	return len1;
}


unsigned long sockMng::getCirSndBuf() {

	return stbp->cb_send.getAvSize();
}


/**
 * pushHpRecvBuf(touPkg pkt)
 */
void sockTb::pushHpRecvBuf(const touPkg &pkt_){

	boost::mutex::scoped_lock lock(stb_minhp_recv_mtx);

	touPkg pkt(pkt_);
	minhp_recv.push(pkt);
}

/**
 * ckHpRecvBuf(const touPkg &pkt):
 * checking the heap(temporary recv buffer) whether the top pkt is 
 * the same as duppkt previously catched 
 * used by processTou::proc_pkt_rec
 */
/*
bool sockTb::ckHpRecvBuf(const touPkg &pkt){

	if( (pkt.t.seq == duppkt.t.seq) && (pkt.t.ack_seq == duppkt.t.ack_seq) ){
		return true;
	}

	duppkt = pkt; // may cause problem 
	return false;
}
*/

/**
 * ck_num_dupack();
 * checking whether the number of incoming acks is above three
 * return true on duplicate acks above or equal to 3.
 * return false on otherwise.
 */
bool sockTb::ck_num_dupack() {

	boost::mutex::scoped_lock lock(stb_mtx);

	return (tc.dupackcount == TCPREXMTTHRESH)? true: false;
}

/**
 * log():
 * Default logging mechanism. It'll log to TOULOG_SOCKTB only
 */
void sockTb::log() {
	log(0);
}

/**
 * log(logflag):
 * Additionaly specify the file that been logged.
 * ex: log(TOULOG_ALL)
 */
void sockTb::log(unsigned short logflag) {
	string ccstate;
	unsigned short def_logflag = TOULOG_SOCKTB;
	def_logflag = (def_logflag | logflag);

	lg.logData("*** SOCKET TABLE RESULT ***", def_logflag);
	lg.logData("* sockd:"+lg.c2s(sockd)+" sport:"+lg.c2s(sport)+" dport:" +
			lg.c2s(dport), def_logflag);

	if (tc.cc_state == 1) ccstate = "Slow Start";
	else if (tc.cc_state == 2) ccstate = "Congestion Avoidance";
	else if (tc.cc_state == 3) ccstate = "Fast Retransmit";
	else ccstate = "CC State Error";

	lg.logData("* cc_state: "+ccstate, def_logflag);
	lg.logData("* snd_una : "+lg.c2s(tc.snd_una)+" snd_nxt : "+lg.c2s(tc.snd_nxt)+
			" rcv_nxt : "+lg.c2s(tc.rcv_nxt), def_logflag);
	lg.logData("* snd_cwnd: "+lg.c2s(tc.snd_cwnd)+" snd_awnd: "+lg.c2s(tc.snd_awnd), 
			def_logflag);
	lg.logData("* snd_ssthresh:"+lg.c2s(tc.snd_ssthresh),
			def_logflag);
	lg.logData("* tc.dupackcount: "+lg.c2s(tc.dupackcount), 
			def_logflag);
	lg.logData("* tc.rtt: "+lg.c2s(tc.t_rtt)+"  tc.rto: " +lg.c2s(tc.t_rto), 
			def_logflag);

}
