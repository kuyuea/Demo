/**
  ******************************************************************************
  * File Name          : timer_msg_test.h
  * Description        : Message-driven timer test header
  ******************************************************************************
  */
 
#ifndef TIMER_MSG_TEST_H
#define TIMER_MSG_TEST_H
 
#ifdef __cplusplus
extern "C" {
#endif
 
#include "uCOSII_Demo.h"
#include "ucos_ii.h"
#include "message.h"
#include "farsight_f407.h"
#include "usart_it.h"
 
// 测试任务定义
#define TASK_TMR_MSG_TEST_PRIO    21
#define TMR_MSG_TEST_STK_SIZE     128
 
// 定时器消息码定义 (基于MC_TMR_EXPIRED_BASE)
#define MSG_TMR_LED1_EXPIRED      (MC_TMR_EXPIRED_BASE + 1)
#define MSG_TMR_LED2_EXPIRED      (MC_TMR_EXPIRED_BASE + 2)
#define MSG_TMR_LED3_EXPIRED      (MC_TMR_EXPIRED_BASE + 3)
#define MSG_TMR_LED4_EXPIRED      (MC_TMR_EXPIRED_BASE + 4)
 
// 定时器配置 (基于10Hz系统时钟，100ms/tick)
#define TMR_LED1_PERIOD          5      // 500ms
#define TMR_LED2_PERIOD          10     // 1000ms
#define TMR_LED3_PERIOD          15     // 1500ms
#define TMR_LED4_PERIOD          20     // 2000ms
 
// 函数声明
INT8U Timer_Msg_Test_Init(void);
void Timer_Msg_Test_Task(void *p_arg);
void Timer_Msg_Test_Show_Status(void);
 
#ifdef __cplusplus
}
#endif
 
#endif /* TIMER_MSG_TEST_H */
