/**
 * timer_node.cpp:
 * Implementation of node_t class.
 *
 * Copyright 2009 by Columbia University; all rights reserved
 * May 04, 2010
 */
#include "timer.h"

using namespace std;

node_t::node_t (const node_t &nt){

	c_id = nt.c_id;		//socket fd(sock id)
	t_id = nt.t_id;		//timer id(reserved)
	p_id = nt.p_id;		//sequence number
	st = nt.st;			//socket table pt
	payload = NULL;
	if (nt.payload != NULL)
		payload = new string(nt.payload->data());
	ms = nt.ms;
	xmitms = nt.xmitms;
	push_ackms = nt.push_ackms;
	nagle_ackms = nt.nagle_ackms;
};

void node_t::operator=(const node_t &nt) {
	
	c_id = nt.c_id;		//socket fd(sock id)
	t_id = nt.t_id;		//timer id(reserved)
	p_id = nt.p_id;		//sequence number
	st = nt.st;			//socket table pt
	payload = NULL;
	if (nt.payload != NULL)
		payload = new string(nt.payload->data());
	ms = nt.ms;
	xmitms = nt.xmitms;
	push_ackms = nt.push_ackms;
	nagle_ackms = nt.nagle_ackms;
};

node_t::node_t (){
	printf("shoudl not use node_t::node_t ()\n");
	exit(1);
};

node_t::node_t (conn_id c, time_id t, seq_id p):
  c_id (c), t_id (t), p_id (p){
	printf("shoudl not use node_t::node_t (c, t, p)\n");
	exit(1);
};

/**
 * node_t (conn_id c, time_id t, seq_id p, long ms_to_fire, sockTb *s):
 * used in fired ack
 */
/*
node_t::node_t (conn_id c, time_id t, seq_id p, long ms_to_fire, sockTb *s):
  c_id (c), t_id (t), p_id (p), st (s){
	payload = NULL;
	xmitms = getTimeMs();
	recvms = 0;
	ms = xmitms + ms_to_fire;
};
*/

/**
 * used with ack deque.
 */
node_t::node_t (conn_id c, time_id t, seq_id p, long pms, long nms, sockTb *s):
  c_id (c), t_id (t), p_id (p), st (s), push_ackms (pms), nagle_ackms (nms){
    payload = NULL;
};

/**
 * node_t (conn_id c, time_id t, seq_id p, sockTb * s, char *pl)
 * constructor which inits a new timer node.
 */
node_t::node_t (conn_id c, time_id t, seq_id p, sockTb * s, char *pl):
  c_id (c), t_id (t), p_id (p),st (s){
	payload = new std::string (pl);
	xmitms = getTimeMs ();
	ms = xmitms + st->tc.t_rto;
	lg.logData ("timer node built ..sockfd is: " + lg.c2s(c) + " sizeof payload is :"
				+ lg.c2s(payload->size()) + " socktb->tc.snd_ack: " + lg.c2s(st->tc.rcv_nxt) +
				" size of payload: " + lg.c2s(payload->size())+ " rto: "+lg.c2s(st->tc.t_rto)+
				" expected expired time: " + lg.c2s(ms) ,TOULOG_TIMER);
};

/**
 * node_t (conn_id c, time_id t, seq_id p, sockTb * s, char *pl)
 * constructor which inits a new timer node.
 */
node_t::node_t (conn_id c, time_id t, seq_id p, sockTb * s, std::string * pl):
  c_id (c), t_id (t), p_id (p), st(s){
	payload = new std::string (*pl);
	xmitms = getTimeMs ();
	ms = xmitms + st->tc.t_rto;
	lg.logData ("timer node built ..sockfd is: " + lg.c2s(c) + " sizeof payload is :"
				+ lg.c2s(payload->size()) + " socktb->tc.snd_ack: " + lg.c2s(st->tc.rcv_nxt) +
				" size of payload: " + lg.c2s(payload->size())+ " rto: "+lg.c2s(st->tc.t_rto)+
				" expected expired time: " + lg.c2s(ms) ,TOULOG_TIMER);
};

/**
 * node_t (conn_id c, time_id t, seq_id p, sockTb * s, char *pl, long time)
 * constructor which inits a new timer node.
 * used only in rexmit_for_dup_ack 
 */
node_t::node_t (conn_id c, time_id t, seq_id p, sockTb * s, std::string * pl, long xmit_t):
  c_id (c), t_id (t), p_id (p), st(s){
	payload = new std::string (*pl);
	xmitms = xmit_t;
	ms = getTimeMs() + s->tc.t_rto;
	lg.logData ("timer node built ..sockfd is: " + lg.c2s(c) + " seq#: "+ lg.c2s(p) +" sizeof payload is :"
				+ lg.c2s(payload->size()) + " socktb->tc.snd_ack: " + lg.c2s(st->tc.rcv_nxt) +
				" size of payload: " + lg.c2s(payload->size())+
				" expected expired time: " + lg.c2s(ms) + " xmit time: " + lg.c2s(xmit_t),TOULOG_TIMER);
};

/**
 * node_t (const node_t &nt, long time)
 * constructor which inits a new timer node and specify the fired time
 * used "only" in timerCk::proc_sndpkt, cause it's rexmit pkt that the
 * xmitms time would be set as negtive value.
 */
node_t::node_t (const node_t &nt, u_long fired_t){
	c_id = nt.c_id;	//socket fd(sock id)
	t_id = nt.t_id;	//timer id(reserved)
	p_id = nt.p_id;	//sequence number
	st = nt.st;		//socket table pt
	payload = NULL;
	if (nt.payload != NULL)
		payload = new std::string(nt.payload->data());
	xmitms = -1; // since it's "rexmit", we ignore rtt calculation
	ms = fired_t;
};

