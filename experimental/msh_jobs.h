// NOTE(maciej): This is very closely modelled after Casey Muratori's work queue, designed
// in handmade hero streams.
// Credits : bgfx for platform detection

/* TODOs:
[ ] If just one thread is requested, we should just use main thread.
[x] When enqueing wait if the queue is full - Remove spin printf
[ ] Malloc overwrites
[ ] Header / docs
[ ] Error handling - thread detaching?

[ ] Remove deadlock if a queue size == 1
[ ] Add Multiple priority queuesg
[ ] Improve the test code
[x] Implement functions wrappers fro winapi
[x] Implement function wrappers for posix
[ ] Multiple Producer / Multiple Consumer Queues?
[ ] Async reading - check sokol async
[ ] Avoid recompiling extra code if msh_std is present 
[ ] Compare to fibers: https://github.com/JodiTheTigger/sewing

[ ] Whenever MSVC adds stdatomic.h and threads.h, use these instead (?)

Some extra meterials:
https://preshing.com/20120612/an-introduction-to-lock-free-programming/
https://gdcvault.com/play/1022186/Parallelizing-the-Naughty-Dog-Engine
http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue
*/

#ifndef MSH_JOBS
#define MSH_JOBS

#define MSH_JOBS_QUEUE_SIZE 1024
#define MSH_JOBS_DEFAULT_THREAD_COUNT 0
#define MSH_JOBS_JOB_SIGNATURE(name) uint32_t name(int thread_idx, void* params)
typedef uint32_t (*msh_jobs_job_signature_t)( int thread_idx, void* data);

#define MSH_JOBS_PLATFORM_WINDOWS 0
#define MSH_JOBS_PLATFORM_LINUX 0
#define MSH_JOBS_PLATFORM_MACOS 0

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
#define MSH_JOBS_PLATFORM_POSIX (0 || MSH_JOBS_PLATFORM_MACOS || MSH_JOBS_PLATFORM_LINUX )

#if MSH_JOBS_PLATFORM_WINDOWS
#define MSH_JOBS_PLATFORM_NAME (char*)"windows"
#elif MSH_JOBS_PLATFORM_LINUX
#define MSH_JOBS_PLATFORM_NAME (char*)"linux"
#elif MSH_JOBS_PLATFORM_MACOS
#define MSH_JOBS_PLATFORM_NAME (char*)"macos"
#else 
#error "MSH_JOBS: Compiling on unknown platform!"
#endif

#if MSH_JOBS_PLATFORM_WINDOWS

#ifdef __cplusplus
#include "intrin.h"
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "windows.h"

#elif MSH_JOBS_PLATFORM_LINUX
#include <pthread.h>    // threads
#include <semaphore.h>  // semaphore
#include <x86intrin.h>  // fences
#include <unistd.h>     // sysconf

#elif MSH_JOBS_PLATFORM_MACOS
#include <pthread.h>    // threads
#include <semaphore.h>  // semaphore
#include <x86intrin.h>  // fences
#include <unistd.h>     // sysconf
#include <sys/sysctl.h>
#else
#error "MSH_JOBS: Platform not supported!"
#endif

#if MSH_JOBS_PLATFORM_WINDOWS

typedef HANDLE msh_jobs_semaphore_t;
typedef HANDLE msh_jobs_thread_t;
#define MSH_JOBS_READ_WRITE_BARRIER() _mm_mfence(); _ReadWriteBarrier()
#define MSH_JOBS_READ_BARRIER() _mm_rfence(); _ReadBarrier()
#define MSH_JOBS_WRITE_BARRIER() _mm_sfence(); _WriteBarrier()
typedef DWORD (*msh_jobs_thrd_proc_t)(void *params);

#elif MSH_JOBS_PLATFORM_POSIX

typedef sem_t msh_jobs_semaphore_t;
typedef pthread_t msh_jobs_thread_t;
#define MSH_JOBS_READ_WRITE_BARRIER() _mm_fence(); __asm__ volatile("" ::: "memory")
#define MSH_JOBS_READ_BARRIER() _mm_rfence(); __asm__ volatile("" ::: "memory")
#define MSH_JOBS_WRITE_BARRIER() _mm_sfence(); __asm__ volatile("" ::: "memory")
typedef void* (*msh_jobs_thrd_proc_t)(void *params);

#endif


// NOTE(maciej): What other info about the processor we might want to have?
typedef struct msh_jobs_processor_info
{
  uint32_t logical_core_count;
} msh_jobs_processor_info_t;

