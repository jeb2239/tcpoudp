/**
 * timerMng.cpp:
 * Implementation of timerMng class. 
 *
 * Copyright 2009 by Columbia University; all rights reserved
 * Jan 09, 2010
 */

#include "timer.h"

using namespace std;
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
 * use timer_heap_mtx to lock the scope and then new a timer, put it into timer
 * heap, including paylaod and seq #s.
 */
bool timerMng::add(conn_id cid, time_id tid, seq_id pid, sockTb *st, 
				   string *payload){

	bool retval = false;

	if (NULL != (timernode = new node_t(cid, tid, pid, st, payload)) ) {
		boost::mutex::scoped_lock lock(timer_heap_mtx);

		timerheap.push(*timernode);
		retval = true;
	}

	lg.logData("*** Add Timer cid: "+lg.c2s(cid)+" seq_id: "+lg.c2s(pid)+" TID: "
			+lg.c2s(tid)+" *** ", TOULOG_TIMER);
	return retval;
}

/**
 * timerMng::add_ack(conn_id cid, time_id tid, seq_id pid, sockTb *st){
 * use timer_heap_mtx to lock the scope and then new a timer, put it into timer
 * heap, including paylaod and seq #s.
 */
/*
bool timerMng::add_ack(conn_id cid, time_id tid, seq_id pid, int deltime, sockTb *st){

	bool retval = false;

	if (NULL != (timernode = new node_t(cid, tid, pid, deltime, st)) ) {
		//new a timernode. TIMER_DELAYED_ACK = 200ms means it will delay for 200 ms
		boost::mutex::scoped_lock lock(timer_heap_mtx);

		timerheap.push(*timernode);
		retval = true;
	}

	lg.logData("*** Add ACK timernode cid: "+lg.c2s(cid)+" seq_id: "+lg.c2s(pid)+" TID: "
			+lg.c2s(tid)+" *** ", TOULOG_TIMER);
	return retval;
}
*/

/**
 * MUST get MUTEX, ack_deque_mtx, before calling this function.
 */
void timerMng::add_ack(conn_id cid, time_id tid, sockTb *stb) {

	u_long cur_t = getTimeMs();
	node_t *n = new node_t(cid, tid, stb->tc.rcv_nxt, cur_t+TIMER_PROMPT_ACK_TIME,
						   cur_t+TIMER_DELAYED_ACK_TIME, stb);

	ackdeque.push_back(*n);
}

/**
 * timerMng::update_ack(conn_id cid, time_id tid, sockTb *stb)
 * update if possible, new a node if can't find the existed.
 * MUST get MUTEX, stb_tc_ack_mtx, before calling this function.
 */
void timerMng::update_ack(conn_id cid, time_id tid, short state, sockTb *stb) {

	boost::mutex::scoped_lock lock(ack_deque_mtx);
	node_t *n = get_ack_node(cid, tid);
	u_long cur_t = getTimeMs();

	if(!n) {
		if (TOUS_DELACK_XMIT == state) {
			add_ack(cid, tid, stb);
			return;
		}else {
			cerr << "Err timerMng::update_ack add_ack\n";
			exit(1);
		}
	}

	n->p_id = n->st->tc.rcv_nxt;
	n->push_ackms = cur_t + TIMER_PROMPT_ACK_TIME;
	n->nagle_ackms = cur_t + TIMER_DELAYED_ACK_TIME;
}


/**
 * MUST get MUTEX, ack_deque_mtx,  before calling this function.
 * Return on NULL if can't grape one.
 */
node_t* timerMng::get_ack_node(conn_id cid, time_id tid) {

	timerDequeType::iterator itr;
	if (ackdeque.empty())
		return NULL;

	for (itr = ackdeque.begin(); itr != ackdeque.end(); ++itr) {

		if (itr->c_id == cid && itr->t_id == tid)
			return &(*itr);
	}
	return NULL;
}




/**
 * delete_timer:
 * Delete the timer whose sequence number is below seq_una. While match, 
 * calculate the RTT. 
 * Note I: this func. is called by processtou.run.
 * Note II: this func. is competing with timer thread; mtxs must get.
 */
void timerMng::delete_timer(conn_id cid, time_id tid, seq_id pid){

	timercker->proc_clear_ackedpkt(cid, tid, pid, getTimeMs());
	lg.logData("*** Delete Timer cid: "+lg.c2s(cid)+" seq_id: "+lg.c2s(pid)+
			   " TID: "+lg.c2s(tid)+" ***", TOULOG_TIMER);
}

/**
 * timerMng::ck_del_timer(conn_id cid, time_id tid, seq_id pid):
 * check if there's already a deletion timer in the del_timer queue
 */
/*
bool timerMng::ck_del_timer(conn_id cid, time_id tid, seq_id pid){

	//ckTimer has been locked by tdvmutex
	bool ret = timercker->ckTimer(cid, tid, pid);
	return ret;
}
*/

/**
 * rexmit_for_dup_ack(conn_id cid, time_id tid, seq_id pid):
 * retransmit specific packet and reset its timer. using this function when in
 * "fast retransmit state as receiving three duplicate acknowledgements".
 */
bool timerMng::rexmit_for_dup_ack(conn_id cid, time_id tid, seq_id pid) {

	//rexmit_for_dup_ack has been locked by timer_heap_mtx
	bool ret = timercker->rexmit_for_dup_ack(cid, tid, pid);
	return ret;
}

/**
 * following code are reserved */
bool timerMng::reset(conn_id cid, time_id tid, seq_id pid){

	boost::mutex::scoped_lock lock(timer_heap_mtx);
}
bool timerMng::reset(conn_id cid, time_id tid, long ms, seq_id pid){

	boost::mutex::scoped_lock lock(timer_heap_mtx);
}
bool timerMng::deleteall(){

	boost::mutex::scoped_lock lock(timer_heap_mtx);
}
bool timerMng::resetall(){

	boost::mutex::scoped_lock lock(timer_heap_mtx);
}


