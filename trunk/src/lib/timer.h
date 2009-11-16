/************************************************************************
 * timer.h
 * Here configure the Project inclusions, definitions, and variables 
 *************************************************************************/
#include "touSockTable.h"
#include "touHeader.h"

#include <iostream>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <string>
#include <sys/time.h>
#include <functional>
#include <queue>
#include <deque>
#include <vector>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/timer.hpp>

/* time that timer would wait for each cking */
#define TIMER_WT	500

/* associate with sock file descriptor */
typedef unsigned int conn_id;
/* associate with connection sequecne number */
typedef unsigned long seq_id;
/* timer id, reserved */
typedef unsigned int time_id;

extern boost::mutex timermutex;
long getCurMs();

/* node of the heap with initialization
 * the data and seq are sotred in this node too */
class node_t {
  public:
    node_t(){
		  std::cerr<<"shouldn't use  node_t()";
		};
    node_t(conn_id c, time_id t,seq_id p)
      :c_id(c), t_id(t), p_id(p) {};
    node_t(conn_id c, time_id t, seq_id p, long m)
      :c_id(c), t_id(t), p_id(p), ms(m) {
			std::cerr<<"shouldn't use  node_t with specifying ms";};
		node_t(conn_id c, time_id t, seq_id p, sockTb *s, char *pl)
			:c_id(c), t_id(t), p_id(p), st(s) {
			payload = new char[strlen(pl)];
      std::cout << "timer node built ..sockfd is: "<<c<<" sizeof payload is : " << strlen(pl) <<std::endl;
			strncpy(payload, pl, strlen(pl));
			ms = getCurMs() + s->tc.t_timeout;
		};

    ~node_t(){ /*delete payload;*/ };

    conn_id	c_id;
    time_id	t_id;
    seq_id	p_id;
		sockTb *st;
		char *payload;
		long  ms;
};

/* compares the lhs and rhs of the heap 
 * HERE ADD CID(connection id - sockfd) and PID
 * (pkt sequence number) FOR MULTIPLE CONNECTIONS */
class heapComp {
  public:
    bool operator() (const node_t& lhs, const node_t& rhs) const {
		  if((lhs.ms==rhs.ms)) {
		    return((lhs.c_id > rhs.c_id));
		  }else{
		    return (lhs.ms >= rhs.ms);
		  }
    }
};

typedef std::priority_queue<node_t, std::vector<node_t>, heapComp> minTmHeapType;
typedef std::deque<node_t> timerDequeType;

extern minTmHeapType timerheap;		// STL, Priority_queue(min-heap)
extern timerDequeType timerdeque;	// STL, Queue

class timerCk {
  public:
    timerCk() 
      :m_thread(boost::bind(&timerCk::doit,this)){
      std::cout << "*** timerCk built *** " << std::endl;	  
    };

    ~timerCk(){
      std::cout << "*** timerCk recycled *** " << std::endl;

			//delete nt;
      m_thread.join(); // RAII designed pattern, recycling self-managment
		};

		//create new node and push it in the vector
    bool addDelNode(conn_id cid, time_id tid, seq_id pid){
      nt = new node_t(cid, tid, pid);
      tdv.push_back(*nt);
      return true;
    }

		//check the vector if match found, delete the pair
    inline bool ckTimerDel(conn_id cid, time_id tid, seq_id pid){
      for(it=tdv.begin(); it<tdv.end(); it++) {
				if ((it->c_id == cid) && (it->t_id == tid) &&(it->p_id == pid)) {
					tdv.erase(it);
					return true;
				}
      }//end of for
      return false;
    }

  private:
    void doit();
    node_t *nt;
	  struct sockaddr_in sockaddrs;
		int assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, std::string ip, unsigned short port);

		/* timer deletion queue, node in here needs to be used as registered node to
		 * revoke the node in priority_queue while timer node in priority_queue is fired */
    std::vector<node_t> tdv;
    std::vector<node_t>::iterator it;

		/* timer thread */
    boost::thread m_thread;
};

class timerMng {
  public:
    timerMng();
    ~timerMng(){ /*delete timercker;*/ };
    bool add(conn_id cid, time_id tid, seq_id pid, long ms);
    bool add(conn_id cid, time_id tid, seq_id pid, sockTb *st, char *payload);
    bool delete_timer(conn_id cid, time_id tid, seq_id pid);
    bool reset(conn_id cid, time_id tid, seq_id pid);
    bool reset(conn_id cid, time_id tid, long ms, seq_id pid);
    bool deleteall();
    bool resetall();

  private:
    node_t *timernode;
    timerCk *timercker;	//thread for cking next-fired timer
};
