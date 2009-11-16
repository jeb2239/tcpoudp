/***********************************************************************
 * Congestion Control Demo File
 * Chenghan LEE, 11/08/09. 
 * ********************************************************************/
#include "tou_process_send.h"
#include <fstream>

#define	MAXMSG		1500	
#define MAXSNDBUF	65536
#define ClIPORT		8887
#define TIMEOUT		4000000
#define ERROR			1
#define	SUCCESS		0

/* >0 if fd is okay
 *  * ==0 if timeout
 *   * <0 if error occur */
int timero(int fd, int usec) {
  fd_set        timeo_rset;
  struct timeval    tv;

  FD_ZERO(&timeo_rset);
  FD_SET(fd, &timeo_rset);

  tv.tv_sec = 0;
  tv.tv_usec = usec;

  return (select(fd+1, &timeo_rset, NULL, NULL, &tv));
}

int assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, string ip, unsigned short port){
  bzero(sockaddr, sizeof(*sockaddr));
  sockaddr->sin_family = sa_family;
  /*
   *   sockaddr->sin_port = htons((short)atoi(port.c_str()));
   *     */
  sockaddr->sin_port = htons(port);
  if( 1 != inet_pton(sa_family, ip.c_str(), &sockaddr->sin_addr) )
    return 0;

  return 1;
}

/***********************************************************************
 * Server: ./tou -s 128.59.15.42
 * Client: ./tou -c ../testcase/test_1kb.txt 128.59.15.42 8888 8887
 * ********************************************************************/
