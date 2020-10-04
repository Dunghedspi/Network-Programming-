#include <time.h>
#include <stdio.h>
#include <stdlib.h>
char *getLocalTime()
{
	char *str = (char *)malloc(sizeof(char) * 255);
	time_t abs_ts, loc_ts;
	struct tm loc_time_info;

	/*Absolute time stamp.*/
	time(&abs_ts);

	localtime_r(&abs_ts, &loc_time_info);
	loc_ts = mktime(&loc_time_info);
	sprintf(str, "%ld", loc_ts);
	return str;
}