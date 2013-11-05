#ifndef _SYS_BRKP_H_
#define _SYS_BRKP_H_

/* 
 * This header file contains the definition of a debug breakpoint
 */
#define BREAKPOINT         _asm { int 3; }            


#endif
