#ifndef __QCC_STDIO__
#define __QCC_STDIO__ 

#define NULL 0

int puts(char *s);
int printf(char *, ...);
int sprintf(char *, char *, ...);
int scanf(char *, ...);
int sscanf(char *, char *, ...);
int getchar();
int putchar(int);
void perror(char *);
char *gets(char *);

#endif