typedef struct msh_jobs_job_entry
{
  uint32_t (*task)(int32_t, void*);
  void* data;
} msh_jobs_job_entry_t;

typedef struct msh_jobs_work_queue
{
  uint32_t volatile completion_count;
  uint32_t volatile completion_goal;
  uint32_t volatile next_entry_to_write;
  uint32_t volatile next_entry_to_read;

  uint32_t volatile max_job_count;
  msh_jobs_job_entry_t* entries;
  msh_jobs_semaphore_t semaphore_handle;
} msh_jobs_work_queue_t;

struct msh_jobs_thread_into;

typedef struct msh_jobs_ctx
{
  msh_jobs_processor_info_t processor_info;
  msh_jobs_work_queue_t queue;

  struct msh_jobs_thread_info* thread_infos;
  uint32_t thread_count;
} msh_jobs_ctx_t;

typedef struct msh_jobs_thread_info
{
  msh_jobs_ctx_t *ctx;
  uint32_t idx;
  msh_jobs_thread_t handle;
} msh_jobs_thread_info_t;


char*   msh_jobs_get_platform_name();
int32_t msh_jobs_get_processor_info( msh_jobs_processor_info_t* info );
int32_t msh_jobs_init_ctx( msh_jobs_ctx_t* ctx, uint32_t n_threads );
int32_t msh_jobs_push_work( msh_jobs_ctx_t* ctx, msh_jobs_job_signature_t task, void* data );
void    msh_jobs_complete_all_work( msh_jobs_ctx_t* ctx );
void    msh_jobs_term_ctx( msh_jobs_ctx_t* ctx );

// sew_stitches_and_wait(sewing, jobs, 10); //-> Nice api, PAss array of jobs and run
// job structure

#endif /* MSH_JOBS */



#ifdef MSH_JOBS_IMPLEMENTATION

typedef enum msh_jobs_error_codes
{
  MSH_JOBS_NO_ERR = 0,
  MSH_JOBS_FAILED_TO_CREATE_THREAD = 1,
  MSH_JOBS_FAILED_TO_CREATE_SEMAPHORE = 2,
  MSH_JOBS_OUT_OF_MEMORY = 3,
} msh_jobs_error_codes_t;

int32_t
msh_jobs_thread_create( msh_jobs_thread_t* thread, msh_jobs_thrd_proc_t func, void* params )
{
  int32_t err = MSH_JOBS_NO_ERR;

#if MSH_JOBS_PLATFORM_WINDOWS
  *thread = CreateThread( NULL, 0, func, params, 0, NULL );
  if (*thread == NULL) { err = MSH_JOBS_FAILED_TO_CREATE_THREAD; }
#else
  int32_t pthread_err = pthread_create( thread, NULL, func, params );
  if (pthread_err) { err = MSH_JOBS_FAILED_TO_CREATE_THREAD; }
#endif

  return err;
}

void
msh_jobs_thread_detach( msh_jobs_thread_t *thread )
{
#if MSH_JOBS_PLATFORM_WINDOWS
  CloseHandle( *thread );
#else
  pthread_detach( *thread );
#endif
}

int32_t
msh_jobs_semaphore_create( msh_jobs_semaphore_t* sem, uint32_t initial_count, uint32_t maximum_count )
{
  int32_t err = MSH_JOBS_NO_ERR;
#if MSH_JOBS_PLATFORM_WINDOWS
  *sem = CreateSemaphoreA( NULL, initial_count, maximum_count, NULL );
  if (*sem == NULL) { err = MSH_JOBS_FAILED_TO_CREATE_SEMAPHORE; }
#else
  (void)initial_count;
  (void)maximum_count;
  int32_t pthread_err = sem_init( sem, 0, 0 );
  if (pthread_err) { err = MSH_JOBS_FAILED_TO_CREATE_SEMAPHORE; }
#endif
  return err;
}

void
msh_jobs_semaphore_destroy( msh_jobs_semaphore_t* sem )
{
#if MSH_JOBS_PLATFORM_WINDOWS
  CloseHandle( *sem );
#else
  sem_destroy( sem );
#endif
}

void
msh_jobs_semaphore_release( msh_jobs_semaphore_t* sem, int32_t release_count )
{
#if MSH_JOBS_PLATFORM_WINDOWS
  ReleaseSemaphore( *sem, release_count, NULL );
#else
  (void)release_count;
  sem_post( sem );
#endif
}

void
msh_jobs_semaphore_wait( msh_jobs_semaphore_t* sem )
{
#if MSH_JOBS_PLATFORM_WINDOWS
  WaitForSingleObjectEx( *sem, INFINITE, FALSE );
#else
  sem_wait( sem );
#endif
}

