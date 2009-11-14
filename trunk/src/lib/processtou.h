/*********************************************************
 * processtou.h
 * io management for receive
 * 
 * ******************************************************/
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bimap.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/timer.hpp>
#include <boost/system/system_error.hpp>
extern boost::mutex recvqmutex;
extern boost::mutex socktabmutex1;

class processTou {
  public:
    processTou(int sd)
    {
      run(sd);
//      std::cout<< "*** process tou recv is running now ***\n";
      sockfd = sd;
    }
    ~processTou(){
//      std::cout << "*** process tou recv is recycled *** " << std::endl;
    }

    //void pushsndq(touheaderack *touack);
    //pushrecvq(touheaderack *touack);

  private:
    //std::queue<struct touheaderack>		sndq;
    //std::queue<struct touheaderack>		iterator;
    int						sockfd;
    //touheaderack			 	touhdack;	
    struct sockaddr_in				sockaddr;
    
    /* error return: 0
     * success return: 1 */
    //int assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, char *ip, char *port); /* get the sockaddr_in infomation */
    //void popsndq(tou);
    void run(int );
    


};
