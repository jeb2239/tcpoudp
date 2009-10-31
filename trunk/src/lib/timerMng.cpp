#include "timer.h"
minTmHeapType timerheap;         // STL, Priority_queue(min-heap)
timerDequeType timerdeque;       // STL, Queue
boost::mutex timermutex;

/*********************************************
 * Initialize the Queue and Map 
 * ******************************************/
timerMng::timerMng()
{
  timercker = new timerCk();

  //Make sure timerheap is empty
  while( !timerheap.empty() ) timerheap.pop();
  std::cout << "*** timerMng built ***\n";
}

/*********************************************
 * 1. Lock the resources.
 * 2. New a timer, and put intot timerheap
 ********************************************/
bool timerMng::add(conn_id cid, time_id tid, long ms, packet_id pid){
  boost::mutex::scoped_lock lock(timermutex);
  timernode = new node_t(cid, tid, getCurMs()+ms, pid);  
  timerheap.push(*timernode);
  std::cout << "timer(ms: "<<timernode->ms <<") (id: "<<timernode->c_id << " pid: " << timernode->p_id<<" is added  "<<ms <<" "<<getCurMs() <<" "<< timernode<<std::endl;
}

/*********************************************
 * 1. Delete the timer by conn_id and time_id
 * Reg the id into vector
 * ******************************************/
bool timerMng::delete_timer(conn_id cid, time_id tid, packet_id pid){
  if(timercker->addDelnode(cid, tid, pid)) std::cout<< " *** Add del Anchor CID: "<<cid << pid<<" TID: "<<tid<<" *** "<<std::endl;
}

bool timerMng::reset(conn_id cid, time_id tid, packet_id pid){

}
bool timerMng::reset(conn_id cid, time_id tid, long ms, packet_id pid){

}
bool timerMng::deleteall(){

}
bool timerMng::resetall(){

}

