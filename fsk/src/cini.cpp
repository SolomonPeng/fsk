#ifndef __CINIFILE_H__ 
#define __CINIFILE_H__ 

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/stat.h> 
#include "cini.h"

/********************************************** 
Name : OpenIniFile 
Input : 
char *File_name : file to be open 
Output : 
0 : File succefully opened 
-1 : Fail to open given file 
-2 : Fail to read buffer data 
Process: Normal 
***********************************************/ 
int TIni::OpenIniFile(const char *pFile) 
{ 
struct stat statbuf; 
stat(pFile,&statbuf); 
iBufferLen = 0; bBufferChanged = 0; 
if ((fp=fopen(pFile, "r+")) == NULL) return -1; 
if(fread(szBuffer,statbuf.st_size,1,fp) != 1) 
{ 
if (fp != NULL) fclose(fp); 
fp = NULL; 
return -2; 
} 
iBufferLen = statbuf.st_size; 
return 0; 
} 

/********************************************** 
Name : CloseIniFile 
Input : None 
Output : 
0 : File succefully closed 
-1 : Fail to close already opened file 
Process: Normal 
***********************************************/ 
int TIni::CloseIniFile(void) 
{ 
if (fp != NULL) { 
if (bBufferChanged) { 
rewind(fp); 
fwrite(szBuffer, iBufferLen, 1, fp); 
} 
if (!fclose(fp)) return 0; 
else return -1; 
} else return 0; 
} 

/********************************************** 
Name : ToUnixStyle 
Input : None 
Output : 
0 : File succefully closed 
-1 : Fail to close already opened file 
Process: Normal 
***********************************************/ 
int TIni::ToUnixStyle(void)  
{ 
int i=0; 
if (fp==NULL) return -1; 
while (i<iBufferLen) { 
if (szBuffer[i]=='\r')
 { szBuffer[i]=' '; bBufferChanged=1; } 
i++; 
} 
return 0; 
} 

/********************************************** 
Name : GetIniString 
Input : 
char *pSection : Session Name 
char *pIdent: Identity Name 
char *pResult: Returned string 
Output : 
0 : Identity Value successfully returned 
-1 : Fail to get the designated identity value 
Process: Normal 
***********************************************/ 
int TIni::GetIniString(const char *pSection,const char *pIdent, char *pResult) 
{ 
int i=0; 
int j=0; 
int min; 
int iKeyFound=-1; 
if (!strlen(pSection) || !strlen(pIdent) || (fp == NULL)) return -1; 
while (i<iBufferLen) { 
while ((i<iBufferLen) && 
((szBuffer[i]==' ') || (szBuffer[i]=='\t'))) i++; 
if (i>=iBufferLen) return -1; 
if (szBuffer[i]=='#') { //ignore the lines beginning with '#' 
while ((i<iBufferLen) && (szBuffer[i] != '\n')) i++; 
if (i>=iBufferLen) return -1; 
//Jump to the next line 
i++; 
} 
else { 
if (szBuffer[i]=='[') 
{ 
i++; 
while ((i<iBufferLen) && 
((szBuffer[i]==' ') || (szBuffer[i]=='\t'))) i++; 
if (i>=iBufferLen) return -1; 
if (strncmp(szBuffer+i, pSection, strlen(pSection))==0)
{ 
//Section may be found, let's see 
i += strlen(pSection); 
while ((i<iBufferLen) && 
((szBuffer[i]==' ') || (szBuffer[i]=='\t'))) i++; 
if (i>=iBufferLen) return -1; 
if (szBuffer[i]==']') iKeyFound=0; i++; 
//matched ] or not, ignore the line 
while ((i<iBufferLen) && (szBuffer[i]!='\n')) i++; 
if (i>=iBufferLen) return -1; 
//Jump to the new line 
i++; 
}
 else
 { //ignore the line and forward 
while ((i<iBufferLen) && (szBuffer[i]!='\n')) i++; 
if (i>=iBufferLen) return -1; 
//Jump to the next line 
i++; 
} 
} 
else
 { 
if (iKeyFound != 0) { //Section has not found, ignore the line 
while ((i<iBufferLen) && (szBuffer[i] != '\n')) i++; 
if (i>=iBufferLen) return -1; 
//Jump to the new line 
i++; 
}
 else 
 { //it may be the Identity to be found, judge it 
if (strncmp(szBuffer+i, pIdent, strlen(pIdent))==0) { 
i += strlen(pIdent); 
if (i>=iBufferLen) return -1; 
while ((i<iBufferLen) && 
((szBuffer[i]=='\t') || (szBuffer[i]==' '))) i++; 
if (szBuffer[i] == '=') 
{ //Value has found 
i++; 
while ((i<iBufferLen) && 
((szBuffer[i]=='\t') || (szBuffer[i]==' '))) i++; 
if (i>=iBufferLen) return -1; 
j=i; 
while ((j<iBufferLen) && 
(szBuffer[j]!='\n')) j++; j--; 
while ((szBuffer[j]==' ') || 
(szBuffer[j]=='\t')) j--; 
min = j-i+1; 
strncpy(pResult, szBuffer+i, min); 
*(pResult+min) = '\0'; 
return 0; 
} else { //ignore the line 
while ((i<iBufferLen) && (szBuffer[i]!='\n')) i++; 
if (i>=iBufferLen) return -1; 
//Jump to the next line 
i++; 
} 

} 
else 
{ //ignore the line 
while ((i<iBufferLen) && (szBuffer[i]!='\n')) i++; 
if (i>=iBufferLen) return -1; 
//Jump to the next line 
i++; 
} 
} 
} 
} 
} 
return -1; 
} 


