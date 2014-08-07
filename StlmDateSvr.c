#include"StlmDateSvr.h"
#include "ApsOnlWS.nsmap"

int gProcessNum=0;
long lPort=11080L;
char sLogfile[32];
char EXHOME[128];
char ZLOG_CONF[128];

pthread_mutex_t sult_Mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t queue_cs;           //队列锁
pthread_cond_t  queue_cv;           //条件变量
SOAP_SOCKET     queue[MAX_QUEUE];   //数组队列
int             head =0, tail =0;   //队列头队列尾初始化 

ODBC_CONN_POOL conn_pool;           //数据库连接池结构


//Main Funtion
int main(int argc,char *argv[])
{
    int status=0;
    status=nInit();
    sprintf(sLogfile,"%s",argv[0]);
    SOAP_SOCKET m,s;
    int i=0;
    struct soap Onlsoap; 
    soap_init(&Onlsoap);
    Onlsoap.fget = http_get;
    Onlsoap.fpost = http_post;
    struct soap * soap_thr[MAX_THR];
    pthread_t tid[MAX_THR];

    sigset(SIGUSR1, signalHandle);
    sigset(SIGTERM, signalHandle);
    sigset(SIGSEGV, signalHandle);

    // 设置UTF-8编码方式
    //soap_set_mode(&Onlsoap, SOAP_C_UTFSTRING);
    
    soap_set_mode(&Onlsoap, SOAP_C_MBSTRING);
    soap_set_namespaces(&Onlsoap, namespaces);
    // ----------
    m = soap_bind(&Onlsoap,NULL,lPort,BACKLOG);
    //循环直至服务套接字合法
    while (!soap_valid_socket(m))
    {
        dzlog_error("Bind port error!");
        m = soap_bind(&Onlsoap,NULL,lPort,BACKLOG);
        sleep(2);
    }
    //锁和条件变量初始化
    pthread_mutex_init(&queue_cs,NULL);
    pthread_cond_init(&queue_cv,NULL);
    //生成服务线程
    for(i = 0; i <MAX_THR; i++)
    {
      soap_thr[i] = soap_copy(&Onlsoap);
      dzlog_info("Starting thread %d ",i);
      pthread_create(&tid[i],NULL,(void*(*)(void*))process_queue,(void*)soap_thr[i]);
      
    }
   
   for(;;)
   {

       s=soap_accept(&Onlsoap);
       if(!soap_valid_socket(s))
       {
            if (Onlsoap.errnum) 
            { 
                 soap_print_fault(&Onlsoap,stderr);
                 continue;
            } 
            else
            {
                dzlog_error("Server timed out.");
                break;
            }
       }
     
       dzlog_info("Main Process[%d] accepted connection from IP=%d.%d.%d.%d", getpid(),
              (Onlsoap.ip >> 24)&0xFF, (Onlsoap.ip >> 16)&0xFF, (Onlsoap.ip >> 8)&0xFF, Onlsoap.ip&0xFF);

        while(enqueue(s) == SOAP_EOM)
         sleep(1);
   }
   
    //服务结束后的清理工作
    for(i = 0; i < MAX_THR; i++)
    {
       while (enqueue(SOAP_INVALID_SOCKET) == SOAP_EOM) 
       {
           sleep(1);
       }
    }
    for(i=0; i< MAX_THR; i++)
    {
        pthread_join(tid[i],NULL);
        dzlog_debug("terminated[%d] ",i);
        soap_done(soap_thr[i]);
        free(soap_thr[i]);
    }
   
    pthread_mutex_destroy(&queue_cs);
    pthread_cond_destroy(&queue_cv);
    soap_done(&Onlsoap);
    DbPoolFree(&conn_pool);
	zlog_fini();
    return 0;
}


