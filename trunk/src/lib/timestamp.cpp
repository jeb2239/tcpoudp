/**
 * timestamp.cpp
 * return the current time stamp in a format of string
 *
 * Copyright 2010 by Columbia University; all rights reserved
 * Jan 22, 2010
 */
#include "timestamp.h"

using namespace std;

double beg_time = 0;

/**
 * timestamp:
 * return the current time in a format of string
 */
string timestamp(void) {

	const tm		*tm;		//time structure
	time_t			current;	//current time
	char			retval_c[30];

	current = time(NULL); //or time(&current);
	tm = localtime(&current);
	strftime(retval_c, 30, "%d %B %Y %I:%M:%S %p", tm);
	
	string retval = retval_c;
	return retval;
}

/**
 * timestamp_long:
 * return the current time in a format of string
 */
long timestamp_long(void) {

    struct timeval tv;  //current time value
    gettimeofday(&tv, NULL);
    return (long)(((tv.tv_sec%1000000)*1000) + (tv.tv_usec/1000));

}
	
/**
 * set_beg_timestamp:
 */
void set_beg_timestamp(void) {

	struct timeval tv;

	if (beg_time == 0) {
		gettimeofday(&tv, NULL);
		beg_time = tv.tv_sec+(tv.tv_usec/1000000.0);
	}
	//printf("set beginning timestamp: %.9lf second.\n", beg_time);
}

/**
 * timestamp_sec:
 * return double where format in xx.xxxxxxxxx (double)
 * start from the beginning of the program.
 */
double timestamp_sec(void) {

	struct timeval tv;
	gettimeofday(&tv, NULL);
	double end_time = tv.tv_sec+(tv.tv_usec/1000000.0);
	return end_time - beg_time;
}


