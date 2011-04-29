/*******************************************************************************
 * timer.cpp
 * The implementation of timerCk.
 ******************************************************************************/

#include "timer.h"
#include "tou_logger.h"

using namespace std;
using namespace boost;

timerMng tmng;
minTmHeapType timerheap;
timerDequeType ackdeque;
boost::mutex timer_heap_mtx; //to lock timer heap(timerheap)
boost::mutex ack_deque_mtx; //to lock ack deque(ackdeque)
boost::mutex ack_mtx; //before use stb_tc_ack_mtx & ack_deque_mtx

unsigned long min(u_long a, u_long b) {
	return (a>b)? b: a;
}

/**
 * get current time in millisecond 
 */
unsigned long getTimeMs(){

    struct timeval tv;  //current time value
    gettimeofday(&tv, NULL);

    return (((tv.tv_sec%1000000)*1000) + (tv.tv_usec/1000));
}

/**
 * ckTimer:
 * Check the vector whether there exists a timer
 * @return true if node is found
 */
/*
bool timerCk::ckTimer (conn_id cid, time_id tid, seq_id pid){

	boost::mutex::scoped_lock lock(tdvmutex);
	bool retval = false;

	for (it = tdv.begin (); it < tdv.end (); it++){
		if ((it->c_id == cid) && (it->t_id == tid) && (it->p_id == pid)){
			retval = true;
			break;
		}
	}//end of for

  return retval;
}
*/

/*
 * check whether the given timer node has been deleted or not. If yes, this
 * given timer node will be removed from tdv(deletion timer queue), otherwise, 
 * everything keeps intact.
 * Notice: the value of sequence number stored in timerheap is unacked seq
 * plus the size of the payload. Yet the sequence number stored in the tdv
 * implies it's acked. So they shuld be exactly the same to be considered 
 * matched 
 */
/*
bool timerCk::ckTimerDel (const node_t &nt) {

	boost::mutex::scoped_lock lock(tdvmutex);
	bool retval = false;

	for (it = tdv.begin (); it < tdv.end (); it++){
		if (it->c_id == nt.c_id && it->t_id == nt.t_id && it->p_id == nt.p_id){
			// calculate the RTT and RTO
			calRTO (it);

			// erase the node
			tdv.erase (it);

			lg.logData("[timerCk::ckTimerDel] *** Erase del_node in tdv("+
					lg.c2s(nt.p_id)+")", TOULOG_ALL);

			retval = true;
			break;
		}
	}//end of for

	return retval;
}
*/

/**
 * check the status of timerheap
 * @return bool
 */
bool timerCk::timerheap_empty() {

	boost::mutex::scoped_lock lock(timer_heap_mtx);
	return timerheap.empty();
}

/**
 * check the status of ackdeque
 * @return bool
 */
bool timerCk::ackdeque_empty() {

	boost::mutex::scoped_lock lock(ack_deque_mtx);
	return ackdeque.empty();
}

/*
 * addDelNode:
 * This function creates new deletion_node and adds newly generated node into
 * tdv queue.
 * @return on successful addition
 */
/*
bool timerCk::addDelNode (conn_id cid, time_id tid, seq_id pid){

	boost::mutex::scoped_lock lock(tdvmutex);
	//4th para. gets the current time, used for calculating RTT
	nt = new node_t (cid, tid, pid, getTimeMs());
	tdv.push_back (*nt);

	return true;
}
*/

/**
 * Calculate the round trip time and estimate the retransmission timeout.
 * @return true on successful culculation
 */
