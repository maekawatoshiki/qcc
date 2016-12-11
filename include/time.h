#ifndef __QCC_TIME__
#define __QCC_TIME__

struct tm {
  int tm_sec;         
  int tm_min;         
  int tm_hour;        
  int tm_mday;        
  int tm_mon;         
  int tm_year;        
  int tm_wday;        
  int tm_yday;        
  int tm_isdst;       
};

typedef int clock_t;
typedef int size_t;
typedef int time_t;

clock_t clock();
time_t time(time_t *);
time_t mktime(struct tm *);
size_t strftime(char *, size_t, char *, struct tm *);
char *ctime(time_t *);
char *asctime(struct tm *);

#endif
