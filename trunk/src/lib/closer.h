
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
#include <ctype.h>
#include <sys/time.h>
#include <fcntl.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/timer.hpp>

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
class closer {
  public:
      int fdc[2];
      int fds[2];
   /*closer() 
      :m_thread(boost::bind(&closer::dothis,this)){
     
      std::cout << "*** closer built *** " << std::endl;
      pipetest = pipe(fdc);
      if (pipetest >= 0) cout << "Pipe created successfully " << endl;
      pipetest = pipe(fds);
    }*/
    
    closer() {
      std::cout << "*** closer built *** " << std::endl;
    /*void clientClose(int sd);
    void serverClose(int sd); 
    void dothis(); */
 //   go();
    int sd;
   
    }

    void clientClose(int sd);
    void serverClose(int sd); 
    void dothis(); 
    void go();
    
    ~closer(){
      std::cout << "*** closer recycled *** " << std::endl;
      m_thread->join(); 
		}


   
  private:
    int pipetest;
  //  node_c *nt;
//    boost::shared_ptr<boost::thread> m_thread;

};


class closer1 {
 
public : 
  closer closerr;
  closer1() {
   // closerr = new closer();
    boost::thread thr1( boost::bind( &closer::dothis, &closerr ) );

  //  closerr->go();
  }
};
