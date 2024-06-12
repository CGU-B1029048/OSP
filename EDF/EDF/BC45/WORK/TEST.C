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

#include "includes.h"
#include "stdio.h"
#include <stdlib.h>
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


/*brady test******************/
#define    PERIODIC_TASK_START_ID     20            //定義優先權20後為自創任務
#define    PERIODIC_TASK_START_PRIO     20			//定義優先權20後為自創任務

typedef struct{                                     //建立每個任務的相關時間
	INT32U  RemainTime;
	INT32U  ExecutionTime;
	INT32U  Period;
	INT32U  Deadline;  
	INT8U   TaskID; 
	INT8U   TaskNum; 
}TASK_EXTRA_DATA;

/**************************************/
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


/*brady test******************/
OS_STK TaskStk[8][TASK_STK_SIZE];             //建立stack
TASK_EXTRA_DATA TaskExtraData[8];			
INT8U  NumberOfTasks;
INT8U  ExecutionTime[8];
INT8U  PeriodTime[8];
INT32U MyStartTime;
INT8U temp;
INT8U *per;
INT8U *pertemp;
INT8U *extemp;
INT8U *sort;
char   wordtemp[80];
INT32U   count1=0;
/**************************************/



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
        void  Task1(void *data);
        void  Task2(void *data);
        void  Task3(void *data);
        void  Task4(void *data);
        void  Task5(void *data);
		
/*brady test******************/
		void  PeriodicTask(void *data);             //預先宣告之所創立任務
/**************************************/

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
	
	/*brady test******************/
		FILE *InputFile;                               //讀取檔案並放入相對位置
		INT8U i,j;

		InputFile = fopen("Input1.txt","r");
		fscanf(InputFile, " %d " ,&NumberOfTasks);
		per=(int*)malloc(sizeof(int)*NumberOfTasks);
		pertemp=(int*)malloc(sizeof(int)*NumberOfTasks);
		extemp=(int*)malloc(sizeof(int)*NumberOfTasks);
		sort=(int*)malloc(sizeof(int)*NumberOfTasks);
		for(i=0;i<NumberOfTasks;i++)
		{
			fscanf(InputFile,"%d %d",&ExecutionTime[i],&PeriodTime[i]);
			TaskExtraData[i].ExecutionTime=ExecutionTime[i]*OS_TICKS_PER_SEC;
			TaskExtraData[i].Period=PeriodTime[i]*OS_TICKS_PER_SEC;
			TaskExtraData[i].Deadline=PeriodTime[i]*OS_TICKS_PER_SEC;
			TaskExtraData[i].RemainTime=ExecutionTime[i]*OS_TICKS_PER_SEC;
			TaskExtraData[i].TaskID=i+1;
			TaskExtraData[i].TaskNum=NumberOfTasks;
			pertemp[i]=PeriodTime[i];
			extemp[i]=ExecutionTime[i];
			sort[i]=i;
			
			
			
		}
		for (i = 0; i < NumberOfTasks - 1; i++)
		{
			for (j = 0; j < NumberOfTasks - 1 - i; j++)
			{
				if (PeriodTime[j] > PeriodTime[j + 1]) 
				{
					temp = PeriodTime[j];
					PeriodTime[j] = PeriodTime[j + 1];
					PeriodTime[j + 1] = temp;
					temp = sort[j];
					sort[j] = sort[j + 1];
					sort[j + 1] = temp;
				}
			}
			
		}
		
		for(i=0;i<NumberOfTasks;i++)            //排序各個任務的優先權
		{
			for(j=0;j<NumberOfTasks;j++)
			{
				if(PeriodTime[i]==pertemp[j])
				{
					per[i]=j;
					break;
				}
			}
		}
		
		
		
		
/**************************************/   


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

