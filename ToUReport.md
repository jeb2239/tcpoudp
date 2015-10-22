TCP over UDP

Cheng-Han Lee
Columbia University
NY, NY 10027
USA
cl2804@columbia.edu

Chinmay Gaikwad
Columbia University
NY, NY 10027
USA
cyg2104@cc.columbia.edu

1. Abstract

The intension of the project is to develop a building block to implement an extensive library which can be used in the scenario where users behind network address translators (NATs) are not able to establish TCP connections but are able to establish UDP connection. The main objective of the project is to implement a TCP-over-UDP (ToU) library that provides socket-like APIs in which users can build connections that have almost the same congestion control, flow control, and connection control mechanism as offered by TCP. In this document, we present architectures and concepts of implementing ToU library based on the IETF draft TCP-over-UDP [draft-baset-tsvwg-tcp-over-udp-01], and describe sample codes for simple scenarios of a client and a server that are using ToU library.


2. Introduction

It becomes a problem sometimes when establishing a direct TCP connection between two hosts behind network address translators (NATs). When both the client and server are behind different NAT devices, the applications running on hosts may not be able to establish a direct TCP connection with one another. However, with certain NAT types, although applications cannot establish TCP connections directly, it is possible for these applications to exchange user datagram protocol (UDP) traffic. In this sense, using UDP is preferable for such applications; nevertheless, applications may require the underlying transport protocol provides reliability, congestion control, and flow control mechanisms. Unlike TCP, UDP fails to provide these semantics. Therefore, the TCP-over-UDP (ToU), a reliable, congestion control, and flow control transport protocol on top of UDP, is proposed, and in order to achieve these mechanisms, ToU almost uses the same header as TCP does which allows ToU to easily implement TCP's reliability, congestion control algorithms and congestion control algorithms.


3. Related

ToU takes advantage of existing user-level-TCP (such as Daytona [Daytona](Daytona.md) and MINET [MINET](MINET.md)) and TCP- over-UDP implementations (such as atou [atou](atou.md)).


4. Background

4.1. General introduction of UDP

UDP uses a simple transmission model without implicit hand-shaking dialogues for guaranteeing reliability, ordering, or data integrity. Thus, UDP provides an unreliable service and datagrams may arrive out of order, appear duplicated, or go missing without notice. UDP assumes that error checking and correction is either not necessary or performed in the application, avoiding the overhead of such processing at the network interface level.
4.2. General introduction of TCP

TCP provides a connection oriented, reliable, byte stream service. Firstly, the term connection-oriented means the two applications using TCP must establish a TCP connection before they can exchange data. Secondly, for achieving reliability, TCP assigns a sequence number to each byte transmitted, and expects a positive acknowledgment (ACK) from the receiving TCP. If the ACK is not received within a timeout interval, the data is retransmitted. The receiving TCP uses the sequence numbers to rearrange the segments when they arrive out of order, and to eliminate duplicate segments.  Finally, TCP transfers a contiguous stream of bytes. TCP does this by grouping the bytes in TCP segments, which are passed to IP for transmission to the destination. TCP itself decides how to segment the data and it may forward the data at its own convenience.

In addition to the properties above, TCP is also a full duplex protocol, meaning that each TCP connection supports a pair of byte streams, one flowing in each direction.

4.3. General introduction of ToU

ToU follows the algorithms described in TCP, including flow-control mechanism, connection control mechanism, and congestion control mechanism. In flow control, while a ToU receiver sends an ACK back to sender, the ACK packet also indicates sender the number of bytes a receiver can receive beyond the last received ToP segment, without causing overrun and overflow in its internal buffers. For congestion control, ToU adopts TCP congestion control [I-D.ietf-tcpm-rfc2581bis] document. In this sense, ToU will go through the slow-start, congestion-avoidance, and fast-retransmit phases. During slow start, a ToU sender starts with a congestion window (cwnd) of two times the MSS bytes, and increments it by at most MSS bytes for each ACK received that cumulatively acknowledges new data. Specifically, the congestion window size doubles in every RTT, which results in the cwnd glows exponentially. When the cwnd exceeds a threshold (ssthresh), the algorithm switches to congestion avoidance. During the phase of congestion control, the congestion window is additively increased by one MSS in every RRT. If a timeout occurs, ToU will reduce the size of cwnd to one MSS and return to slow start phase. If receiving three duplicated acknowledgments, ToU will halve the congestion window size, and enter the fast recovery state.
During fast recovery phase, ToU is following Reno standard. ToU will detect and repair losses. It retransmits the lost packets that was indicated by three
duplicated acknowledgments, and wait for new acknowledgment. Once receiving a new acknowledgment, ToU will return to congestion avoidance phase. If a timeout occurs, it will go back to slow start phase, and reduce the size of cwnd to one MSS.

