#ifndef __ASM_PREEMPT_H
#define __ASM_PREEMPT_H
#include <asm/rmwcc.h>
#include <yaos/percpu.h>
DECLARE_PER_CPU(int, __preempt_count);
#define PREEMPT_ENABLED (0 + PREEMPT_NEED_RESCHED)
