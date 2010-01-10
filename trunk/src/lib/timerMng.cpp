/**
 * timerMng.cpp:
 * Implementation of timerMng class. 
 *
 * Copyright 2009 by Columbia University; all rights reserved
 * Jan 09, 2010
 */

#include "timer.h"

minTmHeapType timerheap;
boost::mutex timermutex;

/**
 * timerMng::timerMng():
 * Initialize the Queue and Map 
 */
timerMng::timerMng(){
	/* create a new timer thread */
	timercker = new timerCk();

  /* make sure timerheap is empty */
  while( !timerheap.empty() ) {
		timerheap.pop();
	}
}

/**
 * timerMng::add(conn_id cid, time_id tid, seq_id pid, long ms):
 * use timermutex to lock the scope and then new a timer, put it into timer
 * heap, including paylaod and seq #s.
 */
bool timerMng::add(conn_id cid, time_id tid, seq_id pid, sockTb *st, 
		std::string *payload){
	boost::mutex::scoped_lock lock(timermutex);
	timernode = new node_t(cid, tid, pid, st, payload);
	timerheap.push(*timernode);
	lg.logData("*** Add Timer cid: "+lg.c2s(cid)+" seq_id: "+lg.c2s(pid)+" TID: "
			+lg.c2s(tid)+" *** ", TOULOG_TIMER);
}

/**
 * timerMng::delete_timer(conn_id cid, time_id tid, seq_id pid)
 * Delete the timer with conn_id and time_id by registering the id into vector
 */
bool timerMng::delete_timer(conn_id cid, time_id tid, seq_id pid){
  if(timercker->addDelNode(cid, tid, pid)) 
		lg.logData("*** Add del Anchor cid: "+lg.c2s(cid)+" seq_id: "+lg.c2s(pid)+
				" TID: "+lg.c2s(tid)+" ***", TOULOG_TIMER);
}

/**
 * timerMng::ck_del_timer(conn_id cid, time_id tid, seq_id pid):
 * check if there's already a deletion timer in the del_timer queue
 */
bool timerMng::ck_del_timer(conn_id cid, time_id tid, seq_id pid){
	return timercker->ckTimer(cid, tid, pid);
}

/**
 * following code reserved */
bool timerMng::reset(conn_id cid, time_id tid, seq_id pid){

}
bool timerMng::reset(conn_id cid, time_id tid, long ms, seq_id pid){

}
bool timerMng::deleteall(){

}
bool timerMng::resetall(){

}
