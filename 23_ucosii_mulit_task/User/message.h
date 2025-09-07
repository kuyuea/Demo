/**
  ******************************************************************************
  * File Name          : message.h
  * Description        : demo code of message header 
  ******************************************************************************
  *
  * COPYRIGHT(c) 2019 jkuang@BUPT
  *
  ******************************************************************************
  */

#ifndef   MESSAGE_H
#define   MESSAGE_H
#define		MC_TMR_EXPIRED_BASE     200	// 定时器超时消息基础码
#ifdef __cplusplus
		 extern "C" {
#endif

			 
/**************************************************************************
	message head format :
 
     mCode         message general code inside one CPU system
     mSendTsk      message sender TaskID or Mudule ID
     mRecvTsk      message receiver TaskID or Mudule ID
			 0-255:Task, -1 and bellow: ISR or Other Module
     mSendInst     message sender instance ID
     mRecvInst     message receiver instance ID
     mLen          Data length follow mLen
**************************************************************************/
typedef struct _MESSAGE_HEAD
{
		INT16U	mCode;
		INT16S 	mSendTsk;
		INT16S 	mRecvTsk;
    INT16U  mSendInst;
    INT16U  mRecvInst;
    INT16U  mLen;
} MESSAGE_HEAD;

#define pMSG_HEAD(x) ((MESSAGE_HEAD *)&(x))


// UART DATA MSG definition ================================================
typedef struct _UART_RX_MSG
{
	INT16U	mCode;
	INT16S	mSendTsk;
	INT16S	mRecvTsk;
	char		aChr;
}UART_RX_MSG;

#define pUART_RX_MSG(x) ((UART_RX_MSG *)&(x))

// 定时器消息结构体
typedef struct _TMR_EXPIRED_MSG
{
    MESSAGE_HEAD  Head; 						 // 消息头
    void   				*OSTmrPtr;         // 定时器指针
    INT16U        OSTmrMatchTime;    // 定时器匹配时间
} TMR_EXPIRED_MSG;

#define pTMR_EXPIRED_MSG(x) ((TMR_EXPIRED_MSG *)&(x))



// message Code definition ==========================================
#define MC_USART1_IRQ_RXNE				100			//USART1 IRQ RXNE
//#define MC_USART1_IRQ_TXE					101			//USART1 IRQ TXE
//#define MC_USART1_DATA_SEND				102			//USART1 data send

#define MC_TMRTST_TMR1						150
#define MC_TMRTST_TMR2						151


#define MC_KEYx_SWITCH						170			//KEY x(Instance:1-4) switch

// message error code ===============================================
//#define EX_ERR_MSG_SEND       		1000


//===================================================================
//mem partition for message
#define MEMPART_MSG_NBLK		10
#define MEMPART_MSG_BLKSIZE	32
#define MEMPART_TMRMSG_BLKSIZE  24  //足够容纳TMR_EXPIRED_MSG

extern char 	 memMsgBuf[MEMPART_MSG_NBLK][MEMPART_MSG_BLKSIZE];
extern OS_MEM *memMsgP;

//===================================================================
// prototypes
INT8U Msg_MemInit(void);
INT8U Msg_MemPut(MESSAGE_HEAD* msgP);

INT8U Msg_SendShort(INT16U mCode, INT16S mSendTsk, INT16S mRecvTsk, 
												INT16U	mSendInst, INT16U	mRecvInst);

INT8U Msg_SendTimerExpired(OS_TMR *ptmr, INT16S task_id, INT16U msg_code, 
                            INT16U send_inst, INT16U recv_inst);


#ifdef __cplusplus
}
#endif 
#endif  //MESSAGE_H
