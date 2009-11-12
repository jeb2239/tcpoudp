#include "tou.h"
vector<sockTb*> SS;
int cid_ = 0;
boost::mutex soctabmutex;
FILE *_fptrace = fopen("../debug.txt", "w");


using namespace std;

	
	
	void touMain::convertFromByteOrder(touPkg tp) {
	
		tp.t.seq = ntohl(tp.t.seq);
    		tp.t.mag = ntohl(tp.t.seq);
    		tp.t.ack_seq = ntohl(tp.t.seq);
    }
	
	//Byte stream functions	
	void touMain::convertToByteOrder(touPkg tp) {
	
		tp.t.seq = htonl(tp.t.seq);
		tp.t.mag = htonl(tp.t.seq);	
		tp.t.ack_seq = htonl(tp.t.seq);
		tp.t.syn = htons(tp.t.syn);
		tp.t.ack = htons(tp.t.ack);
		tp.t.ack_seq = htons(tp.t.ack_seq);
    }
	
    
    //------------------------------- CREATE SOCKET ------------------------------
	int touMain::touSocket(int domain, int type, int protocol) {
	
		if ((domain != AF_INET) || (type != SOCK_DGRAM) || (protocol != 0 ))
		{
			cout << "ERROR CREATING SOCKET" ;
			return -1 ;
		}
		sd = socket(domain,type,0);
		setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); 
		return sd;
	}
	
	//-----------------------------LISTEN -----------------------------------
	//TODO : Modify this 
	
	int touMain::touListen() { 
	int rv;
	rv = listen(sd,1);
	}
	
	//------------------------------ BIND SOCKET ------------------------------------
	
	int touMain::touBind(int sockfd, struct sockaddr *my_addr, int addrlen) {
	
		int rv;
		rv = bind(sockfd,my_addr,addrlen);
		return rv;
	}
	
	//	---------------------------- CONNECT -----------------------------------------
	
	int touMain::touConnect(int sd, struct sockaddr_in *socket1, int addrlen) {
		return true;
	}



	
	//---------------------------ACCEPT --------------------- 
	
	int touMain::touAccept(int sd, struct sockaddr_in *socket2, socklen_t *addrlen) {
		return true;	

	}


	//--------------------------   SEND ----------------------------

	
	int touMain::touSend(int sd, char *sendBufer, int len1, int flags) {
		return true;
	}
	
	
	//-----------------	RECEIVE  --------------------------
	int touMain::touRecv(int sd, char *recvBuffer, int bufferLength, int flags) {
	        touPkg tp;
		memset(tp.buf,0,sizeof(tp.buf));

		char buf12[10];
		memset(buf12,0,10);

		int	nb_cur = 0; // the payload recved this time
		int	nb_total = 0; // total payload been recved
		int 	nb_avail = 0; // # of bytes available in recvBuffer;
	       	
		size_t len = sizeof(sockaddr);
		sockaddr_in sockaddr;
		
		cout << endl << " INSIDE TOURECV () " <<endl;
		/* Start to recv data from other end */
		while(1){
		   nb_cur = recvfrom(sd, &tp, sizeof tp, 0, (struct sockaddr *)&sockaddr, &len);
		   nb_cur = strlen(tp.buf);
		   nb_total += 0;//nb_temp;	//add to number of bytes recved in total XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXWrongXXXXXX
		   
			//Send ACK
			tp.t.ack_seq = tp.t.seq + nb_cur + 1;
			convertToByteOrder(tp);
			memset(tp.buf, 0, sizeof(tp.buf));
			sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)&sockaddr, sizeof(struct sockaddr_in));
			cout << " Ack sent " <<endl; 
			
		   if(timero(sd, 1000000) == 0){ /* wait for one sec to see whether there's sequential pkg */
		     return nb_total;

		   }else{
		     if( 0 < (nb_avail = bufferLength - nb_total) ){
		       //still get room for cping (not gonna happen)

		       /* recv the last nb_avail data from other end */

		     }
		   }
		 }
		/*

			no1 = recvfrom(sd, &tp, sizeof tp, 0, (struct sockaddr *)&sockaddr, &len);
			no2 = no2 + no1;			
			cout << " Data Received :  " << tp.buf << endl; 
			cout << " no of bytes received : " << no1 <<endl;
		
			convertFromByteOrder(tp);
			cout << " Sequence no received is :  " << tp.t.seq << endl;
			
			*/

	return 1;
	
	}

	
	//-------------------------CLOSE------------------------------
	
	int touMain::touClose()
	{
		
		tp.t.fin = 1;
		tp.t.seq = tp.t.seq + 1;
		tp.t.ack_seq = 1;
	}

	
	/* >0 if fd is okay
 * ==0 if timeout
 * <0 if error occur */
	int touMain::timero(int fd, int usec) {
	
		fd_set                timeo_rset;
		struct timeval        tv;
	
		FD_ZERO(&timeo_rset);
		FD_SET(fd, &timeo_rset);
	
		tv.tv_sec = 0;
		tv.tv_usec = usec;
		return (select(fd+1, &timeo_rset, NULL, NULL, &tv));
	}


	/* get the sockaddr_in infomation */
	int touMain::assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, char* ip, u_short port) {
	
		bzero(sockaddr, sizeof(*sockaddr));
		sockaddr->sin_family = sa_family;
		sockaddr->sin_port = htons((short)(port));
		if( 1 != inet_pton(sa_family, ip, &sockaddr->sin_addr) )
		return 0;

		return 1;
	}

