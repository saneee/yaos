# Initial process execs /init.

//#include "yaos/yaoscall.h"
//#include "traps.h"


# exec(init, argv)
.globl start
start:
  mov $init, %rdi
  mov $argv, %rsi
//  mov $SYS_exec, %rax
//  int $T_SYSCALL
int $0x80;

# for(;;) exit();
exit:
//  mov $SYS_exit, %rax
 // int $T_SYSCALL
  int $0x80;
  jmp exit

# char init[] = "/init\0";
init:
  .string "/init\0"

# char *argv[] = { init, 0 };
.p2align 2
argv:
  .quad init
  .quad 0

