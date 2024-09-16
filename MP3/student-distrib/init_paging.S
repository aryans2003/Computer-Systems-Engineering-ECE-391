.globl init_paging
.align 4
init_paging:
    # callee save registers
    pushl %ebp
    movl %esp, %ebp # move current stack into base

    # pass in physical address of page and move into eax
    movl 8(%ebp), %eax
    # load EAX into CR3 (base register) to point to beginning of page directory
    movl %eax, %cr3 # explain what this does -- on exam

    # copy CR4 to enable support > 4 MB
    movl %cr4, %eax
    # Perform bitwise OR to set PSE bit (4th bit) for larger pages
    orl $0x00000010, %eax
    # put EAX back into cr4
    movl %eax, %cr4

    # copy CR0 to set paging bit and enable protection mode
    movl %cr0, %eax
    # set paging bit (31) for paging mode and protection mode
    orl $0x80000001, %eax
    # move contents of CR0 back into EAX
    movl %eax, %cr0

    # Flush the Translation Lookaside Buffer to free addresses, check this
    movl %cr3, %eax
    # Reload CR3
    movl %eax, %cr3
    leave
    ret
