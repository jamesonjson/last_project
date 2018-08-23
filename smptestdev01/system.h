/*
*  COPYRIGHT (c) 1989-2011.
*  On-Line Applications Research Corporation (OAR).
*
*  The license and distribution terms for this file may be
*  found in the file LICENSE in this distribution or at
*  http://www.rtems.org/license/LICENSE.
*/

#include "tmacros.h"

//#include "test_support.h"

/* global variables */

/*
*  Keep the names and IDs in global variables so another task can use them.
*/

void PrintTaskInfo(const char *task_name);

TEST_EXTERN volatile bool TaskRan[ 3 ];
TEST_EXTERN volatile bool  TestFinished;

/* 
* Variable configurations.
*/
#define SCHED_EDF rtems_build_name('E', 'D', 'F', ' ')
#define CPU_COUNT 2

/* 
* Functions declaration.
*/
void PrintSchedInfo(
);
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
#define CONFIGURE_MAXIMUM_PERIODS 2

#define CONFIGURE_APPLICATION_NEEDS_SIMPLE_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_MICROSECONDS_PER_TICK   1000 /* 1 millisecond */

#define CONFIGURE_MAXIMUM_TASKS 4

#define CONFIGURE_MAXIMUM_PROCESSORS CPU_COUNT
#define CONFIGURE_MAXIMUM_PRIORITY 255
//#define CONFIGURE_MAXIMUM_CPU 1

//STEP 1
#define CONFIGURE_SCHEDULER_EDF_SMP

#include <rtems/scheduler.h>

RTEMS_SCHEDULER_EDF_SMP(a, CPU_COUNT);

#define CONFIGURE_SCHEDULER_TABLE_ENTRIES \
  RTEMS_SCHEDULER_TABLE_EDF_SMP(a, SCHED_EDF)

#define CONFIGURE_SCHEDULER_ASSIGNMENTS \
  RTEMS_SCHEDULER_ASSIGN(0, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY),\
  RTEMS_SCHEDULER_ASSIGN(0, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY) 

#define CONFIGURE_INITIAL_EXTENSIONS RTEMS_TEST_INITIAL_EXTENSION

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE


//#define CONFIGURE_INIT_TASK_STACK_SIZE \
  (3 * CONFIGURE_MINIMUM_TASK_STACK_SIZE)

#include <rtems/confdefs.h>