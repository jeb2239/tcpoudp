#include<stdio.h>
#include<stdlib.h>
#include <iostream>
using namespace std;
#include "touheader.h"
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<sstream>
#include <netdb.h>

void addTouHd(char *buf, int n, touheader *th) {
  memset(buf, 0x00, n);
  string a,b,c,d,e,f;
  stringstream outputv,outputv2,outputv3,outputv4,outputv5;
  outputv << th->seq;
  a = outputv.str();
  cout<<"size of seq is "<<sizeof(th->seq)<<endl;
  //a.append(3,'!');
  strncpy(buf,a.c_str(), sizeof(a));
  outputv2 << th->mag;
  b = outputv2.str();
  //b.append(3,'!');
  strncat(buf,b.c_str(), sizeof(a));
  outputv3 << th->ack_seq;
  c = outputv3.str();
  //c.append(3,'!');
  strncat(buf,c.c_str(), sizeof(a));
  //outputv4 << th->flags;
  //cout<<"size of flags is "<<sizeof(th->flags	)<<endl;
  d = outputv4.str();
  //d.append(3,'!');
  strncat(buf,d.c_str(), sizeof(a));
  outputv5 << th->wnd;
  e = outputv5.str();
  //e.append(3,'!');
  strncat(buf,e.c_str(), sizeof(a));
};

class toumain {
 	
	public :
	unsigned long test;
	touheader t;
	int tou_close();
	struct sockaddr_in socket1;
	struct sockaddr_in socket2;
	struct sockaddr_in socket3;
	int sd;
	int sd2;
	//char *buf;
	char buf1[50];
	//int r = initsoc(socket1);	
	
	int tou_socket() {
	sd = socket(AF_INET,SOCK_DGRAM,0);
	return sd;
	}
	
	int tou_bind() {
		int rv;
		rv = bind(sd,(struct sockaddr*) &socket1,sizeof socket1);
		return rv;
	}
	
	int tou_connect() {
		int rv;
		t.seq = rand()%(u_long)65535;
		t.mag = (u_long)9999;
		rv = sendto(sd, &t, sizeof(t), 0, (struct sockaddr*)&socket2, 	sizeof(struct sockaddr_in));
		size_t len = sizeof(sockaddr);
		rv = recvfrom(sd, &t, sizeof t, 0, (struct sockaddr *)&socket2,	 &len);
		perror("talker: sendto");
		cout << "seq no received from server : " << t.seq<<endl;
		cout << "mag no " << t.mag<<endl;
		cout << "ack no received from server : " << t.ack_seq<<endl;
		t.ack_seq = t.seq + 1;
		rv = sendto(sd, &t, sizeof(t), 0, (struct sockaddr*)&socket2, sizeof(struct sockaddr_in));
		return true;
	}
	
	int tou_send(char *buf) {
		ssize_t no;
		buf[strlen(buf)] = '\0';
		no = sendto(sd, buf, strlen(buf),0,(struct sockaddr*)&socket2, sizeof(struct sockaddr_in));
		cout << " Data Sent : " << buf << endl;
		
		//buf1 = itoa(no);
		std::ostringstream buf12;
		buf12 << no;
		cout << "no of bytes sent aftr conversion : " << buf12.str() << endl;
		sendto(sd, buf12, sizeof(buf12),0,(struct sockaddr*)&socket2, sizeof(struct sockaddr_in));
		t.seq = t.seq + 1;
		cout << " Sequence no of data sent  : " << t.seq << endl;
		sendto(sd, &t, sizeof(t),0,(struct sockaddr*)&socket2, sizeof(struct sockaddr_in));
		memset(buf,0x00,sizeof(buf)+1);
	
		return true;
	}
};
	
int main(int argc, char* argv[])
{
	touheader t;
	toumain tm;
	int sd;
	struct hostent *h;
	cout << " Hi ther " <<endl;
	//memset(tm.buf, 0, strlen(argv[2]));
	//cout << "Memory set" <<endl;
	//strcpy(tm.buf, argv[2]);
	
	h = gethostbyname(argv[1]);

    if(h==NULL) {
      printf("%s: unknown host '%s' \n", argv[0], argv[1]);
    }

    printf("%s: sending data to '%s' (IP : %s) \n", argv[0], h->h_name,
    inet_ntoa(*(struct in_addr *)h->h_addr_list[0]));
    memset(&tm.socket2, 0, sizeof(tm.socket2));
   	tm.socket2.sin_family = h->h_addrtype;
   	memcpy((char *) &tm.socket2.sin_addr.s_addr,
    h->h_addr_list[0], h->h_length);
    tm.socket2.sin_port = htons(1500);
    //printf("%d, %d\n", tm.socket2.sin_addr.s_addr, tm.socket2.sin_port);
	sd = tm.tou_socket();
	//cout<<"Socket"<< sd <<endl;
	memset(&tm.socket1,0,sizeof(tm.socket1));
	tm.socket1.sin_family = AF_INET;
	tm.socket1.sin_addr.s_addr=htonl(INADDR_ANY);
	tm.socket1.sin_port = htons(1501);
	sd = tm.tou_bind();
	//cout<<"Bind" << sd <<endl;
	unsigned long len;
	len = sizeof tm.t;
	cout<< "Length of header  : " << len <<endl;
	sd = tm.tou_connect();
	//cout <<"Connect : " <<sd << endl;
	string s1 = inet_ntoa(tm.socket2.sin_addr);
	string s2 = inet_ntoa(tm.socket1.sin_addr);
	//cout << " Ips are  : " << s1 << "  "<< s2<<endl ;
	tm.tou_send(argv[2]);
	return 0;
}
