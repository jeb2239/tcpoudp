#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sys/time.h>

extern double beg_time;

std::string timestamp (void);
long timestamp_long (void);
double timestamp_sec (void);
void set_beg_timestamp(void);