/****************************************************************
 * Function:OnlWS__GetStlmDate()
 * Arguments:   struct soap *soap,char *option,struct OnlWS__StlmdateResp  *resp
 * Description: get stlm——date structure
 * Date: 
 * Author:
*****************************************************************/
int OnlWS__GetStlmDate(struct soap *soap,char *option,struct OnlWS__StlmdateResp  *resp)
{
    int nRet=-1;
    clock_t  start,end;
    struct tms     tTMS;
    start=times(&tTMS);
    //malloc memory spaces for output
    tbl_bat_cut_ctl_def *ptBatCutCtlDef=(tbl_bat_cut_ctl_def *)soap_malloc(soap,sizeof(tbl_bat_cut_ctl_def));
    memset(ptBatCutCtlDef,0x00,sizeof(tbl_bat_cut_ctl_def));
    //For Debugging
    /*
    resp->InsIdCd="99990000";
    resp->SysState="0";
    resp->CurStlmDate="20140715";
    resp->BfStlmDate="20140714";
    resp->DateCutTime=60;
    resp->BfTxnTbl="A";
    resp->CurTxnTbl="B";
    resp->UseTlInsFlg="0";
    end=times(&tTMS);
    dzlog_debug("Call GetStlmDate End[%ld]",end-start);
    return SOAP_OK;
    */
    //DB connection stuct definitions
    ODBC_CONNECTION conn; 
    ODBC_CURROR     cursor;
    ODBC_FETCH_DATA BatCutCtlData;
    
    BatCutCtlData.pData=ptBatCutCtlDef;
    BatCutCtlData.pFuncFetGetData=&GetData_Tbl_BatCutctl;
    
    memset(&conn,0x00,sizeof(ODBC_CONNECTION));
    memset(&cursor,0x00,sizeof(ODBC_CURROR));
    
    dzlog_debug("Call[option:%s] OnlWS__GetStlmDate Begin.",option);
    const char *sqltext="SELECT ins_id_cd,sys_state,bf_stlm_date,cur_stlm_date,date_cut_time,bf_txn_tbl,cur_txn_tbl,use_tl_ins_flg FROM TBL_BAT_CUT_CTL";
    
    //Get DB connection session from Db connection pool
    nRet=DbPoolGetConn(&conn_pool,&conn);
    if(nRet)
    {//!OCI_IsConnected(conn.pHandle)||
         dzlog_debug("Connected failed[%d].",nRet);
         return SOAP_SVR_FAULT;
    }
    //Open DB Curror
    nRet=DbOpenCurror(&conn, sqltext, &cursor);
    if(nRet)
    {
         dzlog_debug("DbOpenCurror failed[%d].",nRet);
         return SOAP_SVR_FAULT;
    }
    nRet=DbFetchCurror(&cursor, &BatCutCtlData);
    if(nRet)
    {
         dzlog_debug("DbFetchCurror failed[%d].",nRet);
         return SOAP_SVR_FAULT;
    };
    end=times(&tTMS);
    resp->InsIdCd=ptBatCutCtlDef->ins_id_cd;
    sTrim(resp->InsIdCd);
    resp->SysState=ptBatCutCtlDef->sys_state;
    resp->CurStlmDate=ptBatCutCtlDef->cur_stlm_date;
    resp->BfStlmDate=ptBatCutCtlDef->bf_stlm_date;
    resp->DateCutTime=ptBatCutCtlDef->date_cut_time;
    resp->BfTxnTbl=ptBatCutCtlDef->bf_txn_tbl;
    resp->CurTxnTbl=ptBatCutCtlDef->cur_txn_tbl;
    resp->UseTlInsFlg=ptBatCutCtlDef->use_tl_ins_flg;
    dzlog_debug("Call GetStlmDate End.Result[%s][%s]:[%ld]",resp->InsIdCd,resp->CurStlmDate,end-start);
    DbCloseCurror(&cursor);
    DbConnFree(&conn);
    return SOAP_OK; 
}