5. Architecture


ToU is a user-level library that provides socket-like function calls to the applications, including touSocket, touAccept, touConnect, touBind, touListen, touSend, touRecv, and touClose. From the application point of view, applications can interact with the ToU library through these calls by including "tou.h" The return values and function parameters of ToU library are like Berkeley Socket API.
The ToU library should interact with application thread along with a supporting timer thread and a close thread. The close thread is instantiated at the near end of the program, and the other two threads are running at the start. Thus, basically, the ToU is a two-thread model library. The library is fully modularized, implementing the reliable, inorder, flow control, connection control, congestion control semantics of TCP. The following paragraphs are going to illustrate various components in detail, including ToU fundamentals, processtou function, timer module, congestion control module, 3-way handshake and closure implementations, connection control module, and logging mechanisms.

5.1. ToU and Its Data Structure

ToU adopts two-thread architecture, and the whole library is implemented on top of UDP in C/C++ with 1.40.0 Boost libraries, including Boost.Thread and Boost.Timer. The library should be used by including the header file "tou.h" in applications and compile with relevant Boost libraries.
Similar to the data structure described in TCP [- TCP Control Block Interdependence](RFC2140.md) documents, ToU exercises the ToU control block (touCb) to handle the meta data and transmission status for each ToU connection. Also, ToU makes use of the ToU socket tables (sockTb) to keep touCb and relevant information for management of ToU connection, including congestion status, circular buffers for incoming/outgoing data, and logging mechanisms. To meet the needs of dealing with multiple connections, ToU has a ToU socket management (sockMng) in which the sockTb queue is used for handling sockTbs, and some supporting functions are implemented so as to govern each sockTb. Following diagram shows the anatomy of ToU data structure.


sockTbQ is a vector that contains all of the sockTbs. Each sockTb maps to a distinguish file descriptor which represents a socket that used in biding and building connections. ToU operates sockTbs by using of sockMng which provides functions that can fetch and push sockTb instantiations from and into the soTbQ vector.

5.2. Processtou  function

Processtou is a class that responsible for handling most of the I/O operations. The run() function in processtou should be called after every I/O functions, e.g., touSend and touRecv, in order to literally send out the data. In the sending scenario, when a touSend is called, the data will be placed into sending circular buffer (CbSendBuf). Upon the run() function in processtou is called, data in circular buffer may be sent out based on the congestion/flow window size and relevant control mechanisms. In the receiving scenario, when a touRecv is called, the data will be copied into the application-specified buffer from the receiving circular buffer (CbRecvBuf).
The behaviors of sending and receiving in processtou are implemented with a state machine (SM) as illustrated below:

In the function of run(), a switch case is performed on the value of "state" which is a result of processGetPktState() function. processGetPktState() basically inspects the incoming packet and, based on current ToU connection status and ToU header of the packet, returns a proper "state". There are various states, and we will give brief explanation below,
PROCESS\_SYN: The receiving of a SYN packet implies a start of a connection, so the main task of this state is to initialize the relevant variables in socket tables, including socket states, sequence numbers, and IP/ports.
PROCESS\_FIN: The receiving of a FIN packet implies an end of a connection, so the main task is to set the socket state to CLOSE\_WAIT state so as to make touClose() performing four-way closure procedure.
PROCESS\_ACK\_WITHOUT\_DATA: While receiving an ACK, we need to decide whether it is a duplicate ACK or not. By looking into the del\_timer\_queue(), if there is no node found in del\_timer\_queue(), we push a new timer node into del\_timer\_queue(), and update the window size and sequence number. If there is a node exactly the same as currently received ACK, we then check whether the duplicate ACK has exceeded three. If it is, we then perform fast-retransmit. In the end of PROCESS\_ACK\_WITHOUT\_DATA state, we try to send data queuing in circular buffer since the ACK may increase window size that leaves more sending permit.
PROCESS\_ACK\_WITH\_DATA\_MATCH\_EXPECTED\_SEQ: The main purpose of this state is to put data into the receiving circular buffer; however, there are cases where the incoming packets may be out-of-order. In order to solve this problem, we firstly check HpRecvBuf, a buffer used to keep out-of-order packets. On one hand, if HpRecvBuf is empty, meaning that we need not to consider out-of-order packet problems. We simply update the sequence number and put the data into circular buffer. On the other hand,if it is not, we then try to recover packets from HpRecvBuf. We will try to recover as many packets as possible as long as the sequence number of recovered packets is continuous. During the process of recovering, we also update the sequence number as well as pop out the top value of HpRecvBuf. After putting data into  circular buffer, we need to send an ACK packet back. So the PROCESS\_ACK\_WITH\_DATA\_MATCH\_EXPECTED\_SEQ state will move to PROCESS\_ACK\_DATARECSUCC\_SENDBACK\_ACK state.
PROCESS\_ACK\_WITH\_DATA\_LESS\_EXPECTED\_SEQ: This state implies that the sequence number of incoming packet is less than the current sequence number maintained in ToU control block in socket table. So we can simply ignore it since the it is already acknowledged.
PROCESS\_ACK\_WITH\_DATA\_MORE\_EXPECTED\_SEQ: This state implies that the sequence number of incoming packet is exceeding the sequence number what ToU has expected in ToU control block in socket table. We consider it as an out-of-order packet, and put it into HpRecvBuf.
PROCESS\_ACK\_DATARECSUCC\_SENDBACK\_ACK: Send an ACK packet back.
PROCESS\_END: Terminal state.

Another important function in processtou is run() function. This function will consider the window size and maximum segment size (MSS), deciding how many bytes can be send in each loop, and tries to send as much as it can. Also, after sending the data, run() will put a timer node into timerMngQ to keep track of the sending data.

Note: implementation alternatives, another thread. tbc

Timer

Timer has its own thread of execution which is instantiated at very beginning and is responsible for keeping track of transmitted packets and retransmitting lost packets. Each time ToU send a packet, timer will store a timer node into timer priority queue along with relevant packet header information and payload. Timer priority queue is sorted in ascending order so that timer can keep track of the time of which timer nodes have exceeded the current time. Timer then keeps check the root of timer heap tree and counts down over time in every 500 millisecond. If the timer expired before an acknowledgment is received for the root timer node in timer priority queue, ToU retransmit the packet and reset the timer node. After reset, the countdown begins again and the process repeated. If an acknowledgment is received for a root timer node before its timer expires, ToU will put a deletion timer node in deletion timer priority queue. Thus, while timer is expired, timer node will be popped from the timer priority queue without any retransmission reaction.

Note: time complexity analysis. tbc

Congestion control
To achieve the goal of avoiding congestion collapse, we have referred to the mechanisms used in TCP. Therefore, the implementation of congestion control in ToU contains four intertwined algorithms, including slow start, congestion avoidance, fast retransmit, and fast recovery. Acknowledgements of data sent, three duplicate acknowledgements, and acknowledgement timeout are three conditions used to infer the situation of connections between the sender and receiver. The keys of controlling the data entering the network are flow window size (awnd) and congestion window size (cwnd). We have implemented tou\_ss\_ca.cpp for the control of window sizes in which the functions like addwnd(), getwnd(), settwnd(), and setdwnd() have been introduced and should be used in the conditions like receiving an ACK, three-duplicate ACKs, and timeout. Following is the finite state machine that illustrates the states of congestion control.

The files that related to the state machine are tou\_ss\_ca.cpp and  tou\_congestion.h, which make use of the ToU tables and data described in ToU data structure section. These files are tou\_control\_block.h and  tou\_sock\_table.h.

3-way Handshake and Close Implementations

(Chinmay should rewrite this part) Close thread is used to handle connection tear down. The purpose of the close thread is that the users thread should be non-blocking after it receives a FIN. Thus, when the server goes in the CLOSE\_WAIT state, a new thread is spawned to handle the 4 way closure.