void timerCk::cal_rto (const node_t &nt, u_long recv_t){

	if (nt.xmitms < 0)
		return;

	sockTb *st = nt.st;
	long cur_rtt = recv_t - nt.xmitms;

	//To set min rtt in case of overblowing(wast of fired)
	if (cur_rtt < 10)
		cur_rtt = 10;

	//RTT
	st->tc.t_rtt = (long)(st->tc.t_rtt * st->tc.t_rtt_alpha) + 
		(long)(cur_rtt * (1 - st->tc.t_rtt_alpha));
	//RTO
	st->tc.t_rto = st->tc.t_rto_beta * st->tc.t_rtt;

	/*
	if (st->tc.t_rto < 0)
		st->tc.t_rto = 30;
	*/
}

/**
 * Calculate the rto after when the timeout occurs: rto is doubled.
 * @return true on successful culculation
 */
void timerCk::cal_backoff_rto (const node_t &nt){

	sockTb *st = nt.st;
	st->tc.t_rto = min(TOU_MAX_TIMEO, st->tc.t_rto * 2);
}


/**
 * calRTO:
 * This function calculate the round trip time and estimate the retransmission
 * timeout.
 * @return true on successful culculation
 */
/*
bool timerCk::calRTO (vector <node_t>::iterator nt){

	if (timerheap.top().xmitms < 0) 
		return true;

	sockTb *st = timerheap.top().st;
	long curRTT = nt->recvms - timerheap.top().xmitms;

	//ignoring the curRTT if the value < 0
	if (nt->recvms > timerheap.top().xmitms && 
			curRTT > 15){
		//RTT
		st->tc.t_rtt = (long)(st->tc.t_rtt * st->tc.t_rtt_alpha) + (long)(curRTT * (1 - st->tc.t_rtt_alpha));
		//RTO
		st->tc.t_rto = st->tc.t_rto_beta * st->tc.t_rtt;

		if (st->tc.t_rto < 30)
			st->tc.t_rto = 30;
	}

	//for demo
	//cerr << " [calRTO] cur_RTT: " << curRTT << " avg_RTT: " << st->tc.t_rtt << " RTO: " << st->tc.t_rto << endl;

	return true;
}
*/

/**
 * doit:
 * A periodic loop function for checking fired timer in timer thread
 * @return void.
 */
void timerCk::doit(){

	int run_ret = 0;
	asio::io_service io;
	asio::deadline_timer t_doit(io, posix_time::milliseconds(TIMER_WT_TIME));
	//asio::deadline_timer t_run(io, posix_time::milliseconds(TIMER_WT_TIME_RUN));
	
	/* 
	 * A loop for consistently checking fired timer(pkt rexmit/ delayed ACK)
	 */
	while(true){

		t_doit.wait();

		if (!ackdeque_empty()) 
			proc_delack(); //delayed ack

		if (timerheap_empty()) 
			continue;

		do { //recv outstanding acks as many as possi.
			run_ret = ptou->run(timerheap.top().c_id);
		/*	
		 	if (run_ret == 0) 
				t_run.wait();
		*/
		} while (run_ret >= 0);

		proc_sndpkt(); //fired data pkt

		/*
		cerr << "avg rtt:" << timerheap.top().st->tc.t_rtt << " rto:" << timerheap.top().st->tc.t_rto <<
			"  | timer#:" << timerheap.size() << " topseq:"<<timerheap.top().p_id << endl;
			*/

	}//End of while(true)

}

/**
 * sendack:
 * This function format the data size and sends the ack to dest.
 * MUST grape ack_deque_mtx before calling this func.
 * MUST grape stb_tc_ack_mtx before calling this func.
 *
 * sendpkt:
 * This function calculates the data size and sends the packet to dest.
 * @return false means tc.rcv_nxt has been updated already. we should't
 * send fired ack. (it implies we hace recv correct pkt but haven't update
 * yet.
 */
