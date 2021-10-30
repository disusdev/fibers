
#include <stdio.h>
#include <stdlib.h>

#include <queue>
#include <vector>
#include <atomic>

#include <mutex>
#include <condition_variable>

// windows
#include <emmintrin.h>

#include <Windows.h>

#include <cmath>

// A threadsafe-queue.
template <class T>
class SafeQueue
{
public:
  SafeQueue(void)
    : q()
    , m()
    , c()
  {}

  ~SafeQueue(void)
  {}

  int size()
  {
    return q.size();
  }

  void Prepare()
  {
    prepared = true;
  }

  bool IsPrepared()
  {
    return prepared;
  }

  // Add an element to the queue.
  void enqueue(T t)
  {
    std::lock_guard<std::mutex> lock(m);
    q.push(t);
    c.notify_one();
  }

  // Get the "front"-element.
  // If the queue is empty, wait till a element is avaiable.
  T dequeue(void)
  {
    std::unique_lock<std::mutex> lock(m);
    while(q.empty())
    {
      // release lock as long as the wait and reaquire it afterwards.
      c.wait(lock);
    }
    T val = q.front();
    q.pop();
    return val;
  }

private:
  std::queue<T> q;
  mutable std::mutex m;
  std::condition_variable c;
  bool prepared = false;
};

struct
context
{
  void *rip, *rsp;
  void *rbx, *rbp, *r12, *r13, *r14, *r15, *rdi, *rsi;
  // windows
  __m128i xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
};

typedef void* job;

typedef std::atomic<int> counter;

// counter g_Counters[10];

counter g_Counter(0);

int g_Int = 0;

static double fooki_val = 0.0;

void fooki()
{
  double k0 = 2.0;
  double nu = 3.0;
  double lat = 4.0;
  double esq = 5.0;

  for (size_t i = 0; i < 100; i++)
  {
    int oki_val = ((k0 * nu /** std::pow(std::cos(lat),3)*/)/6) * (1 - /*std::pow(std::tan(lat),2) +*/ (esq /* * std::pow(std::cos(lat),2)*/));
  }

  //g_Counter--;

  //exit(1);
}

void main_worker()
{
  

  exit(1);
}

void foo()
{
  exit(0);
}

extern "C" void get_context(context *c);
extern "C" void set_context(context *c);
extern "C" void swap_context(context *a, context *b);


void WaitForCounter(counter* Counter, int Count)
{
  //Counter->wait(Count);
  while (Counter->load() != 0);
}

#define JOB_ENTERY_POINT(funcName) void funcName(void* Argument)

JOB_ENTERY_POINT(AnimateObjectJob)
{

}

struct JobDecl
{
  job Job;
  counter* Counter = nullptr;

  JobDecl(job Job = nullptr, void* Argument = nullptr)
  {
    this->Job = Job;
  }
};

struct
job_system
{
  SafeQueue<JobDecl*> Low;
  SafeQueue<JobDecl*> Normal;
  SafeQueue<JobDecl*> High;
};

job_system JobSystem;

void RunJobs(JobDecl* Jobs, int Count, counter* Counter)
{
  Counter->store(Count);

  for (size_t i = 0; i < Count; i++)
  {
    Jobs[i].Counter = Counter;
    JobSystem.High.enqueue(&Jobs[i]);
  }

  JobSystem.High.Prepare();
}

//void PipelineFoo()
//{
//  JobDecl jobDecls[100];
//  for (int i = 0; i < 100; i++)
//  {
//    jobDecls[i].Job = foo;
//  }
//  counter* Counter = nullptr;
//  RunJobs(jobDecls, 100, Counter);
//  WaitForCounter(Counter, 0);
//}

void PipelineFooki()
{
  JobDecl jobDecls[100];
  for (int i = 0; i < 100; i++)
  {
    jobDecls[i] = JobDecl(fooki);
  }
  //counter* Counter = nullptr;
  counter* Counter = &g_Counter;
  RunJobs(jobDecls, 100, Counter);
  WaitForCounter(Counter, 0);
}

struct fiber
{
  char data[4096];
  context Context;
  bool InUse = false;