/* for test */
void sockMng::setSocketTable(struct sockaddr_in *sockettemp, int sd, char *srvip, unsigned short srvport) {
		string iptemp(srvip);
		s = new sockTb;
		cout <<"Address : in  table " << inet_ntoa(sockettemp->sin_addr) <<endl;
		//cout<<"\tport:-> "<<ntohs(socket2->sin_port)<<endl;
//		boost::mutex::scoped_lock lock(soctabmutex);
		s->sockd = sd;
		s->sip = iptemp;
		s->sport = srvport; 
		s->dport = ntohs(sockettemp->sin_port);
		s->dip = inet_ntoa(sockettemp->sin_addr);
                //s->setcid(cid_++);
		SS.push_back(s);

}
	
	 /* return socktable ptr if matchs with sockfd 
	  * return NULL if failure */
         struct sockTb* sockMng::getSocketTable(int sockfd) {
	        sockTb	*s;
	 	for(stbiter=SS.begin(); stbiter!=SS.end(); stbiter++)
	        {
		  if((*stbiter)->sockd == sockfd)
		    return (*stbiter);
	   	}
		return NULL;
	 }
/*
	void sockMng::setSocketTable(sockaddr_in *sockettemp, int sd) {
		s = new sockTb;
		cout <<"Address : in  table " << inet_ntoa(sockettemp->sin_addr) <<endl;
		//cout<<"\tport:-> "<<ntohs(socket2->sin_port)<<endl;
		boost::mutex::scoped_lock lock(soctabmutex);
		s->sockd = sd;
		s->dport = 1500;
		s->sport = ntohs(sockettemp->sin_port);
		s->sip = inet_ntoa(sockettemp->sin_addr);
                s->setcid(cid_++);
		SS.push_back(s);

		for(int i = 0;i < SS.size();i++)
		{
				
			cout << "printing vector"<<endl ;
			cout << i << " : ";
			SS.at(i)->printall();
		}
	
	  }*/
	//called during tou_accept()
	void sockMng::setSocketTableD(struct sockaddr_in *sockettemp,int sd) {
	}
	
/*	
	void sockMng::setSocketState(int state, int sd, unsigned short port, char *srvip, unsigned short srvport) {
	boost::mutex::scoped_lock lock(soctabmutex); 
	s = new sockTb; 
	for(stbiter=SS.begin(); stbiter!=SS.end(); stbiter++)
	{
	if((*stbiter)->sockd == sd)
	{
	//s->sockstate = (*stbiter)->sockstate;
	s->sport = srvport; 
	s->dport = (*stbiter)->dport; 
	s->sip = srvip;
	s->dip = (*stbiter)->dip;
	SS.erase(stbiter);
	break;
	}
	}
	s->sockstate = state;
	s->sockd = sd;
	SS.push_back(s);
	cout << "Socket state pushed " << endl;
	} 
	*/
