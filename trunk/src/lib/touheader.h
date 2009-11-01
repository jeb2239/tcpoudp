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
	u_short res2:2;
	u_short urg:1;
	u_short ack:1;
	u_short psh:1;
	u_short rst:1;
	u_short syn:1;
	u_short fin:1;

	u_short window;
	u_short check;	
};

/******************************************************
 * ToU application layer: ToU hdr + Data(payload)
 * ***************************************************/
class touPkg {
  public:
    touHeader		t;
    char 			buf[TOU_MSS];
};


