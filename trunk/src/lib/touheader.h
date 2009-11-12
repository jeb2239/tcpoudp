/***************************************************
 *TOU HEADER
 **************************************************/
class touHeader {
  public:
    u_long		seq;
    u_long		mag;
    u_long		ack_seq;
    
    u_short doff:4;
	  u_short res1:4;
	  u_short res2:1;
	  u_short cwr:1;
	  u_short ece:1;
	  u_short ack:1;
	  u_short psh:1;
	  u_short rst:1;
	  u_short syn:1;
	  u_short fin:1;

	  u_short window;
};

/******************************************************
 * ToU application layer: ToU hdr + Data(payload)
 * ***************************************************/
class touPkg {
  public:
    touHeader	t;
    char			buf[TOU_MSS];

    int putHeaderSeq(u_long seq=0, u_long ack_seq=0){
      t.seq = htonl(seq);
      t.ack_seq = htonl(ack_seq);
      t.mag = htonl(0x7194B32E);
    }

};

class touheaderack {
  public:
    int			sockfd;
    touHeader	touheader;

    touheaderack(int sockfd_, touHeader touhd_) {
      sockfd = sockfd_;
      touheader = touhd_;
    }
};


