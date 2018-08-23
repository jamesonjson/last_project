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
* Ensure message queue synchronization mechanism works correctly on an SMP configuration.
* Ensure FIFO and task priority fairness work on the task wait queue of the message queue.
*/
const char rtems_test_name[] = "SMP_TEST_DEV 03";

static test_context test_instance;

void PrintTaskInfo(const char *task_name)
{
  uint32_t cpu_num;

  cpu_num = rtems_get_current_processor();
  locked_printf("* CPU %" PRIu32 " running task %s\n", cpu_num, task_name);
}

/*Task 1 - Listener  ##################################*/
rtems_task Task_1(rtems_task_argument arg)
{
  test_context *ctx = (test_context *)arg;
  size_t msg_size = sizeof(task_message);
  rtems_status_code sc;

  /*Waiting on Task3 message on msq2*/
  sc = rtems_message_queue_receive(
    ctx->msq2_id,
    (void *)ctx->msg01.body,
    &msg_size,
    RTEMS_WAIT,
    RTEMS_NO_TIMEOUT
  );
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  locked_printf("* TASK1: (should be the first to report.) \n%s\n\n", ctx->msg01.body);

/*Waiting on Task3 message now msq1*/
  sc = rtems_message_queue_receive(
    ctx->msq1_id,
    (void *)ctx->msg01.body,
    &msg_size,
    RTEMS_WAIT,
    RTEMS_NO_TIMEOUT
  );
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  locked_printf("* TASK1: (should be the first to report.) \n \"%s\"\n\n", ctx->msg01.body);

  TaskRan[0] = true;
  rtems_task_suspend(RTEMS_SELF);
  rtems_test_assert(0);
}

/*Task 2 - Listener ##################################*/
rtems_task Task_2(rtems_task_argument arg)
{
  rtems_status_code sc;
  size_t msg_size = sizeof(task_message);
  test_context *ctx = (test_context *)arg;

    /*Waiting on Task3 message on msq2*/
  sc = rtems_message_queue_receive(
    ctx->msq2_id,
    (void *)ctx->msg02.body,
    &msg_size,
    RTEMS_WAIT,
    RTEMS_NO_TIMEOUT
  );
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  locked_printf("* TASK2: \n%s\n", ctx->msg02.body);

/*Waiting on Task3 message now msq1*/
  sc = rtems_message_queue_receive(
    ctx->msq1_id,
    (void *)ctx->msg02.body,
    &msg_size,
    RTEMS_WAIT,
    RTEMS_NO_TIMEOUT
  );
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  locked_printf("* TASK2: \n%s\n", ctx->msg02.body);

  TaskRan[1] = true;
  rtems_task_suspend(RTEMS_SELF);
  rtems_test_assert(0);
}

/*Task 3 - Speaker ##################################*/
rtems_task Task_3(rtems_task_argument arg)
{
  rtems_status_code sc;
  size_t msg_size = sizeof(task_message);
  test_context *ctx = (test_context *)arg;

  sc = rtems_task_wake_after(4);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  /*Write the message*/
  sprintf(&ctx->msg03.body, "Written on MSQ2. by TASK 03.");

  /*Send it to the message queue 2*/
  sc = rtems_message_queue_send(
      ctx->msq2_id,
      (void *)ctx->msg03.body,
      msg_size
  );
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

    sc = rtems_task_wake_after(1);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);

 /*And again now for task 2 still waiting*/
  sc = rtems_message_queue_send(
      ctx->msq2_id,
      (void *)ctx->msg03.body,
      msg_size
  );
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  sc = rtems_task_wake_after(1);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  /*Write the message*/
  sprintf(&ctx->msg03.body, "Written on MSQ1. by TASK 03.");

  /*Send message to message queue 1 where Task1 and 2 are waiting*/
  sc = rtems_message_queue_send(
      ctx->msq1_id,
      (void *)ctx->msg03.body,
      msg_size
  );
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

    /*And again now for task 2*/
  sc = rtems_message_queue_send(
      ctx->msq1_id,
      (void *)ctx->msg03.body,
      msg_size
  );
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  //job done.
  TaskRan[2] = true;
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

  //_CREATE message with fifo order of waiting tasks
  sc = rtems_message_queue_create(
      MSGQ1_NAME,
      MSG_COUNT,
      sizeof(task_message),
      RTEMS_FIFO,
      &ctx->msq1_id);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  //_CREATE message with priority order of waiting tasks
  sc = rtems_message_queue_create(
      MSGQ2_NAME,
      MSG_COUNT,
      sizeof(task_message),
      RTEMS_PRIORITY,
      &ctx->msq2_id
      );
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  //_CREATE Task_1 (highest priority task) ##################################
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

  //_CREATE Task_2 ##################################
  sc = rtems_task_create(
      rtems_build_name('T', 'A', '2', ' '),
      TASKS_PRIO,
      RTEMS_MINIMUM_STACK_SIZE,
      RTEMS_DEFAULT_MODES,
      RTEMS_DEFAULT_ATTRIBUTES,
      &ctx->task[1]);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  //_START task 2
  sc = rtems_task_start(
      ctx->task[1],
      Task_2,
      (rtems_task_argument)ctx);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  //_CREATE Task_3 ##################################
  sc = rtems_task_create(
      rtems_build_name('T', 'A', '3', ' '),
      TASKS_PRIO,
      RTEMS_MINIMUM_STACK_SIZE,
      RTEMS_DEFAULT_MODES,
      RTEMS_DEFAULT_ATTRIBUTES,
      &ctx->task[2]);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  
  //_START task 3
  sc = rtems_task_start(
      ctx->task[2],
      Task_3,
      (rtems_task_argument)ctx);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  /* Wait on all tasks to run, _DELETE barrier then exit*/
  while (1)
  {

    TestFinished = true;
    for (int i = 0; i < 3; i++)
    {

      if (TaskRan[i] == false)
        TestFinished = false;
    }
    if (TestFinished)
    {
      locked_printf("************ ********* ************\n\n");
      TEST_END();
      rtems_test_exit(0);
    }
  }
}

/*Init function*/
rtems_task Init(rtems_task_argument arg)
{
  rtems_status_code sc;
  test_context *ctx = &test_instance;

  TEST_BEGIN();
  locked_print_initialize();

  locked_printf("\n************ ********* ************\n");
  PrintTaskInfo("Init"); //imprime as informações da task Init
  test(ctx);
  locked_printf("************ ********* ************\n\n");
  TEST_END();
  rtems_test_exit(0);
}