  static void
  create(fiber* Fiber, job Job, void* Arguments = nullptr, int SizeOfArguments = 0)
  {
    char *sp = (char*)(Fiber->data + sizeof fiber::data);
    sp = (char*)((uintptr_t)sp & -16L);

    Fiber->Context = {0};
    Fiber->Context.rip = Job;
    Fiber->Context.rsp = (void*)sp;

    //if (SizeOfArguments > 0)
    //  memcpy(sp, Arguments, SizeOfArguments);
  }

  static void
  exec(fiber* Fiber)
  {
    //context c;
    //get_context(&c);
    //swap_context(&c, &Fiber->Context);
    
    set_context(&Fiber->Context);
  }
};


fiber g_Fibers[1000];

struct
worker
{
  HANDLE Thread;
  fiber* CurrentFiber;

  static fiber* GetFiber()
  {
    for (size_t i = 0; i < 1000; i++)
    {
      if (!g_Fibers[i].InUse) return &g_Fibers[i];
    }
    return nullptr;
  }
};

worker worker_threads[16];
HANDLE threads[16];

// void work(worker* worker)
DWORD WINAPI work(LPVOID lpParameter)
{
  worker* Worker = static_cast<worker*>(lpParameter);
  while (true)
  {
    if (JobSystem.High.IsPrepared() && (JobSystem.High.size() > 0))
    {
      JobDecl* Job = JobSystem.High.dequeue();

      fiber* Fiber = worker::GetFiber();

      Worker->CurrentFiber = Fiber;

      fiber::create(Worker->CurrentFiber, Job->Job);

      fiber::exec(Worker->CurrentFiber);
    }
  }
}

typedef BOOL(WINAPI* LPFN_GLPI)(
  PSYSTEM_LOGICAL_PROCESSOR_INFORMATION,
  PDWORD);

DWORD CountSetBits(ULONG_PTR bitMask)
{
  DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
  DWORD bitSetCount = 0;
  ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;
  DWORD i;

  for (i = 0; i <= LSHIFT; ++i)
  {
    bitSetCount += ((bitMask & bitTest) ? 1 : 0);
    bitTest /= 2;
  }

  return bitSetCount;
}

int
GetCoresCount()
{
  DWORD byteOffset = 0;
  DWORD count = 0;
  PSYSTEM_LOGICAL_PROCESSOR_INFORMATION b = {};

  LPFN_GLPI glpi = (LPFN_GLPI)GetProcAddress(
    GetModuleHandle(TEXT("kernel32")),
    "GetLogicalProcessorInformation");

  bool done = false;

  while (!done)
  {
    glpi(b, &count);

    if (!b)
    {
      if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
      {
        if (b)
          free(b);

        b = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(
          count);

        if (NULL == b)
        {
          printf("\nError: Allocation failure\n");
          return (2);
        }
      }
    }
    else
    {
      done = true;
    }
  }

  int processorCoreCount = 0;

  while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= count)
  {
    switch (b->Relationship)
    {
    case RelationProcessorCore:
      processorCoreCount++;
      break;
    }
    byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
    b++;
  }

  return processorCoreCount;
}

int main()
{
  int cores_count = GetCoresCount();

  printf("cores_count: %d\n", cores_count);

  //for (int i = 1; i < cores_count; i++)
  //{
  //  DWORD id;

  //  HANDLE thread = CreateThread(NULL, 0, work, &worker_threads[i], 0, &id);

  //  SetThreadAffinityMask(thread, 1 << i);

  //  // SuspendThread(thread);

  //  worker_threads[i].Thread = thread;
  //  threads[i] = thread;
  //}

  //job j1((void*)foo);

  //job j2((void*)fooki);

  //job::create(&j1);

  //job::create(&j2);

  //JobSystem.High.enqueue(j1);
  //JobSystem.High.enqueue(j2);

  //g_Counter.store(1);

  //JobDecl jobDecl(fooki);
  //JobSystem.High.enqueue(&jobDecl);
  //JobSystem.High.Prepare();

  //g_Counter.wait(0);

  // PipelineFooki();

   //JobDecl* Job = &jobDecl;
   fiber Fiber;// = worker::GetFiber();
   // Worker->CurrentFiber = Fiber;
   fiber::create(&Fiber, fooki);
   fiber::exec(&Fiber);

  //WaitForMultipleObjects(cores_count, threads, true, 0);

  printf("fooki_val: %f\n", fooki_val);

  //for (size_t i = 1; i < cores_count; i++)
  //{
  //  CloseHandle(worker_threads[i].Thread);
  //}

  return 0;
}