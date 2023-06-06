#include <sytcpwhead.h>
#include <syscom.h>
#include <mqi0001f.h>
#include <cmqc.h>
#include <exmq.h>
#include <exmqparm.h>
#include <exmqmsg_001.h>
#include <exmqsvc.h>
#include <mqimsg001.h>
#include <usrinc/atim.h>
#include <usrinc/tx.h>
#include <sqlca.h>
#include <sessionlist.h>
#include <exi0280.h>
#include <exi0212.h>

/*----------------constant, macro, definitins -----------------*/
#define EXCEPTION_SLEEP_INTV         300     /* Exception Sleep interval 5 minutes */
#define EXMQPARM                     (&ctx->exmqparm)
#define MQMSG001                     (&ctx->exmqmsg_001)
#define MQ_INFO                      (&ctx->mq_info)

#define REQ_DATA                     (SESSION_DATA->reg_data)  //ctx->session-req_data
#define REQ_INDATA                   (REQ_DATA->indata)
#define REQ_HCMIHEAD                 (&REQ_DATA->hcmihead)

#define LEN_TCP_HEAD                 72
#define LEN_COMMON_HEAD              300
#define LEN_MQ_CHNL_TYPE             2
/*-------------structure definitions -------------------------*/
typedef struct session_data_s session_data_t;
struct session_data_s {
    CTX_T             ctxt;
    commbuff_t        cb;
    char              *req_data;
    long              req_len;
    char              *as_data;
};

/* 요청전문 : HCMIHEAD[72] + INDATA[(xx) */
typedef struct req_data_s req_data_t;
struct req_data_s {
    hcmihead_t            hcmihead;
    char                  indata[1];

};

/* 응답전문 : HCMIHEAD(72) + OUTDATA(xx) */
typedef struct res_data_s reg_data_t;
struct res_data_s{
    hcmihead_t            hcmihead;
    char                  outdata[1];
};

typedef struct symqtops_onin_ctx_s symqtops_onin_ctx_t;
struct symqtops_onin_ctx_s {
    exmqparm_t             exmqparm;
    exmqmsg_001_t          exmqmsg_001;
    commbuff_t             *cb;
    commbuff_t             _cb;
    session_t              *session;
    mq_info_t              mq_info;
    long                   is_mq_connected;
    MQHCONN                mqhconn;
    MQHOBJ                 mqhobj;
    char                   ilog_jrn_no[LEN_JRN_NO+1];  //image LOG JRN_NO
    char                   jrn_no[LEN_JRN_NO+1];  //27자리
    char                   corr_id[LEN_EXMQMSG_001_CORR_ID+1];  //24자리
    char                   log_flag;

};

typedef struct symqtops_onin_ctx_s symqtops_onin_global_t;
struct symqtops_onin_global_s {
    char                   mq_chnl_id[LEN_MQ_CHNL_TYPE+1];
    long                   is_mq_connected;
    MQHCONN                mqhconn;
    MQHOBJ                 mqhobj;
    long                   session_num;
    long                   session_tmo;
    long                   min_req_len;
};

/* ------------exported global variable declarations ---------------------*/
char           g_svc_name[32];
char           g_chnl_code[  3 + 1];
char           g_appl_code[  2 + 1];
int            g_session_num = 100;
int            g_session_tmo =  10;
int            g_call_seq_no;

int            g_temp_len;
char           g_temp_buf[8192];
int            g_head_len;

symqtops_onin_ctx_t    _ctx;
symqtops_onin_ctx_t    *ctx = &ctx;

symqtops_onin_global_t   g = {0};
/* ------------  exported function  declarations ---------------------*/
int           symqtops_onin(commbuff_t  *commbuff);
int           symqtops_oninr(commbuff_t *commbuff);
int           apsvrinit(int args, char *argv[]);
int           apsvrdone();

