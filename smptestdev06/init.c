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
const char rtems_test_name[] = "SMP_TEST_DEV 06";

static test_context test_instance;

void PrintTaskInfo(
    const char *task_name)
{
  uint32_t cpu_num;

  cpu_num = rtems_get_current_processor();

  printf("* CPU %" PRIu32 " running task %s\n", cpu_num, task_name);
}
/*
void PrintSchedInfo(
    test_context *ctx)
{
  rtems_status_code sc;
  rtems_id sched_id;

  sc = rtems_scheduler_ident(SCHED_A, &ctx->sched_a);
  //locked_printf("SCHEDULER A ID %d\n", (int)ctx->sched_a);
  sc = rtems_scheduler_ident(SCHED_B, &ctx->sched_b);
  //locked_printf("SCHEDULER B ID %d\n", (int)ctx->sched_b);

  for (int i = 0; i < 2; i++)
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
*/

//Tasks running on SCHEDULER A
/**
 * Task1. Will be preempted at the right time by Task_3.
*/
rtems_task Task_1(rtems_task_argument arg)
{
    test_context *ctx = (test_context *)arg;
    rtems_status_code sc;
    
 rtems_id id, period_id, setup_period_id;
  rtems_name name; 
  rtems_interval period=5000;

  sc = rtems_rate_monotonic_create(rtems_build_name('P','E','R','1'), &period_id);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  while(1){
    sc = rtems_rate_monotonic_period(period_id, period);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL); 
    
    sc = rtems_semaphore_obtain(ctx->sem_id, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
    printf("TA1_sem_obtain_status: %s\n", rtems_status_text(sc));
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    
    

    printf("TA1_sem_release.\n");
    sc = rtems_semaphore_release(ctx->sem_id);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);

    }

    
    TaskRan[0] = true;
    rtems_task_suspend(RTEMS_SELF);
    rtems_test_assert(0);
}

/**
 * Task_3. Must preempt Task_1.
*/
rtems_task Task_3(rtems_task_argument arg)
{ 
    test_context *ctx = (test_context *)arg;
    rtems_status_code sc;

    sc = rtems_semaphore_obtain(ctx->sem_id, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
    printf("TA3_sem_obtain_status: %s\n", rtems_status_text(sc));
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    
    TaskRan[2] = true;
    rtems_task_suspend(RTEMS_SELF);
    rtems_test_assert(0);
}

//Tasks running on SCHEDULER B
/**
 * Task_2. Will let Task_1 run on the current processor in order to release the semaphore.
*/
rtems_task Task_2(rtems_task_argument arg)
{ 
    test_context *ctx = (test_context *)arg;
    rtems_status_code sc;

    sc = rtems_semaphore_obtain(ctx->sem_id, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
    printf("TA2_sem_obtain_status: %s\n", rtems_status_text(sc));
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);

    printf("TA2: success!\n");
    sc = rtems_semaphore_release(ctx->sem_id);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);

    TaskRan[1] = true;
    rtems_task_suspend(RTEMS_SELF);
    rtems_test_assert(0);
}

/*
* ===================================================    Test function    ===================================================
* ===========================================================================================================================
*/
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

  //CRIAR O OMIP SEMAPHORE
  sc = rtems_semaphore_create(
    rtems_build_name('O', 'M', 'I', 'P'),
    1,
    RTEMS_BINARY_SEMAPHORE | RTEMS_MULTIPROCESSOR_RESOURCE_SHARING | RTEMS_LOCAL,
    1,
    &ctx->sem_id
    );
  printf("semaphore_CREATE_status: %s\n", rtems_status_text(sc) );
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  //_CREATE Task_1
  sc = rtems_task_create(
    rtems_build_name('T', 'A', '1', ' '),
    TA1_PRIO,
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
  printf("set_scheduler_status_c: %s\n", rtems_status_text(sc));
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  //rtems_wake_after(3); //start task 2
  printf("Init busy wait:\n");
  for(int i = 0; i < 100000; i++)
  {
  
  }
  printf("Init finished waiting.\n");

  //_START task 2
  sc = rtems_task_start(
    ctx->task[1],
    Task_2,
    (rtems_task_argument)ctx);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
*/
  /* Wait on all tasks to run, then exit*/
  while(1)
  {
    TestFinished = true;
    for (int i = 0; i < 1; i++)
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
  printf("hellooo\n");
  //locked_print_initialize();
  //PrintSchedInfo(ctx);   //imprime quais cpus têm quais escalonadores
  test(ctx);

  TEST_END();
  rtems_test_exit(0);
}
