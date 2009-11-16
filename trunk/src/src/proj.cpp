#include "tou.h"

int assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, string ip, unsigned short port){
	  bzero(sockaddr, sizeof(*sockaddr));
		  sockaddr->sin_family = sa_family;
			  sockaddr->sin_port = htons(port);
				  if( 1 != inet_pton(sa_family, ip.c_str(), &sockaddr->sin_addr) )
						    return 0;
					  return 1;
};


int main(int argc, char* argv[]){
	touMain tm;
	struct sockaddr_in socket1;
	struct sockaddr_in socket2;
	char send_data[1024],recv_data[1024];

	std::cout << " Welcom to ToU \n" ;
  if(argc ==3 && !strncmp( argv[2], "-c", 2))
	{
		cout << "Client mode.. " << endl;
		int sd,sockd;
		int bytes_recieved; 
		struct hostent *h;
		//Get host name
		if(h==gethostbyname(argv[1])) 
      printf("%s: unknown host '%s' \n", argv[0], argv[1]);

    //printf("%s: sending data to '%s' (IP : %s) \n", argv[0], h->h_name, inet_ntoa(*(struct in_addr *)h->h_addr_list[0]));
    
    //Set socket 1
	  //Server socket //WHAT IS THIS????????????????????????????????????????????????????
    memset(&socket1, 0, sizeof(socket1));
   	socket1.sin_family = h->h_addrtype;
   	memcpy((char *) &socket1.sin_addr.s_addr,h->h_addr_list[0], h->h_length);
    socket1.sin_port = htons(1500);

		assignaddr(&socket1, AF_INET, "127.0.0.1", 1500);

		std::cout<<inet_ntoa(socket1.sin_addr)<< "  " <<htons(socket1.sin_port) << std::endl;
		//Client socket
		sockd = tm.touSocket(AF_INET,SOCK_DGRAM,0);
	
		//Set socket 2
		memset(&socket2,0,sizeof(socket2));
		socket2.sin_family = AF_INET;
		socket2.sin_addr.s_addr=inet_addr("127.0.0.1");
		socket2.sin_port = htons(1501);
	
		//BIND //CLIENT SHOULD NOT BINE???????????????????????????????????????????????????????
		sd = tm.touBind(sockd,(struct sockaddr*) &socket2,sizeof socket2);
		unsigned long len;
	
		//CONNECT
		sd = tm.touConnect(sockd,(struct sockaddr_in*)&socket1,sizeof(socket1));
		cout << "Connect returns : "<< sd << endl;
		
		//Initialize process tou
		tm.proTou(sockd);

		//while(1)
		{
			cout<< "Enter data to be sent : " << endl;
			gets(send_data);
			cout<< "Now sending...: " << endl;
			tm.touSend(sockd,send_data,strlen(send_data),0);
			tm.ptou->run(sockd);
		} 
		tm.touClose(sockd);
		/* End of Client Code */
	
	}else if ( argc==2 && !strncmp(argv[1], "-s", 2)){
		std::cout << "Welcome to Sever Mode!\n";
		int sd,bytes_recieved,lis_return;
		int sockd;
		cout << "INSIDE SERVER MODE.. " << endl;

		//CREATE 
		sockd = tm.touSocket(AF_INET,SOCK_DGRAM,0);
		cout << " Socket function returned : " << sockd << endl;

		//Set socket structures
		memset(&socket1,0,sizeof(socket1));
		socket1.sin_family = AF_INET;
		socket1.sin_addr.s_addr=inet_addr("127.0.0.1");
		socket1.sin_port = htons(1500);
		memset(&socket2,0,sizeof(socket2));

		//BIND
		sd = tm.touBind(sockd,(struct sockaddr*) &socket1,sizeof socket1);
		cout << " Bind Returns : "<< sd << endl;

		//LISTEN
		lis_return = tm.touListen(sockd,1);
		cout << "Ready to accept. Return: "<<lis_return  << endl;

		//Check if it is tou socket
    fd_set socks;
 		struct timeval tim;
 		FD_ZERO(&socks);
 		FD_SET(sockd, &socks);
 		tim.tv_sec = 4;

		//Initialize processTou
		tm.proTou(sockd);

		//  while(1)
		{    
    //if (select(sockd+1, &socks, NULL, NULL, &tim))
			{
				//TODO : Write logic for checking tou sockd here
				cout << "Inside ...... " << endl;
			}
		}      

		//processTou *p = new processTou(sockd);
		socklen_t sinlen = sizeof(socket2);
		tm.ptou->run(sockd);
		sd = tm.touAccept(sockd,(struct sockaddr_in*)&socket2,&sinlen);
		cout << " Accept is donw, and now for the receiving part " << endl;

		//while(1){    
		//	if (select(sockd+1, &socks, NULL, NULL, &tim)){ //SELECT FORM WHAT TO WAHT?????????????????????????????????????????????????
				//TODO : Write logic for checking tou sockd here
				cout << "Inside Select ...... " << endl;
				tm.ptou->run(sockd);
				
				memset(recv_data, 0, sizeof(recv_data));
				tm.touRecv(sockd,recv_data,100,0);
				cout << "Received Data : " << recv_data << endl;
			//}
		//}      
		cout << "Done with the receiving part " << endl;
	
		if (select(sockd+1, &socks, NULL, NULL, &tim)){
			//TODO : Write logic for checking tou sockd here
			cout << "Inside Close...... " << endl;
			tm.ptou->run(sockd);
			tm.touClose(sockd);
    }      
  }/*END OF SERVER PART*/

	return 0;
}
