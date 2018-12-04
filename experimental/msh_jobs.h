#ifndef MSH_JOBS
#define MSH_JOBS

#define MSH_JOBS_PLATFORM_WINDOWS 0
#define MSH_JOBS_PLATFORM_LINUX 0
#define MSH_JOBS_PLATFORM_MACOS 0

// TODO(maciej): include correct headers depending on the platform
#if defined(_WIN32) || defined(_WIN64)
#undef  MSH_JOBS_PLATFORM_WINDOWS
#define MSH_JOBS_PLATFORM_WINDOWS 1
#elif defined(__linux__)
#undef MSH_JOBS_PLATFORM_LINUX
#define MSH_JOBS_PLATFORM_LINUX 1
#elif defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__)
#undef MSH_JOBS_PLATFORM_MACOS
#define MSH_JOBS_PLATFORM_MACOS 1
#endif
#define BX_PLATFORM_POSIX (0 || BX_PLATFORM_MACOS || BX_PLATFORM_LINUX)

#ifdef MSH_JOBS_PLATFORM_WINDOWS
#define MSH_JOBS_PLATFORM_NAME "windows"
#elif MSH_JOBS_PLATFORM_POSIX
#define MSH_JOBS_PLATFORM_NAME "posix"
#else 
#error "MSH_JOBS: Compiling on unknown platform!"
#endif

#if MSH_JOBS_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN 1
#include "windows.h"
typedef struct msh_jobs_thread_handle
{
  uint32_t thread_idx;
  HANDLE handle;
} msh_jobs_thread_handle_t;
#endif

// NOTE(maciej): What other info about the processor we might want to have?
typedef struct msh_jobs_processor_info
{
  int32_t logical_core_count;
  int32_t physical_core_count;
} msh_jobs_processor_info_t;

typedef struct msh_jobs_job_entry
{
  uint32_t entry_idx;
  uint32_t (*task)(void*);
  void*  task_params;
} msh_jobs_job_entry_t;

// NOTE(maciej): This should probably be not returned to user.
typedef struct msh_jobs_ctx
{
  msh_jobs_job_entry_t* job_queue;
  volatile int32_t current_job_idx;
  volatile int32_t jobs_to_do;
  volatile int32_t max_jobs;

  msh_jobs_processor_info_t processor_info;

  msh_jobs_thread_handle_t* thread_handles;
  uint32_t thrd_handles_count;
} msh_jobs_ctx_t;

char* msh_jobs_get_platform_name();
int msh_jobs_get_processor_info( msh_jobs_processor_info_t* info );
int msh_jobs_init_ctx( msh_jobs_ctx_t* ctx );

#endif /* MSH_JOBS */

#ifdef MSH_JOBS_IMPLEMENTATION

typedef enum msh_jobs_error_codes
{
  MSH_JOBS_NO_ERR = 0
} msh_jobs_error_codes_t;


char* msh_jobs_get_platform_name()
{
  return MSH_JOBS_PLATFORM_NAME;
}

// from https://docs.microsoft.com/en-us/windows/desktop/api/sysinfoapi/nf-sysinfoapi-getlogicalprocessorinformation
DWORD msh_jobs__count_set_bits( ULONG_PTR bit_mask )
{
  DWORD LSHIFT = sizeof(ULONG_PTR)*8 - 1;
  DWORD bit_set_count = 0;
  ULONG_PTR bit_test = (ULONG_PTR)1 << LSHIFT;

  for( DWORD i = 0; i <= LSHIFT; ++i)
  {
    bit_set_count += ((bit_mask & bit_test) ? 1 : 0 );
    bit_test >>= 1;
  }

  return bit_set_count;
}