/********************************************** 
Name : SetIniString 
Input : 
char *pSection : Session Name 
char *pIdent: Identity Name 
char *pValue: Identity Value to write 
Output : 
0: value is successfully written 
-1: parameter error or file not open 
Process: Normal 
***********************************************/ 
int TIni::SetIniString(const char *pSection,const char *pIdent,const char *pValue) 
{ 
int i=0; 
int j=0; 
int k=0; 
int iBufferMore = 0; 
int bKeyFound=0,bIdentFound=0; 
int iKeyPos=0,iIdentPos=0; 

//Parameter is empty or file has not been openned 
if (!strlen(pSection) || !strlen(pIdent) || (fp == NULL)) return -1; 

while (i<iBufferLen) { 
while ((i<iBufferLen) && 
((szBuffer[i]==' ') || (szBuffer[i]=='\t'))) i++; 
if (i>=iBufferLen) break; 
if (szBuffer[i]=='#')
 { //ignore the lines beginning with '#' 
while ((i<iBufferLen) && (szBuffer[i] != '\n')) i++; 
if (i>=iBufferLen) break; 
//Jump to the next line 
i++; 
} 
else 
{ 
if (szBuffer[i]=='[') { 
i++; 
while ((i<iBufferLen) && 
((szBuffer[i]==' ') || (szBuffer[i]=='\t'))) i++; 
if (i>=iBufferLen) break; 
if (strncmp(szBuffer+i, pSection, strlen(pSection))==0) { 
//key may be found, let's see 
i += strlen(pSection); 
while ((i<iBufferLen) && 
((szBuffer[i]==' ') || (szBuffer[i]=='\t'))) i++; 
if (i>=iBufferLen) break; 
if (szBuffer[i]==']') bKeyFound=1; i++; 
//matched ] or not, ignore the line 
while ((i<iBufferLen) && (szBuffer[i]!='\n')) i++; 
if (i>=iBufferLen) break; 
//Jump to the new line 
i++; 
iKeyPos = i; 
} 
else
 { //ignore the line and forward 
while ((i<iBufferLen) && (szBuffer[i]!='\n')) i++; 
if (i>=iBufferLen) break; 
//Jump to the next line 
i++; 
} 
} 
else
 { 
if (!bKeyFound) 
{ //key has not found, ignore the line 
while ((i<iBufferLen) && (szBuffer[i] != '\n')) i++; 
if (i>=iBufferLen) break; 
//Jump to the new line 
i++; 
}
 else { //it may be the Identity to be found, judge it 
if (strncmp(szBuffer+i, pIdent, strlen(pIdent))==0)
 { 
i += strlen(pIdent); 
if (i>=iBufferLen) break; 
while ((i<iBufferLen) && 
((szBuffer[i]=='\t') || (szBuffer[i]==' '))) i++; 
if (szBuffer[i] == '=') { //Value has found 
i++; iIdentPos=i; 
bIdentFound=1; 
break; 
} 
else
 { //ignore the line 
while ((i<iBufferLen) && (szBuffer[i]!='\n')) i++; 
if (i>=iBufferLen) break; 
//Jump to the next line 
i++; 
} 

} 
else
{ //ignore the line 
while ((i<iBufferLen) && (szBuffer[i]!='\n')) i++; 
if (i>=iBufferLen) break; 
//Jump to the next line 
i++; 
} 
} 
} 
} 
} 
//write lines, if appropriate. 
if (bKeyFound) 
{ 
if (bIdentFound) 
{ 
i=iIdentPos; j=0; 
while ((i<iBufferLen) && (szBuffer[i] != '\n')) { i++; j++; } 
if ((int)strlen(pValue)<=j) { 
//Space is enough, no additional space 
strncpy(szBuffer+iIdentPos, pValue, strlen(pValue)); 
for (i=strlen(pValue); i<j; i++) szBuffer[iIdentPos+i]=' '; 
iBufferMore = 0; 
} 
else
{ 
k = strlen(pValue); 
//Space is limited, need additional space 
for (i=iBufferLen; i>=iIdentPos+j; i--) 
szBuffer[i+k-j] = szBuffer[i]; 
strncpy(szBuffer+iIdentPos, pValue, strlen(pValue)); 
iBufferMore = k-j; 
} 
} 
else 
{ 
i = strlen(" = \n")+strlen(pIdent)+strlen(pValue); 
for (j=iBufferLen-1; j>=iKeyPos; j--) 
szBuffer[j+i]=szBuffer[j]; 
sprintf(szBuffer+iKeyPos, "%s = %s\n", pIdent, pValue); 
iBufferMore = i; 
} 
bBufferChanged = 1; 
} 
else 
{ 
sprintf(szBuffer+iBufferLen, 
"\n[%s]\n%s = %s", pSection, pIdent, pValue); 
iBufferMore = strlen("\n[]\n = ")+strlen(pSection) 
+strlen(pIdent)+strlen(pValue); 
bBufferChanged = 1; 
} 
iBufferLen += iBufferMore; 
return 0; 
} 

