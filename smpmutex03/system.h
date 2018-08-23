/* 
 *  COPYRIGHT (c) 1989-2011.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.org/license/LICENSE.
 */

#include "tmacros.h"
#include "test_support.h"

/* functions */

rtems_task Init(
  rtems_task_argument argument
);

rtems_task Test_task(
  rtems_task_argument argument
);

/******************************* configuration information ************************/
/**********************************************************************************/
//#define CONFIGURE_MAXIMUM_SEMAPHORES 1
#define CONFIGURE_MAXIMUM_MRSP_SEMAPHORES 1

#define CONFIGURE_APPLICATION_DOES_NOT_NEED_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_SIMPLE_CONSOLE_DRIVER

#define CONFIGURE_MAXIMUM_PRIORITY 255

#define CONFIGURE_MAXIMUM_PROCESSORS 4 
//#define CONFIGURE_MAXIMUM_CPU 1

#define CONFIGURE_MAXIMUM_TASKS 3

//STEP 1 tell the system what scheduler algorithm to use
//#define CONFIGURE_SCHEDULER_EDF_SMP
#define CONFIGURE_SCHEDULER_PRIORITY_SMP
#include <rtems/scheduler.h>
//STEP 2 - configure THE SCHEDULER INSTANCES
//#define RTEMS_SCHEDULER_CONTEXT_EDF_SMP(a, CONFIGURE_MAXIMUM_PROCESSORS);
//#define RTEMS_SCHEDULER_CONTEXT_EDF_SMP(b, CONFIGURE_MAXIMUM_PROCESSORS);
RTEMS_SCHEDULER_PRIORITY_SMP(a, CONFIGURE_MAXIMUM_PRIORITY + 1);
RTEMS_SCHEDULER_PRIORITY_SMP(b, CONFIGURE_MAXIMUM_PRIORITY + 1);

//STEP 3
#define CONFIGURE_SCHEDULER_TABLE_ENTRIES \\
  RTEMS_SCHEDULER_TABLE_PRIORITY_SMP( \
    a, \
     rtems_build_name('A', ' ', ' ', ' ') \
  ), \
  RTEMS_SCHEDULER_TABLE_PRIORITY_SMP( \
    b, \
    rtems_build_name('B', ' ', ' ', ' ') \
  )
//STEP 4 Scheduler to processor assignment
#define CONFIGURE_SCHEDULER_ASSIGNMENTS \
  RTEMS_SCHEDULER_ASSIGN(0, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY), \
  RTEMS_SCHEDULER_ASSIGN(0, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY), \
  RTEMS_SCHEDULER_ASSIGN(1, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY), \
  RTEMS_SCHEDULER_ASSIGN(1, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY)

#define CONFIGURE_INITIAL_EXTENSIONS RTEMS_TEST_INITIAL_EXTENSION

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
//#define CONFIGURE_INIT
#define CONFIGURE_INIT_TASK_STACK_SIZE \
    (3 * CONFIGURE_MINIMUM_TASK_STACK_SIZE)

//#define CONFIGURE_INIT_TASK_PRIORITY        5 

#include <rtems/confdefs.h>
/**********************************************************************************/
/**********************************************************************************/

/* global variables */

/*
 *  Keep the names and IDs in global variables so another task can use them.
 */

//TEST_EXTERN volatile bool TaskRan[ 3 ];

void Loop(void);
void PrintTaskInfo(
  const char *task_name
);

//TEST_EXTERN volatile bool  TestFinished;

/* end of include file */