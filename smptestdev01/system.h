/*
*  COPYRIGHT (c) 1989-2011.
*  On-Line Applications Research Corporation (OAR).
*
*  The license and distribution terms for this file may be
*  found in the file LICENSE in this distribution or at
*  http://www.rtems.org/license/LICENSE.
*/

#include "tmacros.h"

/* 
* Variable configurations.
*/
#define SCHED_A rtems_build_name('A', ' ', ' ', ' ')
#define SCHED_B rtems_build_name('B', ' ', ' ', ' ')

/* 
* Functions declaration.
*/

void PrintTaskInfo(
const char *task_name
);

rtems_task Init(
rtems_task_argument argument
);

rtems_task Task_1(
rtems_task_argument arg
);

rtems_task Task_2(
rtems_task_argument arg
);

rtems_task Task_3(
rtems_task_argument arg
);

/******************************* configuration information ************************/
/**********************************************************************************/
#define CONFIGURE_MAXIMUM_SEMAPHORES 3
#define CONFIGURE_MAXIMUM_MRSP_SEMAPHORES 1

#define CONFIGURE_APPLICATION_NEEDS_SIMPLE_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_MICROSECONDS_PER_TICK   1000 /* 1 millisecond */

#define T1PRIO 20
#define T2PRIO 8
#define T3PRIO 10
#define PRIORITY_CEILING 9
#define CONFIGURE_MAXIMUM_TASKS 4

#define CONFIGURE_MAXIMUM_PRIORITY 255
#define CONFIGURE_MAXIMUM_PROCESSORS 4

#define CONFIGURE_SCHEDULER_PRIORITY_SMP
#include <rtems/scheduler.h>
//STEP 2 - configure THE SCHEDULER INSTANCES
RTEMS_SCHEDULER_PRIORITY_SMP(a, CONFIGURE_MAXIMUM_PRIORITY + 1);
RTEMS_SCHEDULER_PRIORITY_SMP(b, CONFIGURE_MAXIMUM_PRIORITY + 1);

//STEP 3
#define CONFIGURE_SCHEDULER_TABLE_ENTRIES \
RTEMS_SCHEDULER_TABLE_PRIORITY_SMP( \
  a, \
  SCHED_A \
), \
RTEMS_SCHEDULER_TABLE_PRIORITY_SMP( \
  b, \
  SCHED_B \
)

//STEP 4 Scheduler to processor assignment
#define CONFIGURE_SCHEDULER_ASSIGNMENTS \
RTEMS_SCHEDULER_ASSIGN(0, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY), \
RTEMS_SCHEDULER_ASSIGN(0, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY), \
RTEMS_SCHEDULER_ASSIGN(1, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY), \
RTEMS_SCHEDULER_ASSIGN(1, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY)

#define CONFIGURE_INITIAL_EXTENSIONS RTEMS_TEST_INITIAL_EXTENSION

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

//#define CONFIGURE_INIT_TASK_STACK_SIZE \
  (3 * CONFIGURE_MINIMUM_TASK_STACK_SIZE)

#include <rtems/confdefs.h>

/* global variables */
/*
*  Keep the names and IDs in global variables so another task can use them.
*/
TEST_EXTERN volatile bool TaskRan[3];
TEST_EXTERN volatile bool  TestFinished;
