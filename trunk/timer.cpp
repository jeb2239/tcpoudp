#include "timer.h"

/*****************************************************************
 * Buggy code, pending.
 * **************************************************************/
void aTimer::prepareDoit(){
  //boost::mutex::scoped_lock lock(m_mutex);
  if (thread_id.get() == NULL) thread_id.reset(new unsigned int(id));
  if (thread_ms.get() == NULL) thread_ms.reset(new long(ms));
}

void aTimer::action(unsigned int id_, long ms_){ 
  //std::cout<< "Timer ID:"<<*thread_id<<" fired. Duration: "<<*thread_ms<<"\n";
  boost::this_thread::interruption_point();
  std::cout<< "Timer ID:"<<id_<<" fired. Duration: "<<ms_<<"\n";
}

void aTimer::doit(unsigned int id_, long ms_){
  unsigned int id__ = id_;
  long ms__ = ms_;

  //mutex the var, and assign current var, id and ms, to thread var
  //prepareDoit();
  boost::asio::io_service io;
  boost::asio::deadline_timer t(io, boost::posix_time::milliseconds(ms__));
  t.async_wait(boost::bind(&aTimer::action, this, id__, ms__));
  io.run();
  //m_thread.detach();
}