/****************************************************************
 * Function:OnlWS__GetTZeroNodeInfo
 * Arguments:  struct soap *soap,char *option,struct OnlWS__TZeroNodeInfoResp  *resp);
 * Description: get TZeroNodeInfo structure
 * Date: 
 * Author:
*****************************************************************/
int OnlWS__GetTZeroNodeInfo(struct soap *soap,char *option,struct OnlWS__TZeroNodeInfoResp  *resp)
{
    int nRet=0;
    int i=0;
    int nRecCnt=0;
    int nRecId=0;   //
    clock_t  start,end;
    struct tms     tTMS;
    start=times(&tTMS);
    //malloc memory spaces for output
    typedef struct tTZeroNodeInfLink
    {
    	tbl_tzero_node_inf_def *ptTZeroNode;	
    	struct tTZeroNodeInfLink *pNext;
    }tTZeroNodeInfLink;
    
    tTZeroNodeInfLink *pLink=(tTZeroNodeInfLink *)soap_malloc(soap,sizeof(tTZeroNodeInfLink));
    tbl_tzero_node_inf_def *ptTZeroNodeInf=(tbl_tzero_node_inf_def *)soap_malloc(soap,sizeof(tbl_tzero_node_inf_def));
    memset(ptTZeroNodeInf,0x00,sizeof(ptTZeroNodeInf));
    //DB connection stuct definitions
    ODBC_CONNECTION conn; 
    ODBC_CURROR     cursor;
    ODBC_FETCH_DATA TZeroNodeData;
    
    TZeroNodeData.pData=ptTZeroNodeInf;
    TZeroNodeData.pFuncFetGetData=&GetData_Tbl_TZeroNodeInf;
    
    memset(&conn,0x00,sizeof(ODBC_CONNECTION));
    memset(&cursor,0x00,sizeof(ODBC_CURROR));
    
    dzlog_debug("Call[option:%s] OnlWS__GetTZeroNodeInfo Begin.",option);
    const char *sqltext="select bat_no, bat_node, to_char(bat_time,'YYYY-MM-DD HH24:MI:SS.FF6') from tbl_tzero_node_inf where bat_no<>'99' order by bat_no";
 
    //Get DB connection session from Db connection pool
    nRet=DbPoolGetConn(&conn_pool,&conn);
    if(nRet)
    {//!OCI_IsConnected(conn.pHandle)||
         dzlog_debug("Connected failed[%d].",nRet);
         return SOAP_SVR_FAULT;
    }
    //Get Num of rows total.
    nRet=DbGetTbRecCnt(&conn,"tbl_tzero_node_inf", "where bat_no<>'99'", &nRecCnt);
    if(nRet)
    {
         dzlog_debug("DbGetTbRecCnt failed[%d].",nRet);
         return SOAP_SVR_FAULT;
    }
    dzlog_debug("The Num of Rows[%d].",nRecCnt);
    //Open DB Curror
    nRet=DbOpenCurror(&conn, sqltext, &cursor);
    if(nRet)
    {
         dzlog_debug("DbOpenCurror failed[%d].",nRet);
         return SOAP_SVR_FAULT;
    }

    resp->__size=nRecCnt;
    resp->__ptr= soap_malloc(soap,(resp->__size+1)*sizeof(*resp->__ptr));
    
    pLink->ptTZeroNode=ptTZeroNodeInf;
    pLink->pNext=NULL;
    tTZeroNodeInfLink *p ,*phead=pLink;
    for(i=0;i<resp->__size-1;i++)
    {
        p=(tTZeroNodeInfLink *)soap_malloc(soap,sizeof(tTZeroNodeInfLink));
        p->ptTZeroNode=(tbl_tzero_node_inf_def *)soap_malloc(soap,sizeof(tbl_tzero_node_inf_def));
        p->pNext=NULL;
        phead->pNext=p;
        phead=p;
    }
    p=pLink;
    nRecId=0;
    while(p&&nRecId<resp->__size)
    {
        TZeroNodeData.pData=p->ptTZeroNode;
        nRet=DbFetchCurror(&cursor, &TZeroNodeData);
        if(nRet)
        {
        	dzlog_debug("No[%d].DbFetchCurror failed[%d].",nRecCnt,nRet);
            break;
        };
        struct OnlWS__TZeroNodeInfo *OnlWS_TZero=(struct  OnlWS__TZeroNodeInfo *)soap_malloc(soap,sizeof( struct OnlWS__TZeroNodeInfo));
        OnlWS_TZero->BatNo=p->ptTZeroNode->bat_no;
        OnlWS_TZero->BatNode=p->ptTZeroNode->bat_node;
        OnlWS_TZero->BatTime=p->ptTZeroNode->bat_time;
        memcpy(&resp->__ptr[nRecId],OnlWS_TZero,sizeof(struct OnlWS__TZeroNodeInfo));
      
        dzlog_debug("=[%s][%s][%s][%d].",resp->__ptr[nRecId].BatNo,resp->__ptr[nRecId].BatNode,resp->__ptr[nRecId].BatTime,nRecId);
        nRecId++;
        p=p->pNext;

    }
    DbCloseCurror(&cursor);
    DbConnFree(&conn);
	dzlog_debug("Call OnlWS__GetTZeroNodeInfo End.");
    return SOAP_OK;
    
}

