#include "tou.h"
#include "processtou.h"
std::vector<sockTb*> SS;
timerMng tm1;
boost::mutex socktabmutex1;

FILE *_fptrace = fopen("../debug.txt", "w");
using namespace std;
/*
 *Byte stream functions
 *@result converts to byteorder	
 */
	
void touMain::convertFromByteOrder(touPkg tp) {

	tp.t.seq = ntohl(tp.t.seq);
  tp.t.mag = ntohl(tp.t.seq);
	tp.t.ack_seq = ntohl(tp.t.seq);
  tp.t.syn = ntohs(tp.t.syn);
  tp.t.ack = ntohs(tp.t.ack);
}

/*
 *Byte stream functions
 *@result converts to byteorder	
 */
void touMain::convertToByteOrder(touPkg tp) {
	
  tp.t.seq = htonl(tp.t.seq);
  tp.t.mag = htonl(tp.t.seq);	
  tp.t.ack_seq = htonl(tp.t.seq);
  tp.t.syn = htons(tp.t.syn);
  tp.t.ack = htons(tp.t.ack);
  tp.t.ack_seq = htons(tp.t.ack_seq);
}
	
    
/*
 *Create Socket
 *@result 1 if successful
 */
int touMain::touSocket(int domain, int type, int protocol) {
	
	if ((domain != AF_INET) || (type != SOCK_DGRAM) || (protocol != 0 )) {
		cout << "ERROR CREATING SOCKET" ;
		return -1 ;
	}
	sd = socket(domain,type,0);
  sm.setSocketTable(sd);
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); 
	return sd;
}

/*
 *Bind Socket
 *return 0 if succesful
 */

int touMain::touBind(int sockfd, struct sockaddr *my_addr, int addrlen) {
	int rv;
  rv = bind(sockfd,my_addr,addrlen);
  char * ip = inet_ntoa(((sockaddr_in*)my_addr)->sin_addr);
  sm.setSocketTable((sockaddr_in *)my_addr,sockfd);
  return rv;
}
	
/* 
 * Listen()
 * @result socket state listening  
 */
	 
int touMain::touListen(int sd, int backlog) {
  s.sockstate = TOUS_LISTEN;
    
}
	
/* 
 * Connect()
 * @result return 1 on successful  
 */
	
int touMain::touConnect(int sd, struct sockaddr_in *socket1, int addrlen) {
	int rv;
  sm.setSocketTableD(socket1,sd);		
	tp.t.seq = rand()%(u_long)65535;
	tp.t.mag = (u_long)9999;
  tp.t.syn = 1;
  TRACE(5,"Seq no while sending : %lu " , tp.t.seq);
	sockaddr_in sockaddrs;
  convertToByteOrder(tp);
	rv = sendto(sd, &tp, sizeof(tp), 0,(struct sockaddr*)socket1,sizeof(struct sockaddr_in));
	perror("send : ");
	size_t len = sizeof(sockaddr);
	int count = 1;
	while(1) {
		fd_set socks;
		struct timeval tim;
		FD_ZERO(&socks);
		FD_SET(sd, &socks);
		tim.tv_sec = 4;
		if (select(sd+1, &socks, NULL, NULL, &tim)) {
 			rv = recvfrom(sd, &tp, sizeof tp, 0, (struct sockaddr*)&sockaddrs,&len);
 			break;
		}
		else {
 			cout <<"SYNACK not received !! "<< endl;
      count++;
      if (count > 5) close(sd);
		}
	}

  convertFromByteOrder(tp);
	perror("talker: sendto");
	tp.t.ack_seq = tp.t.seq + 1;
	tp.t.syn = 0;
	tp.t.ack = 1;
  cout << "Seq number at the end of Connect " << tp.t.seq << endl;
	convertToByteOrder(tp);
		
		//send final 3way handshake
	rv = sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)socket1, addrlen);
	return true;
}


/* 
 * Accept()
 * @result return 1 on successful  
 */

