/**
 * touSockTable.cpp:
 * Implementation of socket table
 *
 * Copyright 2009 by Columbia University; all rights reserved
 * Jan 10, 2010
 */

#include "tou.h"
using namespace std;

/**
 * sockTb():
 * constructor
 */
sockTb::sockTb() {
	CbSendBuf.setSize(TOU_MAX_CIRCULAR_BUF);
	CbRecvBuf.setSize(TOU_MAX_CIRCULAR_BUF);
	sc = new ssca(&tc);
	tc.t_timeout = TOU_INIT_TIMEO;
	tc.t_timeout_postpone = TOU_INIT_TIMEO_PP;
	tc.t_timeout_seq = 0;
}

/**
 * ckHpRecvBuf(const touPkg &pkt):
 * checking the heap(temporary recv buffer) whether the top pkt is 
 * the same as duppkt previously catched 
 */
bool sockTb::ckHpRecvBuf(const touPkg &pkt){
	if( (pkt.t.seq == duppkt.t.seq) && (pkt.t.ack_seq == duppkt.t.ack_seq) ){
		return true;
	}

	duppkt = pkt; // may cause problem 
	return false;
}

/**
 * setSocketTable(struct sockaddr_in *sockettemp, int sd):
 */
void sockMng::setSocketTable(struct sockaddr_in *sockettemp, int sd) {
	boost::mutex::scoped_lock lock(soctabmutex);
	for(stbiter=SS.begin(); stbiter!=SS.end(); stbiter++) {
		if((*stbiter)->sockd == sd) {
			SS.erase(stbiter);
			s->sockd = sd;
			s->sport = ntohs(sockettemp->sin_port);
			s->sip = inet_ntoa(sockettemp->sin_addr);
			SS.push_back(s);
			break;
		}
	}
}

/**
 * setSocketTableD(struct sockaddr_in *sockettemp, int sd):
 */
void sockMng::setSocketTableD(struct sockaddr_in *sockettemp, int sd)  {
	boost::mutex::scoped_lock lock(soctabmutex);
	s = getSocketTable(sd);
	char* ip1 = inet_ntoa(sockettemp->sin_addr);
	u_short dport = ntohs(sockettemp->sin_port);
	std::string ip(ip1);
	s->dip = ip;
	s->dport = dport;
}

/**
 * setSocketTable(int sd):
 * called during tou_socket() 
 */
void sockMng::setSocketTable(int sd) {
	boost::mutex::scoped_lock lock(soctabmutex);
	s = new sockTb;
	s->sockd = sd;
	SS.push_back(s);
}

/**
 * delSocketTable(int sd):
 * called during tou_close() 
 */
void sockMng::delSocketTable(int sd) {
	boost::mutex::scoped_lock lock(soctabmutex);
	for(stbiter=SS.begin(); stbiter!=SS.end(); stbiter++) {
		if((*stbiter)->sockd == sd) {
			SS.erase(stbiter);
			break;
		}
	}
}

/**
 * getSocketTable(int sockfd):
 * return socktable ptr if matchs with sockfd 
 * return NULL if failure 
 */
struct sockTb* sockMng::getSocketTable(int sockfd) {
	for(stbiter=SS.begin(); stbiter!=SS.end(); stbiter++){
		if((*stbiter)->sockd == sockfd)
			return (*stbiter);
	}
	return NULL;
}

/**
 * setSocketState(int state, int sd):
 */
void sockMng::setSocketState(int state, int sd) {
	boost::mutex::scoped_lock lock(soctabmutex);      
	s = getSocketTable(sd);
	s->sockstate = state;
} 

/**
 * setTCB(u_long seq, u_long seq1, int sd):
 */
void sockMng::setTCB(u_long seq, u_long seq1, int sd) {
	boost::mutex::scoped_lock lock(soctabmutex);      
	s = getSocketTable(sd);
	s->tc.snd_nxt = seq;
	s->tc.snd_una = seq1;
}

/**
 * setTCBCwnd(u_long winsize, int sd):
 */
void sockMng::setTCBCwnd(u_long winsize, int sd) {
	boost::mutex::scoped_lock lock(soctabmutex);      
	s = getSocketTable(sd);
	s->tc.snd_cwnd = winsize;
}

/**
 * setTCBAwnd(u_long winsize, int sd):
 */
void sockMng::setTCBAwnd(u_long winsize, int sd) {
	boost::mutex::scoped_lock lock(soctabmutex);      
	s = getSocketTable(sd);
	s->tc.snd_awnd = winsize;
}

/**
 * setTCBRcv(unsigned long seq , int sd):
 */
void sockMng::setTCBRcv(unsigned long seq , int sd)  {
	boost::mutex::scoped_lock lock(soctabmutex);      
	s = getSocketTable(sd);
	s->tc.rcv_nxt = seq;
}

/**
 * setTCBState(int state , int sd):
 */
void sockMng::setTCBState(int state , int sd)  {
	boost::mutex::scoped_lock lock(soctabmutex);      
	s = getSocketTable(sd);
	s->tc.cc_state = state;
}

/**
 * setCbData(char *buf,int len,int sd):
 */
int sockMng::setCbData(const char *buf, int len, int sd) { 
	boost::mutex::scoped_lock lock(soctabmutex);    
	int len1 = len;  
	s = getSocketTable(sd);
	len1 = s->CbRecvBuf.insert(buf,len);
	return len1;
}

/**
 * pushHpRecvBuf(touPkg pkt)
 */
void sockTb::pushHpRecvBuf(const touPkg &pkt_){
	boost::mutex::scoped_lock lock(stmutex);
	touPkg tp(pkt_);
	HpRecvBuf.push(tp);
}

/**
 * ck_dupack();
 * checking whether the number of incoming acks is above three
 * return true on duplicate acks above or equal to 3.
 * return false on otherwise.
 */
bool sockTb::ck_dupack_3() {
	return (tc.dupackcount == 3)? true: false;
}

/**
 * printall()
 * for testing, and it will log the msg into related files
 */
/*
void sockTb::printall() {
	string ccstate;
	lg.logData("*** SOCKET TABLE RESULT ***", TOULOG_ALL|TOULOG_SOCKTB);
  lg.logData("* sockd:"+lg.c2s(sockd)+" sport:"+lg.c2s(sport)+" dport:"+lg.c2s(dport)
		, TOULOG_ALL|TOULOG_SOCKTB);

	if (tc.cc_state == 1) ccstate = "Slow Start";
	else if (tc.cc_state == 2) ccstate = "Congestion Avoidance";
	else if (tc.cc_state == 3) ccstate = "Fast Retransmit";
	else ccstate = "CC State Error";

	lg.logData("* cc_state: "+ccstate, TOULOG_ALL|TOULOG_SOCKTB);
  lg.logData("* snd_una : "+lg.c2s(tc.snd_una)+" snd_nxt : "+lg.c2s(tc.snd_nxt)+
			" rcv_nxt : "+lg.c2s(tc.rcv_nxt), TOULOG_ALL|TOULOG_SOCKTB);
	lg.logData("* snd_cwnd: "+lg.c2s(tc.snd_cwnd)+" snd_awnd: "+lg.c2s(tc.snd_awnd), 
			TOULOG_ALL|TOULOG_SOCKTB);
	lg.logData("* snd_ssthresh:"+lg.c2s(tc.snd_ssthresh),
			TOULOG_ALL|TOULOG_SOCKTB);
}
*/

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
  lg.logData("* sockd:"+lg.c2s(sockd)+" sport:"+lg.c2s(sport)+" dport:"+lg.c2s(dport)
		, def_logflag);

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

}
