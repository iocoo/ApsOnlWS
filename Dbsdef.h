#ifndef __DBS_DEF_H__
#define __DBS_DEF_H__

#ifdef _DB_ORA
#define DBS_NOTFOUND        1403
#endif

#ifdef _DB_DB2
#define DBS_NOTFOUND        100
#endif

#ifdef _DB_ORA
#define DBS_NULL            -1405
#endif

#ifdef _DB_DB2
#define DBS_NULL            -305
#endif

#define DBS_IND_NULL        -1
#define DBS_OK               0
 
 typedef struct  {
  char    ins_id_cd       [  12];
  char    sys_state       [   2];
  char    bf_stlm_date    [   9];
  char    cur_stlm_date   [   9];
  long    date_cut_time;
  char    bf_txn_tbl      [   2];
  char    cur_txn_tbl     [   2];
  char    use_tl_ins_flg  [   2];
} tbl_bat_cut_ctl_def;


#endif  //__DBS_DEF_H__
