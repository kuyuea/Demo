/**
  ******************************************************************************
  * File Name          : timer_msg_test.c
  * Description        : Message-driven timer test implementation
  ******************************************************************************
  */
 
#include "timer_msg_test.h"
 
/* Private variables ---------------------------------------------------------*/
static OS_STK timer_msg_test_task_stk[TMR_MSG_TEST_STK_SIZE];
 
// 消息队列
static void *timer_msg_test_q_array[10];
static OS_EVENT *timer_msg_test_q;
 
// 定时器句柄
static OS_TMR *tmr_led1;
static OS_TMR *tmr_led2;
static OS_TMR *tmr_led3;
static OS_TMR *tmr_led4;
 
// LED状态计数器
static INT32U led1_count = 0;
static INT32U led2_count = 0;
static INT32U led3_count = 0;
static INT32U led4_count = 0;
 
// 时间戳记录
static INT32U last_led1_time = 0;
static INT32U last_led2_time = 0;
static INT32U last_led3_time = 0;
static INT32U last_led4_time = 0;
 
// 测试状态标志
static INT8U test_running = 1;  // 1=运行中, 0=已暂停
static INT8U timers_active = 1; // 1=定时器激活, 0=定时器暂停

/* Private function prototypes -----------------------------------------------*/
static void print_timer_status(void);
static void print_test_header(void);
 
//============================================================================
// 消息驱动定时器测试初始化
INT8U Timer_Msg_Test_Init(void)
{
    INT8U err;
    
    // 创建消息队列
    timer_msg_test_q = OSQCreate(&timer_msg_test_q_array[0], 10);
    if (timer_msg_test_q == NULL) {
        return OS_ERR_Q_FULL;
    }
    
    // 注册任务队列到应用记录
    APP_TQID(APP_TID_msg_tmrtest) = timer_msg_test_q;
    
    // 创建消息驱动的定时器
    tmr_led1 = OSTmrCreateMsgDriven(
        TMR_LED1_PERIOD,              // 初始延迟
        TMR_LED1_PERIOD,              // 周期
        OS_TMR_OPT_PERIODIC,          // 周期性定时器
        APP_TID_msg_tmrtest,         // 目标任务ID
        MSG_TMR_LED1_EXPIRED,        // 消息码
        1,                           // 发送实例
        1,                           // 接收实例
        (INT8U*)"LED1_MSG_TMR",      // 定时器名称
        &err
    );
    
    if (err != OS_ERR_NONE) {
        return err;
    }
    
    tmr_led2 = OSTmrCreateMsgDriven(
        TMR_LED2_PERIOD,
        TMR_LED2_PERIOD,
        OS_TMR_OPT_PERIODIC,
        APP_TID_msg_tmrtest,
        MSG_TMR_LED2_EXPIRED,
        2, 2,
        (INT8U*)"LED2_MSG_TMR",
        &err
    );
    
    if (err != OS_ERR_NONE) {
        return err;
    }
    
    tmr_led3 = OSTmrCreateMsgDriven(
        TMR_LED3_PERIOD,
        TMR_LED3_PERIOD,
        OS_TMR_OPT_PERIODIC,
        APP_TID_msg_tmrtest,
        MSG_TMR_LED3_EXPIRED,
        3, 3,
        (INT8U*)"LED3_MSG_TMR",
        &err
    );
    
    if (err != OS_ERR_NONE) {
        return err;
    }
    
    tmr_led4 = OSTmrCreateMsgDriven(
        TMR_LED4_PERIOD,
        TMR_LED4_PERIOD,
        OS_TMR_OPT_PERIODIC,
        APP_TID_msg_tmrtest,
        MSG_TMR_LED4_EXPIRED,
        4, 4,
        (INT8U*)"LED4_MSG_TMR",
        &err
    );
    
    if (err != OS_ERR_NONE) {
        return err;
    }
    
    // 启动所有定时器
    OSTmrStart(tmr_led1, &err);
    if (err != OS_ERR_NONE) return err;
    
    OSTmrStart(tmr_led2, &err);
    if (err != OS_ERR_NONE) return err;
    
    OSTmrStart(tmr_led3, &err);
    if (err != OS_ERR_NONE) return err;
    
    OSTmrStart(tmr_led4, &err);
    if (err != OS_ERR_NONE) return err;
    
    // 创建测试任务
    err = OSTaskCreateExt(
        Timer_Msg_Test_Task,
        (void*)0,
        &timer_msg_test_task_stk[TMR_MSG_TEST_STK_SIZE-1],
        TASK_TMR_MSG_TEST_PRIO,
        APP_TID_msg_tmrtest,         // 使用新的独立任务ID
        &timer_msg_test_task_stk[0],
        TMR_MSG_TEST_STK_SIZE,
        (void*)0,
        OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
    );
    
    test_running = 1;
    timers_active = 1;
    return err;
}
 
