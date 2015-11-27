#ifndef _ASM_X86_IRQ_H
#define _ASM_X86_IRQ_H
/*
 *	(C) 1992, 1993 Linus Torvalds, (C) 1997 Ingo Molnar
 *
 *	IRQ/IPI changes taken from work by Thomas Radke
 *	<tomsoft@informatik.tu-chemnitz.de>
 */

#include <asm/apicdef.h>
#include <asm/irq_vectors.h>
typedef void (*irq_handler_t) (int irq);
extern irq_handler_t irq_vectors[256];
static inline irq_handler_t register_irq(irq_handler_t pnew,int irq)
{
    int n=irq+T_IRQ0;
    irq_handler_t old=irq_vectors[n];
    irq_vectors[n]=pnew;
    return old;
}
#endif /* _ASM_X86_IRQ_H */
