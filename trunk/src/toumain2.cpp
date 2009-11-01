#include "tou.h"
	
int main()
{
	touMain tm;
	touHeader t;
	//sockTb s1;
	sockMng sm;
	int sd,bytes_recieved;
	int sockd;
	tm.yes = 1;
	struct sockaddr_in socket1;
	struct sockaddr_in socket2;
	struct sockaddr_in socket3;
	
	//CREATE 
	sockd = tm.touSocket(AF_INET,SOCK_DGRAM,0);
	cout << " Socket function returned : " << sockd << endl;
	//Set socket structures
	memset(&socket1,0,sizeof(socket1));
	socket1.sin_family = AF_INET;
	socket1.sin_addr.s_addr=htonl(INADDR_ANY);
	socket1.sin_port = htons(1500);
	memset(&socket2,0,sizeof(socket2));
	socket2.sin_family = AF_INET;
	socket2.sin_addr.s_addr=htonl(INADDR_ANY);
	
	//BIND
	sd = tm.touBind(sockd,(struct sockaddr*) &socket1,sizeof socket1);
	cout << " Bind Returns : "<< sd << endl;
	
	unsigned long len;
	
	while(1) {
		socklen_t sinlen = sizeof(socket2);
		sd = tm.touAccept(sockd,(struct sockaddr_in*)&socket2,&sinlen);
		if (sd == 1) {
				cout << "SUCCESS !! " << endl;
				}
//		tm.touRecv(
	}
	
	
	return 0;
}
