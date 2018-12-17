#define MSH_JOBS_IMPLEMENTATION
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "msh/experimental/msh_jobs.h"


#define COUNTOF(x) sizeof(x) / sizeof(x[0])

MSH_JOBS_JOB_SIGNATURE(task_a) 
{
#if MSH_JOBS_PLATFORM_WINDOWS
  Sleep(1000);
#else
  sleep(1);
#endif
  printf("%6d | TASK_A %s\n", thread_idx, (char*)params );
  return 0;
}

MSH_JOBS_JOB_SIGNATURE(task_b)
{
#if MSH_JOBS_PLATFORM_WINDOWS
  Sleep(2000);
#else
  sleep(2);
#endif
  printf("%6d | TASK_B %s\n", thread_idx, (char*)params );
  return 0;
}
int main()
{
  msh_jobs_ctx_t work_ctx = {0};
  msh_jobs_init_ctx( &work_ctx, MSH_JOBS_DEFAULT_THREAD_COUNT );
  printf("Running on %s\n", msh_jobs_get_platform_name() );
  printf("Spawned Thread Count: %d | Logical Core Count: %d\n", 
          work_ctx.thread_count,
          work_ctx.processor_info.logical_core_count );

  msh_jobs_push_work( &work_ctx, task_a, "0" );
  msh_jobs_push_work( &work_ctx, task_b, "1" );
  msh_jobs_push_work( &work_ctx, task_a, "2" );
  msh_jobs_push_work( &work_ctx, task_b, "3" );
  msh_jobs_push_work( &work_ctx, task_a, "4" );
  msh_jobs_push_work( &work_ctx, task_b, "5" );
  msh_jobs_push_work( &work_ctx, task_a, "6" );
  msh_jobs_push_work( &work_ctx, task_b, "7" );
  msh_jobs_push_work( &work_ctx, task_a, "8" );
  msh_jobs_push_work( &work_ctx, task_b, "9" );

  msh_jobs_wait_for_all_to_finish( &work_ctx );
  msh_jobs_term_ctx( &work_ctx );
  
  return 0;
}
