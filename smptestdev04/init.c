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
* Ensure classic events work correctly on an SMP configuration.
*/
const char rtems_test_name[] = "SMP_TEST_DEV 04";

static test_context test_instance;

void PrintTaskInfo(const char *task_name)
{
    uint32_t cpu_num;

    cpu_num = rtems_get_current_processor();
    locked_printf("* CPU %" PRIu32 " running task %s.\n", cpu_num, task_name);
}

/*Task 1 - only continues after receiving event 6 and 7  ##################################*/
rtems_task Task_1(rtems_task_argument arg)
{
    test_context *ctx = (test_context *)arg;
    rtems_event_set event_out;
    rtems_status_code sc;
locked_printf("* TA1: finished.\n");
    sc = rtems_event_receive(
        RTEMS_EVENT_6 | RTEMS_EVENT_7,
        RTEMS_WAIT | RTEMS_EVENT_ALL,
        RTEMS_NO_TIMEOUT,
        &event_out);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);

    locked_printf("* TA1: finished.\n");
    TaskRan[0] = true;
    rtems_task_suspend(RTEMS_SELF);
    rtems_test_assert(0);
}

/*Task 2 - Listener ##################################*/
rtems_task Task_2(rtems_task_argument arg)
{
    test_context *ctx = (test_context *)arg;
    rtems_event_set event_out;
    rtems_status_code sc;

    sc = rtems_event_receive(
        RTEMS_EVENT_7,
        RTEMS_WAIT | RTEMS_EVENT_ALL,
        RTEMS_NO_TIMEOUT,
        &event_out);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);

    locked_printf("* TA2: finished.\n");
    TaskRan[1] = true;
    rtems_task_suspend(RTEMS_SELF);
    rtems_test_assert(0);
}

/*Task 3 - Speaker ##################################*/
rtems_task Task_3(rtems_task_argument arg)
{
    rtems_status_code sc;
    test_context *ctx = (test_context *)arg;

    sc = rtems_task_wake_after(1);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);

    /*Send EVENT_6 to Task1 who is waiting on EVENT_6 AND EVENT_7*/
    sc = rtems_event_send(
        ctx->task[0],
        RTEMS_EVENT_6);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    locked_printf("* TA3: sent RTEMS_EVENT_6 to TA1.\n");
    /*Send EVENT_7 to Task2 */
    sc = rtems_event_send(
        ctx->task[1],
        RTEMS_EVENT_7);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    locked_printf("* TA3: sent RTEMS_EVENT_7 to TA2.\n");

    /*Send EVENT_6 and 7 to Task1 who is waiting on EVENT_6 AND EVENT_7*/
    sc = rtems_event_send(
        ctx->task[0],
        RTEMS_EVENT_6 | RTEMS_EVENT_7);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    locked_printf("* TA3: sent RTEMS_EVENT_6 and RTEMS_EVENT_7 to TA1.\n");

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

  //_CREATE Task_1 ##################################
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

    /* Wait on all tasks to run, then exit*/
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
