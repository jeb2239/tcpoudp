#include "timer.h"

/*********************************************
 * Initialize the Queue and Map 
 * ******************************************/
timerMng::timerMng()
  : id(0), 
    ms(0)
{
  std::cout << "*** timerMng built ***\n";
}
/*********************************************
 * 1. Reg a key pair into the map
 * 2. Init a timer in new thread
 ********************************************/
bool timerMng::add(unsigned int id, long ms){
  stArg(id, ms);
  //New a timer object. It triggered the timer thread as well
  atimer = new aTimer(id, ms);
  //Insert the id and *timer into bimap for future operation
  timerbimap.insert( timerPair(id, atimer) );
  std::cout << "atimer(ID:"<<id<<") is added. "<<atimer<<std::endl;
}
/*********************************************
 * 1. Delete the timer by specified id
 * ******************************************/
bool timerMng::delete_timer(unsigned int id){
  std::cout << "Killing TIMER ID: "<<id<<"; OBJ: "<<timerbimap.left.at(id)<<std::endl;
  delete timerbimap.left.at(id);
  timerbimap.left.erase(id);
  std::cout << "Killing BIMAP PAIR ID: "<<id<<std::endl;
  /*timeriter = timerbimap.left.find(id);*/
}
bool timerMng::reset(unsigned int id){

}
bool timerMng::reset(unsigned int id, long ms){

}
bool timerMng::deleteall(){

}
bool timerMng::resetall(){

}
void timerMng::stArg(unsigned int id_, long ms_){
  id = id_;
  ms = ms_;
}
/*
void * timerMng::timerEntryPt(void *ms){
  struct timerData timerdata;
  timerdata.id = 345;
  timerdata.elapsed = (unsigned long) ms;

  pthread_detach(pthread_self());
  aTimer timer(timerdata.id, timerdata.elapsed);
}
*/
