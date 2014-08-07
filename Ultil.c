/* 
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "Ultil.h"

#define INI_MAX_LINE 128

int ParseConfigFile(const char * sFileName,const char *section,const char *key,char *value)
{
    int ret=0;
    int nextflg=0;
    char *p,*q,*start=NULL,*end=NULL;
	
    char buf[INI_MAX_LINE];
    FILE *fp=NULL;
    fp = fopen(sFileName, "r");
    if (!fp)
	{
	    return -1;
	}
        
    while(fgets(buf, INI_MAX_LINE-1, fp) != NULL)
    {
        p=sTrim(buf);  
        q=section;		
        if(buf[0]=='#')
        {
             continue; 
        }
		if(nextflg&&buf[0]!='[')
		{
		    continue;
		}
        start=strchr(p, '[' );
        if(start!=NULL)
        {
		    nextflg=0;
            end=strchr(start, ']' );
            start++;
            if(end!=NULL)
            {
                for(q;q&&(q!='\0')&&(start!=end-1);q++,start++)
                {
                    if (*start!=*q)
                    {
				        nextflg=1;
                        break;
                    }
                }  		
            }
        }
		if(nextflg)
	    {
           	 continue;	
	    }

        p=strstr(p, key);
        if(p)
        {
            p=strchr(p, '=');
            strcpy(value,++p);
            break;
        }

    }
	
    return 0;
}

char * sTrim(char * ptr)  
{  
    int start=0,end=0,i=0;  
    if (ptr)  
    {  
        for(start=0; isspace(ptr[start]); start++);  
        for(end=strlen(ptr)-1; isspace(ptr[end]); end--);  
        for(i=start; i<=end; i++)  
            ptr[i-start]=ptr[i];  
        ptr[end-start+1]='\0';  
        return (ptr);  
    }  
    else  
        return NULL;  
}  

/*************************************************************
 * Function:    void GetFmtTime(char *tmBuf,int nLen,const char *Fmt)
 * Arguments:   
 * Description: 
 * Date:
 * Author:      
**************************************************************/
void GetFmtTime(char *tmBuf,int nLen,const char *Fmt)
{
    time_t        clock;
    clock = time((time_t *)0);
    struct tm *Tm =localtime(&clock);
    if (Fmt==NULL)
    {
       strftime(tmBuf,nLen,"%Y%m%d",Tm);
    }
    else
    {
        strftime(tmBuf,nLen,Fmt,Tm);
    }
}