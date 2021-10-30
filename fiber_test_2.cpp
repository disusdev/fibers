
#include <stdio.h>
#include <stdlib.h>
#include <emmintrin.h>
#include <functional>
#include <thread>
#include <mutex>
#include <chrono>
#include <Windows.h>

#define JOB_COUNT 100
#define ITERATION_COUNT 1000000
// 1 000 000 SQRTs in 100 JOBs = 100 000 000 SQRTs

struct
context
{
  void *rip = nullptr, *rsp = nullptr;
  void *rbx = nullptr, *rbp = nullptr,
       *r12 = nullptr, *r13 = nullptr,
       *r14 = nullptr, *r15 = nullptr,
       *rdi = nullptr, *rsi = nullptr;
  // windows part
  __m128i xmm6, xmm7, xmm8, xmm9, xmm10,
      xmm11, xmm12, xmm13, xmm14, xmm15;
  // arguments
  void *rcx = nullptr, *rdx = nullptr, *r8 = nullptr;
};

extern "C" void get_context(context *c);
extern "C" void set_context(context *c);
extern "C" void swap_context(context *a, context *b);

struct
fiber
{
  char Data[64 * 1024] = {0};
  context Context = {};
};

struct counter
{
  std::atomic<int> Count = 0;
  context ResumeContext = {};

  void Wait(context* ThreadContext)
  {
    get_context(&ResumeContext);
    if (Count != 0)
    {
      set_context(ThreadContext);
    }
  }

  void Dec()
  {
    Count.fetch_sub(1);

    if (Count == 0)
    {
      set_context(&ResumeContext);
    }
  }
};

typedef void (* job)(context* Ctx, void* Arg, fiber* Fiber);

struct
JobDecl
{
  job Job = nullptr;
  counter* Counter = nullptr;

  bool empty()
  {
    return (void*)Job == nullptr;
  }
};

template <typename Type, unsigned int Size>
struct pool
{
  struct entry
  {
    Type Data = {};
    bool InUse = false;
  };

  entry Buffer[Size];

  Type* Get()
  {
    for (size_t i = 0; i < Size; i++)
    {
      if (!Buffer[i].InUse)
      {
        Buffer[i].InUse = true;
        return &Buffer[i].Data;
      }
    }
    return nullptr;
  }

  void GiveBack(Type* Value)
  {
    size_t index = (entry*)Value - Buffer;
    Buffer[index].InUse = false;
  }
};

#define ROOM 4
template <typename Type, unsigned int Size>
struct queue
{
  struct entry
  {
    Type Data;
    int Gen;
  };

  struct geni
  {
    int val = 0, Gen = 0;
    void incr()
    {
      if (++val % Size == 0)
      {
        val = 0;
        Gen++;
      }
    }
    operator int()
    {
      return val;
    }
  };

  std::atomic<entry> Buffer[Size];
  std::atomic<geni> Head = {};
  int Space[ROOM];
  std::atomic<geni> Tail = {};

  bool is_zero(entry& e, int gen)
  {
    return e.Gen == gen && e.Data.empty();
  }

  bool is_data(entry& e, int gen)
  {
    return e.Gen == gen && !e.Data.empty();
  }

  bool enq(Type val)
  {
    int prev = 0;
    entry ent;
    geni tmp;
    geni old = tmp = Tail.load(std::memory_order_relaxed);
    do
    {
      ent = Buffer[tmp].load(std::memory_order_relaxed);
      while (!is_zero(ent, tmp.Gen))
      {
        if (ent.Gen < prev)
        {
          while (!Tail.compare_exchange_weak(old, tmp) && old < tmp);
          return false;
        }
        else tmp.incr();
        if(!ent.Data.empty()) prev = ent.Gen;
      }
    } while (!Buffer[tmp].compare_exchange_strong(ent, { val, tmp.Gen }, std::memory_order_release));
    tmp.incr();
    while (!Tail.compare_exchange_weak(old, tmp) && old < tmp);
    return true;
  }