//개설요청 
static int a000_initial(int argc, char *argv[]);
static int b000_mqparam_load(symqtops_onin_ctx_t  *ctx);
static int d100_init_mqcon(symqtops_onin_ctx_t  *ctx);
static int e100_set_session(symqtops_onin_ctx_t  *ctx);
static int e200_make_req_data(symqtops_onin_ctx_t  *ctx);
static int f100_put_mqmsg(symqtops_onin_ctx_t  *ctx);
static int f300_insert_exmqwlog(symqtops_onin_ctx_t  *ctx);
static int g100_get_tpctx(symqtops_onin_ctx_t  *ctx);
//개설요청
static int j100_get_commbuff(symqtops_onin_ctx_t  *ctx);
static int j200_reply_res_data(symqtops_onin_ctx_t  *ctx);
//오류처리
static void x000_error_proc(symqtops_onin_ctx_t  *ctx);
static void y000_timeout_proc(symqtops_onin_ctx_t  *ctx);
static int  z000_free_session(symqtops_onin_ctx_t  *ctx);

int    host_data_conv(int type, char *data, int len, int msg_type);
static int  term_id_conv(char *term_id);

/* --------------------  function  prototype ---------------------*/
/* -------------------------------------------------------------- */
int apsvrinit(int argc, char *argv[])
{
    int      rc = ERR_NONE;

    SYS_TRSF;
    /* set commbuff */
    memset((char*)ctx, 0x00, sizeof(symqtops_onin_ctx_t));

    SYS_DBG("initialize");

    rc = a000_initial(argc, argv);
    if( rc == ERR_ERR)   
        return rc;
    
    rc = b000_mqparam_load(ctx);
    if( rc != ERR_NONE)
    {
        SYS_DBG("b000_mqparam_load failed ++++++++++++");
        ex_syslog(LOG_FATAL, "[APPL_DM] b000_mqparam_load() FAIL [해결방안]시스템 담당자 CALL");
    }
    
    rc = d100_init_mqcon(ctx);
    if (rc == ERR_ERR)
    {
        SYS_DBG("d100_init_mqcon failed STEP 1");
        ex_syslog(LOG_FATAL, "[APPL_DM] d100_init_mqcon() FAIL [해결방안]시스템 담당자 CALL");
    }
    SYS_TRSF;

    return rc;
}

