
#include "WtLog.h"
/*************************************************************
 * Name: WtLog.c
 * Arguments:  
 * Description:	Write Logs
 * Date:
 * Author:Wangzhe 2013
**************************************************************/

static char ErrMsg[4][10] = { "[ERROR]  ", "[WARNING]", "[NORMAL] ", "[DEBUG]  " };

FILE *OpenLogFile(char *FileName)
{
    FILE 	*pfLog;
    time_t	cNow;
	struct tm	*pcNow;
	char	pcFileName[512];
    if ((pfLog = fopen(FileName,"a+")) == NULL)
	{

		return(stderr);
	}
	fseek(pfLog,0,SEEK_END);
    if (ftell(pfLog) >= LOGFILE_MAXSIZE) {
		fclose(pfLog); 
		
		time(&cNow);
		pcNow=localtime(&cNow);
		sprintf(pcFileName, "%s.%04d%02d%02d%02d%02d%02d",
			FileName,
			pcNow->tm_year+1900, 
			pcNow->tm_mon+1,
			pcNow->tm_mday,
			pcNow->tm_hour,
			pcNow->tm_min,
            pcNow->tm_sec
			 );
		rename(FileName,pcFileName);
		
		if ((pfLog = fopen(FileName,"a+")) == NULL)
		   return(stderr);
    }

    return(pfLog);
}

void WtLog(char *sLogFile,int npLogMode,char *sFileName, int nLine,char * fmt, ...)
{
    va_list args;
    char	*pcFmtBuf;
    char	pcMsgBuf[25],FileName[512];
    FILE	*pfLog;
    char    LogFilePath[256];
	int     nLogMode=0;
    memset(LogFilePath,0,sizeof(LogFilePath));
    memset(FileName,0,sizeof(FileName));
    memset(pcMsgBuf,0,sizeof(pcMsgBuf));
	
	if (getenv("LOG_MODE") == NULL)
        nLogMode = LOFF;
    else   
        nLogMode = atoi(getenv("LOG_MODE"));
    if (nLogMode < npLogMode)
        return;
	
    if (getenv("LOG_FILE_PATH"))
        snprintf(LogFilePath, sizeof(LogFilePath) - 1, "%s", (char *)getenv("LOG_FILE_PATH"));
    else
        snprintf(LogFilePath, sizeof(LogFilePath) - 1, "%s",LogFilePath);
		
	if (access(LogFilePath, F_OK) < 0 && mkdir(LogFilePath, S_IRWXU|S_IRWXG|S_IROTH) == -1)
        return ;
        
    GetFmtTime(pcMsgBuf,sizeof(pcMsgBuf),"%Y%m%d");	
	sprintf(FileName, "%s/%s.%s.log",LogFilePath,sLogFile,pcMsgBuf);	
	
    pfLog =OpenLogFile(FileName);
	if(pfLog==NULL)
	{
		return ;
	}
    memset(pcMsgBuf,0,sizeof(pcMsgBuf));
	GetFmtTime(pcMsgBuf,sizeof(pcMsgBuf),"%Y/%m/%d %H:%M:%S");
	
	//fprintf(pfLog,"[%16s]:",pcMsgBuf);
	fprintf(pfLog, "[%s] %-12s: %-5d %s", pcMsgBuf, sFileName,nLine,ErrMsg[npLogMode - 1]);
	va_start(args, fmt);
	vfprintf(pfLog,fmt,args);
    va_end(args);
    fprintf(pfLog,"\n");
    fflush(pfLog);
    fclose(pfLog);
}
