/******************************************************************************
 * timer.h: define timer related classes
 * declare the timer node: node_t
 * declare the timer management: timerMng
 * declare the timer thread: timerCk
 *
 * Copyright 2009 by Columbia University; all rights reserved 
 * Jan 09, 2010
 *****************************************************************************/


#include "tou_sock_table.h"
#include "tou_logger.h"
#include "processtou.h"

/* Time(ms) that timer should wait for each iteration in loop in doit() 
 * func. The reason of adopting 100ms is because the minimum interval of
 * delayed ACK, which is TIMER_DELAYED_ACK_TIME, is 200ms. */	
#define TIMER_WT_TIME			100	//ms
#define TIMER_WT_TIME_RUN		10	//ms

/* RFC1122 specifies the delay MUST be less than 500ms, and in a stream 
 * of full-sized segments there SHOULD be an ACK for at least every sec
 * segment. This delayed time mechanism is known as Nagle's algorithm. 
 * However, the dedault setting of the delayed option in Unix systems
 * adopts 200ms. And as to the "every second" constrain, I use 500 ms*/
#define TIMER_DELAYED_ACK_TIME	200	//ms
#define TIMER_PROMPT_ACK_TIME	500 //ms

/* timer id definition */
#define TIMER_ID_DELACK			10
#define TIMER_ID_SNDPKT			20

/* fired timer para. definition */
typedef unsigned long conn_id;	// type for socket file descriptor
typedef unsigned long seq_id;	// sequence number
typedef unsigned int time_id;	// DELACK: delayed ack, SNDPKT: pkt snet


/**
 * getTimeMs: 
 * get the current time in the format of millisecond
 * return the current time in type of long
 */
unsigned long getTimeMs ();

/**
 * node_t: 
 * Represents timer node which should be used in timer heap tree.
 */
class node_t{
public:
	node_t(); //discarded
	~node_t (){};

	/*
	 	copy constructor
	 */
	node_t (const node_t &nt);
	node_t (const node_t &nt, u_long fired_t);
	node_t (conn_id c, time_id t, seq_id p); //discarded

	/*
		1.used with lost packet
		2.used with lost packet
		3.used with lost packet with specified fired time
	*/
	node_t (conn_id c, time_id t, seq_id p, sockTb * s, char *pl);
	node_t (conn_id c, time_id t, seq_id p, sockTb * s, std::string * pl);
	node_t (conn_id c, time_id t, seq_id p, sockTb * s, std::string * pl, long time);

	/*
		used with ack 
	*/
	node_t (conn_id c, time_id t, seq_id p, long pms, long nms, sockTb *s);
	//node_t (conn_id c, time_id t, seq_id p, long ms_to_fire, sockTb *s);

	
	/*
		assign(copy) the timer node
	*/
	void operator=(const node_t &nt);

	conn_id c_id;		//socket fd(sock id)
	time_id t_id;		//timer id
	seq_id p_id;		//sequence number
	sockTb *st;			//socket table pt
	std::string *payload;
	long ms;			//expected fired time (ms)
	long xmitms;		//xmit time (ms)

	/* used in ack */
	long push_ackms;	//must-send-an-ack time (ms)
	long nagle_ackms;	//delayed ack time (ms)

};

/**
 * heapComp: timer heap tree comparison 
 * compares the lhs and rhs of the heap node
 */
/*
class heapComp{
public:
  bool operator () (const node_t & lhs, const node_t & rhs) const{
    if ((lhs.ms == rhs.ms)){
			if (lhs.c_id == rhs.c_id){
			return (lhs.p_id >= rhs.p_id);	// seq rtation
			}
			return ((lhs.c_id >= rhs.c_id));
    }else{
			return (lhs.ms >= rhs.ms);
    }
  }
};
*/
class heapComp{
  public:
	  bool operator () (const node_t & lhs, const node_t & rhs) const{
		  if (lhs.c_id == rhs.c_id) {
			  return (lhs.p_id >= rhs.p_id);
		  }else {
			  return (lhs.c_id >= rhs.c_id);
		  }
	  }
};

/* timer heap tree(minTmHeapType) is used to store timer node(pkt data)
 * ack deque(timerDequeType) is used to store ack timer node(ack), and
 * they have mutexs timer_heap_mtx and ack_deque_mtx, respectively. */
