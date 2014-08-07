#ifndef __ULTIL_H__
#define __ULTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <time.h>

int ParseConfigFile(const char * sFileName,const char *section,const char *key,char *value);
char * sTrim(char * ptr) ;
void GetFmtTime(char *tmBuf,int nLen,const char *Fmt);

#ifdef __cplusplus
}
#endif

#endif /* __ULTIL_H__ */