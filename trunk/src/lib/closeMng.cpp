#include "tou.h"
void touMain::clientClose(int sd) {
cout << " Inside client close " << endl;
sockTb *s;
sockaddr_in sockaddrs,socket1;
s = sm.getSocketTable(sd);
size_t len = sizeof(sockaddr);
/*
tp.t.seq = s->tc.snd_nxt;
u_short port = (s->dport);
assignaddr(&sockaddrs,AF_INET,s->dip,port);
tp.t.fin = 1;
tp.t.ack_seq = tp.t.seq + 1;
cout << "sending fin " << endl;
int rv = sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)&sockaddrs , sizeof(struct sockaddr_in));
*/
sm.setSocketState(TOUS_FIN_WAIT_1,sd);
cout << " Receiving FIn ACK " << endl;
recvfrom(sd,&tp,sizeof(tp),0,(struct sockaddr *)&socket1,&len);
if(tp.t.fin ==1 && tp.t.ack ==1) {
  sm.setSocketState(TOUS_TIME_WAIT,sd);
  }
tp.t.ack == 1;
cout << " Sending ACK " << endl;
sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)&sockaddrs , sizeof(struct sockaddr_in));
sm.delSocketTable(sd);
close(sd);  
}

void touMain::serverClose(int sd) {
cout << " Inside server close " << endl;
sockTb *s;
sockaddr_in sockaddrs,socket1;
s = sm.getSocketTable(sd);
size_t len = sizeof(sockaddr);
tp.t.seq = s->tc.snd_nxt;
u_short port = (s->dport);
assignaddr(&sockaddrs,AF_INET,s->dip,port);
if(s->sockstate == TOUS_CLOSE_WAIT) {
  cout << "Sending FIN ACK " << endl;
  tp.t.fin = 1;
  tp.t.ack = 1; 
  int rv = sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)&sockaddrs , sizeof(struct sockaddr_in)); 

  fd_set socks;
 	struct timeval tim;
 	FD_ZERO(&socks);
 	FD_SET(sd, &socks);
 	tim.tv_sec = 30; 
  size_t  len = sizeof(sockaddrs);
  while(1){    
			if (select(sd+1, &socks, NULL, NULL, &tim)) {
        cout << " Waiting 2 MSL " << endl ;
        recvfrom(sd,&tp, sizeof(tp),0,(struct sockaddr*)&sockaddrs , &len);
        if(tp.t.ack == 1)
         {
          close(sd);
          return 1;
         }
       }
    else {
        cout << " 2 MSL Over "  << endl;
         close(sd);
          return 1;
      }
    }
  }
}
