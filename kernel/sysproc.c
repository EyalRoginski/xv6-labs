#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.
  int page_num;
  uint64 first_page;
  uint32 accessed_mask = 0;
  uint64 result_buffer;

  argaddr(0, &first_page);
  argint(1, &page_num);
  argaddr(2, &result_buffer);

  if (page_num > 32 || page_num < 1) 
  {
    return -1;
  }

  pagetable_t pagetable = myproc()->pagetable;

  for (int i = 0; i < page_num; i++)
  {
    uint64 va = first_page + i * PGSIZE;
    pte_t *pte = (pte_t *)walk(pagetable, va, 0);
    if (pte == 0)
      break;
    if (*pte & PTE_A)
    {
      accessed_mask |= 1 << i;
      *pte &= ~PTE_A;
    }
  }

  copyout(pagetable, result_buffer, (char *)&accessed_mask, sizeof(uint32));

  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
