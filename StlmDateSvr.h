#ifndef __STLM_DATE_SVR_H_20140707
#define __STLM_DATE_SVR_H_20140707

#include <signal.h>
#include <pthread.h>
#include "soapH.h"
//#include "WtLog.h"
#include "Ultil.h"
#include "DbsDef.h"
#include "odbc_fw.h"
#include "zlog.h"

 #include<sys/times.h>
 #ifdef __cplusplus
 extern "C"
 {
 #endif
/* Marcos  declaration  */

#define MAX_ERROR_INF  10
#define BACKLOG  100 
#define MAX_PROCESS_NUM  6
#define MAX_THR   10
#define MAX_QUEUE  100
#define MAX_CONN    10     //线程池连接数

extern char BATCHHOME[128];

#define ST_IDLE 0
#define ST_BUSY 1
/*  variant   */

tbl_bat_cut_ctl_def tBatCutCtlDef;   //For return SOAP  FOR TEST ...TEMP
//extern OCI_ConnPool *pool;
extern ODBC_CONN_POOL conn_pool; 
extern char ZLOG_CONF[128];
int nInit();
//int OnlWS__GetStlmDate(struct soap *soap,char *option,struct OnlWS__SltmdateRespons *resp );
int http_post(struct soap *soap,  const char *endpoint, const char *host, int port,const char *path, const char *action, size_t count);
int http_get(struct soap *soap);
void signalHandle(int sig);
void * process_queue(void * soap);
int enqueue(SOAP_SOCKET sock);
SOAP_SOCKET dequeue();
#ifdef __cplusplus
}
#endif

#endif //__STLM_DATE_SVR_H_20140707
