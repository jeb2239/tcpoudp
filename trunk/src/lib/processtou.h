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

/* compare the pkt by it's sequence number */
/*class heapPkgComp {
	public:
    bool operator() (const touPkg& lhs, const touPkg& rhs) const {
			return (lhs.t.ack_seq >= rhs.t.ack_seq);
		}
};

typedef std::priority_queue<touPkg, std::vector<touPkg>, heapPkgComp> minPkgHeapType;
*/

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
		processTou(int sd, sockMng *sockmng, touPkg *toupkg){
			std::cout<< "*** processTou(int sd, sockMng *sm) is activized now ***\n";
			sockfd = sd;
			tp = toupkg;
			sm = sockmng;
			recovery = false;
			sndack = true;
      //run(sockfd);
		}
    ~processTou(){ 
			 std::cout << "*** processTou is recycled *** " << std::endl;
		}

    void run(int );
		void send(int sockfd);
  private:
    int										sockfd;
		sockMng								*sm;
	  sockTb								*socktb;
		touPkg								*tp;
    struct sockaddr_in		sockaddrs;
		bool									recovery;     //recovery mode or not
		bool									sndack;

		int popsndq(sockTb *socktb, char *sendbuf, int len);
		int putcircbuf(sockTb *socktb, int sockfd, char *buf, int rvpl);
		int assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, std::string ip, unsigned short port);
};
