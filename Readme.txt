SOME INTERESTING INFO

Xv6 - Virtual OS - Has no effect on our OS
Qemu is used to run virtual environment
Root fxn- sudo
Sudo-apt-get install
Make- calls makefile

Makefile:
Object files .o
Env variables
Qemu settings
Kernel images in order
Name of files or commands
Qemu mei 2 CPUs- virtual hardware mei xv6 ki image hai
Init- shell started
Make qemu nox- qemu in terminal

Init.c compiles and executed when make qemu executed. Fork in this. Child bana to it will exec
shell.
Ab shell rest pe hai but when ls sent-> shell pe fork bana and ek mei exec(ls ho gaya)
Till ls is not completed, you won’t get back the shell

MAKE USER DEFINED PROGRAM:(use directly on terminal)
Make xv6 environment understand ki test.c hai: Makefile mei uprogs update karo. Extra mei
jake test.c\
MAKE SYSTEM CALL:(use in user defined program)
5 files handled:
1. Syscall.h: define SYS_getyear 22 <--- give number to system call
2. Syscall.c:
a. give ptr to system call [SYS_getyear]sys_getyear mei small mei written vala is
pointer to a function jo kahi aur likhenge. It means this function ka pointer will
reference system call #22.
b. Prototype of this fxn as: extern int sys_getyear(void)
3. Sysproc.c: fxn definition int sys_getyear(void)
4. usys.S: define syscall giving interface to user program to call our system call:
SYSCALL(getyear)
5. User.h: for user fxn to use system call, this definition needed. This is the fxn a user
program will call.

Features:
1. Processes
2. Virtual address spaces
3. File/Directory
4. Pipe
5. Multitasking
6. Time slicing
7. System calls(Originally 21)
User programs:
Sh- shell
Cat
Echo
Grep
Kill
Ln-hard link
Ls
Mkdir
Rm
Wc- word count
System calls:
Fork
Exit
Wait
Pipe
Write
Read
Close
Kill
Exec
Open
Mknod - make inode
Unlink - remove hard link
Fstat - get info about files
Link
Mkdir
Chdir - change directory
Dup
Getpid
Sbrk - grow heap
Sleep
Uptime - how long kernel running

NO MALLOC !!!????!?!?!!
Round Robin scheduler with one “READY QUEUE” and timeslice= 1,000,000 cycles
SpinLocks with sleep and wakeup
Kernel files:
User Files: initcode.S, sh.c, cat.c, echo.c and other user program codes
entry.S- set up stack
Start.c- machine mode
Main.c- supervisor //cores here
Main.c:
1. Cpuid:
a. Core 0: lot of init calls and kernel booting+ change volatile started to 1 + userinit
+ sync_synchronize for compiler to finish all other inits before executing
started=1
b. Other cores: wait for core 0 from started + turn on paging+ install kernel trap +
ask PLIC for device interface.

Param.h:
Max num of process= 64
Max num of cpus= 8
Open files per proc= 16
Open files per system= 100
....

Defs.h:
Function prototypes in .c files
#define NELEM
SpinLock:
Struct: uint lock, name, struct cpu*
● Acquire(has push off to not hold spinlock too long/ deadlock)
○ Push off: interrupts off
● Release(has push off to not hold spinlock too long/ deadlock)
○ Pop off: interrupts on
● Initlock
● Holding

User.h:
Function prototypes for the 21 system calls(__attribute__((noreturn)) tells cpu about no return
value for optimization) and library functions like printf, free, atoi, etc.
Syscall.h:
Has define statements for system calls with their number
Usys.pl:
Assembly level code for system calls
initcode.S
User mode program
First executed code
Kalloc and kfree: 4KB pages on freelist
Kalloc.c:
Struct run* next: ptr to next block
Struct run*freelist
User thread Kernel
—--> TRAP
<----- SRET

Memlayout.h:

Trap:
● TimerInterrupt
● Device Interrupt
● SysCall
● Exception
Proc.h:
Stack pointer
12 registers
Cpu structure
Trapframe having 32 registers as in risc v
Process states defined

Kernel startup- Entry.S and start.c

Start.c:
8 cores each having its own stack
entry.S:
Stack pointer= stack0 + (hartid * 4096)
Call start.c
Proc.c:
Yield
Sched
Scheduler
Cpuid: returns tp register value: number of cores executing on (Always call with disabled
interrupt)
Mycpu: return cpu structure using current cpuid
My proc: will get the sttructure and return current process

CHANGES MADE 
1. Main changes are in console.c where we added UP and DOWN key cases and made a
history function.
2. Added sys_history to sysproc.c and added declarations in syscall.c, usys.S, defs.h,
user.h and other required files like the previous function
3. Made a file his.c to call history function and make user level program to call from
terminal.
4. Updated makefile at UPROGS and extras.
