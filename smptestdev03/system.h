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


#define SCHED_A rtems_build_name('A', ' ', ' ', ' ')
#define MSGQ1_NAME rtems_build_name('M', 'S', 'Q', '1')
#define MSGQ2_NAME rtems_build_name('M', 'S', 'Q', '1')

#define CPU_COUNT 4

#define MSG_COUNT 1

#define TASK_COUNT 4
#define TASKS_PRIO 20
#define TASK1_PRIO 10

typedef struct
{
  char body[40];
} task_message;

typedef struct
{
  rtems_id task[TASK_COUNT];
  rtems_id msq1_id, msq2_id;
  task_message msg01;
  task_message msg02;
  task_message msg03;
} test_context;

/* functions */
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
#define CONFIGURE_MAXIMUM_SEMAPHORES 1
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES 2
#define CONFIGURE_MESSAGE_BUFFER_MEMORY \
        CONFIGURE_MESSAGE_BUFFERS_FOR_QUEUE(50, sizeof(task_message))

//#define CONFIGURE_APPLICATION_DOES_NOT_NEED_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_SIMPLE_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_MICROSECONDS_PER_TICK   1000 /* 1 millisecond */

#define CONFIGURE_MAXIMUM_PRIORITY 255

#define CONFIGURE_MAXIMUM_PROCESSORS CPU_COUNT
//#define CONFIGURE_MAXIMUM_CPU 1

#define CONFIGURE_MAXIMUM_TASKS TASK_COUNT

//STEP 1 tell the system what scheduler algorithm to use
//#define CONFIGURE_SCHEDULER_EDF_SMP
#define CONFIGURE_SCHEDULER_PRIORITY_SMP
#include <rtems/scheduler.h>
//STEP 2 - configure THE SCHEDULER INSTANCES
//#define RTEMS_SCHEDULER_CONTEXT_EDF_SMP(a, CONFIGURE_MAXIMUM_PROCESSORS);
//#define RTEMS_SCHEDULER_CONTEXT_EDF_SMP(b, CONFIGURE_MAXIMUM_PROCESSORS);
RTEMS_SCHEDULER_PRIORITY_SMP(a, CONFIGURE_MAXIMUM_PRIORITY + 1);

//STEP 3
#define CONFIGURE_SCHEDULER_TABLE_ENTRIES \
RTEMS_SCHEDULER_TABLE_PRIORITY_SMP( \
  a, \
   SCHED_A \
)

//STEP 4 Scheduler to processor assignment
#define CONFIGURE_SCHEDULER_ASSIGNMENTS \
RTEMS_SCHEDULER_ASSIGN(0, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY), \
RTEMS_SCHEDULER_ASSIGN(0, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY), \
RTEMS_SCHEDULER_ASSIGN(0, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY), \
RTEMS_SCHEDULER_ASSIGN(0, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY)

#define CONFIGURE_INITIAL_EXTENSIONS RTEMS_TEST_INITIAL_EXTENSION

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
//#define CONFIGURE_INIT
#define CONFIGURE_INIT_TASK_STACK_SIZE \
  (3 * CONFIGURE_MINIMUM_TASK_STACK_SIZE)


#include <rtems/confdefs.h>

/* global variables */

/*
*  Keep the names and IDs in global variables so another task can use them.
*/

void PrintTaskInfo(const char *task_name);

TEST_EXTERN volatile bool TaskRan[ TASK_COUNT ];
TEST_EXTERN volatile bool  TestFinished;

/* end of include file */
