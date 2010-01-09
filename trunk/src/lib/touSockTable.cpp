//#include "touSockTable.h"
#include "tou.h"

/******************************************************************
	 * sockTb
	 * ****************************************************************/
sockTb::sockTb() {
	/* touControlBlock(tc) must be initialized*/
	CbSendBuf.setSize(TOU_MAX_CIRCULAR_BUF);
	CbRecvBuf.setSize(TOU_MAX_CIRCULAR_BUF);
	sc = new ssca(&tc);
	tc.t_timeout = TOU_INIT_TIMEO;
	duppkt.clean();
	std::cout << "inside socket table constructor" << std::endl;
	//int checkPipe = pipe(pfd);
	//if(checkPipe == 0) std::cout << " Pipe created " << std::endl;
}

bool sockTb::ckHpRecvBuf(const touPkg &pkt){
	if( (pkt.t.seq == duppkt.t.seq) && (pkt.t.ack_seq == duppkt.t.ack_seq) ){
		return true;
	}

	duppkt = pkt; /* may have problem  */
	std::cerr << "pkt not the same return false\n";
	return false;
}


/******************************************************************
 * sockMng
 * ****************************************************************/
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

//called during tou_accept()
/*  void sockMng::setSocketTableD(struct sockaddr_in *sockettemp, int sd)  {
	s = new sockTb;
	string sip;
	u_short sport;
	char* ip1 = inet_ntoa(sockettemp->sin_addr);
	u_short dport = ntohs(sockettemp->sin_port);
	std::string ip(ip1);
	boost::mutex::scoped_lock lock(soctabmutex);

	for(stbiter=SS.begin(); stbiter!=SS.end(); stbiter++) {
		if((*stbiter)->sockd == sd) {
			sip = (*stbiter)->sip;
			sport = (*stbiter)->sport;                  
			SS.erase(stbiter);
			s->sockd = sd;
			s->dport = dport;
			s->dip = ip;
			s->sip = sip;
			s->sport = sport;
			SS.push_back(s);
			break;
		}
	}
}
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

/*called during tou_socket() */
void sockMng::setSocketTable(int sd) {
	boost::mutex::scoped_lock lock(soctabmutex);
	s = new sockTb;
	s->sockd = sd;
	SS.push_back(s);
}

/* called during tou_close() */
void sockMng::delSocketTable(int sd) {
	boost::mutex::scoped_lock lock(soctabmutex);
	for(stbiter=SS.begin(); stbiter!=SS.end(); stbiter++) {
		if((*stbiter)->sockd == sd) {
			SS.erase(stbiter);
			break;
		}
	}
}


/* return socktable ptr if matchs with sockfd 
 * return NULL if failure */
struct sockTb* sockMng::getSocketTable(int sockfd) {
	for(stbiter=SS.begin(); stbiter!=SS.end(); stbiter++){
		if((*stbiter)->sockd == sockfd)
			return (*stbiter);
	}
	return NULL;
}

void sockMng::setSocketState(int state, int sd) {
	boost::mutex::scoped_lock lock(soctabmutex);      
	s = getSocketTable(sd);
	s->sockstate = state;
} 

void sockMng::setTCB(u_long seq, u_long seq1, int sd) {
	boost::mutex::scoped_lock lock(soctabmutex);      
	s = getSocketTable(sd);
	s->tc.snd_nxt = seq;
	s->tc.snd_una = seq1;
	//	std::cout << "tc.snd_nxt " << seq <<" "<< seq1<< std::endl;

}

void sockMng::setTCBCwnd(u_long winsize, int sd) {
	boost::mutex::scoped_lock lock(soctabmutex);      
	s = getSocketTable(sd);
	s->tc.snd_cwnd = winsize;
}

void sockMng::setTCBAwnd(u_long winsize, int sd) {
	boost::mutex::scoped_lock lock(soctabmutex);      
	s = getSocketTable(sd);
	s->tc.snd_awnd = winsize;
}

void sockMng::setTCBRcv(unsigned long seq , int sd)  {
	boost::mutex::scoped_lock lock(soctabmutex);      
	s = getSocketTable(sd);
	s->tc.rcv_nxt = seq;
}

void sockMng::setTCBState(int state , int sd)  {
	boost::mutex::scoped_lock lock(soctabmutex);      
	s = getSocketTable(sd);
	s->tc.cc_state = state;
}

int sockMng::setCbData(char *buf,int len,int sd) { 
	boost::mutex::scoped_lock lock(soctabmutex);    
	int len1 = len;  
	s = getSocketTable(sd);
	len1 = s->CbRecvBuf.insert(buf,len);
	return len1;
}

