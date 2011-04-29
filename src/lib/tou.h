/*******************************************************************************
 * tou.h
 * This is ToU library header file. User who wants to 
 * use ToU library functions should include this header
 * file.
 ******************************************************************************/

#include "timer.h"
#include "timestamp.h"


/**
 * tou main class
 */
class touMain {
	
	private:
		int		sd;				//ToU socket descriptor
		//sockMng	sm;				//ToU socket table management

		/*
			For sending: pushing data into circular buf
		 */
		int pushsndq(int sockfd, char *sendbuf, int &len);

		/*
			Convert address
		 */
		int assignaddr(struct sockaddr_in *, sa_family_t , std::string , u_short);

		/*
			Initiate processtou
		 */
		int proTou(int sockfd);
		
		/*
			set the socket to nonblock mode
		*/
		void setSocketNonblock(int sockfd);
		
		/*
			set the socket to block mode
		*/
		void setSocketBlock(int sockfd);

		/*
			get the socket recv buffer size in byte
		*/
		int getSocketRcvBuf(int sockfd);

	public:
		sockMng	sm;				//ToU socket table management

		/*
			Default constructor
		 */
		touMain() : sm() {}

		/*
			Default destructor
		 */
		~touMain() {}

		/*
			 run the processtou: run()
		*/
		void run(int sockfd);

		/*
			ToU APIs
		 */
		int touSocket(int domain, int type, int protocol);
		int touAccept(int sd, struct sockaddr_in *socket2, socklen_t *addrlen);
		int touConnect(int sd, struct sockaddr_in *socket1, int addrlen);
		int touBind(int sockfd, struct sockaddr *my_addr, int addrlen);
		int touListen(int sd, int backlog);
		int touSend(int sd, char *sendBufer, int len, int flags);
		int touRecv(int sd, char *recvBuffer, int bufferLength, int flags);
		int touClose(int);

		/*
		 	get the buffer size of circular send buf 
		*/
		unsigned long getCirSndBuf();
};	











/*
 *Close Thread
 *@params socket descriptor
 */
class closer {
	public:

		closer(int sd)
		  :m_thread1(boost::bind(&closer::dothis,this,sd)){
		  std::cout << "*** closer thread started*** " << std::endl;

		}

    int clientClose(int sd);
    int serverClose(int sd); 
    void dothis(int sd); 
    void go();
    void startthread();
    ~closer(){
      std::cout << "*** closer recycled *** " << std::endl;
      m_thread1.join(); 
		}
    int sd;
	int assignaddr(struct sockaddr_in *, sa_family_t , std::string , u_short);
	timerCk *timercker1;
	boost::thread m_thread1;

	private:
    sockMng sm;
    touPkg tp;

};