typedef std::priority_queue <node_t,std::vector <node_t>,heapComp> minTmHeapType;
typedef std::deque <node_t> timerDequeType;
extern minTmHeapType timerheap;
extern timerDequeType ackdeque;
extern boost::mutex timer_heap_mtx; //to lock timer heap(timerheap)
extern boost::mutex ack_deque_mtx; //to lock ack deque(ackdeque)
extern boost::mutex ack_mtx; //before use stb_tc_ack_mtx & ack_deque_mtx

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

	  bool timerheap_empty();
	  bool ackdeque_empty();

	  /**
	   * addDelNode(conn_id cid, time_id tid, seq_id pid):
	   * this function creates new deletion_node and add newly generated
	   * node into tdv queue
	   * return on successful addition
	   */
	  //bool addDelNode (conn_id cid, time_id tid, seq_id pid);

	  /**
	   * check the tdv queue to see whether there exists a deletion node
	   * if find one, delete the pair
	   * @result true if the node is found
	   */
	  //bool ckTimerDel (const node_t &nt);

	  /**
	   * ckTimer (conn_id cid, time_id tid, seq_id pid):
	   * check the vector whether there exists an timer
	   * @result true if node is found
	   */
	  //bool ckTimer (conn_id cid, time_id tid, seq_id pid);

	  /**
	   * ca_rto
	   * calculate the RTT and RTO
	   */
	  void cal_rto (const node_t &nt, u_long recv_t);
	  void cal_backoff_rto(const node_t &nt);

	  /**
	   * ckTimerheap
	   */
	  bool ckTimerheap();

	  /**
	   * rexmit_for_dup_ack(conn_id cid, time_id tid, seq_id pid):
	   */
	  bool rexmit_for_dup_ack(conn_id cid, time_id tid, seq_id pid);

	  void proc_clear_ackedpkt(conn_id cid, time_id tid, 
							   seq_id una_seq, u_long recv_t);

  private:
	  /*
		 A periodic loop function for checking fired timer in timer thread
	   */  
	  void doit();

	  /*
		 when there's a delayed ACK exceeds 200ms, ToU needs to xmit the given delayed
		 ACK immediately.
		 process the fired packet(rexmit the packet) 
	   */
	  void proc_delack();
	  void proc_sndpkt();
	  void proc_clear_ackedpkt();

	  /*
		 subfunction to deal with sending duplicate packet (used by proc_sndpkt)
		 subfunction to deal with sending duplicate ackpkt (used by proc_delack_xmit)
	   */
	  int sendpkt (const node_t &nt);
	  void sendack (const node_t &nt);

	  /*
		 used by proc_delack()
	   */
	  void update_ack(node_t *n, u_long cur_t);
	  void proc_delack_xmit(timerDequeType::iterator itr, u_long t);

	  /*
		 postpone the every timer node with assigned time whose conditions satisfy
		 given timer id and socket file describtor
	   */
	  void postpone_timer(conn_id th_conn, time_id timer_id, seq_id th_seq, u_long rto);


	  sockTb* getSocketTable(int conn_id);
	  int assignaddr (struct sockaddr_in *sockaddr, sa_family_t sa_family,
					  std::string ip, unsigned short port);


	  node_t				*nt;
	  sockTb				*socktb;

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
	  ~timerMng (){};

	  bool add (conn_id cid, time_id tid, seq_id pid, sockTb * st,
				std::string * payload);

	  bool add_ack (conn_id cid, time_id tid, seq_id pid,int deltime, sockTb *st);
	  void add_ack(conn_id cid, time_id tid, sockTb *stb);
	  void update_ack(conn_id cid, time_id tid, short state, sockTb *stb);
	  node_t* get_ack_node(conn_id cid, time_id tid);

	  void delete_timer (conn_id cid, time_id tid, seq_id pid);
	  bool reset (conn_id cid, time_id tid, seq_id pid);
	  bool reset (conn_id cid, time_id tid, long ms, seq_id pid);
	  bool ck_del_timer (conn_id cid, time_id tid, seq_id pid);
	  bool rexmit_for_dup_ack(conn_id cid, time_id tid, seq_id pid);
	  bool deleteall ();
	  bool resetall ();

  private:
	  node_t *timernode;
	  timerCk *timercker; //thread for cking next-fired timer
};


extern timerMng tmng;	//for timer node

