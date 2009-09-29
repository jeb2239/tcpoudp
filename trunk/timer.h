/*************************************************************************
 *  * Here configure the Project inclusions, definitions, and variables 
 *************************************************************************/
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bimap.hpp>
/*#include <boost/filesystem.hpp>*/
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/timer.hpp>
#include <boost/system/system_error.hpp>

class aTimer {
  public:
    aTimer(){}
    aTimer(unsigned int id_, long ms_) 
      : id(id_), 
        ms(ms_), 
	m_thread(boost::bind(&aTimer::doit,this, id_, ms_)){
    };
    ~aTimer(){
      std::cout << "*** atimer dying... *** "<<this <<" "<<&m_thread<< std::endl;
      m_thread.interrupt();
      /*m_thread.join(); // recycle till execution finished */
      m_thread.timed_join( boost::posix_time::milliseconds(0) );
    };

  private:
    void prepareDoit();
    void doit(unsigned int id, long ms);
    void action(unsigned int id, long ms);

    boost::thread m_thread;
    boost::mutex m_mutex;
    boost::thread_specific_ptr<unsigned int> thread_id; //thread specified id var.
    boost::thread_specific_ptr<long> thread_ms; 
    unsigned int id;
    long ms;
};

typedef boost::bimap<unsigned int, aTimer*> timerBimapType;
typedef timerBimapType::value_type timerPair;
typedef timerBimapType::const_iterator timerIter;
class timerMng {
  public:
    timerMng();
    ~timerMng(){};
    bool add(unsigned int id,long ms);
    bool delete_timer(unsigned int id);
    bool reset(unsigned int id);
    bool reset(unsigned int id,long ms);
    bool deleteall();
    bool resetall();

  private:
    void stArg(unsigned int id, long ms);
   // static void* timerEntryPt(void*);
   // int timerRun(void* arg);
    aTimer *atimer;
    timerBimapType timerbimap;
    timerIter timeriter;
    unsigned int id;
    long ms;
};
