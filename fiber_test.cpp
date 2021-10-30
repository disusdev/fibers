
#include <stdio.h>
#include <stdlib.h>

#include <emmintrin.h>

#include <functional>

struct
context
{
  void *rip, *rsp;
  void *rbx, *rbp, *r12, *r13, *r14, *r15, *rdi, *rsi;
  // windows
  __m128i xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
};

typedef void* job;

extern "C" void get_context(context *c);
extern "C" void set_context(context *c);
extern "C" void swap_context(context *a, context *b);

struct
parameters
{
  int Id = 0;
  int Name = 0;
  int Value1 = 0;
  int Value2 = 0;
};

bool workDone = false;

struct main_job
{
  context AwakeContext = {};
  context Context = {};
  job Job;
};

struct
fiber
{
  char Data[4096] = { 0 };
  main_job Main = {};

  static void
  func(uintptr_t ptr)
  {
    main_job *MainJob = (main_job *)ptr;

    void (*ff)() = (void(*)())MainJob->Job;

    ff();

    set_context(&MainJob->AwakeContext);
    //swap_context(&MainJob.Context, &MainJob.AwakeContext);
  }

  fiber(job Job)
  {
    char* sp = (char*)(Data + sizeof(fiber::Data));

    sp = (char*)((uintptr_t)sp & -16L);

    Main.Job = Job;

    uintptr_t* spPointer = (uintptr_t*)Data;
    *spPointer = (uintptr_t)&Main;

    Main.Context = { 0 };
    Main.Context.rip = func;
    Main.Context.rsp = (void*)sp;
  }

  void static
  start(fiber* Fiber)
  {
    //set_context(&Fiber->Context);

    swap_context(&Fiber->Main.AwakeContext, &Fiber->Main.Context);
  }
};

//template<class... Args>
//void f(Args... args)
//{
//    auto x = [args...] { return g(args...); };
//    x();
//}

int id = 0;

void some_work()
{
  id++;
}

fiber Fiber(some_work);

int main()
{
  volatile bool InUse = false;

  //get_context(&Fiber.Main.AwakeContext);

  printf("hello, world!\n");

  if (!InUse)
  {
    InUse = true;
    fiber::start(&Fiber);
  }

  printf("END %d\n", id);

  return 0;
}