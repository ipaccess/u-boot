/********************************************************************
---------------------------------------------------------------------
 Copyright (c) 2020 ipaccess, Inc.
 All Rights Reserved.
----------------------------------------------------------------------
*********************************************************************/

#include <common.h>
#include "ike_api.h"
#include "fastcall_tz.h"

typedef struct
{
    int32_t  cmd_id;
    void*    req;  /* Ike req*/
    int32_t  req_len;
    void*    rsp;  /*Ike resp*/
    int32_t  rsp_len;
}qseecom_req_rsp_t;

typedef struct
{
    uint32_t   result; /*SCM result*/
    uint32_t   res_type;
    uint32_t   data;
}qseecom_resp_return_t;

/*Struct to make sure that cmd_buf is always at lower address than resp_buf*/
typedef struct 
{
    qseecom_req_rsp_t cmd_buf;
    qseecom_resp_return_t resp_buf;
}qsee_com_req_and_resp_t;

/*Generic function to to do a tz fast call*/
int smc_call(uint32_t id, void * cmd_buf , uint32_t cmd_len, void * resp_buf , uint32_t resp_len)
{
    int ret = -1;
    int token;

    if (cmd_buf && resp_buf && resp_len)
    {
        /*TODO: MMU Cache clean and invalidate, once we have d-cache enabled*/
        
        ret = fastcall_tz_ike(id, (uint32_t)&token, (void *)cmd_buf, resp_buf,resp_len);

        /*TODO: MMU Cache clean and invalidate, once we have d-cache enabled*/
    }


    return ret;
}

/*Generic Qseecom send function to pass any ike request to trustzone*/
int QSEECom_send_service_cmd(void *cmd_req, uint32_t cmd_req_len, void *cmd_resp, uint32_t cmd_resp_len, uint32_t qseecom_cmd_id)
{
    int ret = -1;

    qsee_com_req_and_resp_t req_resp;

    memset(&req_resp,0,sizeof(req_resp));   
   
    req_resp.cmd_buf.cmd_id = qseecom_cmd_id;
    req_resp.cmd_buf.req = cmd_req;
    req_resp.cmd_buf.req_len = cmd_req_len;
    req_resp.cmd_buf.rsp = cmd_resp;
    req_resp.cmd_buf.rsp_len = cmd_resp_len;

    /*Initialise the result to none zero value, as successfull smc execution should change this to 0 or -ve non-zero values*/
    req_resp.resp_buf.result=0xFF;

    /*make smc call*/
    ret = smc_call(TZ_EXECUTIVE_ID,(void *)&req_resp.cmd_buf, sizeof(qseecom_req_rsp_t), (void *)&req_resp.resp_buf,sizeof(req_resp.resp_buf));

    /*Not zero, smc failed*/
    if (req_resp.resp_buf.result)
        printf("SMC call failed %d %d \n", ret, req_resp.resp_buf.result );
    else
        ret = 0;
    
    return ret;
    
}

/*Function to get RSM from Trustzone*/
int32_t toeIkeRsm()
{
    int ret = 0;
    toeIkeStatusCmdType req;
    toeIkeRspRsmCmdType rsp;
    genericRequestCmdType *qseeReq = calloc(sizeof(genericRequestCmdType),1);

    if (!qseeReq)
        return BL_RSM_FUSE; /*Use fuses to read RSM*/

    req.id = QSEECOM_FSM_IKE_CMD_RSM;

    /*Successful ike tz should zeroize this*/
    rsp.result=0xFF;
    rsp.rsm = BL_RSM_FUSE; /*Default to FUSE RSM, successful TZ call will modify this*/ 

    qseeReq->cmdId = QSEECOM_FSM_IKE_CMD_RSM;
    memcpy((void *)&(qseeReq->reqBuf), (void *)&req, sizeof(toeIkeStatusCmdType));
    ret = QSEECom_send_service_cmd((void *)qseeReq,
                sizeof(genericRequestCmdType), (void *)&rsp,
                sizeof(toeIkeRspStatusCmdType),
                QSEECOM_FSM_IKE_REQ_RSP);
    free(qseeReq);
    qseeReq = NULL;
    if ((0 == ret) && (0 == rsp.result))
    {
        ret = rsp.rsm; /*Returned RSM from trustzone*/
    }
    else
    {
        ret = BL_RSM_FUSE; /*TZ failed, return use fuse rsm */
    }

    return ret;

}

#define CONFIG_TEST_TZ
#if defined(CONFIG_TEST_TZ)
int do_tz_call(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = 0;
    ret = toeIkeRsm();
    
    switch(ret)
    {
        case BL_RSM_FUSE:
            printf("Boot license: RSM FUSE\n");
            break;
        case BL_RSM_DEV:
            printf("Boot license: RSM DEV\n");
            break;
        case BL_RSM_PROD:
            printf("Boot license: RSM PRODUCTION\n");
            break;
        case BL_RSM_SPECIALS:
            printf("Boot license: RSM SPECIALS\n");
            break;
        default:
            printf("Boot license error: %d\n",ret);
            break;   
    }
    
    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
    tz_call, 1, 1, do_tz_call,
    "For now this just reads RSM from TZ",
    "No args"
);
#endif