void  TaskStart (void *pdata)
{
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
#endif
    INT16S     key;


    pdata = pdata;                                         /* Prevent compiler warning                 */

    TaskStartDispInit();                                   /* Setup the display                        */
	/************brady*******************/
	//sprintf(wordtemp, "%d    %d    %d    %d    %d                                                   ",pertemp[0],pertemp[1],pertemp[2],pertemp[3],pertemp[4]);
	//PC_DispStr( 0, 21,wordtemp, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	//sprintf(wordtemp, "%d    %d    %d    %d    %d                                                   ",per[0],per[1],per[2],per[3],per[4]);
	//PC_DispStr( 0, 20,wordtemp, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	/*************************************/
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
    PC_DispStr( 0,  0, "                         uC/OS-II, The Real-Time Kernel                         ", DISP_FGND_WHITE + DISP_BGND_RED + DISP_BLINK);
    PC_DispStr( 0,  1, "                      B0429012/B0429015/B0429019/B0429039                       ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  2, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  3, "                                 final project     RM                           ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  4, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  5, "  Task       Period    execution   start       end     deadline     times       ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  6, "----------  --------  ----------  -------    -------   --------     -----       ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  7, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    //PC_DispStr( 0, 12, "TaskStart():                                                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    //PC_DispStr( 0, 13, "TaskClk()  :                                                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    
	/*
	PC_DispStr( 0, 14, "Task1()    :                                                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 15, "Task2()    :                                                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 16, "Task3()    :                                                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 17, "Task4()    :                                                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 18, "Task5()    :                                                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 19, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 20, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 21, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	
	*/
	



	for(temp=0;temp<NumberOfTasks;temp++)
	{
		if(pertemp[temp]>=10)
			sprintf(wordtemp, "Task%d()    :   %d          %d                                                       ",temp+1,pertemp[temp],extemp[temp]);
		else
			sprintf(wordtemp, "Task%d()    :    %d          %d                                                      ",temp+1,pertemp[temp],extemp[temp]);
		PC_DispStr( 0, 8+temp,wordtemp, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	}
	for(temp=temp+8;temp<22;temp++)
	{
		PC_DispStr( 0,  temp, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	}
    PC_DispStr( 0, 22, "#Tasks          :        CPU Usage:     %                                       ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 23, "#Task switch/sec:                                                               ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 24, "                            <-PRESS 'ESC' TO QUIT->                             ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY + DISP_BLINK);
/*                                1111111111222222222233333333334444444444555555555566666666667777777777 */
/*                      01234567890123456789012345678901234567890123456789012345678901234567890123456789 */
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

static  void  TaskStartCreateTasks (void)
{
    /*OSTaskCreateExt(TaskClk,
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
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);*/
				   
	/***brady*******************/
	INT8U i;
	MyStartTime=OSTimeGet();
	                                     //建立時鐘任務
	OSTaskCreateExt(TaskClk,
		(void *)0,
		&TaskClkStk[TASK_STK_SIZE-1],
		TASK_CLK_PRIO,
		TASK_CLK_ID,
		&TaskClkStk[0],
		TASK_STK_SIZE,
		(void *)0,
		OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);
		
		
	for(i=0;i<NumberOfTasks;i++)             //依照讀檔結果創立任務 並賦予其優先權
	{
	 OSTaskCreateExt(PeriodicTask,
		(void *)0,
		&TaskStk[i][TASK_STK_SIZE-1],
		(PERIODIC_TASK_START_PRIO+per[i]),
		(PERIODIC_TASK_START_ID+i),
		&TaskStk[i][0],
		TASK_STK_SIZE,
		&TaskExtraData[i],
		OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);
		
	}
	
	/*************************/
	
	
	
}
/*****brady*******************/
void PeriodicTask(void *pdata)           //自行創建的任務
{
	
	INT8U x;
	TASK_EXTRA_DATA *MyPtr;
	char s[34];
	char p[34];
	INT8U i;
	INT32U TaskTime;
	
	
	pdata=pdata;
	MyPtr=OSTCBCur->OSTCBExtPtr;
	x=0;
	
	MyPtr->Deadline=MyStartTime+MyPtr->Period;
	MyPtr->RemainTime=MyPtr->ExecutionTime;
	sprintf(s, "%d->",MyPtr->TaskID);
	PC_DispStr( count1,  15, s, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	count1=count1+3;
	sprintf(s, "switch at %10d",OSTimeGet());
	PC_DispStr( 0,  20, s, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	
	for(;;)
	{
		x++;
		sprintf(s,"%4d",x);
		PC_DispStr(67,8+sort[OSPrioCur-PERIODIC_TASK_START_PRIO],s,DISP_FGND_CYAN + DISP_BGND_LIGHT_GRAY);
		sprintf(s,"%10d",OSTimeGet());
		PC_DispStr(29,8+sort[OSPrioCur-PERIODIC_TASK_START_PRIO],s,DISP_FGND_LIGHT_BLUE + DISP_BGND_LIGHT_GRAY);
		sprintf(s,"%10d",MyPtr->Deadline);
		PC_DispStr(50,8+sort[OSPrioCur-PERIODIC_TASK_START_PRIO],s,DISP_FGND_LIGHT_PURPLE + DISP_BGND_LIGHT_GRAY);
		while(1)
		{
			if(MyPtr->RemainTime<=0)
			{
				break;
			}
		}
		MyPtr->Deadline=MyPtr->Deadline+MyPtr->Period;
		MyPtr->RemainTime=MyPtr->ExecutionTime;
		sprintf(s,"%10d",OSTimeGet());
		PC_DispStr(40,8+sort[OSPrioCur-PERIODIC_TASK_START_PRIO],s,DISP_FGND_LIGHT_RED + DISP_BGND_LIGHT_GRAY);
		
		
		if(MyPtr->Deadline-MyPtr->Period>OSTimeGet())
		{
			OSTimeDly(MyPtr->Deadline-MyPtr->Period-OSTimeGet());
		}
		
		
		
		
	}
	
	
	
}
/*****************************/

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

void  Task1 (void *pdata)
{
    INT8U       err;
    OS_STK_DATA data;                       /* Storage for task stack data                             */
    INT16U      time;                       /* Execution time (in uS)                                  */
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
        OSTimeDlyHMSM(0, 0, 0, 100);                       /* Delay for 100 mS                         */
    }
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                               TASK #2
*
* Description: This task displays a clockwise rotating wheel on the screen.
*********************************************************************************************************
*/

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

void  Task3 (void *data)
{
    char    dummy[500];
    INT16U  i;


    data = data;
    for (i = 0; i < 499; i++) {        /* Use up the stack with 'junk'                                 */
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

void  Task4 (void *data)
{
    char   txmsg;
    INT8U  err;


    data  = data;
    txmsg = 'A';
    for (;;) {
        OSMboxPost(TxMbox, (void *)&txmsg);      /* Send message to Task #5                            */
        OSMboxPend(AckMbox, 0, &err);            /* Wait for acknowledgement from Task #5              */
        txmsg++;                                 /* Next message to send                               */
        if (txmsg == 'Z') {
            txmsg = 'A';                         /* Start new series of messages                       */
        }
    }
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                               TASK #5
*
* Description: This task displays messages sent by Task #4.  When the message is displayed, Task #5
*              acknowledges Task #4.
*********************************************************************************************************
*/

void  Task5 (void *data)
{
    char  *rxmsg;
    INT8U  err;


    data = data;
    for (;;) {
        rxmsg = (char *)OSMboxPend(TxMbox, 0, &err);                  /* Wait for message from Task #4 */
        PC_DispChar(70, 18, *rxmsg, DISP_FGND_YELLOW + DISP_BGND_BLUE);
        OSTimeDlyHMSM(0, 0, 1, 0);                                    /* Wait 1 second                 */
        OSMboxPost(AckMbox, (void *)1);                               /* Acknowledge reception of msg  */
    }
}
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

