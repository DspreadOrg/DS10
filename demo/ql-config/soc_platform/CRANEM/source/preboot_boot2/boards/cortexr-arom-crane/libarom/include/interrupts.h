#ifndef __PLATFORM_INTERRUPTS_H_
#define __PLATFORM_INTERRUPTS_H_

#ifdef __cplusplus
extern "C" {
#endif

int mask_interrupt(unsigned int irq);
int unmask_interrupt(unsigned int irq);

typedef void (*interrupt_handler_t)(void *arg);

void register_int_handler(unsigned int irq, interrupt_handler_t handler, void *arg);
void interrupts_init(void);
void interrupts_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* __PLATFORM_INTERRUPTS_H_ */
