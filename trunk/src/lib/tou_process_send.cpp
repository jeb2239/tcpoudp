/*************************************************
 * tou_process_send.cpp
 * **********************************************/

#include "tou_process_send.h"

boost::mutex sndqmutex;
int touProcessSend::pushsndq(int sockfd, char *sendbuf, int &len) {
  boost::mutex::scoped_lock lock(sndqmutex);
  int an = 0; //actual number of bytes been sent
  socktb = sockmng.getSocketTable(sockfd);

  /* insert the data into circular buf */
  if( 0 < socktb->CbSendBuf.getAvSize()){
    an = socktb->CbSendBuf.insert(sendbuf, len);
    toups = new touProcessSend(sockfd);
  }
  return an;
}

int touProcessSend::popsndq(sockTb *socktb, char *sendbuf, int len) {
  boost::mutex::scoped_lock lock(sndqmutex);
  int	bread, end;
  memset(sendbuf, 0, TOU_MSS);
  /* try to get data form circular buffer */
  if( 0 < (bread = socktb->CbSendBuf.getAt(sendbuf, len, end))) {
    socktb->CbSendBuf.remove(bread);
    return bread;
  }
  return 0;
}

/* get the sockaddr_in infomation */
int touProcessSend::assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, string ip, unsigned short port){
  bzero(sockaddr, sizeof(*sockaddr));
  sockaddr->sin_family = sa_family;
  /*
  sockaddr->sin_port = htons((short)atoi(port.c_str()));
  */
  sockaddr->sin_port = htons(port);
  if( 1 != inet_pton(sa_family, ip.c_str(), &sockaddr->sin_addr) )
    return 0;

  return 1; 
}

  void touProcessSend::run(int sockfd) {
  socktb = sockmng.getSocketTable(sockfd);
  touPkg toupkg;
  //char	*sndbuf;
  int	totalelm;
  int 	len;
  unsigned long	curwnd =  socktb->sc->getwnd();
  unsigned long sndsize = 0;
  struct sockaddr_in sockaddr;

  i_thread.detach();

  /* set up recv's info */
  touProcessSend::assignaddr(&sockaddr, AF_INET, socktb->dip, socktb->dport);

  //sndbuf = (char*)malloc(sizeof(char)*len);
  while( 0 < (totalelm = socktb->CbSendBuf.getTotalElements())) {
	// there're some data in the buf need to be send 
	cout << " number of bytes in the circular buff: "<<totalelm<<endl;

	/* JUST FOR TEST DEMO, Wait a little while */
	sleep(1);

	/* get the current window size */
	curwnd = socktb->sc->getwnd();
	sndsize=((socktb->tc.snd_una + curwnd)-socktb->tc.snd_nxt);
	cout <<" "<< sndsize<<std::endl;
	while( (0 < socktb->CbSendBuf.getTotalElements() ) && (0 < ( sndsize=((socktb->tc.snd_una + curwnd)- socktb->tc.snd_nxt)))) {
	  toupkg.putHeaderSeq(socktb->tc.snd_nxt, socktb->tc.snd_ack);
	  if( sndsize >= TOU_SMSS ){
	    //available size is bigger than MSS, so send size of SMSS at most
		len = popsndq(socktb, toupkg.buf, TOU_SMSS); //sizeof(toupkg.buf) should be the same as TOU_SMS
		cout << "Client (>= TOU_SMSS)"<<endl;

	  }else{
	    //wnd available for sending is less than SMSS
		len = popsndq(socktb, toupkg.buf, sndsize);
		cout << "Client (< TOU_SMSS)"<<endl;

	  }
	  //snd_nxt moves on
	  socktb->tc.snd_nxt += len;

	  //accumulate the ackcounter
	  (socktb->ackcount)++;

	  // send
	  std::cout << " >>> [SEND] data length sent: "<<len<<", and origianl sndsize is "<<sndsize<<std::endl;
	  sendto(sockfd, &toupkg, sizeof(toupkg), 0, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
	}//end of  while( 0 < ( sndsize=((socktb->tc.snd_una + curwnd)-socktb->tc.snd_nxt)))
  }//end of  while( socktb->CbSendBf.getTotalElements() > 0)
}

