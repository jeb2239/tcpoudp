#include "timer.h"

void timerCk::doit(){
  //mutex the current resrouces, and ck if there's a timer fired
	boost::asio::io_service io;
	boost::asio::deadline_timer t(io, boost::posix_time::milliseconds(TIMER_WT));
	
  while(1){
    if( (!timerheap.empty()) &&	(timerheap.top().ms <=  getCurMs()) ) {
      boost::mutex::scoped_lock lock(timermutex);
      if( !ckTimerDel(timerheap.top().c_id, timerheap.top().t_id, timerheap.top().p_id)){
				//not in the del vector, so this fire node needs to be handled.
				//rexmitting this pkt, and reset the timer again.
				touPkg toupkt;//(sizeof(*(timerheap.top().payload)));
				toupkt.putHeaderSeq(timerheap.top().p_id, timerheap.top().st->tc.snd_ack);
				toupkt.t.ack = FLAGON;
				strncpy(toupkt.buf, timerheap.top().payload, strlen(timerheap.top().payload));

				assignaddr(&sockaddrs, AF_INET, timerheap.top().st->dip, timerheap.top().st->dport);
				//sending...
				sendto(timerheap.top().c_id, &toupkt, sizeof(toupkt), 0, (struct sockaddr *)&sockaddrs, sizeof(sockaddr));

				//reset the timer
				nt = new node_t(timerheap.top().c_id, timerheap.top().t_id, timerheap.top().p_id, timerheap.top().st, timerheap.top().payload);
				timerheap.push(*nt);

				/* for test */
        
				std::cout<< "Timer not in delqueue and fired c_id:"<<timerheap.top().c_id <<" "<< timerheap.top().p_id <<" timer id : " <<timerheap.top().t_id << " fired."<< "CurTime: "<<getCurMs()<<"; Timer: "<<timerheap.top().ms<<"Buffer : " << toupkt.buf <<std::endl;
      }	
      timerheap.pop();
    }else{
			/* synchronous wait for TIMER_WT ms */
			t.wait();
    }
  }//End of while
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
