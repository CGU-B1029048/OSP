# OSP 2024 Final Project

## Requirements
### Task Scheduling
- Adopt priority-driven scheduling
- The scheduler always schedules the highest priority ready task to run
- Modify the priority of each task
- Related code in uC/OS II
  - See `OS_Sched()` for scheduling policy
  - See `OSTimeTick()` for time management
  - See `OSIntExit()` for the interrupt management
### Provide the RM and EDF Scheduler
+ Input: A task set, each task is with its execution time and period
+ Output: The printed result of each task

## Input
The input format should be as follows
+ Your program should have the capability to create the assigned number of tasks and their corresponding period and execution time.
+ Example: `taskset.txt`
```txt
3 //number of task
1 3 // task 1: (execution time 1, period 1)
2 9 // task 2: (execution time 2, period 2)
4 12 // task 3: (execution time 3, period 3)
```
+ The total utilization is no more than $65$%
+ The number of tasks is no more than $7$

## Output
Your program output must shows the following information
+ A sequence of the running task over time
+ The time when context switch occurred

A report to describe your implementation
+ Relationship of each function
+ Implementation flow chart
+ Implementation details
