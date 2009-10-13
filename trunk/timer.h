/*************************************************************************
 *  * Here configure the Project inclusions, definitions, and variables 
 *************************************************************************/
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <deque>
#include <time.h>
#include <sys/time.h>
#include <functional>
#include <queue>
#include <vector>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bimap.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/timer.hpp>
#include <boost/system/system_error.hpp>

typedef unsigned int conn_id;
typedef unsigned int time_id;
typedef unsigned int packet_id;

extern boost::mutex timermutex;
long getCurMs();
//node of the heap with initialization
class node_t {
  public:
    node_t(){};
    node_t(conn_id c, time_id t,packet_id p)
      :c_id(c), t_id(t), p_id(p) {};
    node_t(conn_id c, time_id t, long m, packet_id p)
      :c_id(c), t_id(t), ms(m), p_id(p) {};
    ~node_t(){};

    conn_id	c_id;
    time_id	t_id;
    long	ms;
    packet_id   p_id;
};
//compares the lhs and rhs of the heap 
//HERE ADD PID AND CID FOR MULTIPLE CONNECTION
class heapComp {
  public:
    bool operator() (const node_t& lhs, const node_t& rhs) const {
      //return (lhs.ms >= rhs.ms);
    if((lhs.ms==rhs.ms)) {
          return((lhs.c_id > rhs.c_id));
    }
    else 
	return (lhs.ms >= rhs.ms);
     }
};

typedef std::priority_queue<node_t, std::vector<node_t>, heapComp> minTmHeapType;
typedef std::deque<node_t> timerDequeType;

extern minTmHeapType timerheap;		// STL, Priority_queue(min-heap)
extern timerDequeType timerdeque;	// STL, Queue

class timerCk {
  public:
    timerCk() 
      :m_thread(boost::bind(&timerCk::doit,this))
    {
      std::cout << "-" << timerheap.empty() <<"-";
      std::cout << "*** timerCk built *** " << std::endl;	  
    };

    ~timerCk(){
      std::cout << "*** timerCk recycled *** " << std::endl;
      m_thread.join(); // RAII designed pattern, recycling self-managment
};
//create new node and push it in the vector
    bool addDelnode(conn_id cid, time_id tid, packet_id pid)
    {
      nt = new node_t(cid, tid, pid);
      tdv.push_back(*nt);
      return true;
    }


//check the vector if match found, delete the pair
    inline bool ckTimerDel(conn_id cid, time_id tid, packet_id pid)
    {
      for(it=tdv.begin(); it<tdv.end(); it++) {
	if ((it->c_id == cid) && (it->t_id == tid) &&(it->p_id == pid)) {
	  tdv.erase(it);
	  return true;
	}
      }//end of for
      return false;
    }

  private:
    /*
    void prepareDoit();
    void action();
    */
    void doit();
    node_t *nt;
    std::vector<node_t> tdv;
    std::vector<node_t>::iterator it;
    boost::thread m_thread;

};

class timerMng {
  public:
    timerMng();
    ~timerMng(){};
    bool add(conn_id cid, time_id tid, long ms, packet_id pid);
    bool delete_timer(conn_id cid, time_id tid, packet_id pid);
    bool reset(conn_id cid, time_id tid, packet_id pid);
    bool reset(conn_id cid, time_id tid, long ms, packet_id pid);
    bool deleteall();
    bool resetall();

  private:
    node_t *timernode;
    timerCk *timercker;	//thread for cking next-fired timer
};
