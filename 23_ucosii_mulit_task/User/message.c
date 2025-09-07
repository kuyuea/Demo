/**
  ******************************************************************************
  * File Name          : message.c
  * Description        : demo code of message driven 
  ******************************************************************************
  *
  * COPYRIGHT(c) 2020 jkuang@BUPT
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include	"uCOSII_Demo.h"


/* Private variables ---------------------------------------------------------*/

//Message mem partition
char 		memMsgBuf[MEMPART_MSG_NBLK][MEMPART_MSG_BLKSIZE];
OS_MEM *memMsgP;


//============================================================
//Message mem partition initial
INT8U Msg_MemInit(void)
{
	INT8U err;

	//mem partition Create
	memMsgP = OSMemCreate((void*)memMsgBuf, MEMPART_MSG_NBLK, MEMPART_MSG_BLKSIZE, &err);
	return(err);
}

//============================================================
//Release a Message mem block
INT8U Msg_MemPut(MESSAGE_HEAD* msgP)
{
	return(OSMemPut(memMsgP, (void *)msgP));
}


//============================================================
//Short Message send
//
//	get msg buf & send msg to a task Q 
//
// 	Arguments:
//		mCode: 			Message code
//		mSendTsk:		Sender TaskID
//		mRecvTsk:		Recv TaskID
//		mSendInst:	Send instance
//		mRecvInst:	Recv Instance
//
//	Returns:
//		Error code
//
INT8U Msg_SendShort(INT16U mCode, INT16S mSendTsk, INT16S mRecvTsk, 
												INT16U	mSendInst, INT16U	mRecvInst)
{
	INT8U err;
	MESSAGE_HEAD *msgP;
	
	//get msg buf & send msg to task Q
	msgP = (MESSAGE_HEAD*)OSMemGet(memMsgP, &err);
	if(msgP)
	{
		msgP->mCode 		= mCode;
		msgP->mSendTsk 	= mSendTsk;
		msgP->mRecvTsk 	= mRecvTsk;
		msgP->mSendInst = mSendInst;
		msgP->mRecvInst = mRecvInst;
		msgP->mLen			= 0;					//Short msg

		err = OSQPost((OS_EVENT*)APP_TQID(mRecvTsk), (void *)msgP);
		if(err)
		{
			err = OSMemPut(memMsgP, (void *)msgP); //Release memory
			return(err);
		}
		return(OS_ERR_NONE);
	}
	return(err);
}

//============================================================
// Timer Expired Message send
//
//	Send timer expired message to a task Q 
//
// 	Arguments:
//		ptmr:           Pointer to expired timer
//		task_id:        Target task ID
//		msg_code:       Message code to send
//		send_inst:      Sender instance
//		recv_inst:      Receiver instance
//
//	Returns:
//		Error code
//
INT8U Msg_SendTimerExpired(OS_TMR *ptmr, INT16S task_id, INT16U msg_code, 
                            INT16U send_inst, INT16U recv_inst)
{
    INT8U err;
    void *msg_ptr;
    TMR_EXPIRED_MSG *msgP;
    
    if (MEMPART_MSG_BLKSIZE < sizeof(TMR_EXPIRED_MSG)) {
        return OS_ERR_MEM_INVALID_SIZE;
    }
    
    // get msg buf & send msg to task Q
    msg_ptr = OSMemGet(memMsgP, &err);
    if(msg_ptr != (void*)0)
    {
        msgP = (TMR_EXPIRED_MSG*)msg_ptr;
        
        // ?????
        msgP->Head.mCode     = msg_code;
        msgP->Head.mSendTsk  = OS_TID_Timer;
        msgP->Head.mRecvTsk  = task_id;
        msgP->Head.mSendInst = send_inst;
        msgP->Head.mRecvInst = recv_inst;
        msgP->Head.mLen      = sizeof(TMR_EXPIRED_MSG) - sizeof(MESSAGE_HEAD);
        
        // 定时器特定数据
        msgP->OSTmrPtr      = (void*)ptmr;
        msgP->OSTmrMatchTime = (INT16U)(ptmr->OSTmrMatch & 0xFFFF);  
 
        err = OSQPost((OS_EVENT*)APP_TQID(task_id), (void *)msgP);
        if(err)
        {
            err = OSMemPut(memMsgP, (void *)msgP); //Release memory
            return(err);
        }
        return(OS_ERR_NONE);
    }
    return(err);
}

//MESSAGE_C

