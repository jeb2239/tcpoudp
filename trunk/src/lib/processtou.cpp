#include "processtou.h"
#include "tou.h"
boost::mutex recvqmutex;

/*
 * This will handle all the incoming packets and 
 * will check if it is a SYN packet or data packet or FIN packet
 */

void processTou::run(int sockfd) {
  sockMng	sockmng;
	touMain tm;
  touPkg	tp;
  timerMng timer;
  static int timerid;
  size_t len = sizeof(sockaddr_in);
      
  sockaddr_in sockaddrs ;
  cout << "Waiting for data from client "  << endl;
  int rv = recvfrom(sockfd, &tp, sizeof tp, 0, (struct sockaddr*)&sockaddrs,&len);
  tm.convertFromByteOrder(tp);
  /*
   *check for SYN
   */

 if(tp.t.syn == 1) {
    sockmng.setSocketTableD(&sockaddrs,sockfd);
    sockmng.setSocketState(TOUS_SYN_RECEIVED,sockfd);
    sockmng.setTCB(tp.t.seq,sockfd);  
    
  }

  /*
   * Check for ACK
   */
/*  if(tp.t.ack == 1) {
    tm1.delete_timer(sockfd,2,tp.t.seq);
  }
  
	*/
  /*
   *check for data
   */
  
  if((strlen(tp.buf)) > 0) {
  cout << " Accepting data .....  " <<tp.buf <<  endl;
  sockmng.setTCB(tp.t.seq,sockfd);                
  boost::mutex::scoped_lock lock(socktabmutex1);        
  int lenofbuf = strlen(tp.buf);
  int lenofcb = sockmng.s->CbRecvBuf.getSize();
  if(lenofbuf <= lenofcb) {
    sockmng.s->CbRecvBuf.insert(tp.buf, lenofbuf);
  }
  else {
    sockmng.s->CbRecvBuf.insert(tp.buf, lenofcb);
  }
    sockmng.setCbData(tp.buf,sockfd,lenofbuf);
    cout << " Leaving process tou " << endl;
    
  //write code for ack send
  //
  //sockTb = sockmng.getSocketTable(sd);
   //struct sockaddr_in sockaddrs,socket1;
  //u_short port = (s->dport);
  //assignaddr(&sockaddrs,AF_INET,sockTb->dip,port);
  //tp.t.ack = 1;
  //sendto(sd,&tp,sizeof(tp),(struct sockaddr *)&sockaddrs,sizeof(sockaddr));
  }

  /*
   *check for close
   */

  if(tp.t.fin == 1) {
  cout<<"Closing Connection... " << endl;        
  boost::mutex::scoped_lock lock(socktabmutex1);
  tm.convertToByteOrder(tp);
  sockmng.setTCB(tp.t.seq,sockfd); 
  sockmng.setSocketState(TOUS_CLOSE_WAIT,sockfd);
  
  }

}
 

