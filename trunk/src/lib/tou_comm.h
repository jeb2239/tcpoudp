/**
 * touComm.h: define shared types and include common files
 *
 * Copyright 2009 by Columbia University; all rights reserved
 * May 04, 2010
 */

/******************************************************
 * Type define here
 * ***************************************************/
//Types
typedef	unsigned long 	u_long;
typedef	unsigned short	u_short;
typedef unsigned char	u_char;

#ifndef __TOU_COMMON
#define __TOU_COMMON

#define SOCK_TOU			2		//SOCK_DGRAM(2)

/* ToU packet's payload size */
#define TOU_MSS				1456	//segment size 1500-20-24
#define TOU_MAX				1456

/* ToU flag definition */
#define FLAGON				1
#define FLAGOFF				0

/* Timers */
#define TOUT_TIMERS			4
#define TOUT_REXMIT			0
#define TOUT_PERSIST		1
#define TOUT_KEEP			2
#define TOUT_2MSL			3

/* ToU states */
#define TOUS_CLOSED			0
#define TOUS_LISTEN			1
#define TOUS_SYN_SENT		2
#define TOUS_SYN_RECEIVED	3
#define TOUS_ESTABLISHED	4
#define TOUS_CLOSE_WAIT		5
#define TOUS_FIN_WAIT_1		6
#define TOUS_CLOSING		7
#define TOUS_LAST_ACK		8
#define TOUS_FIN_WAIT_2		9
#define TOUS_TIME_WAIT		10

/* process_tou States */
#define PROCESS_END 0
#define PROCESS_SYN 1
#define PROCESS_FIN 2
#define PROCESS_ACK_WITHOUT_DATA 3
#define PROCESS_ACK_WITH_DATA_MATCH_EXPECTED_SEQ 4
#define PROCESS_ACK_WITH_DATA_LESS_EXPECTED_SEQ 5
#define PROCESS_ACK_WITH_DATA_MORE_EXPECTED_SEQ 6
#define PROCESS_ACK_DATARECSUCC_SENDBACK_ACK 7
#define PROCESS_ACK_DATARECSUCC_NO_SENDBACK_ACK 71
#define PROCESS_GET_PKT 8
#define PROCESS_CLEAR_GET_PKT 9
#define PROCESS_PKT_RECOVERY 10

	
/* ToU delayed timers states (FSM)*/
#define TOUS_DELACK_XMIT 1
#define TOUS_DELACK_QUE 2
#define TOUS_DELACK_QUEXMIT 3	
#define TOUS_DELACK_EXPXMIT 4
#define TOUS_DELACK_IMMED_UPDATE_XMIT 5
#define TOUS_DELACK_ERR 0


#endif //__TOU_COMMON

/***************************************************
 * Include from STD library
 **************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <time.h>
#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <deque>
#include <assert.h>

/***************************************************
 * Include from BOOST library
 **************************************************/
#include "tou_boost.h"
