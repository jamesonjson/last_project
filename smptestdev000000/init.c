/*
* Copyright (c) 2017, 2018 embedded brains GmbH.  All rights reserved.
*
*  embedded brains GmbH
*  Dornierstr. 4
*  82178 Puchheim
*  Germany
*  <rtems@embedded-brains.de>
*
* The license and distribution terms for this file may be
* found in the file LICENSE in this distribution or at
* http://www.rtems.org/license/LICENSE.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif



#define CONFIGURE_INIT
#include "system.h"

#include <rtems.h>
#include <rtems/cpuuse.h>

#include "tmacros.h"

#include <stdio.h>
#include <inttypes.h>

/*
Ensure classic events work correctly on an SMP configuration*/

const char rtems_test_name[] = "SMP_TEST_DEV 05";
#define T1PRIO 30
#define T2PRIO 20
#define T3PRIO 31
#define PRIORITY_CEILING 25

typedef struct {
rtems_id task[3];
rtems_id sched_a, sched_b;
} test_context;

static test_context test_instance;

void PrintTaskInfo(
const char *task_name
)
{
uint32_t cpu_num;

cpu_num = rtems_get_current_processor();

locked_printf("  CPU %" PRIu32 " running task %s\n", cpu_num, task_name );
}

void PrintSchedInfo(
  test_context *ctx
)
{
  rtems_status_code sc;
  rtems_id sched_id;

  sc = rtems_scheduler_ident(SCHED_A, &ctx->sched_a);
  //locked_printf("SCHEDULER A has the id %d\n", (int) sched_a );
  sc = rtems_scheduler_ident(SCHED_B, &ctx->sched_b);
  //locked_printf("SCHEDULER B has the id %d\n", (int) sched_b );

  for(int i = 0; i<4; i++){
    sc = rtems_scheduler_ident_by_processor( i , &sched_id);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    if(sched_id == ctx->sched_a){
      locked_printf("SCHEDULER A is attributed to CPU %d\n", i);
    }else{
      locked_printf("SCHEDULER B is attributed to CPU %d\n", i);
    }
    
  }
}

//task 1
rtems_task Task_1(
  rtems_task_argument arg
  )
  {
  test_context *ctx = (test_context *) arg;
  rtems_status_code sc;
  rtems_task_priority priority;

  sc = rtems_task_get_priority(ctx->task[0],ctx->sched_a, &priority);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  locked_printf("TA1-Priority:  %d\n", priority);
  



  //check priority
  sc = rtems_task_get_priority(ctx->task[0],ctx->sched_a, &priority);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  locked_printf("TA1-Priority (after obtaining mrsp sem):  %d\n", priority);

  /* Show that this task is running on cpu X */
  PrintTaskInfo( "TA1: Counting" );
  for(int i = 0;i < 10000000;i++); //execution simulation, task 1 should be preempted here
  PrintTaskInfo( "TA1: Finished counting" );

  TaskRan[0] = true;
  PrintTaskInfo( "TA1: Finished." );
  while(1);
}

/*Task 2 should take the only processor available from the scheduler A and Task 1 should be preempted and then migrated to Scheduler B*/
rtems_task Task_2(
rtems_task_argument arg
)
{
  rtems_status_code sc;
  test_context *ctx = (test_context *) arg;
  rtems_task_priority priority;
  /*
  sc = rtems_task_get_priority(ctx->task[1],ctx->sched_a, &priority);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  locked_printf("TA2-Priority:  %d\n", priority);
  */
  sc = rtems_task_wake_after(5000);//let task 1 obtain the mutex 
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  /* Show that this task is running on cpu X */
  PrintTaskInfo( "TA2" );
  TaskRan[1] = true;
  while(1);
}

rtems_task Task_3(
rtems_task_argument arg
)
{
  /* Show that this task is running on cpu X */
  PrintTaskInfo( "TA3" );
  TaskRan[2] = true;
  while(1);
}

static void test(test_context *ctx)
{
rtems_status_code sc;
rtems_id aux_id;


//CRIAR O MrsP SEMAPHORE



//_CREATE Task_1 - Global
sc = rtems_task_create(
  rtems_build_name('T', 'A', '1', ' '),
  T1PRIO,
  RTEMS_MINIMUM_STACK_SIZE,
  RTEMS_DEFAULT_MODES,
  RTEMS_GLOBAL,
  &ctx->task[0]
);
rtems_test_assert(sc == RTEMS_SUCCESSFUL);

//_CREATE Task_2 - Local
sc = rtems_task_create(
  rtems_build_name('T', 'A', '2', ' '),
  T2PRIO,
  RTEMS_MINIMUM_STACK_SIZE,
  RTEMS_DEFAULT_MODES,
  RTEMS_LOCAL,
  &ctx->task[1]
);
rtems_test_assert(sc == RTEMS_SUCCESSFUL);

//_CREATE Task_3 - Global
sc = rtems_task_create(
  rtems_build_name('T', 'A', '3', ' '),
  T3PRIO,
  RTEMS_MINIMUM_STACK_SIZE,
  RTEMS_DEFAULT_MODES,
  RTEMS_GLOBAL,
  &ctx->task[2]
);
rtems_test_assert(sc == RTEMS_SUCCESSFUL);

sc = rtems_task_set_scheduler(
  ctx->task[2],
  ctx->sched_b,
  T3PRIO
);
rtems_test_assert(sc == RTEMS_SUCCESSFUL);

sc = rtems_task_get_scheduler(
  ctx->task[2],
  &aux_id
);
rtems_test_assert(sc == RTEMS_SUCCESSFUL);
//locked_printf("sc_task_set_scheduler:  %s\n", rtems_status_text(sc));
locked_printf("Sched_T3_id: %d\n", aux_id);
locked_printf("Sched_B_Id: %d\n", ctx->sched_b);

//_START task 1
sc = rtems_task_start(
  ctx->task[0],
  Task_1,
  (rtems_task_argument) ctx
);
rtems_test_assert(sc == RTEMS_SUCCESSFUL);
locked_printf("Init: ..... Started Task1\n");

//_START task 2
sc = rtems_task_start(
  ctx->task[1],
  Task_2,
  (rtems_task_argument) ctx
);
rtems_test_assert(sc == RTEMS_SUCCESSFUL);
locked_printf("Init: ..... Started Task2\n");

//_START task 3
sc = rtems_task_start(
  ctx->task[2],
  Task_3,
  (rtems_task_argument) ctx
);
rtems_test_assert(sc == RTEMS_SUCCESSFUL);
locked_printf("Init: ..... Started Task3\n");


/* Wait on all tasks to run, _DELETE semaphore */
 while (1) {
   TestFinished = true;
   for ( int i=0; i < 2 ; i++ ) {

     if (TaskRan[i] == false)
       TestFinished = false;
   }
   if (TestFinished) {

     TEST_END();
     rtems_test_exit(0);
   }
 }

}

rtems_task Init(rtems_task_argument arg)
{
test_context *ctx = &test_instance;

TEST_BEGIN();
locked_print_initialize();
//PrintSchedInfo(ctx); //imprime quais cpus têm quais escalonadores

PrintTaskInfo( "Init" ); //imprime as informações da task Init
test(ctx);

TEST_END();
rtems_test_exit(0);
}