void timerCk::sendack (const node_t &nt) {

	assert(nt.st->tc.rcv_nxt >= nt.p_id);

	struct sockaddr_in sa;
	touPkg tou_pkt(0);
	tou_pkt.clean();
	assignaddr(&sa, AF_INET, nt.st->dip, nt.st->dport);
	tou_pkt.putHeaderSeq(nt.st->tc.snd_nxt, nt.st->tc.rcv_nxt);
	tou_pkt.t.ack = FLAGON;

	string strpkt = tou_pkt.toString();
	if (0 >= sendto(nt.c_id, strpkt.data(), strpkt.size(), 0,
		   (struct sockaddr *)&sa, sizeof(sa)) )
		perror("Error in timerCk::sendack");

}

int timerCk::sendpkt (const node_t &nt) {

	struct sockaddr_in sa;
	int bufsize = nt.payload->size();
	touPkg toupkt(nt.payload->c_str(), bufsize);
	toupkt.putHeaderSeq((nt.p_id - bufsize),nt.st->tc.rcv_nxt);
	toupkt.t.ack = FLAGON;
	string pktcont = toupkt.toString();
	assignaddr(&sa, AF_INET, nt.st->dip, nt.st->dport);

	// Send the packet
	if (0 >= sendto(nt.c_id, pktcont.data(), pktcont.size(), 0,
			(struct sockaddr *)&sa, sizeof(sa)) ) 
		perror("Error in timerCk::sendpkt");

	return bufsize;
}

/**
 * MUST grape ack_deque_mtx before calling this func.
 * MUST grape stb_tc_ack_mtx 
 */
void timerCk::update_ack(node_t *n, u_long cur_t){

	assert (n);

	n->p_id = n->st->tc.rcv_nxt;
	n->push_ackms = cur_t + TIMER_PROMPT_ACK_TIME;
	n->nagle_ackms = cur_t + TIMER_DELAYED_ACK_TIME;
}						 

/**
 * MUST grape ack_deque_mtx before calling this func.
 * MUST grape stb_tc_ack_mtx 
 */
void timerCk::proc_delack_xmit(timerDequeType::iterator itr, u_long t) {

	boost::mutex::scoped_lock lock(itr->st->stb_tc_ack_mtx);

	switch (itr->st->tc.t_delack_state) {
	  case TOUS_DELACK_IMMED_UPDATE_XMIT: //let processtou handle the xmit
		  break;

	  case TOUS_DELACK_XMIT:
		  if (itr->nagle_ackms < t) {
			  lg.logData("[TIMEO ACK] X("+lg.c2s(itr->st->tc.t_delack_state)+
						 ") rcv_nxt("+lg.c2s(itr->st->tc.rcv_nxt)+") node#("+
						 lg.c2s(itr->p_id)+") cur nagle push "+
						 lg.c2s(t)+" "+lg.c2s(itr->nagle_ackms)+" "+
						 lg.c2s(itr->push_ackms), TOULOG_ALL);

			  sendack(*itr);
			  update_ack(&(*itr), t);
			  itr->st->tc.t_delack_state = TOUS_DELACK_QUEXMIT;
		  }
		  break;

	  case TOUS_DELACK_QUEXMIT:
	  case TOUS_DELACK_EXPXMIT:
	  case TOUS_DELACK_QUE:
		  if (itr->push_ackms < t) {
			  lg.logData("[TIMEO ACK] O("+lg.c2s(itr->st->tc.t_delack_state)+
						 ") rcv_nxt("+lg.c2s(itr->st->tc.rcv_nxt)+") node#("+
						 lg.c2s(itr->p_id)+") cur nagle push "+
						 lg.c2s(t)+" "+lg.c2s(itr->nagle_ackms)+" "+
						 lg.c2s(itr->push_ackms), TOULOG_ALL);

			  /* whiel recovery from loss pkt, rcv_nxt has updated, and timer
			   * fired. it's possible that rcv_nxt > nt.pid. Still send ack */
			  sendack(*itr);
			  update_ack(&(*itr), t);
			  itr->st->tc.t_delack_state = TOUS_DELACK_EXPXMIT;
		  }
		  break;

	  case TOUS_DELACK_ERR:
	  default:
		  cerr << "[ERROR] timerCk::proc_delack_xmit\n";
		  exit(1);
		  break;
	}
}

