#ifndef LOGGER_H_
#define LOGGER_H_
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

#define TOULOG_ALL			0x0001
#define TOULOG_TIMER		0x0002
#define TOULOG_PKTSENT	0x0004
#define TOULOG_PKTRECV	0x0008
#define TOULOG_PKT			0x0010
#define TOULOG_SOCKTB		0x0020
#define TOULOG_PTSRN		0x0040

class Logger{
private:
	unsigned short	logstate;
	void logFile(std::string data);
	void logFileTimer(std::string data);
	void logFilePktsent(std::string data);
	void logFilePktrecv(std::string data);
	void logFilePkt(std::string data);
	void logFileSocktb(std::string data);

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
	std::ofstream file;
	std::ofstream file_timer;
	std::ofstream	file_pktsent;
	std::ofstream	file_pktrecv;
	std::ofstream file_pkt;
	std::ofstream file_socktb;

	static Logger logger;

public:
	Logger();
	virtual ~Logger();
	static Logger& getLogger();	
	void shutdownLogger();
	
	void logData(std::string data,unsigned short LoggingLevel);
};

#endif /*LOGGER_H_*/