int touMain::touAccept(int sd, struct sockaddr_in *socket2, socklen_t *addrlen) {
	int rv, control=0, flagforsyn = 1;
  size_t len = sizeof(sockaddr);
  struct sockaddr_in sockaddrs;
  
  sockTb *s;
  s = sm.getSocketTable(sd);
  tp.t.seq = s->tc.snd_nxt;
  u_short port = (s->dport);
  assignaddr(&sockaddrs,AF_INET,s->dip,port);
  if(s->sockstate == TOUS_SYN_RECEIVED) {  
		boost::mutex::scoped_lock lock(socktabmutex1);
    //send syn ack
	  if(lock)
    {
     	tp.t.ack_seq = tp.t.seq + 1;
   		tp.t.seq = rand()%(u_long)65530;
   		tp.t.syn = 1;
   		tp.t.ack = 1;
   		convertToByteOrder(tp);
    		
    		//Code for testing begin
    			sleep(6);

     	rv = sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr *)&sockaddrs, sizeof(struct sockaddr_in));
       perror("send : ");
  		//Code for testing end
    		
  		//recv third handshake
     		rv = recvfrom(sd, &tp, sizeof tp, 0, (struct sockaddr *)socket2, &len);
     		convertFromByteOrder(tp);
  		
    		if (tp.t.ack_seq == tp.t.seq + 1) 
          cout<<"SUCCESS !!!" << endl;
        return true;
      }
   }
}

/* 
 * Send()
 * @result return no of bytes successfully sent  
 */

int touMain::touSend(int sd, char *sendBufer, int len1, int flags) {
  sockTb *s;
  s = sm.getSocketTable(sd);
  u_short port = (s->dport);
  int rv, control=0, flagforsyn = 1;
  size_t len = sizeof(sockaddr);
  struct sockaddr_in sockaddrs,socket1;
  memset(tp.buf, 0, TOU_MSS);
  tp.t.syn = 0;
  assignaddr(&sockaddrs,AF_INET,s->dip,port);
  //memcpy(tp.buf,sendBufer,len1);
  memcpy(buf,sendBufer,len1);
  tp.t.seq = tp.t.seq + 1;
  cout << "Sequence Number : " << tp.t.seq << endl; 
  convertToByteOrder(tp); 
  int sizeavail = s->CbSendBuf.getSize();
  if(sizeavail > len1)
  {
    s->CbSendBuf.insert(sendBufer,len1);
  }
  u_long len2 = s->sc->getwnd();
  //do conversion from window size to bytes(len2)
  //sendto(sd,&tp,sizeof(tp),0,(struct sockaddr *)&sockaddrs,sizeof(sockaddr));
  memcpy(tp.buf,buf,len2);
  sendto(sd,&tp,sizeof(tp),0,(struct sockaddr *)&sockaddrs,sizeof(sockaddr));
  tm1.add(sd,2,tp.t.seq,s,tp.buf);
  //set seq no, set seq_next in toucb or socktable
  
    
}





/* 
 * Receive()
 * @result return no of bytes successfully received  
 */

int touMain::touRecv(int sd, char *recvBuffer, int bufferLength, int flags) {
    
int rv, control=0, flagforsyn = 1;
size_t len = sizeof(sockaddr);
int end = 0;
struct sockaddr_in sockaddrs;
char buffer[bufferLength];
sockTb *s = sm.getSocketTable(sd);
s->CbRecvBuf.getAt(buffer,bufferLength,end);
s->CbRecvBuf.remove(bufferLength);
 cout << "Received data " << buffer << endl;
convertToByteOrder(tp);
tp.t.seq = s->tc.snd_nxt;
cout << "Seq no recv : " << s->tc.snd_nxt <<"   " <<  tp.t.seq << endl;
u_short port = (s->dport);
assignaddr(&sockaddrs,AF_INET,s->dip,port);
boost::mutex::scoped_lock lock(socktabmutex1);
//send syn ack
  if(lock)
  {
    tp.t.ack_seq = tp.t.seq + strlen(buffer);
	  tp.t.seq = rand()%(u_long)65530;
  	tp.t.syn = 0;
	  tp.t.ack = 1;
  	convertToByteOrder(tp);
   		
	//Code for testing begin
  //	sleep(3);
  	rv = sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr *)&sockaddrs, sizeof(struct sockaddr_in));
    perror("send : ");
    
  }	
  
}