/********************************************** 
Name : GetIniInteger 
Input : 
char *pSection : Session Name 
char *pIdent: Identity Name 
int: Default Value if an error exists. 
Output : 
interger converted, if applicable 
Default value, if an error exists 

Process: Normal 
***********************************************/ 
int TIni::GetIniInteger(const char* pSection, const char* pIdent, const int iDefVal) 
{ 
char szTempBuffer[MAX_VALUE_BUFFER_SIZE]; 
if ( GetIniString(pSection, pIdent, szTempBuffer) == 0 )
 { 
if (strlen(szTempBuffer)>2)
 { 
if ( (szTempBuffer[0]=='0') && 
( (szTempBuffer[1]=='x') || (szTempBuffer[1]=='X')) 
)
 return (int)(strtol(szTempBuffer, (char **)NULL, 16)); 
} 
  return atoi(szTempBuffer); 
} 
else return iDefVal; 
} 

/********************************************** 
Name : GetIniLong 
Input : 
char *pSection : Session Name 
char *pIdent: Identity Name 
long: Default Value if an error exists. 
Output : 
Long converted, if applicable 
Default value, if an error exists 

Process: Normal 
***********************************************/ 
long TIni::GetIniLong(const char* pSection, const char* pIdent, const long iDefVal) 
{ 
char szTempBuffer[MAX_VALUE_BUFFER_SIZE]; 
if ( GetIniString(pSection, pIdent, szTempBuffer) == 0 ) { 
if (strlen(szTempBuffer)>2) { 
if ( (szTempBuffer[0]=='0') && 
( (szTempBuffer[1]=='x') || (szTempBuffer[1]=='X')) 
) return (strtol(szTempBuffer, (char **)NULL, 16)); 
} 
return atol(szTempBuffer); 
} else return iDefVal; 
} 