/** 
 * proc_delack:
 */
void timerCk::proc_delack() {

	boost::mutex::scoped_lock a_lock(ack_mtx);
	boost::mutex::scoped_lock lock(ack_deque_mtx);
	timerDequeType::iterator itr;
	u_long cur_time = getTimeMs();

	for (itr=ackdeque.begin(); itr != ackdeque.end(); ++itr) 
		proc_delack_xmit(itr, cur_time);

}//End of timerCk::proc_delack()

/** 
 * proc_sndpkt:
 * Process the fired packet(rexmit the packet); it will rexmit fired packet
 * and then reset the timer as well as the congestion state.
 * Called by timerCk::doit
 */
void timerCk::proc_sndpkt() {

	boost::mutex::scoped_lock lock(timer_heap_mtx);
	proc_clear_ackedpkt();

	if (timerheap.empty())
		return;

	u_long cur_time = getTimeMs();

	if (timerheap.top().ms < cur_time) { 
		sockTb	*stb = timerheap.top().st;

		lg.logData("[TIMEO] ### >>> >>> FIRE >>> >>> LOSS PKT REXMITING SEQ("+
				   lg.c2s(timerheap.top().p_id)+")"+" current time("+
				   lg.c2s(cur_time)+")"+" node time("+
				   lg.c2s(timerheap.top().ms)+")", TOULOG_ALL|TOULOG_PTSRN);

		sendpkt (timerheap.top());

		node_t nt = timerheap.top();
		nt.xmitms = -1; //mark as invalid for cal rtt
		timerheap.pop();
		//cal_backoff_rto(nt);
		nt.ms = (stb->tc.t_rto * 2) + cur_time; //reset the timer of "this" pkt
		timerheap.push(nt);

		lg.logData("[TIMEO] ### ### TIMER UPDATE(POSTPONE) ### stb->tc.t_rto ("+
				   lg.c2s((unsigned long)stb->tc.t_rto)+") "+"new fired time("+
				   lg.c2s(timerheap.top().ms)+")", TOULOG_ALL|TOULOG_PTSRN|TOULOG_TIMER);

		if (timerheap.size() < 8)
			postpone_timer(timerheap.top().c_id, timerheap.top().t_id,
						   timerheap.top().p_id, stb->tc.t_rto * 2);

		stb->sc->settwnd(); //set "timeout" to congestion control state
	} 

}


/** 
 * proc_clear_ackedpkt:
 * Clean up the acked(useless) nodes from timerheap in terms of snd_una
 * So it doesn't gurantee the time of timerheap's topnode is within cur
 * time and not expired.
 * Called by timerCk::doit, proc_sndpkt
 * Note: timer_heap_mtx MUST be graped before calling.
 */
void timerCk::proc_clear_ackedpkt() {

	if (timerheap.empty())
		return;

	sockTb	*stb = timerheap.top().st;
	boost::mutex::scoped_lock lock(stb->stb_mtx);

	while (!timerheap.empty() &&
		   timerheap.top().p_id <= stb->tc.snd_una) {
		lg.logData("[TIMEOUT TIMER CLEAR] timerheap "+ 
				   lg.c2s(timerheap.top().c_id) + " seq#: "+
				   lg.c2s(timerheap.top().p_id) + " | snd_una: "+
				   lg.c2s(stb->tc.snd_una), TOULOG_ALL);
		timerheap.pop();
	}
}


/** 
 * proc_clear_ackedpkt:
 * clean up the acked(useless) nodes from timerheap.
 * Called by processtou side
 * Note: run_t_mtx has graped already.
 * Return true: If it has cleaned sth.
 */