/* ---------------------------------------------------------------- */
int 
symqtops_onin(commbuff_t  *commbuff)
{
    int              rc  =  ERR_NONE;
    sysiouth_t       sysiouth = {0};

    SYS_TRSF;
    //에러코드 초기화
    sys_error_init();

    //set commbuff
    ctx->cb = commbuff;

    /* 입력 데이터 검증 */
    if (SYSGWINFO == NULL)
    {
        SYS_HSTERR(SYS_NN, ERR_SVC_SNDERR, "INPUT_DATA_ERR");
        sysocbfb(ctx->cb);
        return ERR_ERR;
    }

    PRINT_SYSGWINFO(SYSGWINFO);

    /* 회선관리 명령어 검증 */
    if (SYSGWINFO->conn_type > 0)
        return ERR_NONE;

    /* 입력 데이터 검증 */
    if (HOSTSENDDATA == NULL)
    {
        SYS_HSTERR(SYS_NM, ERR_SVC_SNDERR, "INPUT_DATA_ERR");
        return ERR_ERR;
    }

    /* 2nc MQ Initialization */
    rc = d100_init_mqcon(ctx);
    if (rc == ERR_ERR){
        SYS_DBG("d100_init_mqcon failed STEP 1");
        ex_syslog(LOG_FATAL, "[APPL_DM] d100_init_mqcon (2nd) STEP 1 FAIL [해결방안] 시스템 담당자 CALL");
        ctx->is_mq_connected = 0;
        sysocbfb(ctx->cb);
        return ERR_ERR;
    }

    /* set session   */
    SYS_TRY(e100_set_session(ctx));
    /* make req data */
    SYS_TRY(f100_put_mqmsg(ctx));
    /* get context   */
    SYS_TRY(g100_get_tpctx(ctx));

    sysocbfb(ctx->cb);

    if(SYSIOUTH == NULL){
        rc =sysocbsi(commbuff, IDX_SYSIOUTH, (char *)&sysiouth, sizeof(sysiouth_t));
        if (rc != ERR_NONE){
            SYS_HSTERR(SYS_LC, SYS_GENERR, "SYSIOUTH COMMBUFF 설정에러");
        }
    }
    SYS_TRSF;
    return ERR_ERR;

/* ---------------------------------------------------------------- */
int 
symqtops_oninr(commbuff_t  *commbuff)
{
    int                    rc = ERR_NONE;
    sysiouth_t             sysiouth = {0};
    symqtops_onin_ctx_t    _ctx     = {0};
    symqtops_onin_ctx_t    *ctx     = &_ctx;

    SYS_TRSF;
    /* set commbuff */
    ctx->cb = &ctx->cb;
    ctx->cb = commbuff;

    SYSICOMM->intl_tx_flag = 0; //대문자 c동작안함
    SYS_DBG("================= 개설응답 START ===================");
    //입력데이터 검증
    if (SYSIOUTH == NULL){
        SYS_HSTERR(SYS_NM, ERR_SVC_SNDERR, "INPUT DATA ERR");
        sysocbfb(ctx->cb);
        return ERR_ERR;
    }
    SYS_TRY(j100_get_commbuff(ctx));
    SYS_TRY(j200_reply_res_data(ctx));

    SYS_DBG("================== 개설응답 END ===================");
    SYS_TREF;

    return ERR_NONE;
SYS_CATCH:
    x000_error_proc(ctx);

    z000_free_session(ctx);

    SYS_TREF;
    
    return ERR_ERR;
}
/* ---------------------------------------------------------------- */
int 
usermain(int argc, char *argv[])
{
    int rc = ERR_NONE;
    set_max_session(g.session_num);
    
    set_session_tmo_callback(y000_timeout_proc, g_session_tmo);


    while(1){
        if (db_connect("") != ERR_NONE){
            SYS_DBG("sleep 60");
            tpschedule(60);
            continue;
        }

        rc = tpschedule(TMOCHK_INTERVAL);    //1초에 한번씩 체크
        if (rc < 0){
            if(tperrno  == TPETIME){
                session_tmo_check();
            }else{
                SYS_HSTERR(SYS_LN, 8600, "tpschedule error. tperrno[%d]", tperrno);
            }
            continue;
        }
        session_tmo_check();
    }
    return ERR_NONE;
}

/* ---------------------------------------------------------------- */
int 
a000_initial(int argc, char *argv[])
{
    int  rc = ERR_NONE;
    int  i;

    SYS_TRSF;

    /* command argument 처리 */
    SYS_TRY(a100_parse_custom_args(argc, argv));

    strcpy(g_arch_head.g_svc_name,  g_svc_name);
    memset(g_arch_head.err_file_name, 0x00, sizeof(g_arch_head.err_file_name));

    /* 통신헤더 */
    g_header_len =  sizeof(hcmihead_t);

    SYS_TREF;
    return ERR_NONE;

SYS_CATCH:
    SYS_TREF;

    return ERR_ERR;
}
/* ---------------------------------------------------------------- */
static int 
a100_parse_custom_args(int argc, char *argv[])
{
    int c;

    while((c = getopt(argc, argv, "s:c:a:M:T:")) != EOF){
        switch(c) {
            case 's':
                strcpy(g_svc_name, optarg);
                break;
            case 'c':
                strcpy(g_chnl_code, optarg);
                break;
            case 'a':
                strcpy(g_appl_code, optarg);
                break;
            case 'M':
                g.session_num = atol(optarg);
                break;
            case 'T'
                g.session_tmo = atol(optarg);
                break;
            case '?':
                SYS_DBG("unrecognized option: %c %s", optarg, argv[optarg]);
                return ERR_ERR;
        }
    }
    /* ------------------------------------------------ */
    SYS_DBG("g_svc_name           = [%s]",   g_svc_name);
    SYS_DBG("g_chnl_code          = [%s]",   g_chnl_code);
    SYS_DBG("g_appl_code          = [%s]",   g_appl_code);
    SYS_DBG("session_num          = [%d]",   g.session_num);
    SYS_DBG("session_tmo          = [%d]",   g.session_tmo);
    /* ------------------------------------------------ */
    /* 서비스명 검증 */
    if (strlen(g_svc_name) == 0 ) ||
        (g_svc_name[0]     == 0x20))
        return ERR_ERR;

    return ERR_NONE;
}

