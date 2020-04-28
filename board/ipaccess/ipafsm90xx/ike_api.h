#ifndef IKE_API_H_20200401
#define IKE_API_H_20200401

#include <common.h>

#define SIG_SZ 512
#define PAD_SZ 12
#define KEY_SZ 0x510
#define CIPH_SZ 0x530

/*Valid boot licen RSM values*/
#define BL_RSM_FUSE 0x00
#define BL_RSM_PROD 0x01
#define BL_RSM_DEV  0x02
#define BL_RSM_SPECIALS 0x03


enum QSEECom_command_id {
  QSEECOM_FSM_IKE_REQ = 0x203,
  QSEECOM_FSM_IKE_REQ_RSP = 0x204,
};

enum IkeCmdId{
  QSEECOM_FSM_IKE_CMD_SIGN = 0x200,
  QSEECOM_FSM_IKE_CMD_PROV_KEY = 0x201,
  QSEECOM_FSM_IKE_CMD_ENCRYPT_PRIVATE_KEY = 0x202,
  /*IPA new commands added*/
  QSEECOM_FSM_IKE_CMD_KEY_PRESENT = 0x500,
  QSEECOM_FSM_IKE_CMD_MODEXP = 0x501,
  QSEECOM_FSM_IKE_CMD_WIPE_KEY = 0x502,
  QSEECOM_FSM_IKE_CMD_RSM = 0x503,
  QSEECOM_FSM_IKE_CMD_BOOT_LIC = 0x504,

};


/* Response Structure - Prov Key*/
typedef struct{
    int result;
} toeIkeRspProvKeyCmdType;

/* Request Structure- Prov Key */
typedef struct{
    /* buffer containing the encrypted key */
    uint8_t keybuf[CIPH_SZ];
} toeIkeProvKeyCmdType;

/*Generic command type which doesn't need any input data as such*/
typedef struct {
    uint32_t id;
}toeIkeStatusCmdType;

typedef struct {
    uint8_t *bl;
    uint32_t bl_size;
    uint8_t *bl_sig;
    uint32_t bl_sig_size;
}toeIkeProvBlCmdType;

/*Only Provisioning key command supported for now*/
typedef struct {
    int cmdId;
    union {
      toeIkeProvKeyCmdType    c1;
      toeIkeStatusCmdType     c2;
      toeIkeProvBlCmdType     c3;
    } reqBuf;
} genericRequestCmdType;


/*RSM command response*/
typedef struct {
    int result;
    uint32_t rsm;
} toeIkeRspRsmCmdType;


/*Data structure for generic command response, which just need to send command status.*/
typedef struct  {
    int result;
} toeIkeRspStatusCmdType;


extern int32_t toeIkeKeyPresent(void);
extern int32_t toeIkeRsm(void);

#endif
