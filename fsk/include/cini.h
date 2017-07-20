#ifndef CINI_H
#define CINI_H
#define MAX_BUFFER_SIZE 6000 
#define MAX_VALUE_BUFFER_SIZE 128 

#include <stdio.h>
#include <string.h>

class TIni
{
private:

   char szBuffer[MAX_BUFFER_SIZE]; 
   int iBufferLen; 
   int bBufferChanged; 
   FILE *fp;


public:

int OpenIniFile(const char *); 
int CloseIniFile(void); 
int ToUnixStyle(void); 

int GetIniString(const char *, const char *, char *); 
int SetIniString(const char *, const char *, const char *); 

int GetIniInteger(const char *, const char *, const int); 
long GetIniLong(const char *, const char *, const long); 
double GetIniDouble(const char *, const char *, const double); 

int SetIniInteger(const char *, const char *, const int); 
int SetIniLong(const char *, const char *, const long); 
int SetIniDouble(const char *, const char *, const double); 

int GetSectionValues(const char *, const char *, char *); 
int GetSections(const char *, char *); 
};

#endif