/* ---------------------------------------------------------------- */
static int b100_mqparam_load(symqtops_onin_ctx_t  *ctx)
{
    int          rc       = ERR_NONE;
    static int   mqparam_load_flag = 0;
    mqi0001f_t   mqi0001f;

    SYS TRSF;
    memset (&mqi0001f, 0x00, sizeof (mq10001f_t));

    memcpy(EXMQPARM->chnl_code, g_chnl_code, LEN_EXMQPARM_CHNL_CODE);
    memcpy(EXMQPARM->appl_code, g_appl_code, LEN_EXMQPARM_APPL_CODE);
    
    mqi0001f.in.exmqparm = EXMQPARM;
    rc = mq_exmqparm_select(&mqi0001f);
    if (rc == ERR_ERR) {
        SYS_HSTERR(SYS_LCF, SYS_GENERR, "[FAIL]EXMQPARAM select failed chnl[%s] err[%d][%s]",g_chnl_code, ERR_ERR, g_chnl_code);
        return ERR_ERR;
    }
    utotrim(EXMQPARAM->mq_mngr);
    if(strlen(EXMQPARM->mq_mngr) == 0){
        SYS_HSTERR(SYS_LCF, SYS_GENERR, "[FAIL]EXMQPARAM [%s/%s/%s]",g_chnl_code, ERR_ERR, g_chnl_code);
        return ERR_ERR;
    }
    utotrim(EXMQPARAM->putq_name);
    if(strlen(EXMQPARM->putq_name) == 0){
        SYS_HSTERR(SYS_LCF, SYS_GENERR, "[FAIL]EXMQPARAM [%s/%s/%s] putq_name",g_chnl_code, ERR_ERR, g_chnl_code);
        return ERR_ERR;
    }
    mqparam_load_flag = 1;
    
    SYS_TREF;
    
    return ERR_NONE;
}


