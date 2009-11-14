#include "timer.h"
#define FLAGON	1
#define FLAGOFF	0
typedef	timerheap.top() heaptop;

void timerCk::doit(){
  //mutex the current resrouces, and ck if there's a timer fired
  while(1){
    if( (!timerheap.empty()) &&	(timerheap.top().ms <=  getCurMs()) ) {
      boost::mutex::scoped_lock lock(timermutex);
      if( !ckTimerDel(timerheap.top().c_id, timerheap.top().t_id, timerheap.top().p_id)){
				//not in the del vector, so this fire node needs to be handled.
				//rexmitting this pkt, and reset the timer again.
				touPkg toupkt(sizeof(*(heaptop.payload)));
				toupkt.putHeaderSeq(heaptop.p_id, heaptop.st.tc.snd_ack);
				toupkt.t.ack = FLAGON;
				strncpy(toupkt.buf, heaptop.payload, sizeof(*(heaptop.payload)));

				//sending...
				sendto(sockfd, &toupkg, sizeof(toupkg), 0, (struct sockaddr *)&sockaddr, sizeof(sockaddr));

				//reset the timer
				nt = new node_t(heaptop.c_id, heaptop.t_id, heaptop.p_id, heaptop.st, heaptop.payload, getCurMs()+(heaptop.st->ct.t_timeout));
				timerheap.push(*timernode);

				/* for test */
				std::cout<< "Timer: "<<timerheap.top().c_id <<" "<< timerheap.top().p_id <<" timer id : " <<timerheap.top().t_id << " fired."<< "CurTime: "<<getCurMs()<<"; Timer: "<<timerheap.top().ms<<std::endl;
				std::cout<< "Enter the Deque CID: "<<nt->c_id<< "Enter the Deque PID: "<<nt->p_id<< "; TID: "<<nt->t_id<<std::endl<<std::endl;
      }	
      timerheap.pop();
    }else{
			/* synchronous wait for 500 ms */
			t.wait();
    }
  }//End of while
}

long getCurMs(){
    struct timeval tv;  //current time value
    gettimeofday(&tv, NULL);
    return (((tv.tv_sec%1000000)*1000) + (tv.tv_usec/1000));
}