/********************************************** 
Name : GetIniDouble 
Input : 
char *pSection : Session Name 
char *pIdent: Identity Name 
double: Default Value if an error exists. 
Output : 
Double converted, if applicable 
Default value, if an error exists 

Process: Normal 
***********************************************/ 
double TIni::GetIniDouble(const char* pSection, const char* pIdent, const double iDefVal) 
{ 
char szTempBuffer[MAX_VALUE_BUFFER_SIZE]; 
if ( GetIniString(pSection, pIdent, szTempBuffer) == 0 ) { 
return atof(szTempBuffer); 
} else return iDefVal; 
} 

/********************************************** 
Name : SetIniInteger 
Input : 
char *pSection : Session Name 
char *pIdent: Identity Name 
int: Value to write 
Output : 
0 : Successful 
-1: Error 

Process: Normal 
***********************************************/ 
int TIni::SetIniInteger(const char *pSection, const char *pIdent, const int pValue) 
{ 
char szTempBuffer[MAX_VALUE_BUFFER_SIZE]; 
sprintf(szTempBuffer, "%d", pValue); 
if ( SetIniString(pSection, pIdent, szTempBuffer) == 0 ) 
{ 
return 0; 
} else return -1; 

} 

/********************************************** 
Name : SetIniLong 
Input : 
char *pSection : Session Name 
char *pIdent: Identity Name 
long: Value to write 
Output : 
0 : Successful 
-1: Error 

Process: Normal 
***********************************************/ 
int TIni::SetIniLong(const char *pSection, const char *pIdent, const long pValue) 
{ 
char szTempBuffer[MAX_VALUE_BUFFER_SIZE]; 
sprintf(szTempBuffer, "%ld", pValue); 
if ( SetIniString(pSection, pIdent, szTempBuffer) == 0 ) { 
return 0; 
} else return -1; 

} 

/********************************************** 
Name : SetIniDouble 
Input : 
char *pSection : Session Name 
char *pIdent: Identity Name 
double: Value to write 
Output : 
0 : Successful 
-1: Error 

Process: Normal 
***********************************************/ 
int TIni::SetIniDouble(const char *pSection, const char *pIdent, const double pValue) 
{ 
char szTempBuffer[MAX_VALUE_BUFFER_SIZE]; 
sprintf(szTempBuffer, "%g", pValue); 
if ( SetIniString(pSection, pIdent, szTempBuffer) == 0 ) 
{ 
return 0; 
} else return -1; 

} 

