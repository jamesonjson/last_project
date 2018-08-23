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

#define CONFIGURE_INIT_PRIO 1

#define CONFIGURE_INIT
#include "system.h"

#include <rtems.h>
#include <rtems/test.h>
#include <rtems/cpuuse.h>

#include "tmacros.h"

#include <stdio.h>
#include <inttypes.h>




/*
* Ensure that the task in ownership 
* of the mutex migrates to another scheduler
* instance in case it is preempted.
*
* WICH DOES NOT. why: any configuration error?
*/

const char rtems_test_name[] = "SMP_TEST_DEV 01";
#define T1PRIO 10
#define T2PRIO 9
#define T3PRIO 10
#define PRIORITY_CEILING 8

typedef struct
{
  rtems_id task[3];
  rtems_id semaphore_id;
  rtems_id sched_a, sched_b;
} test_context;

static test_context test_instance;

void PrintTaskInfo(
    const char *task_name)
{
  uint32_t cpu_num;

  cpu_num = rtems_get_current_processor();

  locked_printf("* CPU %" PRIu32 " running task %s\n", cpu_num, task_name);
}

void PrintSchedInfo(
    test_context *ctx)
{
  rtems_status_code sc;
  rtems_id sched_id;

  sc = rtems_scheduler_ident(SCHED_A, &ctx->sched_a);
  //locked_printf("SCHEDULER A ID %d\n", (int)ctx->sched_a);
  sc = rtems_scheduler_ident(SCHED_B, &ctx->sched_b);
  //locked_printf("SCHEDULER B ID %d\n", (int)ctx->sched_b);

  for (int i = 0; i < 4; i++)
  {
    sc = rtems_scheduler_ident_by_processor(i, &sched_id);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    if (sched_id == ctx->sched_a)
    {
      locked_printf("* SCHEDULER A is attributed to CPU %d\n", i);
    }
    else
    {
      locked_printf("* SCHEDULER B is attributed to CPU %d\n", i);
    }
  }
}

//task 1
rtems_task Task_1(
    rtems_task_argument arg)
{
  test_context *ctx = (test_context *)arg;
  rtems_status_code sc;
  rtems_task_priority priority;

  /*obtain semAphore
  the priority of the task has changed*/
  sc = rtems_semaphore_obtain(ctx->semaphore_id, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  /* Show that this task is running on cpu X */
  PrintTaskInfo("TA1: Counting");
  //for (int i = 0; i < 1000000; i++)
  ; //execution simulation, task 1 should be preempted here
  PrintTaskInfo("TA1: Finished counting");
  sc = rtems_semaphore_release(ctx->semaphore_id);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  TaskRan[0] = true;
  PrintTaskInfo("TA1: Finished.");
  while (1)
    ;
}


rtems_task Task_3(
    rtems_task_argument arg)
{
  rtems_status_code sc;
  test_context *ctx = (test_context *)arg;
  uint32_t cpu_num;

  cpu_num = rtems_get_current_processor();
  locked_printf("* CPU %" PRIu32 " running task TA3\n", cpu_num);

  sc = rtems_semaphore_obtain(ctx->semaphore_id, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
  /* Show that this task is running on cpu X */
  PrintTaskInfo("TA3");
  sc = rtems_semaphore_release(ctx->semaphore_id);

  TaskRan[1] = true;
  rtems_task_suspend(RTEMS_SELF);
  rtems_test_assert(0);
}

/*
* ====================================================== Test Function ======================================================
* ===========================================================================================================================
*/
static void test(test_context *ctx)
{
  rtems_status_code sc;
  rtems_id aux_id;
  uint32_t cpu_count;

  cpu_count = rtems_get_processor_count();
  locked_printf("cpu_count = %" PRIu32 "\n", cpu_count);

  //CRIAR O MrsP SEMAPHORE
  sc = rtems_semaphore_create(
      rtems_build_name('M', 'R', 'S', 'P'),
      1,
      RTEMS_MULTIPROCESSOR_RESOURCE_SHARING | RTEMS_BINARY_SEMAPHORE,
      PRIORITY_CEILING,
      &ctx->semaphore_id);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  //_CREATE Task_1 - Global
  sc = rtems_task_create(
      rtems_build_name('T', 'A', '1', ' '),
      T1PRIO,
      RTEMS_MINIMUM_STACK_SIZE,
      RTEMS_DEFAULT_MODES,
      RTEMS_GLOBAL,
      &ctx->task[0]);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  //_CREATE Task_3 - Global
  sc = rtems_task_create(
      rtems_build_name('T', 'A', '3', ' '),
      T3PRIO,
      RTEMS_MINIMUM_STACK_SIZE,
      RTEMS_DEFAULT_MODES,
      RTEMS_LOCAL,
      &ctx->task[2]);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  //_START task 1
  sc = rtems_task_start(
      ctx->task[0],
      Task_1,
      (rtems_task_argument)ctx);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  locked_printf("Init: ..... Started Task1\n");

  //_SET scheduler of task 3
  sc = rtems_task_set_scheduler(
      ctx->task[2],
      ctx->sched_b,
      T3PRIO);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  //_START task 3
  sc = rtems_task_start(
      ctx->task[2],
      Task_3,
      (rtems_task_argument)ctx);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  locked_printf("Init: ..... Started Task3\n");



  /* Wait on all tasks to run, _DELETE semaphore */
  while (1)
  {
    TestFinished = true;
    for (int i = 0; i < 2; i++)
    {

      if (TaskRan[i] == false)
        TestFinished = false;
    }
    if (TestFinished)
    {
      //if all tasks finished delete semaphore
      sc = rtems_semaphore_delete(ctx->semaphore_id);
      rtems_test_assert(sc == RTEMS_SUCCESSFUL);

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
  PrintSchedInfo(ctx);   //imprime quais cpus têm quais escalonadores
  PrintTaskInfo("Init"); //imprime as informações da task Init
  test(ctx);

  TEST_END();
  rtems_test_exit(0);
}

