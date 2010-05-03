/******************************************************
 * tou.h
 * This is ToU library header file. User who wants to 
 * use ToU library functions hould include this header 
 * file.
 *****************************************************/
 
/******************************************************
 * Type define here
 * ***************************************************/
//Types
typedef	unsigned long 	u_long;
typedef	unsigned short	u_short;
typedef unsigned char	u_char;

/*
 *Timers
 */
#define TOUT_TIMERS			4
#define TOUT_REXMIT			0
#define TOUT_PERSIST		1
#define TOUT_KEEP			2
#define TOUT_2MSL			3

/*
 *Ethernet 1500-20-24
 */
#ifndef TOU_MSS
	#define TOU_MSS				1456
#endif

/*
 *Status
 */

#define TOUS_CLOSED			0
#define TOUS_LISTEN			1
#define TOUS_SYN_SENT		2
#define TOUS_SYN_RECEIVED	3
#define TOUS_ESTABLISHED	4
#define TOUS_CLOSE_WAIT		5
#define TOUS_FIN_WAIT_1		6
#define TOUS_CLOSING		7
#define TOUS_LAST_ACK		8
#define TOUS_FIN_WAIT_2		9
#define TOUS_TIME_WAIT		10

/***************************************************
 * Include from STD library
 **************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>

#include <vector>
#include <string>
/***************************************************
 * Include from BOOST library
 **************************************************/

/***************************************************
 * Include from self-define header
 **************************************************/

#include "timer.h"				//tou timer library
#include "processtou.h"
#include "timestamp.h"

extern timerMng tm1;
extern boost::shared_ptr<boost::thread> m_thread;

/******************************************************
 * tou main class
 * ***************************************************/
class touMain {
	private:
	/* socket descriptor */
  int sd;

  /* for sending: pushing data into circular buf */
	int pushsndq(int sockfd, char *sendbuf, int &len);
	
	public:
  touMain() : sm() {
  }

  touPkg tp;				// ToU packet
  sockMng sm;				// ToU socket table management
  processTou *ptou;	//process TOU

	int touSocket(int , int , int );
	int touAccept(int , struct sockaddr_in *, socklen_t * );
	int touConnect(int , struct sockaddr_in *, int );
	int touBind(int , struct sockaddr *, int );
	int touListen(int, int);
	int touSend(int , char *, int , int );
	int touRecv(int , char *, int , int );
	int touClose(int);
	
	int proTou(int sockfd) {
		ptou = new processTou(sockfd, &sm, &tp);
	}

	void convertToByteOrder(touPkg&);
	void convertFromByteOrder(touPkg&);
	int assignaddr(struct sockaddr_in *, sa_family_t , std::string , u_short);
	void dothis(int );
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

