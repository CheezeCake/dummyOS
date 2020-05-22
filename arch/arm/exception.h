#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#define ARM_MODE_USR    0x10
#define ARM_MODE_FIQ    0x11
#define ARM_MODE_IRQ    0x12
#define ARM_MODE_SVC    0x13
// monitor 0x16
#define ARM_MODE_ABT    0x17
#define ARM_MODE_HYP    0x1a
#define ARM_MODE_UND    0x1b
#define ARM_MODE_SYS    0x1f
#define ARM_MODE_MASK   0x1f
#define ARM_MODE_PRIV   0x0f

#define CPSR_IF_MASK    0xc0
#define CPSR_I_MASK     0x80
#define CPSR_F_MASK     0x40

#ifndef ASSEMBLY

void exception_init(void);

// @see exception.S
void exception_vectors_setup(void);
void exception_set_mode_stack(unsigned mode, void* stack);

#endif

#endif
