/**
 * timer.cpp
 * Implementation of timerCk. The job of this class is to perform a periodic
 * loop thread for detecting possible fired timer, and handle the correspondent
 * conditions
 */

#include "timer.h"
#include "Logger.h"
using namespace std;
/**
 * A periodic loop function for cheing fired timer in timer thread.
 */
void timerCk::doit(){
	boost::asio::io_service io;
	boost::asio::deadline_timer t(io, boost::posix_time::milliseconds(TIMER_WT));
	int bufsize;
	long pktTime;
	
  while(1){
		// loop activated if there is timer node in timer heap and timer is fired
		while(1){

			//if there's a fired timer already, t_timeout_postone should be one, or
			//this value should be zero, meaning the first timeo within this wnd(RTT)
			pktTime = timerheap.top().ms + 
				(timerheap.top().st->tc.t_timeout_postpone*timerheap.top().st->tc.t_timeout);

			if (!( !timerheap.empty() && (pktTime <= getCurMs()))) break;
			//if(timermutex.try_lock()) 
			//{
      boost::mutex::scoped_lock lock(timermutex);
			// check if there's record in del vector
			// if yes, pop and discard the fired timer
			// if no, handle the fired timer(rexmit and reset timer)
      if (!ckTimerDel(timerheap.top().c_id, timerheap.top().t_id,timerheap.top().p_id)
					&&((timerheap.top().p_id >= timerheap.top().st->tc.snd_una) 
					&&(timerheap.top().c_id == timerheap.top().st->sockd)) ){

				bufsize = timerheap.top().payload->size();
				touPkg toupkt(timerheap.top().payload->c_str(), bufsize);
				toupkt.putHeaderSeq((timerheap.top().p_id - bufsize), 
						timerheap.top().st->tc.rcv_nxt);

				toupkt.t.ack = FLAGON;
				string pktcont = toupkt.toString();
				assignaddr(&sockaddrs, AF_INET, timerheap.top().st->dip, 
						timerheap.top().st->dport);
				
				sendto(timerheap.top().c_id, pktcont.data(), pktcont.size(), 0, (struct sockaddr *)
						&sockaddrs, sizeof(sockaddr));

				//reset the timer
				nt = new node_t(timerheap.top().c_id, timerheap.top().t_id, 
						timerheap.top().p_id, timerheap.top().st, timerheap.top().payload);
				timerheap.push(*nt);

				//setting timeout congestion control state
				socktb = getSocketTable(timerheap.top().c_id);
				if (socktb != NULL) socktb->sc->settwnd();

				//setting t_timeout_postpone
				socktb->tc.t_timeout_postpone++;

				/* for test */
				std::cerr<< "Timer not in delqueue and fired c_id:"<<timerheap.top().c_id <<" "
					<< timerheap.top().p_id <<" timer id : " <<timerheap.top().t_id << " fired."
					<< "CurTime: "<<getCurMs()<<"; Timer: "<<timerheap.top().ms<<"Buffer size: "<< 
					bufsize << " Buffer: "<< toupkt.buf <<std::endl;
				socktb->log(TOULOG_PTSRN);
				/* end of for test */
			}	
			//timermutex.unlock();
			//}//mutex locker
			timerheap.pop();
		}

		//synchronous wait for TIMER_WT ms
		t.wait();
    
  }//End of while(1)
}

/** 
 * timerCk::assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family,
 *		std::string ip, unsigned short port)
 * for assign sockaddr 
 */
int timerCk::assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, 
		std::string ip, unsigned short port) {

  bzero(sockaddr, sizeof(*sockaddr));
  sockaddr->sin_family = sa_family;
  sockaddr->sin_port = htons((short)(port));

  if( 1 != inet_pton(sa_family, ip.c_str(), &sockaddr->sin_addr) )
    return 0;
  return 1;
}

/**
 * rexmit_for_dup_ack(conn_id cid, time_id tid, seq_id pid):
 */
bool timerCk::rexmit_for_dup_ack(conn_id cid, time_id tid, seq_id pid) {
	minTmHeapType temptimerheap;
	int bufsize;
	bool retbool = false;

	//boost::mutex::scoped_lock lock(timermutex);

	//cerr << "(conn_id cid, time_id tid, seq_id pid): " << cid << " " << tid << " " << pid << endl;
	//cerr << "  timerheap.size()  " <<  timerheap.size() << endl;

	for(int i=0; i < timerheap.size(); i++){
		bufsize = timerheap.top().payload->size();

		//cerr << "i:" << i << " (conn_id cid, time_id tid, seq_id pid): " << timerheap.top().c_id << " " << timerheap.top().t_id << " " << timerheap.top().p_id-bufsize << endl;
		if ((timerheap.top().c_id == cid) && (timerheap.top().t_id == tid) &&
				( (timerheap.top().p_id-bufsize) == pid)) {
			// find the packet(timer node) which should be rexmitted as receiving
			// three duplicate acks.
			touPkg toupkt(timerheap.top().payload->c_str(), bufsize);
			toupkt.putHeaderSeq((timerheap.top().p_id - bufsize), 
					timerheap.top().st->tc.rcv_nxt);
			toupkt.t.ack = FLAGON;
			string pktcont = toupkt.toString();

			assignaddr(&sockaddrs, AF_INET, timerheap.top().st->dip,
					timerheap.top().st->dport);

			sendto(cid, pktcont.data(), pktcont.size(), 0, (struct sockaddr *)
					&sockaddrs, sizeof(sockaddr));

			//reset the timer
			nt = new node_t(cid, tid, pid, timerheap.top().st, timerheap.top().payload);
			timerheap.push(*nt);

			/* for test */
			std::cerr<< "[ .. .. Rexmit_for_dup_ack] c_id:"<<timerheap.top().c_id <<" "
				<< timerheap.top().p_id <<" timer id : " <<timerheap.top().t_id << " fired."
				<< "CurTime: "<<getCurMs()<<"; Timer: "<<timerheap.top().ms<<"Buffer size: "<< 
				bufsize << " Buffer: "<< toupkt.buf <<std::endl;
			/* end of for test */

			timerheap.pop();
			retbool = true;
			break;

		}else{
			temptimerheap.push(timerheap.top());
			timerheap.pop();
		}
	}

	// recovery from temptimerheap
	while (!temptimerheap.empty()){
		timerheap.push(temptimerheap.top());
		temptimerheap.pop();
	}
	return retbool; 
}

/**
 * getSocketTable(int sockfd):
 * return socktable ptr if matchs with sockfd
 * return NULL if failure
 */
sockTb* timerCk::getSocketTable(int sockfd) {
	std::vector<sockTb*>::iterator stbiter;
	for(stbiter=SS.begin(); stbiter!=SS.end(); stbiter++){
		if((*stbiter)->sockd == sockfd)
			return (*stbiter);
	}
  return NULL;
}

/**
 * getCurMs():
 * get current time in millisecond 
 */
long getCurMs(){
    struct timeval tv;  //current time value
    gettimeofday(&tv, NULL);
    return (((tv.tv_sec%1000000)*1000) + (tv.tv_usec/1000));
}

