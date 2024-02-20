#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
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
  if(n < 0)
    n = 0;
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

  backtrace();
  return 0;
}

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

typedef void (*handler_t)();

uint64
sys_sigalarm(void)
{
  uint64 handler;
  int interval;
  struct proc *p = myproc();
  argaddr(1, &handler);
  argint(0, &interval);

  if (interval < 0)
    return -1;

  p->alarm_handler = handler;
  p->alarm_interval = (uint64)interval;
  p->ticks_since_last = 0;
  p->handler_running = 0;

  return 0;
}

void restore_alarm_frame(struct proc *p)
{
  p->trapframe->kernel_satp = p->alarm_trapframe->kernel_satp;
  p->trapframe->kernel_sp = p->alarm_trapframe->kernel_sp;
  p->trapframe->kernel_trap = p->alarm_trapframe->kernel_trap;
  p->trapframe->kernel_hartid = p->alarm_trapframe->kernel_hartid;
  p->trapframe->epc = p->alarm_trapframe->epc;
  p->trapframe->ra = p->alarm_trapframe->ra;
  p->trapframe->sp = p->alarm_trapframe->sp;
  p->trapframe->gp = p->alarm_trapframe->gp;
  p->trapframe->tp = p->alarm_trapframe->tp;
  p->trapframe->t0 = p->alarm_trapframe->t0;
  p->trapframe->t1 = p->alarm_trapframe->t1;
  p->trapframe->t2 = p->alarm_trapframe->t2;
  p->trapframe->t3 = p->alarm_trapframe->t3;
  p->trapframe->t4 = p->alarm_trapframe->t4;
  p->trapframe->t5 = p->alarm_trapframe->t5;
  p->trapframe->t6 = p->alarm_trapframe->t6;
  p->trapframe->a0 = p->alarm_trapframe->a0;
  p->trapframe->a1 = p->alarm_trapframe->a1;
  p->trapframe->a2 = p->alarm_trapframe->a2;
  p->trapframe->a3 = p->alarm_trapframe->a3;
  p->trapframe->a4 = p->alarm_trapframe->a4;
  p->trapframe->a5 = p->alarm_trapframe->a5;
  p->trapframe->a6 = p->alarm_trapframe->a6;
  p->trapframe->a7 = p->alarm_trapframe->a7;
  // Don't copy a7 because we need to know if we came from sigreturn later.
  // Pinky promise we'll copy it over later.
  p->trapframe->s0 = p->alarm_trapframe->s0;
  p->trapframe->s1 = p->alarm_trapframe->s1;
  p->trapframe->s2 = p->alarm_trapframe->s2;
  p->trapframe->s3 = p->alarm_trapframe->s3;
  p->trapframe->s4 = p->alarm_trapframe->s4;
  p->trapframe->s5 = p->alarm_trapframe->s5;
  p->trapframe->s6 = p->alarm_trapframe->s6;
  p->trapframe->s7 = p->alarm_trapframe->s7;
  p->trapframe->s8 = p->alarm_trapframe->s8;
  p->trapframe->s9 = p->alarm_trapframe->s9;
  p->trapframe->s10 = p->alarm_trapframe->s10;
  p->trapframe->s11 = p->alarm_trapframe->s11;
}

uint64
sys_sigreturn(void)
{
  struct proc *p = myproc();
  restore_alarm_frame(p);
  p->handler_running = 0;
  p->coming_from_sigreturn = 1;
  return 0;
}
