/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                          (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
*                                               EXAMPLE #2
*********************************************************************************************************
*/
#include<stdio.h>
#include "includes.h"

/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/

#define          TASK_STK_SIZE     512                /* Size of each task's stacks (# of WORDs)       */

#define          TASK_START_ID       0                /* Application tasks IDs                         */
#define          TASK_CLK_ID         1
#define          TASK_1_ID           2
#define          TASK_2_ID           3
#define          TASK_3_ID           4
#define          TASK_4_ID           5
#define          TASK_5_ID           6

#define          TASK_START_PRIO    10                /* Application tasks priorities                  */
#define          TASK_CLK_PRIO      11
#define          TASK_1_PRIO        12
#define          TASK_2_PRIO        13
#define          TASK_3_PRIO        14
#define          TASK_4_PRIO        15
#define          TASK_5_PRIO        16


#define		PERIODIC_TASK_START_ID	20
#define		PERIODIC_TASK_START_PRIO 20

typedef struct {
	INT32U RemainTime;
	INT32U ExecutionTime;
	INT32U Period;
	INT32U Deadline;
    INT8U TaskID;
    /*INT8U *OrderOfTask;*/
}TASK_EXTRA_DATA;

/*
*********************************************************************************************************
*                                              VARIABLES
*********************************************************************************************************
*/

OS_STK        TaskStartStk[TASK_STK_SIZE];            /* Startup    task stack                         */
OS_STK        TaskClkStk[TASK_STK_SIZE];              /* Clock      task stack                         */
OS_STK        Task1Stk[TASK_STK_SIZE];                /* Task #1    task stack                         */
OS_STK        Task2Stk[TASK_STK_SIZE];                /* Task #2    task stack                         */
OS_STK        Task3Stk[TASK_STK_SIZE];                /* Task #3    task stack                         */
OS_STK        Task4Stk[TASK_STK_SIZE];                /* Task #4    task stack                         */
OS_STK        Task5Stk[TASK_STK_SIZE];                /* Task #5    task stack                         */

OS_EVENT     *AckMbox;                                /* Message mailboxes for Tasks #4 and #5         */
OS_EVENT     *TxMbox;

/*----------------------------------------*/
OS_STK TaskStk[8][TASK_STK_SIZE];
TASK_EXTRA_DATA TaskExtraData[8];
INT8U NumberOfTasks;
INT8U ExecutionTime[8];
INT8U PeriodTime[8];
INT32U MyStartTime;

/*INT8U TaskID;*/
INT8U *TaskList;
INT8U *TempPeriodTime;
INT32U task_display_counter = 0;
/*-----------------------------------------*/
/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

        void  TaskStart(void *data);                  /* Function prototypes of tasks                  */
static  void  TaskStartCreateTasks(void);
static  void  TaskStartDispInit(void);
static  void  TaskStartDisp(void);
        void  TaskClk(void *data);
        /*void  Task1(void *data);
        void  Task2(void *data);
        void  Task3(void *data);
        void  Task4(void *data);
        void  Task5(void *data);*/

	void  PeriodicTask(void*data);

    void bubbleSort(INT8U size);
    void  swap(TASK_EXTRA_DATA* a,TASK_EXTRA_DATA *b);

/*$PAGE*/
/*
*********************************************************************************************************
*                                                  MAIN
*********************************************************************************************************
*/

void main (void)
{
    OS_STK *ptos;
    OS_STK *pbos;
    INT32U  size;
    INT8U testt;
    INT8U temp;

	FILE *InputFile;
	INT8U i;
    INT8U j;
    InputFile = fopen("Input.txt","r");
	fscanf(InputFile,"%d", &NumberOfTasks);
    TaskList = (int *)malloc(sizeof(int) * NumberOfTasks);

    for(i=0; i<NumberOfTasks;i++)
	{
		fscanf(InputFile,"%d%d",&ExecutionTime[i],&PeriodTime[i]);
			TaskExtraData[i].ExecutionTime = ExecutionTime[i] * OS_TICKS_PER_SEC;
			TaskExtraData[i].Period = PeriodTime[i]* OS_TICKS_PER_SEC;
			TaskExtraData[i].Deadline = PeriodTime[i] * OS_TICKS_PER_SEC;
			TaskExtraData[i].RemainTime = ExecutionTime[i] * OS_TICKS_PER_SEC;
            TaskExtraData[i].TaskID = i + 1;
            TaskList[i] = i;
    }
	fclose(InputFile);

    bubbleSort( NumberOfTasks);

    PC_DispClrScr(DISP_FGND_WHITE);                        /* Clear the screen                         */

    OSInit();                                              /* Initialize uC/OS-II                      */

    PC_DOSSaveReturn();                                    /* Save environment to return to DOS        */
    PC_VectSet(uCOS, OSCtxSw);                             /* Install uC/OS-II's context switch vector */

    PC_ElapsedInit();                                      /* Initialized elapsed time measurement     */

    ptos        = &TaskStartStk[TASK_STK_SIZE - 1];        /* TaskStart() will use Floating-Point      */
    pbos        = &TaskStartStk[0];
    size        = TASK_STK_SIZE;
    OSTaskStkInit_FPE_x86(&ptos, &pbos, &size);            
    OSTaskCreateExt(TaskStart,
                   (void *)0,
                   ptos,
                   TASK_START_PRIO,
                   TASK_START_ID,
                   pbos,
                   size,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSStart();                                             /* Start multitasking                       */
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                               STARTUP TASK
*********************************************************************************************************
*/

void swap(TASK_EXTRA_DATA *a,TASK_EXTRA_DATA *b){
    TASK_EXTRA_DATA temp = *a;
    *a = *b;
    *b = temp;
}

void bubbleSort(INT8U size){
    INT8U i, j,swapped;
    for (i = 0; i < size - 1; i++) {
        swapped = 0;
        for (j = 0; j < size - i - 1; j++) {
            if (TaskExtraData[j].Period > TaskExtraData[j + 1].Period) {
                swap(&TaskExtraData[j], &TaskExtraData[j + 1]);
                swapped = 1;
            }
        }

        // If no two elements were swapped
        // by inner loop, then break
        if (swapped == 0)
            break;
    }
    
}


void  TaskStart (void *pdata)
{
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
#endif
    INT16S     key;


    pdata = pdata;                                         /* Prevent compiler warning                 */

    TaskStartDispInit();                                   /* Setup the display                        */

    OS_ENTER_CRITICAL();                                   /* Install uC/OS-II's clock tick ISR        */
    PC_VectSet(0x08, OSTickISR);
    PC_SetTickRate(OS_TICKS_PER_SEC);                      /* Reprogram tick rate                      */
    OS_EXIT_CRITICAL();

    OSStatInit();                                          /* Initialize uC/OS-II's statistics         */

    AckMbox = OSMboxCreate((void *)0);                     /* Create 2 message mailboxes               */
    TxMbox  = OSMboxCreate((void *)0);

    TaskStartCreateTasks();                                /* Create all other tasks                   */

    for (;;) {
        TaskStartDisp();                                   /* Update the display                       */

        if (PC_GetKey(&key)) {                             /* See if key has been pressed              */
            if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                PC_DOSReturn();                            /* Yes, return to DOS                       */
            }
        }

        OSCtxSwCtr = 0;                                    /* Clear context switch counter             */
        OSTimeDly(OS_TICKS_PER_SEC);                       /* Wait one second                          */
    }
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                        INITIALIZE THE DISPLAY
*********************************************************************************************************
*/

static  void  TaskStartDispInit (void)
{
/*                                1111111111222222222233333333334444444444555555555566666666667777777777 */
/*                      01234567890123456789012345678901234567890123456789012345678901234567890123456789 */
/*
    PC_DispStr( 0,  0, "                         uC/OS-II, The Real-Time Kernel                         ", DISP_FGND_WHITE + DISP_BGND_RED + DISP_BLINK);
    PC_DispStr( 0,  1, "                                Jean J. Labrosse                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  2, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  3, "                                    EXAMPLE #2                                  ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  4, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  5, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  6, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  7, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  8, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  9, "Task           Total Stack  Free Stack  Used Stack  ExecTime (uS)               ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 10, "-------------  -----------  ----------  ----------  -------------               ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 11, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 12, "TaskStart():                                                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 13, "TaskClk()  :                                                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 14, "Task1()    :                                                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 15, "Task2()    :                                                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 16, "Task3()    :                                                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 17, "Task4()    :                                                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 18, "Task5()    :                                                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 19, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 20, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 21, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 22, "#Tasks          :        CPU Usage:     %                                       ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 23, "#Task switch/sec:                                                               ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 24, "                            <-PRESS 'ESC' TO QUIT->                             ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY + DISP_BLINK);
*/
/*                                1111111111222222222233333333334444444444555555555566666666667777777777 */
/*                      01234567890123456789012345678901234567890123456789012345678901234567890123456789 */

    char s[80];
    INT8U i;
    INT8U j;

    PC_DispStr( 0,  0, "                         Final Project on uC/OS-II                              ", DISP_FGND_WHITE + DISP_BGND_RED + DISP_BLINK);
    PC_DispStr( 0,  1, "                                B1029015                                        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  2, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  3, "                             Scheduling Result                                  ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  4, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  5, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  6, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  7, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  8, "Task         Start Time   End Time     Deadline    Period    Execution      Run ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  9, "-----------  -----------  ----------  ----------  -------    ---------      --- ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    for(i=0; i< NumberOfTasks; i++)
    {
      /*sprintf(s,"Task%1d()    :                                         %2d       %2d                ",i+1,PeriodTime[i],ExecutionTime[i]);*/
      INT8U p_time = TaskExtraData[i].Period/OS_TICKS_PER_SEC;
      INT8U e_time = TaskExtraData[i].ExecutionTime/OS_TICKS_PER_SEC;
      sprintf(s,"Task%1d()    :                                       %2d          %2d               ", TaskExtraData[i].TaskID,p_time , e_time);
      PC_DispStr(0, 10+i, s,DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);
    }
    for(j=(NumberOfTasks+10);j<22;j++)
    {
      PC_DispStr( 0, j,"                                                                                 ",DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);
    }

    PC_DispStr( 0, 22,"#Tasks         :           CPU Usage:     %                                   ",DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 23,"#Task switch/sec:                                                            ",DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 24,"                              <-PRESS 'ESC' TO QUIT->                        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY + DISP_BLINK);
	
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                           UPDATE THE DISPLAY
*********************************************************************************************************
*/

static  void  TaskStartDisp (void)
{
    char   s[80];


    sprintf(s, "%5d", OSTaskCtr);                                  /* Display #tasks running               */
    PC_DispStr(18, 22, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    sprintf(s, "%3d", OSCPUUsage);                                 /* Display CPU usage in %               */
    PC_DispStr(36, 22, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    sprintf(s, "%5d", OSCtxSwCtr);                                 /* Display #context switches per second */
    PC_DispStr(18, 23, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    sprintf(s, "V%4.2f", (float)OSVersion() * 0.01);               /* Display uC/OS-II's version number    */
    PC_DispStr(75, 24, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    switch (_8087) {                                               /* Display whether FPU present          */
        case 0:
             PC_DispStr(71, 22, " NO  FPU ", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;

        case 1:
             PC_DispStr(71, 22, " 8087 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;

        case 2:
             PC_DispStr(71, 22, "80287 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;

        case 3:
             PC_DispStr(71, 22, "80387 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;
    }
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                             CREATE TASKS
*********************************************************************************************************
*/
/*
static  void  TaskStartCreateTasks (void)
{
    OSTaskCreateExt(TaskClk,
                   (void *)0,
                   &TaskClkStk[TASK_STK_SIZE - 1],
                   TASK_CLK_PRIO,
                   TASK_CLK_ID,
                   &TaskClkStk[0],
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSTaskCreateExt(Task1,
                   (void *)0,
                   &Task1Stk[TASK_STK_SIZE - 1],
                   TASK_1_PRIO,
                   TASK_1_ID,
                   &Task1Stk[0],
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSTaskCreateExt(Task2,
                   (void *)0,
                   &Task2Stk[TASK_STK_SIZE - 1],
                   TASK_2_PRIO,
                   TASK_2_ID,
                   &Task2Stk[0],
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSTaskCreateExt(Task3,
                   (void *)0,
                   &Task3Stk[TASK_STK_SIZE - 1],
                   TASK_3_PRIO,
                   TASK_3_ID,
                   &Task3Stk[0],
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSTaskCreateExt(Task4,
                   (void *)0,
                   &Task4Stk[TASK_STK_SIZE-1],
                   TASK_4_PRIO,
                   TASK_4_ID,
                   &Task4Stk[0],
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSTaskCreateExt(Task5,
                   (void *)0,
                   &Task5Stk[TASK_STK_SIZE-1],
                   TASK_5_PRIO,
                   TASK_5_ID,
                   &Task5Stk[0],
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
}
*/

static void TaskStartCreateTasks(void)
{
	INT8U i;
	char s[80];
    INT8U taskID;
	MyStartTime = OSTimeGet();

    OSTaskCreateExt(TaskClk,
	                (void *)0,
	                &TaskClkStk[TASK_STK_SIZE-1],
	                TASK_CLK_PRIO,
	                TASK_CLK_ID,
	                &TaskClkStk[0],
	                TASK_STK_SIZE,
	                (void*)0,
	                OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    for(i=0; i<NumberOfTasks; i++)
    {
        taskID = TaskList[i];
        OSTaskCreateExt(PeriodicTask,
                        (void *)0,
	                    &TaskStk[i][TASK_STK_SIZE-1],
	                    (PERIODIC_TASK_START_PRIO + i),
	                    (PERIODIC_TASK_START_ID + taskID),
	                    &TaskStk[taskID][0],
	                    TASK_STK_SIZE,
	                    &TaskExtraData[i],
	                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
    }
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                               TASK #1
*
* Description: This task executes every 100 mS and measures the time it task to perform stack checking
*              for each of the 5 application tasks.  Also, this task displays the statistics related to
*              each task's stack usage.
*********************************************************************************************************
*/
/*

    void  Task1 (void *pdata)
    {
        INT8U       err;
        OS_STK_DATA data;                    
        INT16U      time;                  
        INT8U       i;
        char        s[80];


        pdata = pdata;
        for (;;) {
            for (i = 0; i < 7; i++) {
                PC_ElapsedStart();
                err  = OSTaskStkChk(TASK_START_PRIO + i, &data);
                time = PC_ElapsedStop();
                if (err == OS_NO_ERR) {
                    sprintf(s, "%4ld        %4ld        %4ld        %6d",
                            data.OSFree + data.OSUsed,
                            data.OSFree,
                            data.OSUsed,
                            time);
                    PC_DispStr(19, 12 + i, s, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
                }
            }
            OSTimeDlyHMSM(0, 0, 0, 100);                      
        }
    }
*/
/*$PAGE*/
/*
*********************************************************************************************************
*                                               TASK #2
*
* Description: This task displays a clockwise rotating wheel on the screen.
*********************************************************************************************************
*/
/*
void  Task2 (void *data)
{
    data = data;
    for (;;) {
        PC_DispChar(70, 15, '|',  DISP_FGND_YELLOW + DISP_BGND_BLUE);
        OSTimeDly(10);
        PC_DispChar(70, 15, '/',  DISP_FGND_YELLOW + DISP_BGND_BLUE);
        OSTimeDly(10);
        PC_DispChar(70, 15, '-',  DISP_FGND_YELLOW + DISP_BGND_BLUE);
        OSTimeDly(10);
        PC_DispChar(70, 15, '\\', DISP_FGND_YELLOW + DISP_BGND_BLUE);
        OSTimeDly(10);
    }
}
*/
/*$PAGE*/
/*
*********************************************************************************************************
*                                               TASK #3
*
* Description: This task displays a counter-clockwise rotating wheel on the screen.
*
* Note(s)    : I allocated 500 bytes of storage on the stack to artificially 'eat' up stack space.
*********************************************************************************************************
*/
/*
void  Task3 (void *data)
{
    char    dummy[500];
    INT16U  i;


    data = data;
    for (i = 0; i < 499; i++) {        
        dummy[i] = '?';
    }
    for (;;) {
        PC_DispChar(70, 16, '|',  DISP_FGND_YELLOW + DISP_BGND_BLUE);
        OSTimeDly(20);
        PC_DispChar(70, 16, '\\', DISP_FGND_YELLOW + DISP_BGND_BLUE);
        OSTimeDly(20);
        PC_DispChar(70, 16, '-',  DISP_FGND_YELLOW + DISP_BGND_BLUE);
        OSTimeDly(20);
        PC_DispChar(70, 16, '/',  DISP_FGND_YELLOW + DISP_BGND_BLUE);
        OSTimeDly(20);
    }
}
*/
/*$PAGE*/
/*
*********************************************************************************************************
*                                               TASK #4
*
* Description: This task sends a message to Task #5.  The message consist of a character that needs to
*              be displayed by Task #5.  This task then waits for an acknowledgement from Task #5
*              indicating that the message has been displayed.
*********************************************************************************************************
*/
/*
void  Task4 (void *data)
{
    char   txmsg;
    INT8U  err;


    data  = data;
    txmsg = 'A';
    for (;;) {
        OSMboxPost(TxMbox, (void *)&txmsg);                             
        OSMboxPend(AckMbox, 0, &err);                       
        txmsg++;                                                             
        if (txmsg == 'Z') {
            txmsg = 'A';                                          
        }
    }
}
*/
/*$PAGE*/
/*
*********************************************************************************************************
*                                               TASK #5
*
* Description: This task displays messages sent by Task #4.  When the message is displayed, Task #5
*              acknowledges Task #4.
*********************************************************************************************************
*/
/*
void  Task5 (void *data)
{
    char  *rxmsg;
    INT8U  err;


    data = data;
    for (;;) {
        rxmsg = (char *)OSMboxPend(TxMbox, 0, &err);                  *//* Wait for message from Task #4 */
/*        PC_DispChar(70, 18, *rxmsg, DISP_FGND_YELLOW + DISP_BGND_BLUE);
        OSTimeDlyHMSM(0, 0, 1, 0);                                    *//* Wait 1 second                 */
        /*OSMboxPost(AckMbox, (void *)1);                              */ /* Acknowledge reception of msg  */
/*    }
}*/
/*$PAGE*/
/*
*********************************************************************************************************
*                                               CLOCK TASK
*********************************************************************************************************
*/

void  TaskClk (void *data)
{
    char s[40];


    data = data;
    for (;;) {
        PC_GetDateTime(s);
        PC_DispStr(60, 23, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);
        OSTimeDly(OS_TICKS_PER_SEC);
    }
}


void PeriodicTask(void *pdata)
{
	INT8U x;
	TASK_EXTRA_DATA *MyPtr;
	char s[34];
	char p[34];
	INT8U i;
	INT32U TaskTime;

	pdata=pdata;
	MyPtr = OSTCBCur->OSTCBExtPtr;
	x=0;

	MyPtr->Deadline = MyStartTime + MyPtr->Period;
	MyPtr->RemainTime = MyPtr->ExecutionTime;
	for (;;)
	{
        sprintf(s, "%d->",MyPtr->TaskID);
        PC_DispStr( task_display_counter,  18, s, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
        task_display_counter = task_display_counter+3;
        sprintf(s, "switch at %10d",OSTimeGet()/OS_TICKS_PER_SEC-1);
        PC_DispStr( 0,  20, s, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);

		x++;
		sprintf(s,"%4d",x);
        PC_DispStr(75, 10 + TaskList[OSPrioCur - PERIODIC_TASK_START_PRIO], s, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	    sprintf(s, "%10d ",OSTimeGet()/OS_TICKS_PER_SEC-1); 
        PC_DispStr(15, 10 + TaskList[OSPrioCur - PERIODIC_TASK_START_PRIO], s, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY); 
	    sprintf(s, "%10d ", MyPtr->Deadline/OS_TICKS_PER_SEC-1); 
        PC_DispStr(39, 10 + TaskList[OSPrioCur - PERIODIC_TASK_START_PRIO], s, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
		while(1)
		{
		  if(MyPtr->RemainTime<=0) {break;}
		}

		MyPtr->Deadline = MyPtr->Deadline + MyPtr->Period;
		MyPtr->RemainTime= MyPtr->ExecutionTime;

		sprintf(s,"%10d",OSTimeGet()/OS_TICKS_PER_SEC-1);
     	PC_DispStr(27,10 + OSPrioCur - PERIODIC_TASK_START_PRIO,s,DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);

     	if(MyPtr->Deadline - MyPtr->Period>OSTimeGet()) {OSTimeDly(MyPtr->Deadline - MyPtr->Period - OSTimeGet());}
	}

}