//============================================================================
// 消息驱动定时器测试任务主函数
void Timer_Msg_Test_Task(void *p_arg)
{
    INT8U err;
    void *msg_ptr;
    TMR_EXPIRED_MSG *msgP;
    char *uart_buf;
    INT32U current_time;
    
    p_arg = p_arg;  // 避免编译器警告
    
    // 初始化LED
    LED1_OFF;
    LED2_OFF;
    LED3_OFF;
    LED4_OFF;
    
    print_test_header();
    
    while (1) {
        // 等待定时器消息
        msg_ptr = OSQPend(timer_msg_test_q, 0, &err);
        
        if (err == OS_ERR_NONE) {
            // 转换为正确的消息类型
            msgP = (TMR_EXPIRED_MSG*)msg_ptr;
            current_time = OSTimeGet();
            
            // 验证消息是否发送给正确的任务
            if (msgP->Head.mRecvTsk != APP_TID_msg_tmrtest) {
                uart_buf = Uart_Sendbuf_Get();
                if (uart_buf) {
                    sprintf(uart_buf, "Wrong task ID: %d, expected: %d\n", 
                            msgP->Head.mRecvTsk, APP_TID_msg_tmrtest);
                    USER_USART1_print(uart_buf);
                }
                Msg_MemPut(&msgP->Head);
                continue;
            }
            
            // 处理定时器消息
            switch (msgP->Head.mCode) {
                case MSG_TMR_LED1_EXPIRED:
                    // 只有在测试运行且定时器激活时才处理
                    if (test_running && timers_active) {
                        LED1_TOGGLE;
                        led1_count++;
                        last_led1_time = current_time;
                    }
                    break;
                    
                case MSG_TMR_LED2_EXPIRED:
                    // 只有在测试运行且定时器激活时才处理
                    if (test_running && timers_active) {
                        LED2_TOGGLE;
                        led2_count++;
                        last_led2_time = current_time;
                    }
                    break;
                    
                case MSG_TMR_LED3_EXPIRED:
                    // 只有在测试运行且定时器激活时才处理
                    if (test_running && timers_active) {
                        LED3_TOGGLE;
                        led3_count++;
                        last_led3_time = current_time;
                    }
                    break;
                    
                case MSG_TMR_LED4_EXPIRED:
                    // 只有在测试运行且定时器激活时才处理
                    if (test_running && timers_active) {
                        LED4_TOGGLE;
                        led4_count++;
                        last_led4_time = current_time;
                    }
                    break;
                    
                case MC_KEYx_SWITCH:
                    // 处理按键消息
                    switch (msgP->Head.mRecvInst) {
                        case 3: // Key3, 显示状态
                            print_timer_status();
                            // 显示暂停状态
                            uart_buf = Uart_Sendbuf_Get();
                            if (uart_buf) {
                                sprintf(uart_buf, "Test Status: %s, Timers: %s\n", 
                                        test_running ? "RUNNING" : "PAUSED",
                                        timers_active ? "ACTIVE" : "STOPPED");
                                USER_USART1_print(uart_buf);
                            }
                            break;
                            
                        case 4: // Key4, 暂停/继续测试
                        {
                            test_running ^= 1;
                            
                            if (test_running) {
                                // 恢复定时器
                                OSTmrStart(tmr_led1, &err);
                                OSTmrStart(tmr_led2, &err);
                                OSTmrStart(tmr_led3, &err);
                                OSTmrStart(tmr_led4, &err);
                                timers_active = 1;
                                
                                uart_buf = Uart_Sendbuf_Get();
                                if (uart_buf) {
                                    sprintf(uart_buf, "Test Resumed - Timers restarted, LEDs enabled\n");
                                    USER_USART1_print(uart_buf);
                                }
                            } else {
                                // 暂停定时器
                                OSTmrStop(tmr_led1, OS_TMR_OPT_NONE, NULL, &err);
                                OSTmrStop(tmr_led2, OS_TMR_OPT_NONE, NULL, &err);
                                OSTmrStop(tmr_led3, OS_TMR_OPT_NONE, NULL, &err);
                                OSTmrStop(tmr_led4, OS_TMR_OPT_NONE, NULL, &err);
                                timers_active = 0;
                                
                                // 关闭所有LED
                                LED1_OFF;
                                LED2_OFF;
                                LED3_OFF;
                                LED4_OFF;
                                
                                uart_buf = Uart_Sendbuf_Get();
                                if (uart_buf) {
                                    sprintf(uart_buf, "Test Paused - Timers stopped, LEDs disabled\n");
                                    USER_USART1_print(uart_buf);
                                }
                            }
                            break;
                        }
                    }
                    break;
                    
                default:
                    uart_buf = Uart_Sendbuf_Get();
                    if (uart_buf) {
                        sprintf(uart_buf, "Unknown message: %d\n", msgP->Head.mCode);
                        USER_USART1_print(uart_buf);
                    }
                    break;
            }
            
            // 每20次切换打印一次状态（只在运行时）
            if (test_running && timers_active && 
                (led1_count + led2_count + led3_count + led4_count) % 20 == 0) {
                print_timer_status();
            }
            
            // 释放消息内存
            Msg_MemPut(&msgP->Head);
        }
    }
}
 