/********************************************** 
Name : GetSectionValues 
Input : 
char *pSection : Session Name 
char *pDelimiter: Delimiter to separate values in the section 
char *pValues : Buffer to store returned values in the section 
Output : 
0 : Successful 
-1: Error 

Process: Normal 
***********************************************/ 
int TIni::GetSectionValues(const char *pSection, const char *pDelimiter, char *pValues) 
{ 
int i=0; 
int j=0; 
int min; 
int iKeyFound=-1; 
int iDelimLen=strlen(pDelimiter); 
int iSum=0; 

if (!strlen(pSection) || !strlen(pDelimiter) || (fp == NULL)) return -1; 

while (i<iBufferLen) { 
while ((i<iBufferLen) && 
((szBuffer[i]==' ') || (szBuffer[i]=='\t'))) i++; 
if (i>=iBufferLen) break; 
if (szBuffer[i]=='#') { //ignore the lines beginning with '#' 
while ((i<iBufferLen) && (szBuffer[i] != '\n')) i++; 
if (i>=iBufferLen) break; 
//Jump to the next line 
i++; 
} 
else
 { 
if (szBuffer[i]=='[') 
{ 
i++; 
while ((i<iBufferLen) && 
((szBuffer[i]==' ') || (szBuffer[i]=='\t'))) i++; 
if (i>=iBufferLen) break; 
if (strncmp(szBuffer+i, pSection, strlen(pSection))==0) 
{ 
//key may be found, let's see 
i += strlen(pSection); 
while ((i<iBufferLen) && 
((szBuffer[i]==' ') || (szBuffer[i]=='\t'))) i++; 
if (i>=iBufferLen) break; 
if (szBuffer[i]==']') { 
if (iKeyFound==0) break; 
else iKeyFound=0; 
} 
else 
{ if (iKeyFound==0) break; } 
i++; 
//matched ] or not, ignore the line 
while ((i<iBufferLen) && (szBuffer[i]!='\n')) i++; 
if (i>=iBufferLen) break; 
//Jump to the new line 
i++; 
} 
else
 { //ignore the line and forward 
i += strlen(pSection); 
while ((i<iBufferLen) && (szBuffer[i]!='\n') && 
(szBuffer[i]!=']')) i++; 
if (i>=iBufferLen) break; 
if (szBuffer[i] == '\n') 
{ i++; continue; } 
if ( iKeyFound == 0 ) break; 
else 
{ 
while ((i<iBufferLen) && (szBuffer[i]!='\n')) i++; 
if (i>=iBufferLen) break; 
//Jump to the next line 
i++; 
} 
} 
} 
else
 { 
if (iKeyFound != 0) 
{ //key has not found, ignore the line 
while ((i<iBufferLen) && (szBuffer[i] != '\n')) i++; 
if (i>=iBufferLen) return -1; 
//Jump to the new line 
i++; 
} 
else 
{ //it may be the Identity to be found, judge it 
while ((i<iBufferLen) && (szBuffer[i] != '\n') && 
(szBuffer[i] != '=')) i++; 
if (i>=iBufferLen) break; 
if (szBuffer[i] == '\n') { i++; continue; } 
i++; 
while ((i<iBufferLen) && 
((szBuffer[i]=='\t') || (szBuffer[i]==' '))) i++; 
if (i>=iBufferLen) break; 
j=i; 
while ((j<iBufferLen) && 
(szBuffer[j]!='\n')) j++; j--; 
while ((szBuffer[j]==' ') || 
(szBuffer[j]=='\t')) j--; 
min = j-i+1; 
strncpy(pValues+iSum, szBuffer+i, min); 
iSum += min; 
strncpy(pValues+iSum, pDelimiter, iDelimLen); 
iSum += iDelimLen; 
//Jump to the new line 
i=j+1; 
while ((i<iBufferLen) && (szBuffer[i]!='\n')) i++; 
if (i>=iBufferLen) break; 
//Jump to the next line 
i++; 
} 
} 
} 
} 
*(pValues+iSum)='\0'; 
return 0; 
} 

/********************************************** 
Name : GetSections 
Input : 
char *pDelimiter: Delimiter to separate sections in the file 
char *pSection : Buffer to store returned sections in the file 
Output : 
0 : Successful 
-1: Error 

Process: Normal 
***********************************************/ 
int TIni::GetSections(const char *pDelimiter, char *pSections) 
{ 
int i=0; 
int j=0; 
int iSum=0; 
int iDelimLen=strlen(pDelimiter); 

if (!strlen(pDelimiter) || (fp == NULL)) return -1; 

while (i<iBufferLen)
 { 
while ((i<iBufferLen) && 
((szBuffer[i]==' ') || (szBuffer[i]=='\t'))) i++; 
if (i>=iBufferLen) break; 
if ( szBuffer[i]=='[' )
 { 
i++; 
while ((i<iBufferLen) && 
((szBuffer[i]==' ') || (szBuffer[i]=='\t'))) i++; 
if (i>=iBufferLen) break; 
j=i; 
while ((j<iBufferLen) && (szBuffer[j]!='\n') && 
(szBuffer[j]!=']')) j++; 
if ((j>=iBufferLen)) break; 
if ((szBuffer[j]=='\n')) { i++; continue; }//Jump to the new line 
j--; 
while ((szBuffer[j]==' ') || (szBuffer[j]=='\t')) j--; 
strncpy(pSections+iSum, szBuffer+i, j-i+1); 
iSum += j-i+1; 
strncpy(pSections+iSum, pDelimiter, iDelimLen); 
iSum += iDelimLen; 
//Jump to the new line 
i=j+2; 
while ((i<iBufferLen) && (szBuffer[i] != '\n')) i++; 
if (i>=iBufferLen) break; 
//Jump to the next line 
i++; 
}
else
 { //ignore the lines beginning with '#' 
while ((i<iBufferLen) && (szBuffer[i] != '\n')) i++; 
if (i>=iBufferLen) break; 
//Jump to the next line 
i++; 
} 
} 
*(pSections+iSum)='\0'; 
return 0; 
} 

#endif
