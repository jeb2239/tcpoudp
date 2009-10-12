#include<stdio.h>
#include <iostream>
using namespace std;
#include "touheader.h"
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include <netdb.h>

void addTouHd(char *buf, int n, touheader *th) {
  strncpy(buf,(char*) th->seq, sizeof(th->seq));
  strncat(buf,(char*) th->magic, sizeof(th->magic));
  strncat(buf,(char*) th->ack_seq, sizeof(th->ack_seq));
  strncat(buf,(char*) th->res1,1);
  strncat(buf,(char*) th->doff,1);
  strncat(buf,(char*) th->fin, 1);
  strncat(buf,(char*) th->syn, 1);
  strncat(buf,(char*) th->rst, 1);
  strncat(buf,(char*) th->psh, 1);
  strncat(buf,(char*) th->ack, 1);
  strncat(buf,(char*) th->rev, 1);
  strncat(buf,(char*) th->ece, 1);
  strncat(buf,(char*) th->cwr, 1);
  strncat(buf,(char*) th->wnd, 1);

};

class toumain {
 	
	public :
	unsigned long test;
	touheader t;
	//int tou_socket(void);
	//int tou_accept();
	//int tou_bind();
	//int tou_listen();
	int tou_send();
	int tou_recv();
	int tou_close();
	struct sockaddr_in socket1;
	struct sockaddr_in socket2;
	struct sockaddr_in socket3;
	int sd;
	int sd2;
	char buf[50],buf1[50];
	/*
	
	int initsoc(struct sockaddr_in &socket) {
	memset(&socket,0,sizeof(socket));
	socket.sin_family = AF_INET;
	socket.sin_addr.s_addr=htonl(INADDR_ANY);	
	//return socket;
	return 0;	
	}
	
	int r = initsoc(socket1);	
	*/
	int tou_socket() {
	sd = socket(AF_INET,SOCK_DGRAM,0);
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
	/*
	int tou_accept() {
	int rv;
	rv = recvfrom(sd, buf1, 50, 0, (struct sockaddr *)&socket3, sizeof(struct sockaddr));
	return rv;
	}
	*/
	int tou_connect() {
	int rv;
	unsigned long seqnum = 0;
	t.syn = ++seqnum;
	t.ack = 1;
	char msgbuf[101] = {0};
        addTouHd(buf,sizeof(buf),&t);
	rv = sendto(sd, buf, sizeof(buf), 0, (struct sockaddr*)&socket2, sizeof(struct sockaddr_in));
	printf("%d, %s, %d \n", sd, buf, sizeof(buf));
	perror("talker: sendto");
	return rv;
	}//*/
};
	
int main(int argc, char* argv[])
{
	touheader t;
	toumain tm;
	int sd;
	struct hostent *h;
	memset(tm.buf, 0, sizeof(tm.buf));
	strcpy(tm.buf, argv[2]);
	//tm.buf = argv[2];
	
	h = gethostbyname(argv[1]);
    if(h==NULL) {
      printf("%s: unknown host '%s' \n", argv[0], argv[1]);
      //exit(1);
    }

    printf("%s: sending data to '%s' (IP : %s) \n", argv[0], h->h_name,
    inet_ntoa(*(struct in_addr *)h->h_addr_list[0]));
      memset(&tm.socket2, 0, sizeof(tm.socket2));
   	tm.socket2.sin_family = h->h_addrtype;
   	memcpy((char *) &tm.socket2.sin_addr.s_addr,
    h->h_addr_list[0], h->h_length);
    tm.socket2.sin_port = htons(1500);
    printf("%d, %d\n", tm.socket2.sin_addr.s_addr, tm.socket2.sin_port);
	
	sd = tm.tou_socket();
	cout<<"Socket"<< sd <<endl;
	memset(&tm.socket1,0,sizeof(tm.socket1));
	tm.socket1.sin_family = AF_INET;
	tm.socket1.sin_addr.s_addr=htonl(INADDR_ANY);
	tm.socket1.sin_port = htons(1501);
/*
	memset(&tm.socket2,0,sizeof(tm.socket2));
	tm.socket2.sin_family = AF_INET;
	tm.socket2.sin_addr.s_addr=htonl(INADDR_ANY);
*/
	//sd = tm.tou_bind();
	cout<<"Bind" << sd <<endl;

	unsigned long len;
	len = sizeof tm.t;
	cout<< len <<endl;
	sd = tm.tou_connect();
	cout << sd << endl;
	//sd = tou_accept();
	//cout << sd << endl;
	//sd = tm.tou_listen();
	//cout<<sd<<endl;
	return 0;
}
