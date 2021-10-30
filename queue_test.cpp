
#include <stdio.h>
#include <vector>

#include <mutex>

// template <typename T>
// struct laxatomic : std::atomic<T>()
// {
//   void store( T desired) noexcept
//   {

//   }
 
// };


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
    int val, Gen;
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
    //operator<() = default;
  };

  std::atomic<entry> Buffer[Size];
  std::atomic<geni> Head;
  int Space[ROOM];
  std::atomic<geni> Tail;

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
      // entry newg { val, tmp.Gen };
    } while (!Buffer[tmp].compare_exchange_strong(ent, { val, tmp.Gen }, std::memory_order_release));
    tmp.incr();
    while (!Tail.compare_exchange_weak(old, tmp) && old < tmp);
    return true;
  }

  Type* deq()
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
          return nullptr;
        }
        else tmp.incr();
      }
      //entry zero { nullptr, tmp.gen + 1 };
    } while (!Buffer[tmp].compare_exchange_strong(ent, { Type(), tmp.Gen + 1 }, std::memory_order_acquire));
    tmp.incr();
    while (!Head.compare_exchange_weak(old, tmp) && old < tmp);
    return &ent.Data;
  }
};

//
struct JobDecl
{
  void* Job = nullptr;
  void* Arg = nullptr;

  bool empty()
  {
    return Job == nullptr;
  }
};

void me(int i)
{
  printf("me: %d", i);
}

int main()
{
  queue<JobDecl, 10> q1;

  q1.enq({me, (void*)15});
  q1.enq({me, (void*)30});
  q1.enq({me, (void*)35});

  printf("%d\n", (int)q1.deq()->Arg);
  printf("%d\n", (int)q1.deq()->Arg);
  printf("%d\n", (int)q1.deq()->Arg);

  return 0;
}