void timerCk::proc_clear_ackedpkt(conn_id cid, time_id tid, 
								  seq_id una_seq, u_long recv_t) {

	boost::mutex::scoped_lock lock(timer_heap_mtx);
	/*
	minTmHeapType tempheap;
	int timerheapsize;
	*/
	conn_id	th_conn;
	time_id	th_time;
	seq_id	th_seq;

	while (!timerheap.empty()) {
		th_conn = timerheap.top().c_id;
		th_time = timerheap.top().t_id;
		th_seq = timerheap.top().p_id;

		lg.logData("[PROCESSTOU TIMER CLEAR] timerheap sz:" + lg.c2s(timerheap.size()) +
				   " rtt:"+lg.c2s(recv_t-timerheap.top().xmitms)+" seq:"+ 
				   lg.c2s(th_seq)+" | updated snd_una is "+
				   lg.c2s(una_seq), TOULOG_ALL);

		if (th_time == tid && th_conn == cid && th_seq < una_seq) {
			timerheap.pop();
			continue;

		}else if (th_time == tid && th_conn == cid && th_seq == una_seq) {
			cal_rto(timerheap.top(), recv_t);
			timerheap.pop();
			break;

		}else if (th_time == tid && th_conn == cid && th_seq > una_seq){
			lg.logData("[PROCESSTOU TIMER CLEAR] BREAK timerheap seq > una_seq: "+
					   lg.c2s(th_seq) + " > " + lg.c2s(una_seq), TOULOG_ALL);
			break;

		}else {
			lg.logData("[PROCESSTOU TIMER CLEAR] ^^^^^^^^^^^^ TIEMR HEAP CLEAR ERROR TIMER.CPP ^^ "+
					   lg.c2s(th_seq), TOULOG_ALL | TOULOG_PTSRN);
			timerheap.top().st->log(TOULOG_ALL);
			//tempheap.push(timerheap.top());
			timerheap.pop();
			exit(1);
		}
	}

	//recover from tempteap
	/*
	timerheapsize = tempheap.size();
	lg.logData("[TIMER CLEAR] ### INFO recover from tempheap size("+lg.c2s(timerheapsize)+")",
			TOULOG_ALL);
	for(int i=0; i < timerheapsize; i++){
		timerheap.push(tempheap.top());
		tempheap.pop();
	}
	*/
}


/**
 * postpone_timer:
 * every timer node with assigned time(t_rtt) whose conditions satisfy
 * given timer id and socket file describtor
 */
void timerCk::postpone_timer(conn_id th_conn, time_id timer_id, seq_id th_seq, 
							 u_long cur_rto){

	lg.logData("[TIMEO] ### postpone_timer ### stb->tc.t_rto ("+
			   lg.c2s(cur_rto)+")", TOULOG_ALL|TOULOG_PTSRN|TOULOG_TIMER);

	minTmHeapType tempheap;
	int timerheap_sz = timerheap.size();

	for(int i=0; i < timerheap_sz; i++){
		if (timerheap.top().c_id != th_conn ||
			timerheap.top().t_id != timer_id) {
			tempheap.push(timerheap.top());
			timerheap.pop();
			continue;
		}

		if (timerheap.top().p_id > th_seq) {
			assert(timerheap.top().xmitms > 0);
			lg.logData("[TIMEO PT] This SEQ("+lg.c2s(timerheap.top().p_id)+")"+
					   " > th_seq("+ lg.c2s(th_seq)+") " + ": old fired time("+
					   lg.c2s(timerheap.top().ms)+") Reset",
					   TOULOG_ALL|TOULOG_PTSRN|TOULOG_TIMER);

			node_t nt = timerheap.top();
			nt.ms = nt.xmitms + cur_rto;
			timerheap.pop();
			tempheap.push(nt);

		}else if (timerheap.top().p_id < th_seq) {
			timerheap.pop();

		}else if (timerheap.top().p_id == th_seq) {
			tempheap.push(timerheap.top());
			timerheap.pop();

		}else {
			lg.logData("[TIMEO PT] [ERROR] "+lg.c2s(timerheap.top().p_id)+
				")"+" current time("+" node time("+lg.c2s(timerheap.top().ms)+
				")", TOULOG_ALL|TOULOG_PTSRN|TOULOG_TIMER);
			exit(1);
		}
	}

	//recovery of tempheap
	timerheap_sz = tempheap.size();
	lg.logData("[TIMEO PT] ### PT END ###: Recovery from tempheap Size:("+
			   lg.c2s(timerheap_sz)+")",TOULOG_ALL|TOULOG_TIMER);

	for(int i=0; i < timerheap_sz; i++){

		timerheap.push(tempheap.top());
		tempheap.pop();
	}
	
}