uint32_t
msh_jobs_atomic_add( uint32_t volatile *value, uint32_t val )
{
#if MSH_JOBS_PLATFORM_WINDOWS
  return (uint32_t)InterlockedAdd( (LONG volatile*)value, val );
#else
  return (uint32_t)__sync_fetch_and_add( value, val );
#endif
}

uint32_t
msh_jobs_atomic_increment( uint32_t volatile *value )
{
#if MSH_JOBS_PLATFORM_WINDOWS
  return (uint32_t)InterlockedIncrement( (LONG volatile*)value );
#else
  return (uint32_t)__sync_fetch_and_add( value, 1 );
#endif
}

uint32_t
msh_jobs_atomic_compare_exchange( uint32_t volatile *dest, uint32_t new_val, uint32_t old_val )
{
#if MSH_JOBS_PLATFORM_WINDOWS
  return (uint32_t)InterlockedCompareExchange( (LONG volatile *)dest, new_val, old_val );
#else
  return (uint32_t)__sync_val_compare_and_swap( dest, old_val, new_val );
#endif
}

void
msh_jobs__sleep(uint64_t ms) {
#if MSH_JOBS_PLATFORM_WINDOWS
  Sleep((DWORD)ms);
#elif MSH_JOBS_PLATFORM_POSIX
  usleep(1000 * ms);
#endif
}

int32_t
msh_jobs_push_work( msh_jobs_ctx_t* ctx, msh_jobs_job_signature_t task, void* data )
{
  msh_jobs_work_queue_t* queue = &ctx->queue;
  uint32_t next_entry_to_write = queue->next_entry_to_write;
  uint32_t new_next_entry_to_write = (next_entry_to_write + 1) % queue->max_job_count;
  while( new_next_entry_to_write == queue->next_entry_to_read ) { msh_jobs__sleep(1); };// Spin until we can write again
  msh_jobs_job_entry_t *job = queue->entries + next_entry_to_write;
  job->task = task;
  job->data = data;
  ++queue->completion_goal;
  MSH_JOBS_WRITE_BARRIER();
  queue->next_entry_to_write = new_next_entry_to_write;
  msh_jobs_semaphore_release( &queue->semaphore_handle, 1 );
  return MSH_JOBS_NO_ERR;
}

int32_t
msh_jobs_execute_next_job_entry( int32_t thread_idx, msh_jobs_work_queue_t* queue )
{
  int32_t should_sleep = false;
  if (!queue->entries ||
       queue->completion_goal == 0 || 
       queue->max_job_count == 0) { return true; }

  uint32_t original_next_entry_to_read = queue->next_entry_to_read;
  uint32_t new_next_entry_to_read = (original_next_entry_to_read + 1) % queue->max_job_count;
  if( original_next_entry_to_read != queue->next_entry_to_write )
  {
    // 'increment' queue->next_entry_to_read here
    uint32_t idx = msh_jobs_atomic_compare_exchange( &queue->next_entry_to_read,
                                                     new_next_entry_to_read,
                                                     original_next_entry_to_read );
    if (idx == original_next_entry_to_read)
    {
      msh_jobs_job_entry_t* entry = queue->entries + idx;
      entry->task( thread_idx, entry->data );
      msh_jobs_atomic_increment( &queue->completion_count );
    }
  }
  else
  {
    should_sleep = true;
  }
  return should_sleep;
}

void
msh_jobs_complete_all_work( msh_jobs_ctx_t* ctx )
{
  uint32_t thrd_idx = 0;
  if( !ctx->thread_infos || !ctx->queue.entries ) 
  { 
    return;
  }

  while( ctx->queue.completion_goal != ctx->queue.completion_count )
  {
    msh_jobs_execute_next_job_entry( thrd_idx, &ctx->queue );
  }
  
  ctx->queue.completion_goal = 0;
  ctx->queue.completion_count = 0;
}

#if MSH_JOBS_PLATFORM_WINDOWS
DWORD WINAPI msh_jobs_thread_procedure(void *params)
#else
void* msh_jobs_thread_procedure(void* params )
#endif
{
  msh_jobs_thread_info_t* ti = (msh_jobs_thread_info_t*)params;
  msh_jobs_ctx_t* ctx = ti->ctx;
  uint32_t thrd_idx = ti->idx;
  for(;;)
  {
    if( msh_jobs_execute_next_job_entry( thrd_idx, &ctx->queue ) )
    {
      msh_jobs_semaphore_wait( &ctx->queue.semaphore_handle );
    }
  }
  return 0;
}

