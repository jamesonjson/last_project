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

const char rtems_test_name[] = "SMP_TEST_DEV 05";

static test_context test_instance;

/*
* =====================================================    Functions    =====================================================
* ===========================================================================================================================
*/
/* Task 1 */
rtems_task Task_1(rtems_task_argument arg)
{
    test_context *ctx = (test_context *)arg;
    rtems_event_set event_out;
    rtems_status_code sc;

    printf("task1: finished.\n");
    //locked_printf("* TA2: finished.\n");
    TaskRan[0] = true;
    rtems_task_suspend(RTEMS_SELF);
    rtems_test_assert(0);
}
/* Task 2 */
rtems_task Task_2(rtems_task_argument arg)
{ 
    test_context *ctx = (test_context *)arg;
    rtems_event_set event_out;
    rtems_status_code sc;

    printf("task2: finished\n");
    //locked_printf("* TA2: finished.\n");
    TaskRan[1] = true;
    rtems_task_suspend(RTEMS_SELF);
    rtems_test_assert(0);
}



static void test(test_context *ctx)
{
  rtems_status_code sc;

  //ident the schedulers
  sc = rtems_scheduler_ident(
      SCHED_A,
      &ctx->sched_a
  );
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  sc = rtems_scheduler_ident(
    SCHED_B,
    &ctx->sched_b
  );
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  //_CREATE Task_1
  sc = rtems_task_create(
    rtems_build_name('T', 'A', '1', ' '),
    TASK1_PRIO,
    RTEMS_MINIMUM_STACK_SIZE,
    RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES,
    &ctx->task[0]);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  //_START task 1
  sc = rtems_task_start(
    ctx->task[0],
    Task_1,
    (rtems_task_argument)ctx);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  //_CREATE Task_2
  sc = rtems_task_create(
    rtems_build_name('T', 'A', '2', ' '),
    TASKS_PRIO,
    RTEMS_MINIMUM_STACK_SIZE,
    RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES,
    &ctx->task[1]);
  printf("create_ta2_status_c: %s\n", rtems_status_text(sc));
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  //set scheduler of task_2
  sc = rtems_task_set_scheduler(
    ctx->task[1],
    ctx->sched_b,
    TASKS_PRIO);
  printf("set_scheduler_status_c: %s\n", rtems_status_text(sc));
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  //_START task 2
  sc = rtems_task_start(
    ctx->task[1],
    Task_2,
    (rtems_task_argument)ctx);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  /* Wait on all tasks to run, then exit*/
  while(1)
  {
    TestFinished = true;
    for (int i = 0; i < TASK_COUNT-1; i++)
    {
      if (TaskRan[i] == false)
        TestFinished = false;
    }
    if (TestFinished)
    {
      printf("end.\n");
      //locked_printf("************ ********* ************\n\n");
      TEST_END();
      rtems_test_exit(0);
    }
  }
}

/*
* =========================================================   Init   ========================================================
* ===========================================================================================================================
*/

rtems_task Init(rtems_task_argument arg)
{
test_context *ctx = &test_instance;
TEST_BEGIN();
printf("cpu_Count: %" PRIu32 "\n", rtems_get_processor_count());
//locked_print_initialize();
//srand(rtems_clock_get_ticks_since_boot()); //seed the rand() function

test(ctx);
TEST_END();
rtems_test_exit(0);
}


