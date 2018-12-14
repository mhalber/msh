#define MSH_JOBS_IMPLEMENTATION
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include "msh/experimental/msh_jobs.h"


#define COUNTOF(x) sizeof(x) / sizeof(x[0])

MSH_JOBS_JOB_SIGNATURE(task_a) 
{
  
  printf("%6d | TASK_A %s\n", thread_idx, (char*)data );
  // Sleep(10);
  return 0;
}



int main()
{
  msh_jobs_ctx_t work_ctx = {0};
  msh_jobs_init_ctx( &work_ctx, 1 );
  printf("%s\n", msh_jobs_get_platform_name() );
  printf("No. of threads: %d\n", work_ctx.thread_count);
  msh_jobs_push_work( &work_ctx, task_a, "0" );
  msh_jobs_push_work( &work_ctx, task_a, "1" );
  msh_jobs_push_work( &work_ctx, task_a, "2" );
  msh_jobs_push_work( &work_ctx, task_a, "3" );
  msh_jobs_push_work( &work_ctx, task_a, "4" );
  msh_jobs_push_work( &work_ctx, task_a, "5" );
  msh_jobs_push_work( &work_ctx, task_a, "6" );
  msh_jobs_push_work( &work_ctx, task_a, "7" );
  msh_jobs_push_work( &work_ctx, task_a, "8" );
  msh_jobs_push_work( &work_ctx, task_a, "9" );
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

  msh_jobs_wait_for_all_to_finish( &work_ctx );
  msh_jobs_term_ctx( &work_ctx );
  return 0;
}
