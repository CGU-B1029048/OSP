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


/*Che-Wei: The program needs to read some input file and thus has to include stdio.h *******************/
#include <stdio.h>
/*******************************************************************************************************/


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

/* Che-Wei:  *******************************************************************************************/
#define          PERIODIC_TASK_START_ID      20   
#define          PERIODIC_TASK_START_PRIO    20                                 


/*Che-Wei: It has to create a new data type*************************************************************/
typedef struct {
    INT32U RemainTime;
    INT32U ExecutionTime;
    INT32U Period;
    INT32U Deadline;
    INT8U TaskID;
} TASK_EXTRA_DATA;
/*******************************************************************************************************/


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

/*Che-Wei: There are some variables which are going to be used *****************************************/
OS_STK TaskStk[8][TASK_STK_SIZE];
TASK_EXTRA_DATA  TaskExtraData[8]; 
INT8U NumberOfTasks; 
INT8U ExecutionTime[8]; 
INT8U PeriodTime[8]; 
// INT8U *exe;
// INT8U *period;
INT8U *TaskList;
INT8U *TempPeriodTime;
// INT8U *priority;
INT32U MyStartTime;
INT32U task_display_counter = 2;
/*******************************************************************************************************/


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

/*Che-Wei: PeriodicTask is the function for periodic tasks *********************************************/
	void  PeriodicTask(void *data); 
	void  quickSort (INT8U *PeriodTime , INT8U *TaskList,const INT8U left, const INT8U right);
	void  swap(INT8U* a,INT8U *b);
    void selectionSort(INT8U *PeriodTime , INT8U *TaskList, INT8U size);
/*******************************************************************************************************/

/*$PAGE*/
/*
*********************************************************************************************************
*                                                  MAIN
*********************************************************************************************************
*/
//bool checkSort(INT8U *PeriodTime,INT8U size)
//{
//	INT8U i
//	for(i=1;i<size;i++)
//	{
//		if(PeriodTime[i-1]>PeriodTime[i])
//		{
//			return true;
//		}
//	}
//	return false;
//}


