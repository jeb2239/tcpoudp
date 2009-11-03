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
		int rv;
		

		

		//send syn and seq no
		//TODO : change the mod to 2^32 and the magic number also
		
		tp.t.seq = rand()%(u_long)65535;
		tp.t.mag = (u_long)9999;
		convertToByteOrder(tp);
		tp.t.syn = 1;
		cout <<"Address" << inet_ntoa(socket1->sin_addr) <<endl;
		rv = sendto(sd, &tp, sizeof(tp), 0,(struct sockaddr*)socket1,sizeof(struct sockaddr_in));
		perror("send : ");
		cout << " rv : "<< rv <<endl;
		size_t len = sizeof(sockaddr);
		cout << endl << " INSIDE TOU CONNECT () " <<endl;
		
		//Check if the ayn ack has received
		
		while(1)
		{
 			fd_set socks;
 			struct timeval tim;
 			FD_ZERO(&socks);
 			FD_SET(sd, &socks);
 			tim.tv_sec = 4;
 			
 			if (select(sd+1, &socks, NULL, NULL, &tim))
			{
 				//recvfrom(sock, data, length, 0, sockfrom, &length);
 				rv = recvfrom(sd, &tp, sizeof tp, 0, (struct sockaddr*)socket1,&len);
 				cout<<"SYN ACK received : "<< endl;
 				break;
			}
			else
 				cout <<"SYNACK not received !! "<< endl;
		
		}

		convertFromByteOrder(tp);
		perror("talker: sendto");
		cout << "seq no received from server : " << tp.t.seq<<endl;
		cout << "ack no received from server : " << tp.t.ack_seq<<endl;
		
		
		tp.t.ack_seq = tp.t.seq + 1;
		tp.t.syn = 0;
		tp.t.ack = 1;
		convertToByteOrder(tp);
		
		//send final 3way handshake
		

		rv = sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)socket1, addrlen);

		cout << " Sent the third handshake " << endl;
		cout<<" sport : in connect : "<< ntohs(socket1->sin_port)<<endl;
		cout <<"Address" << inet_ntoa(socket1->sin_addr) <<endl;
		
		sm.setSocketTable(socket1, sd);


		


		return true;
	}



	
	//---------------------------ACCEPT --------------------- 
	
	int touMain::touAccept(int sd, struct sockaddr_in *socket2, socklen_t *addrlen) {

		int rv, control=0, flagforsyn = 1;

		size_t len = sizeof(sockaddr);
		convertToByteOrder(tp);
		cout << endl << " INSIDE TOUACCEPT () " <<endl;


		

		//sockTb s2;
		// receive first handshake

		
		rv = recvfrom(sd, &tp, sizeof tp, 0,(struct sockaddr *)&socket2, &len);
		cout << "seq no received from client : " << tp.t.seq<<endl;
		
		//send syn ack
		
		tp.t.ack_seq = tp.t.seq + 1;
		cout << "ack no sent to client : " << tp.t.ack_seq<<endl;
		tp.t.seq = rand()%(u_long)65530;
		tp.t.syn = 1;
		tp.t.ack = 1;
		convertToByteOrder(tp);
		
		//Code for testing begin
		//	sleep(3);
		
				rv = sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr *)&socket2, sizeof(struct sockaddr_in));
		
		//Code for testing end
		
		//recv third handshake
		
		rv = recvfrom(sd, &tp, sizeof tp, 0, (struct sockaddr *)&socket2, &len);
		convertFromByteOrder(tp);
		cout << "ack no received for the third handshake : " << tp.t.ack_seq<<endl;
		cout << endl << " LEAVING TOUACCEPT () " <<endl;
		
		if (tp.t.ack_seq == tp.t.seq + 1) cout<<"SUCCESS !!!" << endl;

		

		
		sm.setSocketTable((struct sockaddr_in *)&socket2, sd);
		

		return true;	

	}


	//--------------------------   SEND ----------------------------

	
	int touMain::touSend(int sd, char *sendBufer, int len1, int flags) {
		
		ssize_t no;
		int rec;	
		char buf12[10];	
		char buf[len1+1];
		size_t len = sizeof(sockaddr);
		memset(tp.buf, 0, TOU_MSS);
		int act = (sizeof tp.t) + len1;
		int act2 = len1;
		int index;
		char temp[TOU_MSS];
		int len3, len2;
		len3 = len1;
		index = 0;
		sockTb s;
		ssca wnd(&(s.tc));
		s.CbSendBuf.setSize(4000);
		s1 = sm.getSocketTable(sd);
		cout << endl << " INSIDE TOUSEND () " <<endl;
		// check circular buffer size and insert data into it 
		while(1)
		{		
		        TRACE(6, "cur wnd size %d\n", wnd.getwnd());
			int checkSize = s.CbSendBuf.getSize();
			std::cout<< "checksize" << checkSize <<std::endl;
			std::cout<< "length" << len3 <<std::endl;
			if(len3 <= checkSize)
			{
				s.CbSendBuf.insert(sendBufer,len3);
				s.CbSendBuf.print();
				
				len2 = len1;
				break;
			}
			else             //What if the circular buffer is full 
			{
				int j = 0;
				for(int i = index; i < (index + checkSize); i++)
				{
					temp[j++] = sendBufer[i];
					len2 = len2 + checkSize;
				}	
				index = index + checkSize;
				std::cout<< "TEMP" << temp <<std::endl;
							
				s.CbSendBuf.insert(temp,(checkSize));
				
				
				if( len2 >= len1) break;
				
			}
		}

		int seq = tp.t.seq;
		int numbytes;		
		struct sockaddr_in sockaddr;
		int end;
		char *buffer;
			u_long w = wnd.getwnd();
			int  n = int(w/int(TOU_MSS));
		      	//std::cout<<"N is " <<n << " "<<w << " " <<wnd.getwnd()<<endl;
			for (int i = 0; i < n; i++)
			{
				//memcpy(tp.buf,CbSendBuf,TOU_MSS);
				buffer = (char *) malloc(TOU_MSS);
				s.CbSendBuf.getAt(buffer, int(TOU_MSS), end);
				memcpy(tp.buf,buffer,TOU_MSS);				
				numbytes = TOU_MSS;
				std::cout<<"tp.buf is " <<tp.buf <<endl;
				int ck = assignaddr(&sockaddr, AF_INET, s1->sip, s1->sport); 
				cout << " Socket built : "<< ck << endl;
				sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr_in));
				if(timero(sd,1000000) == 0)
				{
					end = end - TOU_MSS;
					wnd.settwnd();
					continue;
				}
				else
				{
					s.CbSendBuf.remove(TOU_MSS);
					wnd.addwnd();
					int rv = recvfrom(sd, &tp, sizeof tp, 0, (struct sockaddr*)&sockaddr, &len);
					cout << "Data received : " << tp.buf << endl;					
					convertFromByteOrder(tp);					
					tp.t.seq = tp.t.seq + rv - sizeof(tp.t);
					seq = seq + n;
					if(tp.t.seq + 1 == tp.t.ack)
					{
						cout<<"Transfer completed " << endl;
						break;
					}
				}
			}