/* ---------------------------------------------------------------- */
static int( d100_init_mqcon(symqtops_onin_ctx_t  *ctx)
{
    int   rc = ERR_NONE;
    SYS_TRSF;

    if ( ctx->is_mq_connected == 1){
        return ERR_NONE;
    }

    if (ctx->mqhconn != 0){
        ex_mq_disconnect(&ctx->mqhconn);
        ctx->mqhconn = 0;

    }

    /* queue Manager Connect*/
    rc = ex_mq_connect(&ctx->mqhconn, EXMQPARAM->mq_mngr);
    if (rc == MQ_ERR_SYS){
        ex_syslog(LOG_ERROR, "[CLIENT DM] %s d100_init_mqcon MQ CONNECT failed");
        return ERR_ERR;
    }
    /*
    mq_con_type : Queue Connect Type
     1 = onlu GET, 2 = only PUT, 3 = BOTH
    */
    /* FWR Queue OPEN EIS.KR.KTI_01 LISTEN */
    rc = ex_mq_open(ctx->mqhconn, &ctx->mqhobj, EXMQPARAM->putq_name, MQ_OPEN_OUTPUT);
    if (rc == MQ_ERR_SYS) {
        ex_syslog(LOG_ERROR, "[CLIENT DM] %s d100_init_mqcon MQ CONNECT failed g_chnl_code");
        ex_mq_disconnect(&ctx->mqhconn);
        ctx->mqhconn = 0;
        return ERR_ERR;
    }

    ctx->is_mq_connected = 1;
    SYS_TREF;

    return ERR_NONE;
}
 


/* ---------------------------------------------------------------- */
static int 
e100_set_session(symqtops_onin_ctx_t  *ctx)
{
    int     rc              = ERR_NONE;
    char    jrn_no[LEN_JRN_NO+1]           = {0};
    char    corr_id[LEN_SESSION_CORR_ID+1] = {0};

    SYS_TRSF;

    /* 채널번호, corr_id, msg_id 채번 */
    utoclck(jrn_no);
    utocick(corr_id);

    memcpy(ctx->ilog_jrn_no, jrn_no+1, LEN_JRN_NO);
    memcpy(ctx->jrn_no,    jrn_no+1, LEN_JRN_NO);
    
    /* set session to list */
    SESSION = set_session_to_list(ctx->ilog_jrn_no, SESSION_DATA_SIZE);
    SYS_DBG("SESSION=[%p]", SESSION);

    if (SESSION == 0x00){
        SYS_HSTERR(SYS_LN, sys_error_code(), "%s", sys_error_msg());
        return ERR_ERR;
    }

    memcpy(SESSION->jrn_no, ctx->jrn_no, LEN_JRN_NO);

    memcpy(SESSION->corr_id, "20230607153012345678", LEN_HCMIHEAD_QUEUE_NAME);

    if(EXPARM){
        SESSION->wait_sec = atoi(EXPARM->time_val);

    }
    SYS_DBG("SESSION DATA %s", SESSION_DATA);

    rc = sysocbdb(ctx->cb, &SESSION_DATA->cb);
    if (rc = ERR_ERR){
        SYS_HSTERR(SYS_LN, 8600, "sysocbdb failed jrno[%.27s]", ctx->ilog_jrn_no);
    }
} 


/* ---------------------------------------------------------------- */
static int
e200_make_req_data(symqtops_onin_ctx_t  *ctx)
{
    long               rc       = ERR_NONE
    long               data_len = 0;
    long               req_len  = 0;
    int                size;
    int                len;
    char               *hp, *dp, *dp2, *dp3, buff[10];
    req_data_t         *req_data = 0;
    exi0280_t          exi0280;
    hcmihead_t         hcmihead;


    SYS_TRSF;

    memset(g_temp_buf, 0x00 , IDX_HOSTSENDDATA);
    dp = g_temp_buf;

    /* 송신 길이 검증 */
    len = sysocbgs(ctx->cb, IDX_HOSTSENDDATA);
    if (len <= 0){
        SYS_HSTERR(SYS_NN, ERR_SVC_SNDERR, "DATA NOT FOUND");
        return ERR_ERR;
    }

    /* 송신할 데이터 */
    dp2 = (char *) sysocbgp(ctx->cb, IDX_HOSTSENDDATA);
    if(dp2 == NULL){
        SYS_HSTERR(SYS_NN, ERR_SVC_SNDERR, "DATA NOT FOUND");
        return ERR_ERR;
    }
    SYSGWINFO->sys_type = SYSGWINFO_SYS_CORE_BANK;

    if (SYSGWINFO->msg_type == SYSGWINFO_MSG_1200){
        memset(&exi0280, 0x00, sizeof(exi0280_t));
        exi0280.in.type = 2;
        memcpy(exi0280.in.data, dp2, LEN_EXMQMSG1200);
        exmsg1200_null_proc(&exi0280);

        /* 전송데이터 복사 */
        memcpy(&dp[g_header_len], exi0280.out.data, LEN_EXMQMSG1200_COMM);
        g_temp_len = LEN_EXMQMSG1200_COMM;

        /* 기타 데이터 추가 되어 있는 경우 1304 보다 클때 */
        if (len > LEN_EXMQMSG1200){
            memcpy(&dp[g_header_len + g_temp_len], &dp2[LEN_EXMQMSG1200], (len - LEN_EXMQMSG1200));
            g_temp_len += (len - LEN_EXMQMSG1200);
        }
    }
    else{
        /* 전송 데이터 복사 */
        memcpy(&dp[g_head_len], dp2, len); 
        g_temp_len = len;
    }
    /*
     HCMIHEAD
    */
    /* 통신 헤더 생성 */
    memset(dp, 0x20, g_head_len);

    hcmihead = (hcmihead_t *) dp;
    /* 응답으로 호스트계에 전송하는 경로 */
    hp = sysocbgp(ctx->cb, IDX_TCPHEAD);
    if (hp != NULL){
        memcpy(dp, hp, g_head_len);
    }
    
    if(SYSGWINFO->msg_type == SYSGWINFO_MSG_1200){
        memcpy(hcmihead->tx_code, &dp2[10], LEN_TX_CODE);
    }else{
        memcpy(hcmihead->tx_code, &dp2[9] , LEN_TCP_CODE);
    }
    hcmihead->comm_type = 'S';

    sprintf(buff, "%3d", SYSGWINFO->time_val);
    memcpy(hcmihead->wait_time, buff, LEN_HCMIHEAD_WAIT_TIME);
    hcmihead->cont_type = CONT_TYPE_END;

    sprintf(buff, "%5d", g_temp_len);
    memcpy(hcmihead->data_len, buff, LEN_HCMIHEAD_DATA_LEN);
    hcmihead->cnvt_type = '0';

    memcpy(hcmihead->tran_id, dp2, LEN_HCMIHEAD_TRAN_ID);
    memcpy(hcmihead->resp_code, '0', LEN_HCMIHEAD_RESP_CODE);

    //queue name 저장
    memcpy(hcmihead->queue_name, SESSION->corr_id, LEN_HCMIHEAD_QUEUE_NAME);

    if (IS_SYSGWINFO_SVC_REPLAY){
        g_call_seq_no++;
        if (g_call_seq_no > 5999999)
            g_call_seq_no = 1;
            sprintf(buff, "%7d", g_call_seq_no);
            memcpy(hcmihead->mint_rtk, buff, 7);
    }

    SYS_DBG("e200_make_seq_data: input len[%d]", len);
    SYS_DBG("e200_make_seq_data: tx_code  [%10.10s] tran_id[%-4.4s]"
    , hcmihead->tx_code, hcmihead->tran_id);

    /* --------------session save start ---------------------------*/
    req_len  = g_head_len + g_temp_len + 256;
    req_data = malloc(req_len + 100);
    id(req_data == NULL){
        SYS_HSTERR(SYS_LN, SYS_GENERR, " sys_tpcall failed [%d]jrn_no[%s]"len, SESSION->ilog_jrn_no);
        return ERR_ERR;
    }
    memset(req_data, 0x00, req_len + 100);
    memset(req_data, 0x20, req_len);

    if( SYSGWINFO->msg_type == SYSGWINFO_MSG_1200){
        memcpy(req_data->hcmihead.tx_code, &dp2[10], LEN_TX_CODE);
    }else{
        memcpy(req_data->hcmihead.tx_code, &dp2[9] , LEN_TX_CODE);
    }
    req_data->hcmihead.comm_type = 'S';
    /* --------------session save -end --------------------------*/
    /* 전문변환 */
    size = g_temp_len + g_head_len +256;
    dp3 = (char *) malloc(size);
    if (dp3 == NULL){
        ex_syslog(LOG_FATAL, "[APPL_DM] syonhtsend :메모리 설정 에러
                              해결방안 시스템 담당자 CALL" );
        ex_syslog(LOG_FATAL, "[APPL_DM] %s syonhtsend :메모리 설정 에러: %d"
                           __FILE__, size);
        SYS_HSTERR(SYS_NN, ERR_SVC_SNDERR, "MEMORY ALLOC ERR");
        return ERR_ERR;
    }
    //SYS_DBG
    //SYS_DBG

    memcpy(req_data->indata, g_temp_buf, (g_head_len + g_temp_len));
    SESSION_DATA->reg_data = reg_data;
    SESSION_DATA->req_len  = g_temp_len;

    //전문전 로그 기록
    if ( f300_insert_exmqwlog(ctx) == ERR_ERR){
    
        ex_syslog(LOG_ERROR, "f300_insert_exmqwlog() FAIL CALL system admin");
        sysocbfb(ctx->cb);
        return ERR_ERR;
    }

    utoae2eb((unsigned char *)g_temp_buf, g_head_len, (unsigned char*)dp3);
    host_data_conv(AS_TO_BE, g_temp_buf, (g_temp_len + g_head_len));
    len = g_head_len + g_temp_len;

    memcpy(req_data->indata, dp3, len);
    SYS_TREF;

    return ERR_NONE;
} 


/* ---------------------------------------------------------------- */
static int
f100_put_mqmsg(symqtops_onin_ctx_t  *ctx)
{
    int           rc        = ERR_NONE;
    mq_info_t     mq10001f  = {0};
    char          *hp;
    MQMD          md        = {MQMD_DEFAULT};
    int           size, len;
    int           g_temp_len;
    char          *dp, *dp2;
    char          corridTemp[20];
    char          l_head_len[ 4 + 1];
    char          l_head_buf[72 + 1];
    char          l_temp_buf[ 5000 ];

    SYS_TRSF;

    /* MQ 기본 정보 셋팅 */
    mqinfo                  = ctx->mqinfo;
    mqinfo.hobj             = &ctx->mqhobj;
    mqinfo.hcon             = ctx->mqhconn;
    mqinfo.coded_charset_id = EXMQPARM->charset_id;
    memcpy(mqinfo.msgid,   SESSION->corr_id,  LEN_EXMQMSG_001_MSG);
    memcpy(mqinfo.corrid,  SESSION->corr_id,  LEN_EXMQMSG_001_CORR_ID);

    SYS_DBG("ilog_jrn_no[%s], corr_id[%s]", ctx->ilog_jrn_no, mqinfo.corr_id);

    mqinfo.msglen = SESSION_DATA->req_len + g_head_len;

    memcpy(mqinfo.msgbuf,  ((req_data_t *) REQ_DATA)->indata, mqinfo.msglen);
    rc = ex_mq_put_with_opts(&mqinfo, &md, 0);
    if(rc = ERR_NONE){
        rc = ex_mq_commit(&ctx->mqhconn);
    }else{
        /* reset flag for mq_put FAIL !!! */
        ctx->is_mq_connected = 0;
        SYS_DBG("CALL ex_mq_put FAIL");
        ex_syslog(LOG_FATAL, "[APPL_DM] Call ex mq put FAIL [해결방안 ] 시스템 담당자 CALL");
        rc = ex_mq_commit(&ctx->mqhconn);

        return ERR_ERR;

    }
    SYS_TREF;

    return ERR_NONE;
}
/* ---------------------------------------------------------------- */
static int
f200_conv_as_to_eb(symqtops_onin_ctx_t  *ctx)
{
    int     rc          = ERR_NONE;
    int     cd          = 0;
    int     size;
    long    min_req_len  = 0;
    req_data_t *req_data = 0;
    reg_data_t *eb_data  = 0;

    SYS_TRSF;

    reg_data = SESSION_DATA->req_data;
    req_len  = SESSION_DATA->req_len;

    eb_data = malloc(req_len+1);
    if(eb_data == NULL){
        SYS_HSTERR(SYS_LN, SYS_GENERR, "malloc failed [%d]", req_len);
    }

    memset(eb_data, 0x00, req_data);
    memset(eb_data, 0x20, req_len );

    SYS_DBG("sizeof(req_data->indata):[%d]", sizeof(req_data->indata));
    SYS_DBG("req_len: [%d]", req_len);

    /* CODE conversion (ASCII->EBCDIC) */
    utoae2eb(&req_data->hcmihead, LEN_HCMIHEAD, &eb_data->hcmihead);
    utoae2eb(req_data->indata, sizeof(req_data->indata), eb_data->indata);

    memcpy(req_data, eb_data, req_len);
    free(eb_data);

    SYS_TREF;

    return ERR_NONE;
}
/* ---------------------------------------------------------------- */
static int
f300_insert_exmqwlog(symqtops_onin_ctx_t  *ctx)
{
    int             db_rc       = ERR_NONE;
    mq_info_t       mq_info     = {0};
    MQMD            md          = {MQMD_DEFAULT};
    exmqmsg_001_t   exmqmsg_001;
    mqi0001f_t      mqimsg001;

    SYS_TRSF;

    /* ------------------MQ 기본 정보 세팅 ----------------------------*/
    mqinfo          = ctx->mqinfo;
    utocick(mqinfo.msgid);
    memcpy(mqinfo.corr_id, SESSION->corr_id, LEN_EXMQMSG_001_CORR_ID);

    SYS_DBG("req_data ASCII: [%s]", ((req_data_t) REQ_DATA)->indata);
    mqinfo.msglen = SESSION_DATA->req_len + g_head_len;
    SYS_DBG("reqinfo.msglen: [%d]", reqinfo.msglen);  
    memcpy(mqinfo.msgbuf, ((req_data_t *)REQ_DATA)->indata, mqinfo.msglen);

    /* ------------------mqinfo_001 세팅 ----------------------------*/
    memset(&mqmsg_001, 0x00, sizeof(exmqmsg_001_t));
    memcpy(mqmsg_001.chnl_code, g_chnl_code , LEN_EXMQPARM_CHNL_CODE);
    memcpy(mqmsg_001.appl_code, g_appl_code , LEN_EXMQPARM_APPL_CODE);
    memcpy(mqmsg_001.msg_id   , mqinfo.msg_id ,  LEN_EXMQPARM_MSG_ID);
    memcpy(mqmsg_001.corr_id  , mqinfo.corr_id, LEN_EXMQPARM_CORR_ID);
    mqmsg_001.mqlen = mqinfo.msglen;
    memcpy(mqmsg_001.mqmsg   , mqinfo.msgbuf   , mqmsg_001.mqlen);
    mqmsg_001.io_type = 1;   //0이 get 1이 put
    memcpy(mqmsg_001.ilog_jrn_no, SESSION->ilog_jrn_no, LEN_EXMQMSG_001_ILOG_JRN_NO);
    
    /* ------------------MQ 로그 처리    ----------------------------*/
    memset(&mqmsg_001, 0x00, sizeof(exmqmsg_001_t));
    mqmsg_001.mqmsg_001 = &mqmsg_001;

    db_rc = mqomsg001(&mqimsg001);
    if (db_rc == ERR_ERR){
        EXEC SQL ROLLBACK WORK;
        ex_syslog(LOG_ERROR, "[APPL_DM] %s f300_insert_exmqwlog: MQMSG001 INSERT ERROR 시스템 담당자 CALL" );
        return ERR_ERR;
    }

    SYS_TREF;

    return  ERR_NONE;

}
static int
g100_get_tpctx(symqtops_onin_ctx_t  *ctx)
{
    int     rc          = ERR_NONE;

    SYS_TRSF;
 
    /* get tmax context to relay svc */
    if(tpgetctx(&SESSION->ctx) < 0){
        SYS_HSTERR(SYS_LN, 782900, "tpgetctx failed jrn_no[%s]", SESSION->ilog_jrn_no);
        return ERR_ERR;        
    }
    // 응답필요없는 것 tprelay
    if (!IS_SYSGWINFO_SVC_REPLAY){
        sys_tprelay(“EXTRELAY”, &SESSION_DATA->cb, 0, &SESSION_DATA->ctxt);
        if(rc == ERR_ERR){
            SYS_HSTERR(SYS_LN, SYS_GENERR, "sys_tprelay failed jrn_no[%s]", SESSION->ilog_jrn_no);
            return ERR_ERR;            
        }
        return GOB_NRM;
    }
    SESSION->cd = SESSION_READY_CD;

    SYS_TREF;

    return ERR_NONE;
}
/* ---------------------------------------------------------------- */
static int 
x000_error_proc(symqtops_onin_ctx_t *ctx)
{
     int rc = ERR_NONE;
     SYS_TRSF;

     if(session = 0x00){
         SYS_HSTERR(SYS_LN, SYS_GENERR, “Session is NULL. err , sys_err_msg());
    } else {
         SYS_HSTERR(SYS_LN, sys_error_code, “%s”, sys_err_msg());

        //service return
        if(SESSION->cd == SESSION_READY_CD){
        rc = tprelay(“EXTRELAY”, &SESSION_DATA->cb, 0, &SESSION_DATA->ctxt);
        if (rc==ERR_ERR){
            SYS_HSTERR(SYS_LN, SYS_GENERR, “x000_error_proc(): sys_tprelay(EXTRELAY) failed jrn_no[%s]”, SESSION->ilog_jrn_no);
            }
        }
    }
    z000_free_session();
    SYS_TREF;
    return ERR_NONE;
}
