/*
 * @author wangzh
 * @version 1.0
 * @date 2013-07-07
*/
//gsoap OnlWS service name: ApsOnlWS
//gsoap OnlWS service namespace: http://192.168.101.181:11080/ApsOnlWS.wsdl
//gsoap OnlWS service location: http://192.168.101.181:11080
//gsoap OnlWS service encoding: encoded 
//gsoap OnlWS schema namespace: urn:ApsOnlWS

//#import "stlvector.h"
//soapcpp2 -i -S -x  Sltmdate.h -I/usr/local/share/gsoap/import
// Web Service Interface
//soapcpp2 -c -L -S -x  StlmDate.h 
//soapcpp2 -c -C -L -x  StlmDate.h
struct  OnlWS__StlmdateResp
{
   char *InsIdCd;
   char *SysState;
   char *BfStlmDate;
   char *CurStlmDate;
   long  DateCutTime;
   char *BfTxnTbl;
   char *CurTxnTbl;
   char *UseTlInsFlg;
};

struct  OnlWS__TZeroNodeInfo
{
    char *BatNo;
    char *BatNode;
    char *BatTime;
};

struct OnlWS__TZeroNodeInfoResp
{
    struct OnlWS__TZeroNodeInfo * __ptr;
    int  __size;
}; 
int OnlWS__GetStlmDate(char *option,struct OnlWS__StlmdateResp  *resp);
int OnlWS__GetTZeroNodeInfo(char *option,struct OnlWS__TZeroNodeInfoResp  *resp);