/* 
 * Close()
 */

int touMain::touClose(int sd) {
		
  sockTb *s;
  sockaddr_in sockaddrs,socket1;
  s = sm.getSocketTable(sd);
  size_t len = sizeof(sockaddr);
  tp.t.seq = s->tc.snd_nxt;
  u_short port = (s->dport);
  assignaddr(&sockaddrs,AF_INET,s->dip,port);
  if(s->sockstate == TOUS_CLOSE_WAIT) {
   tp.t.fin = 1;
   tp.t.ack = 1;
   int rv = sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)&sockaddrs , sizeof(struct sockaddr_in));   
   sm.delSocketTable(sd);
   close(sd);
   return 1;
 }
   tp.t.fin = 1;
   tp.t.ack_seq = tp.t.seq + 1;
	 int rv = sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)&sockaddrs , sizeof(struct sockaddr_in));
   recvfrom(sd,&tp,sizeof(tp),0,(struct sockaddr *)&socket1,&len);
   if(tp.t.fin ==1 && tp.t.ack ==1) {
    sm.delSocketTable(sd);
    fd_set socks;
		struct timeval tim;
		FD_ZERO(&socks);
		FD_SET(sd, &socks);
		tim.tv_sec = 4;
    close(sd);
   }
	  
}
  


/* >0 if fd is okay
 * ==0 if timeout
 * <0 if error occur 
 */
int touMain::timero(int fd, int usec) {
	
	fd_set                timeo_rset;
	struct timeval        tv;
	FD_ZERO(&timeo_rset);
	FD_SET(fd, &timeo_rset);
	tv.tv_sec = 0;
	tv.tv_usec = usec;
	return (select(fd+1, &timeo_rset, NULL, NULL, &tv));
}


/* 
 *get the sockaddr_in infomation 
 */
int touMain::assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, string ip, u_short port) {
	
  bzero(sockaddr, sizeof(*sockaddr));
  sockaddr->sin_family = sa_family;
	sockaddr->sin_port = htons((short)(port));
    
  if( 1 != inet_pton(sa_family, ip.c_str(), &sockaddr->sin_addr) )
		return 0;

		return 1;
}

/*
 *called during tou_bind()
 */
  
