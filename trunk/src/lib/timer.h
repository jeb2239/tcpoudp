/******************************************************************************
 * timer.h
 * Here configure the Project inclusions, definitions, and variables 
 *****************************************************************************/
#include "touSockTable.h"
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <functional>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/timer.hpp>

/* time that timer would wait for each cking loop*/
#define TIMER_WT	500

/* type ofr socket file descriptor */
typedef unsigned int conn_id;
/* type for connection sequence number */
typedef unsigned long seq_id;
/* timer id, reserved */
typedef unsigned int time_id;
extern boost::mutex timermutex;

/**
 * return the current time in long 
 */
long getCurMs();

/**
 * node_t is a timer node which is stored in timer heap tree
 * it contains conn_id: socket file descriptor
 *						 timer_id: reserved
 *						 seq_id: connection sequence number
 *						 pl: payload
 */
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
		  payload = new std::string(pl);
      std::cout << "timer node built ..sockfd is: "<<c<<" sizeof payload is : " 
				<< payload->size() <<" socktb->tc.snd_ack: "<<st->tc.rcv_nxt <<" size of payload: "<< payload->size() << std::endl;
			ms = getCurMs() + s->tc.t_timeout;
			};
		node_t(conn_id c, time_id t, seq_id p, sockTb *s, std::string *pl)
			:c_id(c), t_id(t), p_id(p), st(s) {
		  payload = new std::string(*pl);
      std::cout << "timer node built ..sockfd is: "<<c<<" sizeof payload is : " 
				<< payload->size() <<" socktb->tc.snd_ack: "<<st->tc.rcv_nxt <<" size of payload: "<< payload->size() << std::endl;
			ms = getCurMs() + s->tc.t_timeout;
			};

		~node_t(){ 
			//delete payload; 
		};

    conn_id	c_id;
    time_id	t_id;
    seq_id	p_id;
		sockTb *st;
		std::string *payload;
		long  ms;
};

/**
 * heapComp for timer heap tree comparison 
 * compares the lhs and rhs of the heap node
 */
class heapComp {
  public:
    bool operator() (const node_t& lhs, const node_t& rhs) const {
		  if((lhs.ms==rhs.ms)) {
				if(lhs.c_id == rhs.c_id){
					return (lhs.p_id >= rhs.p_id);	/* NOTE Rotation XXXX*/
				}
		    return((lhs.c_id >= rhs.c_id));
		  }else{
		    return (lhs.ms >= rhs.ms);
		  }
    }
};

/* typedef tiemr heap tree */
typedef std::priority_queue<node_t, std::vector<node_t>, heapComp> minTmHeapType;
/* typedef timer deletion deque */
typedef std::deque<node_t> timerDequeType;
/* tiemr heap tree, which is implemented in priority_queue(min-heap) */
extern minTmHeapType timerheap;
/* timer deletion deque, which is implemented in deque */
extern timerDequeType timerdeque;

/**
 * timerCk is timer thread that running at the beginning of program
 * it's operated by timerMng
 */
class timerCk {
  public:
		/* constructor
		 * initiate a new thread
		 */
    timerCk() 
      :m_thread(boost::bind(&timerCk::doit,this)){
      std::cout << "*** timerCk built *** " << std::endl;	  
    };

		/* deconstructor
		 * follow RAII designed pattern: recycling and self-managment
		 */
    ~timerCk(){
      std::cout << "*** timerCk recycled *** " << std::endl;
      m_thread.join();
		};

		/* addDelNode
		 * this function create new deletion node and add newly generated
		 * node into tdv queue
		 */
    bool addDelNode(conn_id cid, time_id tid, seq_id pid){
      nt = new node_t(cid, tid, pid);
      tdv.push_back(*nt);
      return true;
    }

		/* check the tdv queue to see whether there exist a deletion node
		 * if find one, delete the pair
		 * @result true if the node is found
		 */
    inline bool ckTimerDel(conn_id cid, time_id tid, seq_id pid){
      for(it=tdv.begin(); it<tdv.end(); it++) {
				if ((it->c_id == cid) && (it->t_id == tid) &&(it->p_id == pid)) {
					tdv.erase(it);
					return true;
				}
      }//end of for
      return false;
    }

		/* check the vector whether there exists an timer
		 * @result true if node is found
		 */
		bool ckTimer(conn_id cid, time_id tid, seq_id pid){
      for(it=tdv.begin(); it<tdv.end(); it++) {
				if ((it->c_id == cid) && (it->t_id == tid) &&(it->p_id == pid))
					return true;
      }//end of for
      return false;
    }

  private:
    void doit();
    node_t *nt;
	  struct sockaddr_in sockaddrs;
		int assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, std::string ip, unsigned short port);

		/* timer deletion queue
		 * nodes here as notification of given nodes are deleted. So while timer
		 * has an expired node finds there already exists an deletion node here.
		 * timer pops the expired node without further reaction.
		 */
    std::vector<node_t> tdv;
    std::vector<node_t>::iterator it;

		/* timer thread */
    boost::thread m_thread;
};

/**
 * timerMng
 * timerMng is responsible for timer management. All the operation related to
 * timer should be operated through timerMng
 */
class timerMng {
  public:
    timerMng();
    ~timerMng(){};
    bool add(conn_id cid, time_id tid, seq_id pid, long ms);
    bool add(conn_id cid, time_id tid, seq_id pid, sockTb *st, std::string *payload);
    bool delete_timer(conn_id cid, time_id tid, seq_id pid);
    bool reset(conn_id cid, time_id tid, seq_id pid);
    bool reset(conn_id cid, time_id tid, long ms, seq_id pid);
		bool ck_del_timer(conn_id cid, time_id tid, seq_id pid);
    bool deleteall();
    bool resetall();

  private:
    //node_t *timernode;
    timerCk *timercker;	//thread for cking next-fired timer
};

