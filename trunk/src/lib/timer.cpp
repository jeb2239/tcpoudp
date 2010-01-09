#include "timer.h"
/**
 * A periodic loop function for cheing fired timer in timer thread.
 */
void timerCk::doit(){
	boost::asio::io_service io;
	boost::asio::deadline_timer t(io, boost::posix_time::milliseconds(TIMER_WT));
	int bufsize;
	
  while(1){
		// loop activated if there is timer node in timer heap and timer is fired
    while( !timerheap.empty() && (timerheap.top().ms <= getCurMs()) ){
      boost::mutex::scoped_lock lock(timermutex);
			// check if there's record in del vector
			// if yes, pop and discard the fired timer
			// if no, handle the fired timer(rexmit and reset timer)
      if( !ckTimerDel(timerheap.top().c_id, timerheap.top().t_id, timerheap.top().p_id) &&
					((timerheap.top().p_id >= timerheap.top().st->tc.snd_una) &&
					(timerheap.top().c_id == timerheap.top().st->sockd)) ){
				bufsize = timerheap.top().payload->size();
				touPkg toupkt(timerheap.top().payload->c_str(), bufsize);
				toupkt.putHeaderSeq((timerheap.top().p_id - bufsize), timerheap.top().st->tc.rcv_nxt);
				toupkt.t.ack = FLAGON;
				assignaddr(&sockaddrs, AF_INET, timerheap.top().st->dip, timerheap.top().st->dport);
				sendto(timerheap.top().c_id, &toupkt, sizeof(toupkt), 0, (struct sockaddr *)&sockaddrs, sizeof(sockaddr));

				//reset the timer
				nt = new node_t(timerheap.top().c_id, timerheap.top().t_id, timerheap.top().p_id, timerheap.top().st, timerheap.top().payload);
				timerheap.push(*nt);

				/* for test */
				std::cerr<< "Timer not in delqueue and fired c_id:"<<timerheap.top().c_id <<" "
					<< timerheap.top().p_id <<" timer id : " <<timerheap.top().t_id << " fired."
					<< "CurTime: "<<getCurMs()<<"; Timer: "<<timerheap.top().ms<<"Buffer size: "<< 
					bufsize << " Buffer: "<< toupkt.buf <<std::endl;
				/* end of for test */
				}	
				timerheap.pop();
    }
		//synchronous wait for TIMER_WT ms
		t.wait();
    
  }//End of while(1)
}

/* for assign sockaddr */
int timerCk::assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, std::string ip, unsigned short port) {
  bzero(sockaddr, sizeof(*sockaddr));
  sockaddr->sin_family = sa_family;
  sockaddr->sin_port = htons((short)(port));

  if( 1 != inet_pton(sa_family, ip.c_str(), &sockaddr->sin_addr) )
    return 0;
  return 1;
}

/* get current time in ms */
long getCurMs(){
    struct timeval tv;  //current time value
    gettimeofday(&tv, NULL);
    return (((tv.tv_sec%1000000)*1000) + (tv.tv_usec/1000));
}
