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
* Ensure that the task in ownership 
* of the mutex migrates to another scheduler
* instance in case it is preempted.
*
*/
const char rtems_test_name[] = "SMP_TEST_DEV 07";

static test_context test_instance;

void PrintTaskInfo(const char *task_name)
{
    uint32_t cpu_num;

    cpu_num = rtems_get_current_processor();
    printf("- CPU %" PRIu32 " running task %s.\n", cpu_num, task_name);
}

//Tasks running on SCHEDULER A
/**
 * Task1. Will be preempted at the right time by Task_3.
*/
rtems_task Task_1(rtems_task_argument arg)
{
  test_context *ctx = (test_context *)arg;
  rtems_status_code sc;
  rtems_interval period=5000;
  rtems_event_set event_out;
  rtems_task_priority my_priority;

  PrintTaskInfo("1");
  
  for(int i = 0; i < 2; i++)
  {
    if(i == 0){ //setup period
      rtems_rate_monotonic_period(ctx->period_id, period);

      sc = rtems_event_receive(
          RTEMS_EVENT_2,
          RTEMS_WAIT | RTEMS_EVENT_ALL,
          RTEMS_NO_TIMEOUT,
          &event_out
      );

    }else{
      while(1){
        rtems_rate_monotonic_period(ctx->period_id, period);
      
        sc = rtems_semaphore_obtain(ctx->sem_id, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
        printf("TA1_sem_obtain_status: %s\nNext action: send event.\n", rtems_status_text(sc));
        rtems_test_assert(sc == RTEMS_SUCCESSFUL);

        sc = rtems_event_send(
          ctx->task[1],
          RTEMS_EVENT_3
        );
        rtems_test_assert(sc == RTEMS_SUCCESSFUL);
        PrintTaskInfo("1 (finish).");
        TaskRan[0] = true;
        while(1);
      }
    }
  }
  
  
  
  TaskRan[0] = true;
  rtems_task_suspend(RTEMS_SELF);
  rtems_test_assert(0);
}

/**
 * Task_2.
*/
rtems_task Task_2(rtems_task_argument arg)
{ 
  test_context *ctx = (test_context *)arg;
  rtems_status_code sc;
  rtems_event_set event_out;
  rtems_interval period=300;

  PrintTaskInfo("2");

  for(int i = 0; i < 2; i++)
  {
    if(i == 0){ //setup period
      rtems_rate_monotonic_period(ctx->period_id2, period);
      sc = rtems_event_send(
          ctx->task[2],
          RTEMS_EVENT_1
        );
      rtems_test_assert(sc == RTEMS_SUCCESSFUL);
      printf("task init acordada.");

      

    }else{

      while(1){
        rtems_rate_monotonic_period(ctx->period_id2, period);

        sc = rtems_event_receive(
          RTEMS_EVENT_3,
          RTEMS_WAIT | RTEMS_EVENT_ALL,
          RTEMS_NO_TIMEOUT,
          &event_out
        );
        PrintTaskInfo("2 (finish).");
        TaskRan[1] = true;
        rtems_task_suspend(RTEMS_SELF);
        rtems_test_assert(0);
      }
    }
  }
}


/*
* ===================================================    Test function    ===================================================
* ===========================================================================================================================
*/
static void test(test_context *ctx)
{
  rtems_status_code sc;
  rtems_event_set event_out;

  /**
   * START OF SETUP CODE.
  */
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

  //criar os periodos
  sc = rtems_rate_monotonic_create(rtems_build_name('P','E','R','1'), &ctx->period_id);
  sc = rtems_rate_monotonic_create(rtems_build_name('P','E','R','2'), &ctx->period_id2);



  /**
   * END OF SETUP CODE.
  */
  ctx->task[2] = rtems_task_self();
  //_CREATE Task_1
  sc = rtems_task_create(
    rtems_build_name('T', 'A', '1', ' '),
    TA1_PRIO,
    RTEMS_MINIMUM_STACK_SIZE,
    RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES,
    &ctx->task[0]);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  //set scheduler of task_1
  sc = rtems_task_set_scheduler(
    ctx->task[0],
    ctx->sched_b,
    TA1_PRIO);
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
    TA2_PRIO,
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
    TA2_PRIO);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  //_START task 2
  sc = rtems_task_start(
    ctx->task[1],
    Task_2,
    (rtems_task_argument)ctx);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  sc = rtems_event_receive(  //wait for task2 signal
    RTEMS_EVENT_1,
    RTEMS_WAIT | RTEMS_EVENT_ALL,
    RTEMS_NO_TIMEOUT,
    &event_out
  );

  //CRIAR O OMIP SEMAPHORE
  sc = rtems_semaphore_create(
    rtems_build_name('O', 'M', 'I', 'P'),
    1,
    RTEMS_BINARY_SEMAPHORE | RTEMS_PRIORITY | RTEMS_INHERIT_PRIORITY | RTEMS_LOCAL,
    1,
    &ctx->sem_id
    );
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  sc = rtems_event_send(
          ctx->task[0],
          RTEMS_EVENT_2
        );
      rtems_test_assert(sc == RTEMS_SUCCESSFUL);
      printf("task 1 acordada.\n");

/*
  //_CREATE Task_3
  sc = rtems_task_create(
    rtems_build_name('T', 'A', '3', ' '),
    TA3_PRIO,
    RTEMS_MINIMUM_STACK_SIZE,
    RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES,
    &ctx->task[2]);
  printf("create_ta3_status_c: %s\n", rtems_status_text(sc));
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  rtems_task_wake_after(1); //make sure task1 is running

  //_START task 3
  sc = rtems_task_start(
    ctx->task[2],
    Task_3,
    (rtems_task_argument)ctx);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);


*/
  /* Wait on all tasks to run, then exit*/
  while(1)
  {
    TestFinished = true;
    for (int i = 0; i < CONFIGURE_MAXIMUM_TASKS - 1; i++)
    {
      if (TaskRan[i] == false)
        TestFinished = false;
    }
    if (TestFinished)
    {
      printf("************ ********* ************\n\n");
      TEST_END();
      rtems_test_exit(0);
    }
  }
}

rtems_task Init(rtems_task_argument arg)
{
  test_context *ctx = &test_instance;
  TEST_BEGIN();
  test(ctx);
  TEST_END();
  rtems_test_exit(0);
}
