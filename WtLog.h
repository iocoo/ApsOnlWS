/********************************************************
 * @name Wt_Log.h
 * @author i@gejo.co  wangzhe
 * @version 1.0
 * @date 2013-02-27 (create)
 * @funtion write information to log files
 * @modify
********************************************************/

#ifndef _Wt_Log_H_2013_08_05
#define _Wt_Log_H_2013_08_05

#ifdef __cplusplus
extern "C"
{
#endif

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include "Ultil.h"


#define	LOGFILE_MAXSIZE	 20971520

#define LOFF			0, __FILE__, __LINE__
#define LERROR			1, __FILE__, __LINE__
#define LWARNING		2, __FILE__, __LINE__
#define LNORMAL			3, __FILE__, __LINE__
#define LDEBUG			4, __FILE__, __LINE__

void WtLog(char *sLogFile,int npLogMode,char *sFileName, int nLine,char * fmt, ...);
static FILE *OpenLogFile(char *FileName);
extern char sLogfile[32];

#ifdef __cplusplus
}
#endif

#endif  //_Wt_Log_H_2013_02_27