int main(int argc, char* argv[])
{
  int                   sockfd;
  struct addrinfo       *adinfo;
  struct sockaddr_in    cliaddr;
  struct sockaddr_in    svraddr;
  socklen_t							len;
  struct in_addr        a;
  struct hostent        *he;
  touPkg								recvbuf;
  touPkg								sendbuf;
  char									senddatabuf[MAXSNDBUF];
  int										datasize;
  int										n = 0;
  touMain								tm;
  ifstream							indata;
	int										readsize;
	int										payloadsize;
	char									str[100];
	unsigned short				clientPort = ClIPORT;

	int										discard = 0;

/* Init local network status */
  struct addrinfo       curinfo;
  int                   error;
  memset(&curinfo, 0, sizeof(curinfo));
  curinfo.ai_family = AF_INET;
  curinfo.ai_socktype = SOCK_DGRAM;
  curinfo.ai_flags = AI_PASSIVE;

/* Get the parameters and decide whether it's a server or
 * client process */
  if( argc==3 && !strncmp(argv[1], "-s", 2) ) { //tou -s <local IP>
    /* *****************************************************
		 * server side 
		 * *****************************************************/
  
    /* get the local network information */
    error = getaddrinfo(NULL, "8888", &curinfo, &adinfo);
    if (0 != error){
      fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(error));
      printf("error in server's getaddrinfo");
    }
	
	/* socket initialization  | type: adinfo->ai_protoco*/
    if( -1 == (sockfd = socket(adinfo->ai_family, adinfo->ai_socktype, adinfo->ai_protocol)))
      printf("error in server's socket creation");
      std::cout << "SERVER socket function returned" << std::endl;

    /* bind */
	  if( 0 != bind(sockfd, adinfo->ai_addr, adinfo->ai_addrlen))
      printf("error in server binding");
      std::cout << "SERVER bind function returned" << std::endl;

    printf("[Welcome to the ToU Server Mode.]\n");
	/* keep waiting for incoming connection */

	  sockMng sockmng;
	  sockmng.setSocketTable(&cliaddr, sockfd, argv[2], 8888);
		sockTb	*stb = sockmng.getSocketTable(sockfd);
		std::cout<<"local addr: "<<argv[2]<<std::endl;

		stb->tc.snd_una = 0;
		stb->tc.snd_nxt = 0;
		stb->tc.snd_ack = 0;
		stb->tc.snd_awnd = 65536;

		int sendtomsg;

		stb->printall();
	/* waiting for incoming ack and sending data */
	while(1) {
	  if( timero(sockfd, TIMEOUT) == 0){
	    /* time out here */
	    /* should reset the wnd size */ 
			std::cout << " ***SERVER TIME OUT(4000ms)!!! ***\n"<<std::endl;

	  }else{
		  /* recv ack correctly */
			n = recvfrom(sockfd, &recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&cliaddr, &len);
			std::cout << "ACKCOUNT: "<<stb->ackcount<<" Recv: "<<n<<" Bytes Seq#: "<<ntohl(recvbuf.t.seq)<<std::endl;
			std::cerr << "Size of total pkg: "<< sizeof(recvbuf) << std::endl;

			/* pkg format issue, buf will read including unexpected field: talk to chinmay */
			if( TOU_MSS < (payloadsize = strlen(recvbuf.buf)) )
        payloadsize  = TOU_MSS;
			stb->tc.snd_ack += payloadsize;
			sendbuf.t.ack_seq = htonl(stb->tc.snd_ack);
			cerr << "Number of bytes received: "<< payloadsize << endl;

			/* For test, discard #1600 */
			//if (stb->tc.snd_ack == 1600)	discard = 1;
	    
			assignaddr(&cliaddr,AF_INET, "128.59.15.42" , clientPort);
			inet_ntop(AF_INET, &(cliaddr.sin_addr), str, 100);
			/* send ack back */
			std::cerr<< "Send Ack back: ack#: "<<stb->tc.snd_ack<<" Send to "<<str<<" "<<ntohs(cliaddr.sin_port) << endl;
			if(!discard){
				sendtomsg = sendto(sockfd, &sendbuf, sizeof(sendbuf), 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
			}else{
				std::cerr<< "\n *** PKG# 1600 is lost *** \n";
				discard = 0;
			}

			/* sentto error check, log stored in debug.txt */ 
			if(sendtomsg == -1){
				TRACE(5, "%d :sendto Error in server \n", 5);
			  perror("sendto errmsg:");
			}
		}// end of  if( timero(sockfd, 3000000) == 0)

	}//while(1)
  }else if ( argc==6 && !strncmp(argv[1], "-c", 2) ){
   /************************************************** 
		* client side 
		**************************************************/
    error = getaddrinfo(NULL, argv[5], &curinfo, &adinfo);
    if( 0 != error ){
      fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(error));
      printf("error in client's getaddrinfo");
    }

	/* socket |type: adinfo->ai_protocol*/
    if( -1 == (sockfd = socket(adinfo->ai_family, adinfo->ai_socktype, 0)))
      printf("error in server's socket creation");
	  
	/* client bind with local machine */
    if( 0 != bind(sockfd, adinfo->ai_addr, adinfo->ai_addrlen))
      printf("error in client binding");
	
	//Set socket structures
	memset(&svraddr, 0, sizeof(svraddr));
	svraddr.sin_family = AF_INET;
	inet_pton(AF_INET,argv[3],&(svraddr.sin_addr));
	svraddr.sin_port = htons(atoi(argv[4]));
	/* reg sending sockaddr */
	assignaddr((sockaddr_in *)&svraddr, AF_INET, argv[3],(unsigned short)atoi(argv[4]));

	/* connect to client server */
	std::cout << "Connect returns successfully" <<std::endl;
	
  /* client side */
  error = getaddrinfo(NULL, argv[5], &curinfo, &adinfo);

	sockMng sockmng;
	sockmng.setSocketTable(&svraddr, sockfd, (char*)"127.0.0.1",(unsigned short)atoi(argv[5]));// 1. 2. 3.local ip 4.local port
	sockTb	*stb = sockmng.getSocketTable(sockfd);
	std::cout<<"Local Address: "<<stb->sip<<std::endl;

  stb->ackcount = 0;
	stb->tc.snd_una = 0;
	stb->tc.snd_nxt = 0;
	stb->tc.snd_ack = 0;
	stb->tc.snd_awnd = 65536;

	/* reading file */
	indata.open(argv[2]); // opens the file
  if(!indata) { // file couldn't be opened
    cerr << "Error: file could not be opened" << endl;
    exit(1);
  }

	/* sending thread */
	int active = 0;

	/* waiting for incoming ack and sending data */
	while(1) {

	  /* try to send data */
		int datainserttobuf;
		cerr << " *** Now is trying to put data into circular buff. Data available: "<<stb->CbSendBuf.getAvSize()<<" *** \n";
	  if( 0 < (datasize=stb->CbSendBuf.getAvSize())) {
	  	if( !indata.eof() ){
				cerr << "Reading data from file: "<< argv[2] << std::endl;
		    indata.read(senddatabuf, readsize = std::min(datasize, MAXSNDBUF));
				readsize = indata.gcount();	
			  std::cout<< "Checking circular buf size: "<<datasize<<std::endl;
				std::cout<< "Now getting the data from text file: "<< readsize <<" bytes read!" << std::endl;
			  datainserttobuf = stb->CbSendBuf.insert(senddatabuf, readsize);
				std::cout<< " "<<datainserttobuf<<" bytes of data put into the circular buff"<< std::endl;
				/* active the process_send tread for handling sending pkg */
				if (!active){
					touProcessSend toups(sockfd);
					active = 1;
				}
			}
		}// end of if( 0 < (datasize=stb->CbSendBuf.getAvSize()))

		/* JUST FOR TEST DEMO, Wait a little while */
		if( stb->ackcount <=0 ) {
			sleep(1);
		}

	  while(stb->ackcount>0){
			cerr << " *** Waiting for ACK, and see whether i can enlarge the wnd size! ackcount is "<<stb->ackcount<< " *** \n";

	    if( timero(sockfd, TIMEOUT) == 0){
  	    /* time out here */
				/* should reset the wnd size */ 
				cerr << "\n [TIMEOUT] Do stb->sc->settwnd(), and show the table as following."<<endl<<endl;
		    stb->sc->settwnd();//set timeout window
				stb->printall();

		  }else{
		    /* recv correctly */
		    n = recvfrom(sockfd, &recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&cliaddr, &len);

		    std::cout << "Get ACK, actcount: "<<stb->ackcount<<". ACK_SEQ#: "<<ntohl(recvbuf.t.ack_seq)<<std::endl;

				/* pkg's ack does't match with nxt seq # . discard it and keep waiting */
				/*
        if( stb->tc.snd_nxt != ntohl(recvbuf.t.ack_seq) ) {
					std::cout << "Received pkg which is not in-order: "<< ntohl(recvbuf.t.ack_seq)  <<std::endl;
				*/

		    stb->tc.snd_una = ntohl(recvbuf.t.ack_seq);
				stb->sc->addwnd();//set window

				/* show tb */
				cerr<< "Showing table as following \n";
				stb->printall();

		    /* send back ack 
		    stb->tc.snd_ack += strlen(recvbuf.t.seq);
		    sendbuf.t.seq = stb->tc.snd_ack;
		    memset(sendbuf.buf, 0, sizeof(sendbuf.buf));
			
		    sending 
		    sendto(sockfd, sendbuf, sizeof(sendbuf), 0, (struct sockaddr *)&pcliaddr, sizeof(pcliaddr));
		    */
	    }
		  stb->ackcount--;
	  }

		std::cout<< " \n ###### NEXT ROUND ###### \n";
		/*
		if(indata.eof()) {
			cerr << " *** Transmission Is Complete! *** \n";
      break;
		}
		*/
	}/*while(1)*/
	
	}else{
	  std::cout<< "Format Error" <<std::endl;
    std::cout<< "[server] ./tou -s <local IP> \n";
	  std::cout<< "[client] ./tou -c ./filename.txt <server ip> <server port> <local port> \n";
	}
	indata.close();
	close(sockfd);
 	return 0;	
}