Connection Control

(Chinmay should rewrite this part) Multiple users and multiple clients.
In addition to the tou thread, main thread, run by the user , ToU has a timer thread which will run along with the tou thread at the beginning of the  process; and has another close thread run while the user calls the tou\_close function. ToU (main) thread is consist of three main parts which are
Circular buffer acts as an interface of receiving and sending data between ToU library and applications. When applications try to send or receive data through ToU function calls, such as tou\_send and tou\_recv, ToU library will first push data into circular buffer, and manages to send and receive data in circular buffer depends on the network situation by congestion control mechanism.

Logging Mechanism

The logging mechanism consists of seven different categories of logging results: tou.log, tou\_timer.log, tou\_pktsent.log, tou\_pktrecv.log, tou\_pkt.log, tou\_socktb.log, and stdout (print to screen). And I have define constance variables corresponding to these results, which are TOULOG\_ALL (0x0001), TOULOG\_TIMER (0x0002), TOULOG\_PKTSENT (0x0004), TOULOG\_PKTRECV (0x0008), TOULOG\_PKT (0x0010), TOULOG\_SOCKTB (0x0020),and TOULOG\_PTSRN 0x0040.
The default logging object is Logger lg, and the logging function that used to write the data into file is logData(string data, unsigned short LoggingLevel). The parameter data is the message that programmer want to put into the log. The loggingLevel is the category of the given message. The LoggingLevel can be a combination of many categories, for example
lg.logData("ToU", TOULOG\_ALL|TOULOG\_SOCKTB|TOULOG\_PTSRN); will put the message ToU into tou.log, tou\_socktb.log, and stdout.


6. Program Documentation

This is a link to another stand-alone set of HTML files that describe:
Authors, affiliations and contact information (similar to the style of this document)
an abstract describing briefly what the program does
system requirements, such as operating system version, hardware, any multimedia devices
installation instructions
configuration (if any)
operation (similar in detail to a Unix "man" page); provide examples and screen dumps where applicable
program internal operation
things that do not work or other restrictions, such as the size of the problem, network properties
useful enhancements that you can think of
acknowledgements for code and ideas borrowed
This part is meant to be part of the program documentation, while parts of this document might become a technical report or a conference paper.
7. Measurements

The measurements have been done in Internet Teaching Laboratory.
this describes any measurements or tests conducted.


8. Task List

ToU consists of many parts, including circular buffer, timer, ToU data structure, processtou, congestion control, connection control, and logging mechanism. Among them, circular buffer is originally written by Salman A. Baset, and enhanced and featured with thread lock by Cheng-Han Lee. Chinmay Gaikwad is responsible for implementing connection control, 3-way handshake and closure mechanism and takes part in some of the other modules. Cheng-Han Lee mainly works on the implementation of timer, data structure, processtou, congestion control, and logging mechanism.


9. References

1. [draft-baset-tsvwg-tcp-over-udp-01]
Salman A. Baset and Henning Schulzrinne, "TCP-over-UDP", baset-tsvwg-tcp-over-udp-01 (work in progress), Dec., 2009.

2. [RFC0793](RFC0793.md) Postel, J., "Transmission Control Protocol", STD 7, RFC 793, September 1981.

3. [RFC2861](RFC2861.md) Mark Handley, Jitendra Padhye, and Sally Floyd, "TCP Congestion Window Validation", Jun., 2000.


4. [RFC3782](RFC3782.md) S. Floyd, T. Henderson, and A. Gurtov, "The NewReno Modification to TCP's Fast Recovery Algorithm", April 2004

5. [I-D.ietf-mmusic-ice] Rosenberg, J., "Interactive Connectivity Establishment (ICE): A Protocol for Network Address Translator (NAT) Traversal for Offer/Answer Protocols", draft-ietf-mmusic-ice-19 (work in progress), October 2007.

6. [I-D.ietf-mmusic-ice-tcp] Rosenberg, J., "TCP Candidates with Interactive Connectivity Establishment (ICE)", draft-ietf-mmusic-ice-tcp-07 (work in progress),


7 http://www.faqs.org/rfcs/rfc5681.html


Last updated: 2010-05-05 By Cheng-Han Lee