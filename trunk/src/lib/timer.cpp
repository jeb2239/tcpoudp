#include "timer.h"

void timerCk::doit(){
  //mutex the current resrouces, and ck if there's a timer fired
  while(1){
    if( (!timerheap.empty()) &&	(timerheap.top().ms <=  getCurMs()) ) {
      boost::mutex::scoped_lock lock(timermutex);
      if( !ckTimerDel(timerheap.top().c_id, timerheap.top().t_id, timerheap.top().p_id)){
	//not in the del vector, so put into deque
	nt = new node_t(timerheap.top().c_id, timerheap.top().t_id, timerheap.top().p_id);
	timerdeque.push_back(*nt);
	std::cout<< "Timer: "<<timerheap.top().c_id <<" "<< timerheap.top().p_id <<" timer id : " <<timerheap.top().t_id << " fired."<< "CurTime: "<<getCurMs()<<"; Timer: "<<timerheap.top().ms<<std::endl;
	std::cout<< "Enter the Deque CID: "<<nt->c_id<< "Enter the Deque PID: "<<nt->p_id<< "; TID: "<<nt->t_id<<std::endl<<std::endl;
      }	
      timerheap.pop();
    }else{
      sleep(0.5);
    }
  }//End of while
}

long getCurMs(){
    struct timeval tv;  //current time value
    gettimeofday(&tv, NULL);
    return (((tv.tv_sec%1000000)*1000) + (tv.tv_usec/1000));
}