//============================================================================
// 打印测试头信息
static void print_test_header(void)
{
    USER_USART1_print("\n");
    USER_USART1_print("==========================================\n");
    USER_USART1_print("=== Message-Driven Timer Test Started ===\n");
    USER_USART1_print("==========================================\n");
    USER_USART1_print("LED1: 500ms period (Message Code: 201)\n");
    USER_USART1_print("LED2: 1000ms period (Message Code: 202)\n");
    USER_USART1_print("LED3: 1500ms period (Message Code: 203)\n");
    USER_USART1_print("LED4: 2000ms period (Message Code: 204)\n");
    USER_USART1_print("==========================================\n");
    USER_USART1_print("KEY3: Show timer status\n");
    USER_USART1_print("KEY4: Pause/Resume test\n");
    USER_USART1_print("==========================================\n");
}
 
//============================================================================
// 打印定时器状态
static void print_timer_status(void)
{
    char *uart_buf;
    INT32U current_time = OSTimeGet();
    
    USER_USART1_print("\n");
    USER_USART1_print("==========================================\n");
    USER_USART1_print("=== Timer Status Report ===\n");
    
    uart_buf = Uart_Sendbuf_Get();
    if (uart_buf) {
        sprintf(uart_buf, "System Time: %lu ticks\n", current_time);
        USER_USART1_print(uart_buf);
    }
    
    uart_buf = Uart_Sendbuf_Get();
    if (uart_buf) {
        sprintf(uart_buf, "LED1: %lu toggles, Last: %lu\n", led1_count, last_led1_time);
        USER_USART1_print(uart_buf);
    }
    
    uart_buf = Uart_Sendbuf_Get();
    if (uart_buf) {
        sprintf(uart_buf, "LED2: %lu toggles, Last: %lu\n", led2_count, last_led2_time);
        USER_USART1_print(uart_buf);
    }
    
    uart_buf = Uart_Sendbuf_Get();
    if (uart_buf) {
        sprintf(uart_buf, "LED3: %lu toggles, Last: %lu\n", led3_count, last_led3_time);
        USER_USART1_print(uart_buf);
    }
    
    uart_buf = Uart_Sendbuf_Get();
    if (uart_buf) {
        sprintf(uart_buf, "LED4: %lu toggles, Last: %lu\n", led4_count, last_led4_time);
        USER_USART1_print(uart_buf);
    }
    
    // 显示测试状态
    uart_buf = Uart_Sendbuf_Get();
    if (uart_buf) {
        sprintf(uart_buf, "Test Status: %s, Timers: %s\n", 
                test_running ? "RUNNING" : "PAUSED",
                timers_active ? "ACTIVE" : "STOPPED");
        USER_USART1_print(uart_buf);
    }
    
    USER_USART1_print("==========================================\n");
}
 
//============================================================================
// 外部调用：显示定时器状态
void Timer_Msg_Test_Show_Status(void)
{
    print_timer_status();
}
