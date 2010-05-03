/**
 * touHeader
 * Primitive ToU header definitions.
 *
 * Copyright 2009 by Columbia University; all rights reserved
 * Jan 10, 2010
 * Feb 21, 2010
 */

/* ToU packet's payload size */
#define TOU_MSS				1456
#define TOU_MAX				1456

/* ToU flag definition */
#define FLAGON				1
#define FLAGOFF				0

#include <iostream>
#include <string>
#include <stdio.h>

/**
 * touHeader: 
 * The size of fixed ToU header is 16 bytes, whereas the size of fixed TCP 
 * header is 20 bytes. The fixed ToU header and UDP header have a cumulative 
 * size of 24 bytes, four more bytes than a fixed TCP header.
 */
class touHeader {
  public:
	u_long	seq;		// Sequence Number: 32 bits
	u_long	mag;		// Magic Cookie: 32-bits
	u_long	ack_seq;// Acknowledgment Number: 32 bits
	
	u_short	doff:4; // Data Offset: 4 bits
	u_short	dres:4; // Reserved: 4 bits
	u_short	res:1;	// Reserved flag: 1 bit
	u_short	cwr:1;	// Congestion window reduced flag: 1 bit
	u_short	ece:1;  // ECN-Echo flag: 1 bit
	u_short	ack:1;	// Acknowledgement: 1 bit
	u_short	psh:1;  // Push Function: 1 bit
	u_short	rst:1;	// Reset the connection: 1 bit
	u_short	syn:1;	// Synchronize sequence numbers: 1 bit
	u_short	fin:1;	// No more data from sender: 1 bit
	u_short	window; // Window size: 16 bits
};

/**
 * touPkg:
 * ToU Header plus Data(payload)
 */
class touPkg {
	public:
		touHeader	t;			// ToU's header
		std::string	*buf;	// Payload
		int buflen;				// Length of payload

		/**
		 * default constructor 
		 * new a pkg with size of TOU_MAX 
		 */
		touPkg();

		/**
		 * copy contructor
		 */
		touPkg(const touPkg &toupkt);

		/**
		 * constructor
		 * specify the size with user specified number
		 */
		touPkg(int payloadsize);
		
		/**
		 * constructor
		 * convert string to touPkg format
		 */
		touPkg(std::string content);
		
		/**
		 * constructor
		 * specify the size the data
		 */
		touPkg(const char *str, int payloadsize);

		/**
		 * destructor
		 */
		~touPkg(){
			//delete buf;
		}

		/**
		 * toString
		 * Convert ToU packet class into string format
		 */
		std::string toString();

		void setBuf( const char *str, int size );

		int putHeaderSeq(u_long seq=0, u_long ack_seq=0);

		int clearFlag();

		int clean();
		
		/**
		 * This is a test function for logging or printing into screen.
		 */
		void log();

		void log(unsigned short logflag);

		unsigned long getSeq();

		unsigned long getAckseq();

		std::string getBuf();
};
