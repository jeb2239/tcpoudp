/**
 * timer.h: define timer related classes
 * declare the timer node: node_t
 * declare the timer management: timerMng
 * declare the timer thread: timerCk
 *
 * Copyright 2009 by Columbia University; all rights reserved 
 * Jan 09, 2010
 */

#include "touSockTable.h"
#include "Logger.h"
#include <string>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <functional>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/timer.hpp>

/* logging */
extern Logger lg;

/* time that timer would wait for each loop iteration in doit() func.*/
#define TIMER_WT	500

/* type ofr socket file descriptor */
typedef unsigned long conn_id;
/* type for connection sequence number */
typedef unsigned long seq_id;
/* timer id, reserved */
typedef unsigned int time_id;
/* timer mutex, used for timer buf */
extern boost::mutex timermutex;

/**
 * getCurMs: get the current time in millisecond
 * return the current time in long 
 */
long getCurMs ();

/**
 * node_t: node_t represents timer node which should be used in timer heap 
 *         tree.
 */
class node_t{
public:
  node_t (){
    lg.logData ("Using old version constructor. Shouldn't use node_t()",
		TOULOG_TIMER);
  };
	
	node_t (conn_id c, time_id t, seq_id p):c_id (c), t_id (t), p_id (p){
  };

  node_t (conn_id c, time_id t, seq_id p, long m):
  c_id (c), t_id (t), p_id (p), ms (m){
  };

	/**
	 * node_t (conn_id c, time_id t, seq_id p, sockTb * s, char *pl)
	 * constructor which inits a new timer node.
	 */
  node_t (conn_id c, time_id t, seq_id p, sockTb * s, char *pl):
  c_id (c), t_id (t), p_id (p),st (s){
    payload = new std::string (pl);
		lg.logData ("timer node built ..sockfd is: " + lg.c2s(c) + " sizeof payload is :"
      + lg.c2s(payload->size()) + " socktb->tc.snd_ack: " + lg.c2s(st->tc.rcv_nxt) +
			" size of payload: " + lg.c2s(payload->size()),TOULOG_TIMER);
    ms = getCurMs() + s->tc.t_timeout;
  };

	/**
	 * node_t (conn_id c, time_id t, seq_id p, sockTb * s, char *pl)
	 * constructor which inits a new timer node.
	 */
	node_t (conn_id c, time_id t, seq_id p, sockTb * s, std::string * pl):
	c_id (c), t_id (t), p_id (p), st(s){
    payload = new std::string (*pl);
		lg.logData ("timer node built ..sockfd is: " + lg.c2s(c) + " sizeof payload is :"
      + lg.c2s(payload->size()) + " socktb->tc.snd_ack: " + lg.c2s(st->tc.rcv_nxt) +
			" size of payload: " + lg.c2s(payload->size()),TOULOG_TIMER);
    ms = getCurMs () + s->tc.t_timeout;
  };

	/**
	 * node_t (conn_id c, time_id t, seq_id p, sockTb * s, char *pl, long time)
	 * constructor which inits a new timer node.
	 */
	node_t (conn_id c, time_id t, seq_id p, sockTb * s, std::string * pl, long time):
	c_id (c), t_id (t), p_id (p), st(s){
    payload = new std::string (*pl);
		lg.logData ("timer node built ..sockfd is: " + lg.c2s(c) + " sizeof payload is :"
      + lg.c2s(payload->size()) + " socktb->tc.snd_ack: " + lg.c2s(st->tc.rcv_nxt) +
			" size of payload: " + lg.c2s(payload->size()),TOULOG_TIMER);
    ms = time;
  };

  ~node_t (){
    //delete payload; 
  };
  conn_id c_id;
  time_id t_id;
  seq_id p_id;
  sockTb *st;
  std::string *payload;
  long ms;
};

/**
 * heapComp: timer heap tree comparison 
 * compares the lhs and rhs of the heap node
 */
class heapComp{
public:
  bool operator () (const node_t & lhs, const node_t & rhs) const{
    if ((lhs.ms == rhs.ms)){
			if (lhs.c_id == rhs.c_id){
	    return (lhs.p_id >= rhs.p_id);	/* NOTE Rotation XXXX */
			}
			return ((lhs.c_id >= rhs.c_id));
    }else{
			return (lhs.ms >= rhs.ms);
    }
  }
};