int32_t msh_jobs_get_processor_info( msh_jobs_processor_info_t* info  )
{
  int32_t error = MSH_JOBS_NO_ERR;
#if MSH_JOBS_PLATFORM_WINDOWS

  PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
  PCACHE_DESCRIPTOR Cache;
  DWORD buffer_byte_size = 0;
  BOOL success = 0;

  // TODO(maciej): Introduce proper error handling
  success = GetLogicalProcessorInformation( buffer, &buffer_byte_size );
  buffer  = malloc(buffer_byte_size);
  success = GetLogicalProcessorInformation( buffer, &buffer_byte_size  );
  if( !success ) { error = 1; goto msh_jobs_get_processor_info_exit; } 
  
  PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = buffer;
  int32_t byte_offset = 0;
  info->logical_core_count = 0;
  info->physical_core_count = 0;
  while( byte_offset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= buffer_byte_size ) 
  {
    switch( ptr->Relationship )
    {
      case RelationProcessorCore:
        info->physical_core_count++;
        info->logical_core_count += msh_jobs__count_set_bits( ptr->ProcessorMask );
        break;
      default:
       break;
    }
    byte_offset += sizeof( SYSTEM_LOGICAL_PROCESSOR_INFORMATION );
    ptr++;
  }
  free( buffer );
#endif

msh_jobs_get_processor_info_exit:
  return error;
}



// NOTE(maciej): This should probably acquire a semaphore...
int32_t
msh_jobs_push_work( msh_jobs_ctx_t* ctx, uint32_t (*task)(void*), void* task_params )
{
  uint32_t cur_idx = ctx->current_job_idx;
  ctx->job_queue[ cur_idx ].task = task;
  ctx->job_queue[ cur_idx ].task_params = task_params;
  ctx->current_job_idx++;
  return MSH_JOBS_NO_ERR;
}

// NOTE(maciej): How to ensure that this waits for jobs
long unsigned int 
msh_jobs_do_work( void* params )
{
  msh_jobs_ctx_t* ctx = (msh_jobs_ctx_t*)params;
  msh_jobs_job_entry_t* jobs = ctx->job_queue;

  int32_t cur_idx = ctx->current_job_idx;
  // if( cur_idx < ctx->jobs_to_do )
  // {
    // jobs[cur_idx].task( jobs[cur_idx].task_params );
    // cur_idx 
    // ctx->jobs_to_do--;
  // }
  // else
  // {
  //   //NOTE(maciej): Here we should signal to os that this thread can sleep, but we do not want to
  //   // destroy thread
  // }
  return 0;
}

int32_t
msh_jobs__create_threads( msh_jobs_ctx_t* ctx )
{
  assert( ctx->processor_info.logical_core_count > 0 );
#if MSH_JOBS_PLATFORM_WINDOWS
  uint32_t logical_core_count = 1;//ctx->processor_info.logical_core_count;
  ctx->thread_handles = malloc( logical_core_count * sizeof( msh_jobs_thread_handle_t ) );
  assert( ctx->thread_handles );
  for( uint32_t thrd_idx = 0; thrd_idx < logical_core_count; ++thrd_idx )
  {
    // NOTE(maciej): We want this threads to go to sleep imediatelly, if the queue is empty
    ctx->thread_handles[thrd_idx].thread_idx = thrd_idx;
    HANDLE handle = CreateThread( 0, 0, msh_jobs_do_work, ctx, 0, 0 );
    ctx->thread_handles[thrd_idx].handle = handle;
  }

  // NOTE(maciej): Is this sufficient?
  ctx->max_jobs = 256;
  ctx->current_job_idx = 0;
  ctx->job_queue = malloc( ctx->max_jobs * sizeof( msh_jobs_job_entry_t) );

#endif
  return MSH_JOBS_NO_ERR;
}

int32_t
msh_jobs_wait_for_all_to_finish( msh_jobs_ctx_t* ctx )
{
  return MSH_JOBS_NO_ERR;
}

int32_t
msh_jobs_init_ctx( msh_jobs_ctx_t* ctx )
{
  int32_t err = MSH_JOBS_NO_ERR;
#if MSH_JOBS_PLATFORM_WINDOWS
  msh_jobs_get_processor_info( &ctx->processor_info );
  msh_jobs__create_threads( ctx );
#endif
  return err;
}

#endif /* MSH_JOBS_IMPLEMENTATION */