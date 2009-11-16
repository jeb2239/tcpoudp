#include "timer.h"
/* STL, Priority_queue(min-heap) 
 * for storing all pkt's timer */
minTmHeapType timerheap;
boost::mutex timermutex;

/*********************************************
 * Initialize the Queue and Map 
 * ******************************************/
timerMng::timerMng(){
	//run TimerCk ia a new thread: timer thread
  timercker = new timerCk();

  //Make sure timerheap is empty
  while( !timerheap.empty() ) {
		std::cerr << "Timer heap doesn't empty! Cleaning it!\n";
		timerheap.pop();
	}
  std::cout << "*** timerMng built ***\n";
}

/*********************************************
 * 1. Lock the resources.
 * 2. New a timer, and put intot timerheap
 * 3. payload, char* int, seq#, 
 ********************************************/
bool timerMng::add(conn_id cid, time_id tid, seq_id pid, long ms){
  boost::mutex::scoped_lock lock(timermutex);
  timernode = new node_t(cid, tid, pid, getCurMs()+ms);  
  timerheap.push(*timernode);
}
bool timerMng::add(conn_id cid, time_id tid, seq_id pid, sockTb *st, char *payload){
	boost::mutex::scoped_lock lock(timermutex);
	timernode = new node_t(cid, tid, pid, st, payload);
	timerheap.push(*timernode);
	
  //test
  //std::cout << "timer(ms: "<<timernode->ms <<") (id: "<<timernode->c_id << " pid: " << timernode->p_id<<" is added  "<<ms <<" "<<getCurMs() <<" "<< timernode<<std::endl;
}
/*********************************************
 * 1. Delete the timer by conn_id and time_id
 * Reg the id into vector
 * ******************************************/
bool timerMng::delete_timer(conn_id cid, time_id tid, seq_id pid){
  if(timercker->addDelNode(cid, tid, pid)) std::cout<< " *** Add del Anchor CID: "<<cid << pid<<" TID: "<<tid<<" *** "<<std::endl;
}

bool timerMng::reset(conn_id cid, time_id tid, seq_id pid){

}
bool timerMng::reset(conn_id cid, time_id tid, long ms, seq_id pid){

}
bool timerMng::deleteall(){

}
bool timerMng::resetall(){

}