/*************************************************************
 * Function:http_post()
 * Arguments:   
 * Description: 
 * Date:
 * Author:
**************************************************************/
int http_post(struct soap *soap, 
              const char *endpoint, const char *host,
              int port,const char *path, const char *action, size_t count)
{
    return http_get( soap ); // http_get
}

/*************************************************************
 * Function:    http_get(struct soap *soap)
 * Arguments:   
 * Description: 
 * Date:        
 * Author:      
**************************************************************/
int http_get(struct soap *soap)
{
    char* fielPath=soap->path;
    char WSDLPATH[128];
    memset(WSDLPATH,0,sizeof(WSDLPATH));
    sprintf(WSDLPATH,"%s/etc/ApsOnlWS.wsdl",EXHOME);
    FILE* fd = fopen(WSDLPATH, "rb");
    if (!fd)
    {
    // return HTTP not found error
    return 404;
    }
    // HTTP header with text/xml content
    soap->http_content = "text/xml";
    soap_response(soap, SOAP_FILE);
    for(;;)
    {
        // READ
        size_t r = fread(soap->tmpbuf, 1, sizeof(soap->tmpbuf), fd);
        if (!r)
        {
            break;
        }

        // SEND DATA
        if (soap_send_raw(soap, soap->tmpbuf, r))
        {
            // can't send, but little we can do about that
             break;
        }
    }

    // close fd
    fclose(fd);
    soap_end_send(soap);
    return SOAP_OK;
}


int nInit()
{
    int ret=0;
	
	sprintf(EXHOME,"%s",getenv("FEHOME"));
	sprintf(ZLOG_CONF,"%s/etc/ZLOGCONF",EXHOME);
	ret = dzlog_init(ZLOG_CONF, "ZLOG");
    memset(&conn_pool,0x00,sizeof( ODBC_CONN_POOL));    
    strcpy(conn_pool.dbname,getenv("DBNAME"));
    strcpy(conn_pool.usr,getenv("DBUSER"));
    strcpy(conn_pool.pwd,getenv("DBPWD"));

    conn_pool.minconn=2;        
    conn_pool.maxconn=MAX_CONN;     
    ret=DbPoolCreate(&conn_pool);
    
    
    return ret;

}

void * process_queue(void * soap)
{
    struct soap * tsoap = (struct soap *)soap;
    for(;;)
    {
          tsoap->socket = dequeue();
          if (!soap_valid_socket(tsoap->socket))
          {
              break;
          }
          soap_serve(tsoap);
          soap_destroy(tsoap);
          soap_end(tsoap);
    }
    return NULL;
}

//入队列操作
int enqueue(SOAP_SOCKET sock)
{
    int status = SOAP_OK;
    int next;
    pthread_mutex_lock(&queue_cs);
    next = tail +1;
    if (next >= MAX_QUEUE) 
      next = 0;
    if (next == head) 
        status = SOAP_EOM;
    else
    {
      queue[tail] =sock;
      tail = next;
    }
    pthread_cond_signal(&queue_cv);
    pthread_mutex_unlock(&queue_cs);
    return status;
}

//出队列操作
SOAP_SOCKET dequeue()
{
    SOAP_SOCKET sock;
    pthread_mutex_lock(&queue_cs);
    while (head == tail )
    {
         pthread_cond_wait(&queue_cv,&queue_cs);
    }
    sock = queue[head++];
    if (head >= MAX_QUEUE)
    {
        head =0;
    }
    pthread_mutex_unlock(&queue_cs);
    return sock;
}

void signalHandle(int sig)
{
    switch (sig) {
        case SIGTERM:
        case SIGSEGV:
        case SIGUSR1:
            dzlog_info("Capture Signal and Exit");
            exit(0);
        case SIGUSR2:
            break;
        case SIGCHLD:
            break;  
        default:
            break;
    }

}