void sockMng::setSocketTable(struct sockaddr_in *sockettemp, int sd) {

  boost::mutex::scoped_lock lock(soctabmutex);
  for(stbiter=SS.begin(); stbiter!=SS.end(); stbiter++) {
  if((*stbiter)->sockd == sd) {
    SS.erase(stbiter);
    s->sockd = sd;
    s->sport = ntohs(sockettemp->sin_port);
  	s->sip = inet_ntoa(sockettemp->sin_addr);
	  SS.push_back(s);
    break;
    }
  }

}

  //called during tou_accept()
  void sockMng::setSocketTableD(struct sockaddr_in *sockettemp, int sd)  {
    s = new sockTb;
    string sip;
    u_short sport;
    char* ip1 = inet_ntoa(sockettemp->sin_addr);
        u_short dport = ntohs(sockettemp->sin_port);
        std::string ip(ip1);
		boost::mutex::scoped_lock lock(soctabmutex);
    for(stbiter=SS.begin(); stbiter!=SS.end(); stbiter++) {
      if((*stbiter)->sockd == sd) {
          sip = (*stbiter)->sip;
          sport = (*stbiter)->sport;                  
          SS.erase(stbiter);
          s->sockd = sd;
          s->dport = dport;
      		s->dip = ip;
          s->sip = sip;
          s->sport = sport;
		      SS.push_back(s);
        break;
      }
    }
}

  //called during tou_socket()
  void sockMng::setSocketTable(int sd) {
    boost::mutex::scoped_lock lock(soctabmutex);
    s = new sockTb;
		s->sockd = sd;
    SS.push_back(s);
  }
	
  /* called during tou_close() */
  void sockMng::delSocketTable(int sd) {
    boost::mutex::scoped_lock lock(soctabmutex);
    for(stbiter=SS.begin(); stbiter!=SS.end(); stbiter++) {
      if((*stbiter)->sockd == sd) {
        SS.erase(stbiter);
        break;
      }
    }
  }
  
  
	 /* return socktable ptr if matchs with sockfd 
	  * return NULL if failure */
  struct sockTb* sockMng::getSocketTable(int sockfd) {
	  boost::mutex::scoped_lock lock(soctabmutex);      
    sockTb	*s;
	 	for(stbiter=SS.begin(); stbiter!=SS.end(); stbiter++)
	        {
		  if((*stbiter)->sockd == sockfd)
		    return (*stbiter);
	   	}
		return NULL;
	 }

  void sockMng::setSocketState(int state, int sd) {
    boost::mutex::scoped_lock lock(soctabmutex);      
    s = new sockTb;    
    for(stbiter=SS.begin(); stbiter!=SS.end(); stbiter++)
	  {
		  if((*stbiter)->sockd == sd)
      {
        s->sport = (*stbiter)->sport; 
        s->dport = (*stbiter)->dport; 
        s->sip = (*stbiter)->sip;
        s->dip = (*stbiter)->dip;
        SS.erase(stbiter);
        break;
      }
     }
     s->sockstate = state;
     s->sockd = sd;
      SS.push_back(s);
} 

  void sockMng::setTCB(u_long seq,int sd) {
     boost::mutex::scoped_lock lock(soctabmutex);      
    s = new sockTb;
    for(stbiter=SS.begin(); stbiter!=SS.end(); stbiter++)
	  {
		  if((*stbiter)->sockd == sd)
      {
        s->sport = (*stbiter)->sport; 
        s->dport = (*stbiter)->dport; 
        s->sip = (*stbiter)->sip;
        s->dip = (*stbiter)->dip;
        s->sockstate = (*stbiter)->sockstate;
        SS.erase(stbiter);
        break;
      }
     }
     
     s->tc.snd_nxt = seq;
     s->sockd = sd;
      SS.push_back(s);
    char *cstr = new char [(s->sip).size()+1];
    strcpy (cstr, (s->sip).c_str());
    char *cstr1 = new char [(s->dip).size()+1];
    strcpy (cstr1, (s->dip).c_str());
    int sport = int(s->sport);
    int dport = int(s->dport);
    int st = int(s->sockstate);
    TRACE(5," Source IP is : %s , Source port is : %d , Destination IP is : %s , Destination port is : %d , State : %d \n", cstr,sport,cstr1,dport,st);
    delete[] cstr;
    delete[] cstr1;		
}

void sockMng::setCbData(char *buf,int sd,int len) {
 boost::mutex::scoped_lock lock(soctabmutex);      
 s = new sockTb;
 for(stbiter=SS.begin(); stbiter!=SS.end(); stbiter++)
	  {
		  if((*stbiter)->sockd == sd)
      {
        s->sport = (*stbiter)->sport; 
        s->dport = (*stbiter)->dport; 
        s->sip = (*stbiter)->sip;
        s->dip = (*stbiter)->dip;
        s->sockstate = (*stbiter)->sockstate;
        s->tc.snd_nxt = (*stbiter)->tc.snd_nxt;
        SS.erase(stbiter);
        break;
      }
     }
  s->CbRecvBuf.insert(buf,len);
  s->sockd = sd;
  
  SS.push_back(s);
}  

