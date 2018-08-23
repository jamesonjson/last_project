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

const char rtems_test_name[] = "SMPMUTEX03";

#define TASK1_PRIO 30

typedef struct {
  rtems_id task[1];
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


rtems_task Test_task(
  rtems_task_argument task_index
)
{

  locked_printf("TEST TASK RUNNING; task_index=%d", (int) task_index);
  char task_name[5];

  /* Show that this task is running on cpu X */
  sprintf( task_name, "TA%" PRIuPTR, (int) task_index );
  PrintTaskInfo( task_name );
  /** 
  TaskRan[task_index] = true;

  
  while(1)
    ;
    */
}

static void test(test_context *ctx)
{
  rtems_status_code sc;

  //criar uma task global
  sc = rtems_task_create(
    rtems_build_name('T', 'A', '1', ' '),
    TASK1_PRIO, 
    RTEMS_MINIMUM_STACK_SIZE,
    RTEMS_DEFAULT_MODES,
    RTEMS_LOCAL,
    &ctx->task[0]
  );
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  sc = rtems_task_start(
    ctx->task[0],
    Test_task,
    (rtems_task_argument) 0
  );
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  
}

rtems_task Init(rtems_task_argument arg)
{
  test_context *ctx = &test_instance;

  TEST_BEGIN();

  test(ctx);

  TEST_END();
  rtems_test_exit(0);
}

