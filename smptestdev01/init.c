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
* WICH DOES NOT. why: any configuration error?
*/

const char rtems_test_name[] = "SMP_TEST_DEV 01";
#define T1PRIO 5
#define T3PRIO 10
#define PRIORITY_CEILING 1

typedef struct
{
  rtems_id task[2];
  rtems_id semaphore_id;
  rtems_id semaphore_id2;
  rtems_id sched_a, sched_a1;
} test_context;

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

//task 1
rtems_task Task_1(
    rtems_task_argument arg)
{
  test_context *ctx = (test_context *)arg;
  rtems_status_code sc;
  rtems_task_priority priority;

  rtems_id id, period_id;
  rtems_name name; 
  rtems_interval periodicity=500;
  
  sc = rtems_rate_monotonic_create(rtems_build_name('P','E','R','2'), &period_id);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  sc = rtems_scheduler_ident(SCHED_EDF, &ctx->sched_a);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);


  for(int i = 0; i<4; i++){

    sc = rtems_rate_monotonic_period(period_id, periodicity);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL); 
  
    //Obtain the semaphore
    printf("ta1_sem_obtain_status: %s\n", rtems_status_text(sc));
    /*
    sc = rtems_semaphore_obtain(ctx->semaphore_id, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
    printf("ta1_sem_obtain_status: %s\n", rtems_status_text(sc));
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
*/

    sc = rtems_task_get_priority(rtems_task_self(), ctx->sched_a, &priority);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    printf("* ta1: My priority is %d\n", priority);
    /*
    //Release the semaphore
    sc = rtems_semaphore_release(ctx->semaphore_id);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    */
  }
  
  TaskRan[0] = true;
  rtems_task_suspend(RTEMS_SELF);
  rtems_test_assert(0);
}

//task 3
rtems_task Task_3(rtems_task_argument arg)
{
  rtems_status_code sc;
  test_context *ctx = (test_context *)arg;
  uint32_t cpu_num;
  rtems_task_priority priority;
  rtems_id id, period_id, setup_period_id;
  rtems_name name; 
  rtems_interval periodicity=10000;
  
  //Show that this task is running on cpu X
  //PrintTaskInfo("TA3");

  sc = rtems_rate_monotonic_create(rtems_build_name('P','E','R','1'), &period_id);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  sc = rtems_scheduler_ident(SCHED_EDF, &ctx->sched_a);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  for(int i = 0; i<4; i++){

    sc = rtems_rate_monotonic_period(period_id, periodicity);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL); 

    //Obtain the semaphore
    sc = rtems_semaphore_obtain(ctx->semaphore_id, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
    printf("ta3_sem_obtain_status: %s\n", rtems_status_text(sc));
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    while(1);
      
  }

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
  printf("CPU COUNT = %" PRIu32 "\n\n", cpu_count);

  //CRIAR O MrsP SEMAPHORE
  sc = rtems_semaphore_create(
      rtems_build_name('M', 'R', 'S', 'P'),
      1,
      RTEMS_BINARY_SEMAPHORE,
      1,
      &ctx->semaphore_id
      );
  printf("create_semaphore_status: %s\n", rtems_status_text(sc) );
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  //_CREATE Task_1
  sc = rtems_task_create(
      rtems_build_name('T', 'A', '1', ' '),
      T1PRIO,
      RTEMS_MINIMUM_STACK_SIZE,
      RTEMS_DEFAULT_MODES,
      RTEMS_LOCAL,
      &ctx->task[0]);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  //_CREATE Task_3
  sc = rtems_task_create(
      rtems_build_name('T', 'A', '3', ' '),
      T3PRIO,
      RTEMS_MINIMUM_STACK_SIZE,
      RTEMS_DEFAULT_MODES,
      RTEMS_LOCAL,
      &ctx->task[1]);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  //_START task 3
  sc = rtems_task_start(
      ctx->task[1],
      Task_3,
      (rtems_task_argument)ctx);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  printf("Init: ..... Started Task3\n");

  rtems_task_wake_after(4);

  //_START task 1
  sc = rtems_task_start(
      ctx->task[0],
      Task_1,
      (rtems_task_argument)ctx);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  printf("Init: ..... Started Task1\n");

  /* Wait on all tasks to run, _DELETE semaphore */
  while (1)
  {
    TestFinished = true;
    for (int i = 1; i < 2; i++)
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
  //locked_print_initialize();
  //PrintSchedInfo(ctx);   //imprime quais cpus têm quais escalonadores
  PrintTaskInfo("Init"); //imprime as informações da task Init
  test(ctx);

  TEST_END();
  rtems_test_exit(0);
}