int32_t
msh_jobs__init_queue( msh_jobs_ctx_t* ctx, uint32_t queue_size )
{
  ctx->queue.max_job_count = queue_size;
  ctx->queue.completion_goal = 0;
  ctx->queue.completion_count = 0;
  ctx->queue.next_entry_to_read = 0;
  ctx->queue.next_entry_to_write = 0;
  ctx->queue.entries = (msh_jobs_job_entry_t*)malloc( ctx->queue.max_job_count * sizeof(msh_jobs_job_entry_t) );
  if (!ctx->queue.entries) { return MSH_JOBS_OUT_OF_MEMORY; }

  return MSH_JOBS_NO_ERR;
}


int32_t
msh_jobs__create_threads( msh_jobs_ctx_t* ctx, uint32_t n_threads )
{
  assert( ctx->processor_info.logical_core_count > 0 );
  int32_t err = MSH_JOBS_NO_ERR;
  
  // Semaphore
  uint32_t initial_count = 0;
  ctx->thread_count = n_threads ? n_threads : ctx->processor_info.logical_core_count - 1;
  err = msh_jobs_semaphore_create( &ctx->queue.semaphore_handle, initial_count, ctx->thread_count );
  if (err) { return err; }

  // Thread info
  ctx->thread_infos = (msh_jobs_thread_info_t*)malloc( ctx->thread_count * sizeof( msh_jobs_thread_info_t ) );
  if (!ctx->thread_infos) { err = MSH_JOBS_OUT_OF_MEMORY; return err; }
  
  for( uint32_t thrd_idx = 0; thrd_idx < ctx->thread_count; ++thrd_idx )
  {
    ctx->thread_infos[thrd_idx].idx = thrd_idx + 1;
    ctx->thread_infos[thrd_idx].ctx = ctx;
    err = msh_jobs_thread_create( &ctx->thread_infos[thrd_idx].handle,
                                  msh_jobs_thread_procedure, &ctx->thread_infos[thrd_idx] );
    if (err) { return err; }
  }

  return err;
}

int32_t
msh_jobs_init_ctx( msh_jobs_ctx_t* ctx, uint32_t n_threads )
{
  int32_t err = MSH_JOBS_NO_ERR;
  err = msh_jobs_get_processor_info( &ctx->processor_info );
  if( err ) { return err; }
  
  err = msh_jobs__init_queue( ctx, MSH_JOBS_QUEUE_SIZE );
  if( err ) { return err; }
  
  err = msh_jobs__create_threads( ctx, n_threads );
  if( err ) { return err; }
  
  return err;
}

void
msh_jobs_term_ctx( msh_jobs_ctx_t* ctx )
{
  msh_jobs_complete_all_work( ctx );
  for( uint32_t i = 0; i < ctx->thread_count; ++i )
  {
    msh_jobs_thread_detach( &ctx->thread_infos[i].handle );
  }
  free( ctx->thread_infos );
  ctx->thread_infos = NULL;
  ctx->queue.max_job_count = 0;
  ctx->queue.completion_goal = 0;
  ctx->queue.completion_count = 0;
  ctx->queue.next_entry_to_read = 0;
  ctx->queue.next_entry_to_write = 0;
  free( ctx->queue.entries );
  ctx->queue.entries = NULL;
  msh_jobs_semaphore_release( &ctx->queue.semaphore_handle, ctx->thread_count );
  msh_jobs_semaphore_destroy( &ctx->queue.semaphore_handle );
}

char* 
msh_jobs_get_platform_name()
{
  return MSH_JOBS_PLATFORM_NAME;
}

int32_t 
msh_jobs_get_processor_info( msh_jobs_processor_info_t* info  )
{
  int32_t error = MSH_JOBS_NO_ERR;
#if MSH_JOBS_PLATFORM_WINDOWS

  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  info->logical_core_count = sysinfo.dwNumberOfProcessors;

#elif MSH_JOBS_PLATFORM_LINUX

  info->logical_core_count = sysconf( _SC_NPROCESSORS_ONLN );
  goto msh_jobs_get_processor_info_exit;

#elif MSH_JOBS_PLATFORM_MACOS

  #warning "NOT TESTED ON MACOS - ASSUME IT DOES NOT WORK!"
  size_t physical_count_len = sizeof(count);
  sysctlbyname("hw.logicalcpu", &info->logical_core_count, &logical_count_len, NULL, 0);

#endif

  return error;
}

#endif /* MSH_JOBS_IMPLEMENTATION */
