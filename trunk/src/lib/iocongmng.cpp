#inlcude "ioCongMng.h"
boost::mutex sndqmutex;

void ioCongMng::pushsndq(touheaderack *touack) {
  boost::mutex::scoped_lock lock(sndqmutex);
  sndq.push_back(*touack);
}

int ioCongMng::popsndq() {
  boost::mutex::scoped_lock lock(sndqmutex);
  touheaderack	touhdack_temp;
  touhdack_temp = sndq.front();
  sockfd = touhdack_temp.sockfd;
  touhdack = touhdack_temp.touheader;
  sndq.pop();
  return 0;
}

/* get the sockaddr_in infomation */
int ioCongMng::assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, char *ip, char *port){
  bzero(sockaddr, sizeof(*sockaddr));
  sockaddr->sin_family = sa_family;
  sockaddr->sin_port = htons((short)ctoi(port));
  if( 1 != inet_pton(sa_family, ip, &sockaddr->sin_addr) )
    return 0;
					    
  return 1; 
}
void ioCongMng::run() {
  sockMng	sockmng;
  sockTb	*socktb;
  touPkg	toupkg;
  
  while(1){
    /*
     * snding part 
     * */
    if( !sndq.empty() ){
      //there's data in the vector 
      if ( 0 == popsndq()) // success return 0
        socktb = sockmng.getSocketTable(sockfd);
      
      toupkg.putHeaderSeq(socktb->tc.snd_una, socktb->tc.snd_nxt); 
      //put thing from socktb's circular buf

      /* set up recv's info */
      assignaddr(&sockaddr, AF_INET, socktb->dip, socktb->dport);
      sendto(sockfd, toupkg, sizeof(toupkg), 0,(struct sockaddr *) &sockaddr, sizeof(sockaddr));

      //parse the sendto func
    }/* End of cking sending part */

    /*
     * recving part 
     * */

  }
}


