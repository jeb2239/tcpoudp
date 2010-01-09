#include "timer.h"

minTmHeapType timerheap;
boost::mutex timermutex;

/*********************************************
 * Initialize the Queue and Map 
 * ******************************************/
timerMng::timerMng(){
	/* create a new timer thread */
	timercker = new timerCk();

  /* make sure timerheap is empty */
  while( !timerheap.empty() ) {
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

bool timerMng::add(conn_id cid, time_id tid, seq_id pid, sockTb *st, std::string *payload){
	boost::mutex::scoped_lock lock(timermutex);
	timernode = new node_t(cid, tid, pid, st, payload);
	timerheap.push(*timernode);
	std::cout<< " *** Add Timer cid: "<<cid <<" seq_id: "<< pid<<" TID: "<<tid<<" *** "<<std::endl;
}

/*********************************************
 * 1. Delete the timer by conn_id and time_id
 * Reg the id into vector
 * ******************************************/
bool timerMng::delete_timer(conn_id cid, time_id tid, seq_id pid){
  if(timercker->addDelNode(cid, tid, pid)) 
		std::cout<< " *** Add del Anchor cid: "<<cid <<" seq_id: "<< pid<<" TID: "<<tid<<" *** "<<std::endl;
}

/*********************************************
 * check if there's already a deletion timer in the
 * del_timer queue
 * ******************************************/
bool timerMng::ck_del_timer(conn_id cid, time_id tid, seq_id pid){
	return timercker->ckTimer(cid, tid, pid);
}

bool timerMng::reset(conn_id cid, time_id tid, seq_id pid){

}
bool timerMng::reset(conn_id cid, time_id tid, long ms, seq_id pid){

}
bool timerMng::deleteall(){

}
bool timerMng::resetall(){

}
