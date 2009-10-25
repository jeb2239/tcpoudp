#include<stdio.h>
#include <iostream>
using namespace std;
#include "touheader.h"
#include "timer.h"
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>

//-------------------  TOUMAIN CLASS ---------------
boost::mutex soctabmutex;
class toumain {
 	
	public :
	unsigned long test;
	toupkg tp;
	//touheader t;
	socktb s;
	int tou_close();
	struct sockaddr_in socket1;
	struct sockaddr_in socket2;
	struct sockaddr_in socket3;
	int sd;
	int sd2;
	int yes;
	string touhstring;
	char buf[50],buf1[50],buf3[50],buf4[50],buf5[50];
	timerMng timermng;
	
	void converttobyteorder(toupkg tp) {
	tp.t.seq = htonl(tp.t.seq);
    tp.t.mag = htonl(tp.t.mag);
    tp.t.ack_seq = htonl(tp.t.ack_seq);
    tp.t.syn = htons(tp.t.syn);
    tp.t.ack = htons(tp.t.ack);
    
    }
	
	void convertfrombyteorder(toupkg tp) {
	tp.t.seq = ntohl(tp.t.seq);
    tp.t.mag = ntohl(tp.t.seq);
    tp.t.ack_seq = ntohl(tp.t.seq);
    }
    
	int tou_socket() {
	sd = socket(AF_INET,SOCK_DGRAM,0);
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	return sd;
	
	}
	
	int tou_bind() {
	int rv;
	rv = bind(sd,(struct sockaddr*) &socket1,sizeof socket1);
	return rv;
	}
	
	int tou_listen() { 
	int rv;
	rv = listen(sd,1);
	}
	
	
	//-------------------ACCEPT --------------------- 
	
	int tou_accept() {
		int rv, control=0, flagforsyn = 1;
		size_t len = sizeof(sockaddr);
		converttobyteorder(tp);
		cout << endl << " INSIDE TOU_ACCEPT () " <<endl;
		// receive first handshake
		
		rv = recvfrom(sd, &tp, sizeof tp, 0, (struct sockaddr *)&socket2, &len);
		cout << "seq no received from client : " << tp.t.seq<<endl;
		cout << "mag no received from client : " << tp.t.mag<<endl;
		
		//send syn ack
		tp.t.ack_seq = tp.t.seq + 1;
		cout << "ack no sent to client : " << tp.t.ack_seq<<endl;
		tp.t.seq = rand()%(u_long)65530;
		tp.t.syn = 1;
		tp.t.ack = 1;
		converttobyteorder(tp);
		
			sleep(3);
		
				rv = sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)&socket2, sizeof(struct sockaddr_in));
		
		
		//recv third handshake
		
		rv = recvfrom(sd, &tp, sizeof tp, 0, (struct sockaddr *)&socket2, &len);
		convertfrombyteorder(tp);
		cout << "ack no received for the third handshake : " << tp.t.ack_seq<<endl;
		len = sizeof tp.t;
		cout << " tou header size received :  "<< len<<endl; 
		cout << endl << " LEAVING TOU_ACCEPT () " <<endl;
		
		if (tp.t.ack_seq == tp.t.seq + 1) cout<<"SUCCESS !!!" << endl;
		return true;
	}
	
	
	//-----------------	RECEIVE  --------------------------
	
	int tou_recv() {
	memset(tp.buf,0,50);
	char buf12[10];
	int no1;
	memset(buf12,0,10);
	size_t len = sizeof(sockaddr);
	cout << endl << " INSIDE TOU_RECV () " <<endl;
		
		//timermng.add(1,3333,4000,101);
		no1 = recvfrom(sd, &tp, sizeof tp, 0, (struct sockaddr *)&socket2, &len);
		cout << " Data Received :  " << tp.buf << endl; 
		cout << " no of bytes received : " << no1 <<endl;
		if(tp.buf == "q" )
		{
			cout << " Connection closed by client "<< endl;
			close(sd);
			return true;
		}
		recvfrom(sd, buf12, sizeof (buf12), 0, (struct sockaddr *)&socket2, &len);
		//cout << buf12 << endl;
		int no = atoi(buf12);
		cout << " No of bytes received is :  " << no << endl;
		//recvfrom(sd, &t, sizeof t, 0, (struct sockaddr *)&socket2, &len);
		convertfrombyteorder(tp);
		cout << " Sequence no received is :  " << tp.t.seq << endl;
		if(no1 == no) {
			tp.t.ack_seq = tp.t.seq + 1;
			converttobyteorder(tp);
			sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)&socket2, sizeof(struct sockaddr_in));
			cout << " Ack sent " <<endl; 
		}
		return true;
	}	
};
	
int main()
{
	toumain tm;
	touheader t;
	socktb s;
	int sd,bytes_recieved;
	int sockd;
	tm.yes = 1; 
	sockd = tm.tou_socket();
	
	memset(&tm.socket1,0,sizeof(tm.socket1));
	tm.socket1.sin_family = AF_INET;
	tm.socket1.sin_addr.s_addr=htonl(INADDR_ANY);
	tm.socket1.sin_port = htons(1500);
	memset(&tm.socket2,0,sizeof(tm.socket2));
	tm.socket2.sin_family = AF_INET;
	tm.socket2.sin_addr.s_addr=htonl(INADDR_ANY);
	
	sd = tm.tou_bind();
	unsigned long len;
	
	while(1) {
		sd = tm.tou_accept();
		if (sd == 1) {
		boost::mutex::scoped_lock lock(soctabmutex);
			s.sockd = sockd;
			s.dport = tm.socket1.sin_port;
			s.sport = tm.socket2.sin_port;
			s.dip = inet_ntoa(tm.socket2.sin_addr);
			s.sip = inet_ntoa(tm.socket1.sin_addr);
				
			cout <<"socket table info : "<<endl<<"Sockfd : " << s.sockd<< endl<<"_________________________"<<endl<<" ports : "<< ntohs(s.dport)<<" " << ntohs(s.sport) <<endl<<" ip : "<< s.sip <<" "<<s.dip << endl ;
			}
			while(1) 
		{
			cout << "get the msg from client connect() \n";	
			
			tm.tou_recv();
			/*if (strcmp(recv_data , "q") == 0 || strcmp(recv_data , "Q") == 0)
              {
                close(connected);
                break;
              }

              else 
              printf("\n RECIEVED DATA = %s " , recv_data);*/
              fflush(stdout);

		}
	
	}
	return 0;
}
