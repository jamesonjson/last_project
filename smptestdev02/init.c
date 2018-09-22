/*
* Ruben Gonçalves
* 1150785@isep.ipp.pt
* CISTER
* 
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
* Ensure barrier works on an SMP configuration.
*/

const char rtems_test_name[] = "SMP_TEST_DEV 02";

#define TASKS_PRIO 20

typedef struct
{
  rtems_id task[3];
  rtems_id bar_id;
} test_context;

static test_context test_instance;

void PrintTaskInfo(const char *task_name)
{
  uint32_t cpu_num;

  cpu_num = rtems_get_current_processor();
  locked_printf("* CPU %" PRIu32 " running task %s\n", cpu_num, task_name);
}

/*Task 1*/
rtems_task Task_1(rtems_task_argument arg)
{
  test_context *ctx = (test_context *)arg;
  rtems_status_code sc;
  rtems_task_priority priority;

  /* Show that this task is waiting on the barrier, running on cpu X */
  PrintTaskInfo("TA1: Waiting at the barrier");
  sc = rtems_barrier_wait(
      ctx->bar_id,
      RTEMS_WAIT);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  PrintTaskInfo("TA1 END.");
  TaskRan[0] = true;
  rtems_task_suspend(RTEMS_SELF);
  rtems_test_assert(0);
}

/*Task 2*/
rtems_task Task_2(rtems_task_argument arg)
{
  rtems_status_code sc;
  test_context *ctx = (test_context *)arg;

  /* Show that this task is waiting on the barrier, running on cpu X */
  PrintTaskInfo("TA2: Waiting at the barrier");
  sc = rtems_barrier_wait(
      ctx->bar_id,
      RTEMS_WAIT);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  /* Show that this task is running on cpu X */
  PrintTaskInfo("TA2 END.");
  TaskRan[1] = true;
  rtems_task_suspend(RTEMS_SELF);
  rtems_test_assert(0);
}

/*Task 3*/
rtems_task Task_3(rtems_task_argument arg)
{
  rtems_status_code sc;
  test_context *ctx = (test_context *)arg;
  /* Show that this task is waiting on the barrier, running on cpu X */
  PrintTaskInfo("TA3: Waiting at the barrier");
  sc = rtems_barrier_wait(
      ctx->bar_id,
      RTEMS_WAIT);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  /* Show that this task is running on cpu X */
  PrintTaskInfo("TA3 END.");
  TaskRan[2] = true;
  rtems_task_suspend(RTEMS_SELF);
  rtems_test_assert(0);
}

/*
* ====================================================== Init Function ======================================================
* ===========================================================================================================================
*/
static void test(test_context *ctx)
{
  rtems_status_code sc;
  bool go = true;

  //_CREATE barrier that oneps automatically when third task arrives
  sc = rtems_barrier_create(
      BAR_NAME,
      RTEMS_BARRIER_AUTOMATIC_RELEASE,
      3,
      &ctx->bar_id);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  //_CREATE Task_1 - Global
  sc = rtems_task_create(
      rtems_build_name('T', 'A', '1', ' '),
      TASKS_PRIO,
      RTEMS_MINIMUM_STACK_SIZE,
      RTEMS_DEFAULT_MODES,
      RTEMS_GLOBAL,
      &ctx->task[0]);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  //_CREATE Task_2 - Local
  sc = rtems_task_create(
      rtems_build_name('T', 'A', '2', ' '),
      TASKS_PRIO,
      RTEMS_MINIMUM_STACK_SIZE,
      RTEMS_DEFAULT_MODES,
      RTEMS_LOCAL,
      &ctx->task[1]);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  //_CREATE Task_3 - Local
  sc = rtems_task_create(
      rtems_build_name('T', 'A', '3', ' '),
      TASKS_PRIO,
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

  //_START task 2
  sc = rtems_task_start(
      ctx->task[1],
      Task_2,
      (rtems_task_argument)ctx);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  //_START task 3
  sc = rtems_task_start(
      ctx->task[2],
      Task_3,
      (rtems_task_argument)ctx);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  /* Wait on all tasks to run, _DELETE barrier then exit*/
  while (go)
  {

    TestFinished = true;
    for (int i = 0; i < 3; i++)
    {

      if (TaskRan[i] == false)
        TestFinished = false;
    }
    if (TestFinished)
    {
      sc = rtems_barrier_delete(ctx->bar_id);
      rtems_test_assert(sc == RTEMS_SUCCESSFUL);
      go = false;
    }
  }
}

/*Init function*/
rtems_task Init(rtems_task_argument arg)
{
  test_context *ctx = &test_instance;

  TEST_BEGIN();
  locked_print_initialize();

  for (int i = 0; i < 3; i++)
  {
    TaskRan[i] == false;
  }
  locked_printf("\n************ ********* ************\n");
  PrintTaskInfo("Init"); //imprime as informações da task Init
  test(ctx);
  locked_printf("************ ********* ************\n\n");
  TEST_END();
  rtems_test_exit(0);
}
