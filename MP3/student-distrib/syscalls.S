#define ASM     1


# Verify stack management

# void syscall_linkage(int sys_call_number, int arg1, int arg2, int arg3);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: EAX - system call number
#            EBX - 1st arg
#            ECX - 2nd arg
#            EDX - 3rd arg
#   Outputs: Return value in EAX (not all calls return), -1 for error
# Registers: EBP, EBX, ESI, EDI are preserved
#            Caller needs to ensure arguments are properly set in EBX, ECX, and EDX registers before calling
#References: Week 11 Discussion

# Function sets up generic assembly linkage for system calls
# including syscall value checking, register save and restore,
# and a jump table to C functions to implement syscalls

.globl syscall_linkage
.align 4
syscall_linkage:
    # callee save registers
    # pushl %ebp
    # movl %esp, %ebp # move current stack into base
    pushl %ebx
    pushl %esi
    pushl %edi
    pushfl # Push EFLAGS Register onto the stack
    sti # Set Interrupt Flag in the flags register

    # For CP4, we do 8 syscall functions
    cmpl $8, %eax 
    jg ERR # if outside valid range, return error

    cmpl $0, %eax
    jle ERR # if outside valid range, return error

    # pushing arguments in accordance with LIFO
    pushl %edx # third arg
    pushl %ecx # second arg
    pushl %ebx # first arg

    call *jumptable(,%eax,4) # call jumptable

    addl $12, %esp           # restore stack pointer by 12 bytes
    popfl # Pop EFLAGS register
    # callee teardown   
    popl %edi
    popl %esi
    popl %ebx
    # leave
    iret
ERR:
    movl $-1, %eax
    popfl # Pop EFLAGS register
    popl %edi # callee teardown
    popl %esi
    popl %ebx
    iret

jumptable:
    .long 0x0, halt, execute, read, write, open, close, getargs, vidmap

.globl flush_tlb
.align	4
flush_tlb:
    movl %cr3, %eax
    movl %eax, %cr3
    ret

.globl	parent_return
.align 4
parent_return:
    movl %esp, %ebp
    movl 12(%ebp), %ebx   
    movl %ebx, %eax         
    movl 8(%ebp), %ebx     
    movl %ebx, %esp  
    movl 4(%ebp), %ebx       
    movl %ebx, %ebp
    leave
    ret 

.globl context_switch
.align 4
context_switch:
    movl %esp, %ebp
    movl 12(%ebp), %ebx     
    pushl %ebx
    movl 4(%ebp), %ebx    
    pushl %ebx
    pushfl
    popl %ebx
    orl $0x0200, %ebx
    pushl %ebx
    movl 8(%ebp), %ebx     
    pushl %ebx
    movl 16(%ebp), %ebx    
    pushl %ebx
    iret
