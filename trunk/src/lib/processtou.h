/*********************************************************
 * processtou.h
 * sending/receiving via circular buffer for tou 
 * send and recv function.
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

#define PROCESS_END 0
#define PROCESS_SYN 1
#define PROCESS_FIN 2
#define PROCESS_ACK_WITHOUT_DATA 3
#define PROCESS_ACK_WITH_DATA_MATCH_EXPECTED_SEQ 4
#define PROCESS_ACK_WITH_DATA_LESS_EXPECTED_SEQ 5
#define PROCESS_ACK_WITH_DATA_MORE_EXPECTED_SEQ 6
#define PROCESS_ACK_DATARECSUCC_SENDBACK_ACK 7

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
    }
		/**
		 * contructor
		 * should adopt this constructor in updated code 
		 */
		processTou(int sd, sockMng *sockmng, touPkg *toupkg){
			std::cout<< "*** processTou(int sd, sockMng *sm) is activized now ***\n";
			sockfd = sd;
			sm = sockmng;
		}
		/**
		 * destructor
		 */
    ~processTou(){ 
			 std::cout << "*** processTou is recycled *** " << std::endl;
		}

		/**
		 * receiving pkt from underlying layer and handle the pkt
		 * 1. update socket table 
		 * 2. put pkts into circular buffer
		 * 3. recovery duplicated pkts
		 */
    void run(int );

		/**
		 * sending pkt from circular buffer to network
		 */
		void send(int sockfd);
  private:
    int										sockfd;
		sockMng								*sm;
	  sockTb								*socktb;
		//touPkg								*tp;
    struct sockaddr_in		sockaddrs;

		int popsndq(sockTb *socktb, char *sendbuf, int len);
		int putcircbuf(sockTb *socktb, int sockfd, std::string  *buf, int rvpl);
		int processGetPktState(touPkg *pkt);
		int assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, std::string ip, unsigned short port);
		unsigned long getSendSize(sockTb *socktb);
};
