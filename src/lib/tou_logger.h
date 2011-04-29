/******************************************************************************
 * tou_logger.h
 * logging
 *
 * Copyright 2009 by Columbia University; all rights reserved
 * March 02, 2010
 * ***************************************************************************/


#ifndef LOGGER_H_
#define LOGGER_H_
#include <bitset>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

#define TOULOG_ALL			0x0001
#define TOULOG_TIMER		0x0002
#define TOULOG_PKTSENT		0x0004
#define TOULOG_PKTRECV		0x0008
#define TOULOG_PKT			0x0010
#define TOULOG_SOCKTB		0x0020
#define TOULOG_PTSRN		0x0040
#define TOULOG_PROBE		0x0080

/* public log flag for user program */
extern unsigned short 		LOGFLAG;
extern bool					LOGON;

class Logger{
private:
	int pb_cwnd, pb_awnd;
	short pb_ccstat;
	double pb_time;
	unsigned long pb_ssthresh, pb_snd_nxt;

	void logFile(std::string data);
	void logFileTimer(std::string data);
	void logFilePktsent(std::string data);
	void logFilePktrecv(std::string data);
	void logFilePkt(std::string data);
	void logFileSocktb(std::string data);
	void logFileProbe(int cwnd, unsigned long ssthresh, int awnd, 
					  unsigned long snd_nxt, short ccstat);
	void logFileProbe(double time, int cwnd, unsigned long ssthresh, int awnd, 
					  unsigned long snd_nxt, short ccstat);

public:
	inline std::string cs2s(short us){
		std::stringstream s;
		s << us;
		return s.str();
	}

	inline std::string ci2s(int us){
		std::stringstream s;
		s << us;
		return s.str();
	}

	inline std::string cl2s(long us){
		std::stringstream s;
		s << us;
		return s.str();
	}

	template<class T> inline std::string c2s(T us){
		std::stringstream s;
		s << us;
		return s.str();
	}


protected:
	std::ofstream	file;
	std::ofstream	file_timer;
	std::ofstream	file_pktsent;
	std::ofstream	file_pktrecv;
	std::ofstream	file_pkt;
	std::ofstream	file_socktb;
	std::ofstream	file_probe;

	/* static Logger logger; */

public:
	Logger();
	virtual ~Logger();
	/* static Logger& getLogger() { return logger;}; */
	void shutdownLogger();
	
	void logData(std::string data,unsigned short LoggingLevel);
	void logData(int cwnd, unsigned long ssthresh, int awnd, 
				 unsigned long snd_nxt, short ccstat, unsigned short LoggingLevel);
};

#endif /*LOGGER_H_*/

extern Logger lg;

