/******************************************************************************
 * processtou.h
 * Processtou is a class that responsible for handling most of the I/O
 * operations. The sending/receiving via circular buffers.
 * ***************************************************************************/

#include "tou_comm.h"

#define RECVBUFSIZE     2000 //size of the buffer used in receiving pkt from NW
#define MIN_SEND_SIZE	1400 //size of playload that can ba sent

#define ACK_BIGTHAN_UNA	1
#define ACK_SMLTHAN_UNA	-1
#define ACK_EQUALTO_UNA 0

class processTou {
  public:
		/**
		 * Default constructor
		 */
		processTou() {
				std::cerr<< "Should not call processTou without arguments!\n";
		}
		/**
		 * Legacy constructor
		 */
		processTou(int sd){
				std::cout<< "*** processTou(int sd) is activized now ***\n";
				sockfd = sd;
		}

		/**
		 * Constructor
		 * should adopt this constructor in updated code 
		 */
		processTou(int sd, sockMng *sockmng): sockfd(sd), sm(sockmng) {

		}
		
		/**
		 * destructor
		 */
		~processTou(){}

		/**
		 * receiving pkt from underlying layer and handle the pkt
		 * 1. update socket table 
		 * 2. put pkts into circular buffer
		 * 3. recovery duplicated pkts
		 * return on number bytes received or -1 on fail to receive
		 */
		int run(int sockfd);

		/**
		 * sending pkt from circular buffer to network
		 */
		void send(int sockfd);

		/**
		 * testing the throughput
		 */
		int* setThroughput(int lossnum, int totalnum);

		/* tend: For testing throughput. 
		 * It records the end time when data in circular buffer has been sent out.
		 */
		struct timeval						tend;

  private:
		int									sockfd;
		sockMng								*sm;
		sockTb								*socktb;
		struct sockaddr_in					sockaddrs;
		char        						recv_cnt[RECVBUFSIZE];
		char 								run_buf[TOU_MSS];
		boost::try_mutex					run_t_mtx; //protect run func.
		boost::mutex						send_mtx; //protect send func.

		/* 
			process the ack in PROCESS_ACK_DATARECSUCC_SENDBACK_ACK
			contains TOUS_DELACK_INIT and TOUS_DELACK_QUEACK state
		*/
		void proc_delack(sockTb *stb);
		void set_ack_state(sockTb *stb, int state);

        /* 
         	out-of-order buffer(minhp_recv) not empty implies recovery is possible.
		 	Recover form the HpRecvBuf: Try to put the out-of-order data back into
		 	circular buffer until the matching is fail. 
		 */
		int proc_pkt_rec(sockTb *stb, int sockfd);
		int do_proc_pkt_rec(sockTb *stb, int sockfd);


		int proc_ack(int sockfd, sockTb *stb, touPkg *tp);;

		int run_recvfrom(int sockfd, touPkg** pkt);

		std::string gen_pkt(sockTb *stb, touPkg **pkt, char *buf, int len);
		std::string gen_pkt(sockTb *stb, struct sockaddr_in *sa);

		int popsndq(sockTb *socktb, char *sendbuf, int len);
		int putcircbuf(sockTb *socktb, int sockfd, std::string  *buf, int rvpl);

		int processGetPktState(sockTb *socktb, touPkg *pkt);
		int assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, std::string ip, unsigned short port);
		unsigned long getSendSize(sockTb *socktb);
		bool calThroughput();
		int cal_num_caxmit(int snd_sz);

};


extern processTou *ptou;		//process TOU
