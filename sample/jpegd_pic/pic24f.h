#ifndef DEF_PIC24F
#define DEF_PIC24F

#define FCY		14745600UL

#define _DI()		__asm__ volatile("disi #0x3FFF")
#define _EI()		__asm__ volatile("disi #0")

#endif