/*
		memcpy(tp.buf, sendBuffer, len1);
		tp.t.seq = tp.t.seq + act2;
		cout << " Sequence no of data sent  : " << tp.t.seq << endl;
		convertToByteOrder(tp);
		
		no = sendto(sd, &tp, act,0,(struct sockaddr*)&sockets, sizeof(struct sockaddr_in));
		cout << " Data Sent : " << tp.buf << endl;
	
		//TODO : Remove this part
		//Recv ACK
		*/	
		
//		rec = recvfrom(sd, &tp, sizeof tp, 0, (struct sockaddr *)&socket2,&len);
//		convertFromByteOrder(tp);
//		cout <<" received ack is : " << tp.t.ack_seq << endl;
		
		return no;
	}
	
	
	//-----------------	RECEIVE  --------------------------
	
	int touMain::touRecv(int sd, char *recvBuffer, int bufferLength, int flags) {
	
		memset(tp.buf,0,50);
		char buf12[10];
		int no1, no2 = 0;
		memset(buf12,0,10);
		size_t len = sizeof(sockaddr);
		sockaddr_in sockaddr;
		
		cout << endl << " INSIDE TOURECV () " <<endl;
		
		//Recv data
		int ck = assignaddr(&sockaddr, AF_INET, s.sip, s.sport); 
		cout << "Socket created : " << ck << endl;	
		if(timero(sd,6000000)==0)
			return no2;
		else
		{		
			no1 = recvfrom(sd, &tp, sizeof tp, 0, (struct sockaddr *)&sockaddr, &len);
			no2 = no2 + no1;			
			cout << " Data Received :  " << tp.buf << endl; 
			cout << " no of bytes received : " << no1 <<endl;
		
			convertFromByteOrder(tp);
			cout << " Sequence no received is :  " << tp.t.seq << endl;
			
			//Send ACK
			
			tp.t.ack_seq = tp.t.seq + no1 + 1;
			convertToByteOrder(tp);
			sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)&sockaddr, sizeof(struct sockaddr_in));
			cout << " Ack sent " <<endl; 
			
		}

	return no2;
	
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

void sockMng::setSocketTable(struct sockaddr_in *sockettemp, int sd) {
		s = new sockTb;
		cout <<"Address : in  table " << inet_ntoa(sockettemp->sin_addr) <<endl;
		//cout<<"\tport:-> "<<ntohs(socket2->sin_port)<<endl;
		boost::mutex::scoped_lock lock(soctabmutex);
		s->sockd = sd;
		strcpy(s->dip, "127.0.0.1");
		s->dport = 8888; 
		s->sport = ntohs(sockettemp->sin_port);
		s->sip = inet_ntoa(sockettemp->sin_addr);
                //s->setcid(cid_++);
		SS.push_back(s);

		for(int i = 0;i < SS.size();i++)
		{
				
			cout << "printing vector"<<endl ;
			cout << i << " : ";
			SS.at(i)->printall();
		}
	
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
	
	  }*/
