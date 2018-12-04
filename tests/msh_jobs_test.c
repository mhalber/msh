#define MSH_JOBS_IMPLEMENTATION
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "msh/experimental/msh_jobs.h"

#define COUNTOF(x) sizeof(x) / sizeof(x[0])

uint32_t task_a( void* params )
{
  printf("TASK_A %s\n", (char*)params );
  Sleep(10);
  return 0;
}

uint32_t task_b( void* params )
{
  printf("TASK_B %s\n", (char*)params );
  Sleep(10);
  return 0;
}

uint32_t task_c( void* params )
{
  printf("TASK_C %s\n", (char*)params );
  Sleep(10);
  return 0;
}



int main()
{ 
  msh_jobs_ctx_t work_queue = {0};
  msh_jobs_init_ctx( &work_queue );

  msh_jobs_push_work( &work_queue, task_a, "1" );
  msh_jobs_push_work( &work_queue, task_c, "2" );
  msh_jobs_push_work( &work_queue, task_b, "3" );
 
  msh_jobs_wait_for_all_to_finish( &work_queue );
  
  return 0;
}