  Type deq()
  {
    entry ent;
    geni tmp;
    geni old = tmp = Head.load(std::memory_order_relaxed);
    do
    {
      ent = Buffer[tmp].load(std::memory_order_relaxed);
      while (!is_data(ent, tmp.Gen))
      {
        if (ent.Gen == tmp.Gen)
        {
          while (!Head.compare_exchange_weak(old, tmp) && old < tmp);
          return Type();
        }
        else tmp.incr();
      }
    } while (!Buffer[tmp].compare_exchange_strong(ent, { Type(), tmp.Gen + 1 }, std::memory_order_acquire));
    tmp.incr();
    while (!Head.compare_exchange_weak(old, tmp) && old < tmp);
    return ent.Data;
  }
};

static pool<fiber, 160> Fibers;
static queue<JobDecl, 128> JobQueue;

static void
StartFiber(fiber* Fiber, JobDecl Job)
{
  context Ctx = {};

  Fiber->Context = { 0 };

  char* sp = Fiber->Data + sizeof(fiber::Data);
  sp = (char*)((uintptr_t)sp & -16L);

  Fiber->Context.rip = Job.Job;
  Fiber->Context.rsp = (void*)sp;
  Fiber->Context.rcx = &Ctx;
  Fiber->Context.rdx = Job.Counter;
  Fiber->Context.r8  = Fiber;

  swap_context(&Ctx, &Fiber->Context);
}

static int g_TimesCall = 0;

#define JOB_ENTERY_POINT(funcName)\
void funcName(context* Ctx, void* Arg, fiber* Fiber)
#define JOB_END_POINT()\
if (Arg != nullptr)\
{counter* p_Counter = (counter*)Arg; g_TimesCall++; p_Counter->Dec();}\
Fibers.GiveBack(Fiber);\
set_context(Ctx);

DWORD WINAPI mwork(LPVOID lpParameter)
{
  DWORD core = GetCurrentProcessorNumber();

  while (true)
  {
    JobDecl Job = JobQueue.deq();
    if (Job.empty())
    {
      continue;
    }
    else
    {
      fiber* Fiber = Fibers.Get();
      StartFiber(Fiber, Job);
    }
  }

  return 0;
}

JOB_ENTERY_POINT(sqrt_me)
{
  for (size_t i = 0; i < ITERATION_COUNT; i++)
  {
    double asdas = std::sqrt(0.5);
  }
  
  JOB_END_POINT();
}

JOB_ENTERY_POINT(init_job)
{
  counter Counter = {};
  Counter.Count = JOB_COUNT;

  // create jobs array on stack, and pass pointers to queue !

  for (size_t i = 0; i < JOB_COUNT; i++)
  {
    JobDecl job = {};
    job.Job = sqrt_me;
    job.Counter = &Counter;
    JobQueue.enq(job);
  }

  Counter.Wait(Ctx);

  exit(0);

  // no need in END_POINT after exit(0)
  // JOB_END_POINT();
}

#define CORE_COUNT 8

int main()
{
  LPVOID worker_threads[CORE_COUNT];

  HANDLE MainThread = OpenThread(THREAD_ALL_ACCESS,
                               FALSE,
                               GetCurrentThreadId());

  for (size_t i = 0; i < CORE_COUNT; i++)
  {
    DWORD id;

    HANDLE thread = CreateThread(NULL, 1 * 1024 * 1024, mwork, &worker_threads[i], 0x00000004, &id);

    SetThreadAffinityMask(thread, ((uint64_t)1 << i));

    worker_threads[i] = thread;

    ResumeThread(thread);
  }

  {
    JobDecl Job = {};
    Job.Job = init_job;
    Job.Counter = nullptr;
    JobQueue.enq(Job);
  }

  DWORD exitCode;

  if(GetExitCodeThread(MainThread, &exitCode) != 0)
  {
    ExitThread(exitCode);
    CloseHandle(MainThread);
  }

  return 0;
}