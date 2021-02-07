// #define MSH_STD_INCLUDE_LIBC_HEADERS
#define MSH_JOBS_IMPLEMENTATION
// #define MSH_STD_IMPLEMENTATION
// #include "msh_std.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "experimental/msh_jobs.h"


MSH_JOBS_JOB_SIGNATURE(task_a) 
{
  // msh_sleep(1000);
  Sleep(1000);
  printf("%6d | TASK_A %s\n", thread_idx, (char*)params );
  return 0;
}

int32_t 
main()
{
  msh_jobs_ctx_t work_ctx = {0};
  msh_jobs_init_ctx( &work_ctx, 1 );
  printf("Running on %s\n", msh_jobs_get_platform_name() );
  printf("Spawned Thread Count: %d | Logical Core Count: %d\n", 
          work_ctx.thread_count,
          work_ctx.processor_info.logical_core_count );

  // uint64_t t1, t2;
  // t1 = msh_time_now();

  msh_jobs_push_work( &work_ctx, task_a, "00" );
  msh_jobs_push_work( &work_ctx, task_a, "01" );
  msh_jobs_push_work( &work_ctx, task_a, "02" );
  msh_jobs_push_work( &work_ctx, task_a, "03" );
  msh_jobs_push_work( &work_ctx, task_a, "04" );
  msh_jobs_push_work( &work_ctx, task_a, "05" );
  msh_jobs_push_work( &work_ctx, task_a, "06" );
  msh_jobs_push_work( &work_ctx, task_a, "07" );
  msh_jobs_push_work( &work_ctx, task_a, "08" );
  msh_jobs_push_work( &work_ctx, task_a, "09" );
  msh_jobs_push_work( &work_ctx, task_a, "10" );
  msh_jobs_push_work( &work_ctx, task_a, "11" );
  msh_jobs_push_work( &work_ctx, task_a, "12" );
  msh_jobs_push_work( &work_ctx, task_a, "13" );
  msh_jobs_push_work( &work_ctx, task_a, "14" );
  msh_jobs_push_work( &work_ctx, task_a, "15" );
  msh_jobs_push_work( &work_ctx, task_a, "16" );
  msh_jobs_push_work( &work_ctx, task_a, "17" );
  msh_jobs_push_work( &work_ctx, task_a, "18" );
  msh_jobs_push_work( &work_ctx, task_a, "19" );
  msh_jobs_push_work( &work_ctx, task_a, "20" );
  msh_jobs_push_work( &work_ctx, task_a, "21" );
  msh_jobs_push_work( &work_ctx, task_a, "22" );
  msh_jobs_push_work( &work_ctx, task_a, "23" );

  printf("TEST\n");

  msh_jobs_complete_all_work( &work_ctx );
  // t2 = msh_time_now();
  // printf("Time: %f\n", msh_time_diff_ms(t2, t1));

  msh_jobs_term_ctx( &work_ctx );
  


  return 0;
}
