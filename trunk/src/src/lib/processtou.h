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
#include <string>
extern boost::mutex recvqmutex;
extern boost::mutex socktabmutex1;

class processTou {
  public:
    processTou() {
			std::cerr<< "Should not call processTou without arguments!\n";
		}
    processTou(int sd){
			std::cout<< "*** processTou(int sd) is activized now ***\n";
      sockfd = sd;      
      //run(sockfd);
      
    }
		processTou(int sd, sockMng *sockmng){
			std::cout<< "*** processTou(int sd, sockMng *sm) is activized now ***\n";
			sockfd = sd;
			sm = sockmng;
      //run(sockfd);
		}
    ~processTou(){ 
			 std::cout << "*** processTou is recycled *** " << std::endl;
		}

    void run(int );
  private:
    int										sockfd;
		sockMng								*sm;
	  sockTb								*socktb;
    struct sockaddr_in		sockaddr;

		void send(int sockfd);
		int popsndq(sockTb *socktb, char *sendbuf, int len);
		int assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, std::string ip, unsigned short port);
};