void main (void){
    OS_STK *ptos;
    OS_STK *pbos;
    INT32U  size;
	INT8U testt;
    INT8U temp;

/* Che-Wei: Read the input file ************************************************************************/
    FILE *InputFile; 
    INT8U i;
    INT8U j;
    InputFile = fopen("Input1.txt","r");
    fscanf(InputFile, "%d", &NumberOfTasks);
    // period = (int*)malloc(sizeof(int)*NumberOfTasks);
    // exe = (int*)malloc(sizeof(int)*NumberOfTasks);
    TaskList = (int*)malloc(sizeof(int)*NumberOfTasks);
	// priority = (int*)malloc(sizeof(int)*NumberOfTasks);
    for(i=0; i< NumberOfTasks; i++)
    {
        fscanf(InputFile,"%d %d", &ExecutionTime[i], &PeriodTime[i]); 

        TaskExtraData[i].ExecutionTime = ExecutionTime[i] * OS_TICKS_PER_SEC;
        TaskExtraData[i].Period = PeriodTime[i] * OS_TICKS_PER_SEC; 
        TaskExtraData[i].Deadline = PeriodTime[i] * OS_TICKS_PER_SEC; 
        TaskExtraData[i].RemainTime = ExecutionTime[i] * OS_TICKS_PER_SEC;
        TaskExtraData[i].TaskID=i+1;
        TaskList[i] = i; 
        // exe[i]=ExecutionTime[i];
        // period[i]=PeriodTime[i];
    }
    fclose(InputFile);

// toby : order task

//     for (i = 0; i < NumberOfTasks - 1; i++){
//         for (j = 0; j < NumberOfTasks - 1 - i; j++){
//             if (PeriodTime[j] > PeriodTime[j + 1]){
//                 temp = PeriodTime[j];
//                 PeriodTime[j] = PeriodTime[j + 1];
//                 PeriodTime[j + 1] = temp;
//                 temp = TaskList[j];
//                 TaskList[j] = TaskList[j + 1];
//                 TaskList[j + 1] = temp;
//             }
//         }
//	 }
	
	  selectionSort(PeriodTime,TaskList,NumberOfTasks);


/*******************************************************************************************************/


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
void swap(INT8U* a,INT8U *b){
    INT8U temp = *a;
    *a = *b;
    *b = temp;
}
// sam: quick sort 
void quickSort (INT8U *PeriodTime , INT8U *TaskList,const INT8U left, const INT8U right){
    if (left<right ){
        INT8U i=left,j=right+1,pivot=PeriodTime[left];
        INT8U temp;
        do{
            do i++; while( PeriodTime[i]<pivot ); 
            do j--; while( PeriodTime[j]>pivot ); 
            // swap( ptr[i],ptr[j] )
             if ( i<j ){
                swap(&PeriodTime[i],&PeriodTime[j]);
                swap(&TaskList[i],&TaskList[j]);
             }
        }while( i<j );
        swap(&PeriodTime[left],&PeriodTime[j]);
        swap(&TaskList[left],&TaskList[j]);
        temp=1;
        quickSort(PeriodTime,TaskList,left,j-1 );
        quickSort(PeriodTime,TaskList,j+1,right );
    }
}
void selectionSort(INT8U *PeriodTime , INT8U *TaskList, INT8U size){
    INT8U i, j, min_idx, temp;
    for (i=0; i<size-1; i++) {
        min_idx = i;
        for (j=i+1; j<size; j++) {
            if (PeriodTime[j] < PeriodTime[min_idx]) {
                min_idx = j;
            }
        }
        // swap
        // temp = PeriodTime[min_idx];
        // PeriodTime[min_idx] = PeriodTime[i];
        // PeriodTime[i] = temp;
        swap(&PeriodTime[min_idx],&PeriodTime[i]);
        swap(&TaskList[min_idx],&TaskList[i]);
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

    char s[80]; 
    INT8U i;
    INT8U j;

    PC_DispStr( 0,  0, "                            Final Project on uC/OS-II                           ", DISP_FGND_WHITE + DISP_BGND_GREEN + DISP_BLINK);
    PC_DispStr( 0,  1, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  2, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  3, "                                    B10xx0xx                                    ", DISP_FGND_BLUE  + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  4, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  5, "                              EDF Scheduling Results                            ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  6, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  7, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  8, "Task           Start Time  End Time    Deadline    Period      Excution    Run  ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  9, "-------------  ----------  ----------  ----------  ----------  ----------  ---- ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    for(i=0; i< NumberOfTasks; i++)
    {
        // sprintf(s,"Task%1d()    :                                       %2d          %2d               ", i+1, period[i], exe[i]);
        INT8U p_time = TaskExtraData[i].Period/OS_TICKS_PER_SEC;
        INT8U e_time = TaskExtraData[i].ExecutionTime/OS_TICKS_PER_SEC;
        sprintf(s,"Task%1d()    :                                       %2d          %2d               ", TaskExtraData[i].TaskID,p_time , e_time);
        PC_DispStr(0, 10+i, s, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    }
    for(j=(NumberOfTasks+10); j<22; j++)
    {
        PC_DispStr( 0, j, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    }

    PC_DispStr( 0, 22, "#Tasks          :        CPU Usage:     %                                       ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 23, "#Task switch/sec:                                                               ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 24, "                            <-PRESS 'ESC' TO QUIT->                             ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY + DISP_BLINK);

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
    INT8U i;
    char s [80];
    INT8U taskID;
    MyStartTime = OSTimeGet(); 

    OSTaskCreateExt(TaskClk,
                   (void *)0,
                   &TaskClkStk[TASK_STK_SIZE - 1],
                   TASK_CLK_PRIO,
                   TASK_CLK_ID,
                   &TaskClkStk[0],
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);


    for(i=0; i< NumberOfTasks; i++)
    {
        // OSTaskCreateExt(PeriodicTask,
        //                (void *)0,
        //                &TaskStk[i][TASK_STK_SIZE-1],
        //                (PERIODIC_TASK_START_PRIO + priority[i]),
        //                (PERIODIC_TASK_START_ID + i),
        //                &TaskStk[i][0],
        //                TASK_STK_SIZE,
        //                &TaskExtraData[i],
        //                OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
        taskID = TaskList[i];
        OSTaskCreateExt(PeriodicTask,
                       (void *)0,
                       &TaskStk[taskID][TASK_STK_SIZE-1],
                       (PERIODIC_TASK_START_PRIO + i),
                       (PERIODIC_TASK_START_ID + taskID),
                       &TaskStk[taskID][0],
                       TASK_STK_SIZE,
                       &TaskExtraData[taskID],
                       OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
/*
        sprintf(s,"           Task%1d is created with piority %1d                                   ", i+1 , PERIODIC_TASK_START_PRIO + i);
	PC_DispStr(0, 8+i, s, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
*/
    }
}
/*******************************************************************************************************/

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


/*$PAGE*/
/*
*********************************************************************************************************
*                                        Periodic TASK   
*********************************************************************************************************
* Che-Wei **********************************************************************************************/

void  PeriodicTask (void *pdata)
{
    INT8U  x;
    TASK_EXTRA_DATA *MyPtr; 
    char s[34];
    char p[34];
    INT8U i;
    INT32U TaskTime;
    
    pdata=pdata;
    MyPtr = OSTCBCur->OSTCBExtPtr;
    x =0;

    MyPtr->Deadline = MyStartTime + MyPtr->Period;    
    MyPtr->RemainTime = MyPtr->ExecutionTime;

    for (;;){
//         toby, display running task to screen
        sprintf(s, "%d->",MyPtr->TaskID);
        PC_DispStr( task_display_counter,  18, s, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
        task_display_counter = task_display_counter+3;
        //sprintf(s, "switch at %10d",OSTimeGet()/OS_TICKS_PER_SEC-1);
        //PC_DispStr( 0,  20, s, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);

	    x++;
	    sprintf(s, "%4d",x); 
        PC_DispStr(75, 10 + TaskList[OSPrioCur - PERIODIC_TASK_START_PRIO], s, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	    sprintf(s, "%10d ",OSTimeGet()/OS_TICKS_PER_SEC-1); 
        PC_DispStr(15, 10 + TaskList[OSPrioCur - PERIODIC_TASK_START_PRIO], s, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY); 
	    sprintf(s, "%10d ", MyPtr->Deadline/OS_TICKS_PER_SEC-1); 
        PC_DispStr(39, 10 + TaskList[OSPrioCur - PERIODIC_TASK_START_PRIO], s, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
        while(1){
            if(MyPtr->RemainTime <=0) {break;}
        }
          
        MyPtr->Deadline = MyPtr->Deadline + MyPtr->Period;
        MyPtr->RemainTime = MyPtr->ExecutionTime;

	    sprintf(s, "%10d ",OSTimeGet()/OS_TICKS_PER_SEC-1); 
        PC_DispStr(27, 10 + TaskList[OSPrioCur - PERIODIC_TASK_START_PRIO], s, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);

        if(MyPtr->Deadline - MyPtr->Period > OSTimeGet()){
            OSTimeDly(MyPtr->Deadline - MyPtr->Period - OSTimeGet());
        }
    }
}

/*******************************************************************************************************/