/* typedef tiemr heap tree */
typedef std::priority_queue <node_t,std::vector <node_t>,heapComp> minTmHeapType;
/* typedef timer deletion deque */
typedef std::deque <node_t>timerDequeType;
/* tiemr heap tree, which is implemented in priority_queue(min-heap) */
extern minTmHeapType timerheap;
/* timer deletion deque, which is implemented in deque */
extern timerDequeType timerdeque;

/**
 * timerCk: is timer thread that running at the beginning of program
 * it's operated by timerMng
 */
class timerCk{
public:
  /**
	 * timerCk(): constructor
   * initiate a new thread
   */
  timerCk ():
  m_thread (boost::bind (&timerCk::doit, this)){
		lg.logData("*** timerCk built ***", TOULOG_TIMER);
  };

  /** 
	 * ~timerCk (): deconstructor
   * follow RAII designed pattern: recycling and self-managment
   */
  ~timerCk (){
		lg.logData("*** timerCk recycled ***", TOULOG_TIMER);
    m_thread.join ();
  };

  /**
	 * addDelNode(conn_id cid, time_id tid, seq_id pid): 
   * this function creates new deletion_node and add newly generated
   * node into tdv queue
	 * return on successful addition
   */
  bool addDelNode (conn_id cid, time_id tid, seq_id pid){
    nt = new node_t (cid, tid, pid);
    tdv.push_back (*nt);
    return true;
  }

  /**
	 * ckTimerDel (conn_id cid, time_id tid, seq_id pid):
	 * check the tdv queue to see whether there exists a deletion node
   * if find one, delete the pair
   * @result true if the node is found
   */
  inline bool ckTimerDel (conn_id cid, time_id tid, seq_id pid){
    for (it = tdv.begin (); it < tdv.end (); it++){
			if ((it->c_id == cid) && (it->t_id == tid) && (it->p_id == pid)){
	    tdv.erase (it);
	    return true;
			}
    }//end of for
    return false;
  }

  /**
	 * ckTimer (conn_id cid, time_id tid, seq_id pid):
	 * check the vector whether there exists an timer
   * @result true if node is found
   */
  bool ckTimer (conn_id cid, time_id tid, seq_id pid){
    for (it = tdv.begin (); it < tdv.end (); it++)
    {
			if ((it->c_id == cid) && (it->t_id == tid) && (it->p_id == pid))
				return true;
    }//end of for
    return false;
  }

	/**
	 * rexmit_for_dup_ack(conn_id cid, time_id tid, seq_id pid):
	 */
	bool rexmit_for_dup_ack(conn_id cid, time_id tid, seq_id pid);

private:
  void doit();
  node_t							*nt;
  struct sockaddr_in	sockaddrs;
  int assignaddr (struct sockaddr_in *sockaddr, sa_family_t sa_family,
		std::string ip, unsigned short port);

  /* timer deletion queue
   * nodes here as notification of given nodes are deleted. So while timer
   * has an expired node finds there already exists an deletion node here.
   * timer pops the expired node without further reaction.
   */
  std::vector < node_t > tdv;
  std::vector < node_t >::iterator it;

  /* timer thread */
  boost::thread m_thread;
};

/**
 * timerMng:
 * timerMng is responsible for timer management. All the operation related to
 * timer should be operated through timerMng
 */
class timerMng {
public:
  timerMng ();
  ~timerMng (){
  };

  bool add (conn_id cid, time_id tid, seq_id pid, sockTb * st,
		std::string * payload);
  bool delete_timer (conn_id cid, time_id tid, seq_id pid);
  bool reset (conn_id cid, time_id tid, seq_id pid);
  bool reset (conn_id cid, time_id tid, long ms, seq_id pid);
  bool ck_del_timer (conn_id cid, time_id tid, seq_id pid);
	bool rexmit_for_dup_ack(conn_id cid, time_id tid, seq_id pid);

  bool deleteall ();
  bool resetall ();

private:
  node_t * timernode;
  timerCk * timercker;			//thread for cking next-fired timer
};
