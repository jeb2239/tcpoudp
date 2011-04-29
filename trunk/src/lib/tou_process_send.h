/*********************************************************
 * "./tou_process_send.h"
 * touProcessSend for processing all input data form net
 * io management for congetion control 
 * note: thread per socket 
 * ******************************************************/
#include "tou.h"
extern boost::mutex sndqmutex;

class touProcessSend {
  public:
    touProcessSend(){}
	  touProcessSend(int sockfd_)
	    :i_thread(boost::bind(&touProcessSend::run, this, sockfd_))
	  {
	    std::cout<< "**** touProcessSend is running now ***\n";
	  }

    ~touProcessSend(){
      std::cout << "*** touProcessSend is detached *** " << std::endl;
      i_thread.join(); // RAII designed pattern, recycling self-managment
    }

	/* 1. protected by mutex. circular buf & toutb.
	 * 2. push data into sockfd's circular buff, and return 
	 *	  how much bytes it writes into the circular buff.
	 * 3. return 0, if there's no space in circular buff.
	 * 4. return -1, if error occurs */
    int pushsndq(int sockfd, char *sndbuf, int &len);

	/* 1. protected by mutex. circular buf & toutb.
	 * 2. it should only be called in tou_send func, and it'll
	 *    return how much bytes wrote into sending buff.
	 * 3. return 0, if there's nothing in circular buff.
	 * 4. return -1, if error occurs */
	  int popsndq(sockTb *socktb, char *sendbuf, int len);

  private:
    boost::thread			i_thread;
    int								sockfd;
	  sockMng						sockmng;
	  sockTb						*socktb;
	  touProcessSend		*toups;	//for handling sending thread

		/* get the sockaddr_in infomation
     * error return: 0
     * success return: 1 */
    int assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, string ip, unsigned short port);
	
    void run(int sockfd);

};