/** 
 * assignaddr:
 * Assign sockaddr
 * @return on success
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
 * rexmit_for_dup_ack:
 * the seq_id is future seq number: seq current + leN
 * action: seq_id(pid) is the pkt that duped, rexmit it. For others, we need to
 * < pid: pop it and del correspondent copy in tdv(deletion queue)
 * > pid: repush into the queue
 */
bool timerCk::rexmit_for_dup_ack(conn_id cid, time_id tid, seq_id pid) {

	boost::mutex::scoped_lock lock(timer_heap_mtx);
	bool retval = false;
	conn_id	th_cid;
	time_id	th_tid;
	seq_id	th_pid;

	while (!timerheap.empty()) {
		 /* receiver side is looking for "pid"; however, the seq number stored in
		 * timerheap is seq + pkt length. We have to deduce the difference.*/
		lg.logData("[REXMIT DUPACK] ### INFO SEQ("+lg.c2s(pid)+")"+
				" timerheap size("+lg.c2s(timerheap.size())+")"+
				" node time("+lg.c2s(timerheap.top().ms)+")", TOULOG_ALL);

		th_cid = timerheap.top().c_id;
		th_tid = timerheap.top().t_id;
		th_pid = timerheap.top().p_id - timerheap.top().payload->size();

		if (th_cid == cid && th_tid == tid && th_pid == pid) {

			sendpkt (timerheap.top());

			/* reset the timer changing the xmitms time to -1, since rexmit */
			nt = new node_t(cid, tid, timerheap.top().p_id, timerheap.top().st, 
							timerheap.top().payload, -1);
			timerheap.pop();
			timerheap.push(*nt);
			
			lg.logData("[REXMIT DUPACK] >>> >>> XMITTING..("+lg.c2s(pid)+")"+" == "
				"timerheap.top("+lg.c2s(th_pid)+") ori. expected fired time("+
				lg.c2s(timerheap.top().ms)+")",TOULOG_ALL|TOULOG_PTSRN);

			retval = true;
			break;

		}else if (th_cid == cid && th_tid == tid && th_pid < pid) {
			lg.logData("[REXMIT DUPACK] ### SEQ("+lg.c2s(pid)+")"+" > "
				"timerheap.top("+lg.c2s(th_pid)+"):del tdv", TOULOG_ALL);
			timerheap.pop();

		}else {
			/* th_cid == cid && th_tid == tid && th_pid > pid */
			lg.logData("[REXMIT DUPACK] ### SEQ("+lg.c2s(pid)+")"+" ?<?  "
				"timerheap.top("+lg.c2s(th_pid)+")", TOULOG_ALL|TOULOG_PTSRN);
			exit(1);
		}

	}//End of While

	return retval; 
}

/**
 * getSocketTable:
 * @return socktable ptr if matchs with sockfd
 * @return NULL if failure
 */
sockTb* timerCk::getSocketTable(int sockfd) {

	//lock the stbq
	boost::mutex::scoped_lock lock(stbq_mutex);
	vector<sockTb*>::iterator stbiter;

	for(stbiter=stbq.begin(); stbiter!=stbq.end(); stbiter++){
		
		if((*stbiter)->sockd == sockfd)
			return (*stbiter);

	}//End of for
	return NULL;
}


