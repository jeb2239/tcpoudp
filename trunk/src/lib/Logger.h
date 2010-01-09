#ifndef LOGGER_H_
#define LOGGER_H_
#include <fstream>

class Logger
{
protected:
	std::ofstream file;
	static Logger logger;

public:
	Logger();
	virtual ~Logger();
	static Logger& getLogger();	
	void shutdownLogger();
	
	void logData(std::string data,int LoggingLevel);
};

#endif /*LOGGER_H_*/
