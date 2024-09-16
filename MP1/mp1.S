.data                                   # section declaration

        # Useful offset constants for accessing members of a 
        # struct mp1_blink_struct structure
        LOCATION   = 0    
        ON_CHAR    = 2
        OFF_CHAR   = 3 
        ON_LENGTH  = 4
        OFF_LENGTH = 6
        COUNTDOWN  = 8
        STATUS     = 10
        NEXT       = 12

        STRUCT_SIZE = 16

# Pointer to head of list (initialized to NULL)
mp1_list_head:
        .long   0
jumptable: # initialization of jumptable for ioctl cmds
        .long mp1_ioctl_add, mp1_ioctl_remove, mp1_ioctl_find, mp1_ioctl_sync

.text                                   # section declaration

# Export the function symbol names

.global mp1_rtc_tasklet
.global mp1_ioctl

# void mp1_poke(void);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX
mp1_poke:
        
        movl    vmem_base_addr(,1),%edx
        movb    %cl,(%edx,%eax,1)
        ret
# void mp1_rtc_tasklet (unsigned long);
#
# Interface: Register-based arguments
#       Inputs: none
#       Outputs: When an RTC interrupt is generated, each video memory
#       character will print its opposite value onto the screen
# Registers: Clobbers ECX, EDX
mp1_rtc_tasklet:
        # callee save registers
        pushl %ebp
        movl %esp, %ebp # move current stack into base
        pushl %ebx
        pushl %esi
        pushl %edi

        # get address of head
        movl mp1_list_head, %edi
        cmpl $0, %edi # error-check head value
        je RET
        jmp LOOP # walk down mp1_list_head_list

LOOP: # loop to walk down mp1_list_head_list
        # Decrement the countdown field of the structure
        xorl %edx, %edx 
        movw COUNTDOWN(%edi), %dx 
        subw $1, %dx 
        movw %dx, COUNTDOWN(%edi)
        cmpw $0, COUNTDOWN(%edi) # reached zero, examine the status field
        je EXAMINE_STATUS
        cmpl $0, %edi # if head is zero, return
        je RET
        movl NEXT(%edi), %edi # keep moving to next value in list
        cmpl $0, %edi # ensure next points to a valid next value
        je RET
        jmp LOOP # recursive loop call until head is zero or no more next values

RET: # teardown stack and return
        # callee teardown
        popl %edi
        popl %esi
        popl %ebx
        leave
        ret

EXAMINE_STATUS: # loop to examine status field
        # check if status field is equal to 1 or 0
        xorl %edx, %edx
        cmpw $1, STATUS(%edi)
        je PRINT_ON # if equal to 1, has on_char, must print off_char
        cmpw $0, STATUS(%edi)
        je PRINT_OFF # if equal to 0, has off_char, must print on_char
        jmp RET

PRINT_OFF: # if on_char, print off_char
        # interchange status field value
        movw $1, STATUS(%edi)
        xorl %ecx, %ecx
        # move on_char into %cl for poke call
        movb ON_CHAR(%edi), %cl
        xorl %eax, %eax
        # check range for location value
        cmpw $1999, LOCATION(%edi) # location out of bounds if greater than 80*25-1=1999
        ja RET
        cmpw $0, LOCATION(%edi) # location out of bounds if less than 0
        jl RET
        # move location into %eax for poke call and shift left
        movw LOCATION(%edi), %ax
        shlw $1, %ax
        call mp1_poke # make call to print character
        xorl %edx, %edx
        # set countdown equal to on_length
        movw ON_LENGTH(%edi), %dx 
        movw %dx, COUNTDOWN(%edi)
        movl NEXT(%edi), %edi # move onto next element in list
        cmpl $0, %edi # ensure next points to a valid element
        je RET
        jmp LOOP # revisit loop

PRINT_ON: # if off_char, print on_char
        # interchange status field value
        movw $0, STATUS(%edi)
        xorl %ecx, %ecx
        # move off_char into %cl for poke call
        movb OFF_CHAR(%edi), %cl 
        xorl %eax, %eax 
        # check range for location value
        cmpw $1999, LOCATION(%edi) # location out of bounds if greater than 80*25-1=1999
        ja RET
        cmpw $0, LOCATION(%edi) # location out of bounds if less than 0
        jl RET
        # move location into %eax for poke call and shift left
        movw LOCATION(%edi), %ax 
        shlw $1, %ax
        call mp1_poke # make call to print character
        xorl %edx, %edx
        # set countdown equal to off_length
        movw OFF_LENGTH(%edi), %dx
        movw %dx, COUNTDOWN(%edi)
        movl NEXT(%edi), %edi # move onto next element in list
        cmpl $0, %edi # ensure next points to a valid element
        je RET
        jmp LOOP # revisit loop

# int mp1_ioctl (unsigned long arg, unsigned long cmd);
# Registers:
# esi - unsigned long arg
# ebx - unsigned long cmd
# eax - Return Value
mp1_ioctl:

        movl 8(%esp), %ebx # load unsigned long cmd into ebx
        # check range of values for passed command
        cmpl $3, %ebx # cmd value 0-3 corresponds to a core function
        jg OTHER 
        cmpl $0, %ebx 
        jl OTHER 
        jmp *jumptable(,%ebx,4) # jumptable call
        
OTHER: # case for invalid command
        movl $-1, %eax # set -1 as return value to indicate failure
        leave
        ret

# int mp1_ioctl_add (unsigned long arg)
#
# Interface: Register-based arguments
#       Inputs: user-level pointer to a mp1_blink_struct structure
#       Outputs: Adds characters to video memory to facilitate
#       the display of the text-mode fish. Returns -1 if
#       any issues encountered, else 0 for a successful add
# Registers: Clobbers ECX, EDX
#            Return Value in EAX
mp1_ioctl_add:

        # callee save routine
        pushl %ebp
        movl %esp, %ebp # move current stack position in base pointer 
        pushl %ebx
        pushl %esi
        pushl %edi

        movl 8(%ebp), %ecx # load unsigned long arg into ecx
        # invalid arg check
        cmpl $0, %ecx 
        je ERR 

        pushl %ecx # save ecx value for call

        # push and pop size needed for allocation, call C function to allocate memory
        pushl $STRUCT_SIZE
        call mp1_malloc
        addl $4, %esp # adjusts stack pointer back since 4 bytes passed with STRUCT_SIZE 

        popl %ecx # pop saved ecx value for call

        # look for errors
        cmpl $0, %eax 
        je ERR 
        movl %eax, %edx 

        # push registers needed to be saved
        pushl %ecx
        pushl %edx

        # push size of struct, ecx (from value, original arg), edx (to value, copy of struct)
        # make call, and pop afterwards
        pushl $STRUCT_SIZE
        pushl %ecx 
        pushl %edx 
        call mp1_copy_from_user
        addl $12, %esp # adjusts stack pointer back since 12 bytes passed with STRUCT_SIZE, ECX, and EDX
        popl %edx
        popl %ecx

        # look for errors
        cmpl $0, %eax
        jne FREE_ERR

        # set countdown equal to on_length and status field to 1
        xorl %ecx, %ecx
        movw ON_LENGTH(%edx), %cx 
        movw %cx, COUNTDOWN(%edx)
        movw $1, STATUS(%edx) 

        # store next value into current head
        movl mp1_list_head, %ebx
        movl %ebx, NEXT(%edx) 
        movl %edx, mp1_list_head 

        
        cmpw $1999, LOCATION(%edx) # location out of bounds if greater than 80*25-1=1999
        ja FREE_ERR
        cmpw $0, LOCATION(%edx) # location out of bounds if less than 0
        jl FREE_ERR

        # move on_char to %cl, location to %ax, and shifting %eax
        # left, pushing %edx to save it, call mp1_poke, popping %edx
        movb ON_CHAR(%edx), %cl
        xorw %ax, %ax 
        movw LOCATION(%edx), %ax
        shlw $1, %ax
        pushl %edx
        call mp1_poke
        popl %edx

        movl $0, %eax # return 0 if successful add
        # general register teardown
        popl %ebx
        popl %esi
        popl %edi
        leave
        ret

ERR: # function to handle error cases
        movl $-1, %eax # move -1 into return value to indicate error
        # general register teardown
        popl %edi
        popl %esi
        popl %ebx
        leave
        ret
FREE_ERR: # function to handle error cases and free memory
        # %edi used for memory allocation, call mp1_free
        # to free allocated memory when error detected
        pushl %edx
        call mp1_free
        popl %edx
        # pushl %edx
        # call mp1_free
        # popl %edx
        # move -1 into return value to indicate error
        movl $-1, %eax
        # general register teardown
        popl %edi
        popl %esi
        popl %ebx
        leave
        ret

# int mp1_ioctl_remove(unsigned long arg)
#
# Interface: Register-based arguments
#       Inputs: integer location
#       Outputs: Removes characters whose location field
#       matches the passed integer location. Will remove 'M'
#       character following sync. Returns 0 if element
#       was found or -1 if no location matches.
# Registers: Clobbers EDX
#            Return Value in EAX       
mp1_ioctl_remove:

        # callee save routine
        pushl %ebp
        movl %esp, %ebp # move current stack position into base pointer
        pushl %ebx
        pushl %esi
        pushl %edi

        # load unsigned long arg into ecx
        xorl %ecx, %ecx
        movl 8(%ebp), %ecx

        # push and pop arg into helper function to traverse list
        pushl %ecx
        call mp1_linked_list
        popl %ecx

        # ensure return value is valid
        cmpl $-1, %eax
        je ERR

        # move return value into esi
        movl %eax, %esi
        jmp REMOVE

REMOVE: # checks if element to be removed is the head or not
        # compare found element and head to see if equal
        movl mp1_list_head, %edi
        cmpl %edi, %esi 
        je REMOVE_HEAD # if equal, remove head
        jmp REMOVE_OTHER # if not, remove another elem

REMOVE_HEAD: # process to remove head
        # move next element as new head
        movl NEXT(%edi), %ebx 
        movl %ebx, mp1_list_head 
        # deallocate memory
        pushl %edi
        call mp1_free
        popl %edi
        # callee teardown
        popl %edi
        popl %esi
        popl %ebx
        leave
        ret
REMOVE_OTHER: # process to remove other element
        # remove next element
        movl NEXT(%edx), %ebx
        movl %ebx, NEXT(%edx)
        pushl %esi
        call mp1_free
        popl %esi
        # set return value as 0
        movl $0, %eax
        # callee teardown
        popl %edi
        popl %esi
        popl %ebx
        leave
        ret
        
# int mp1_ioctl_find (unsigned long arg)
# Interface: Register-based arguments
#       Inputs: a pointer to mp1_blink_struct
#       Outputs: Matching location returns 0, else -1
# Registers: Clobbers EDX, ECX
#            Return Value in EAX         
mp1_ioctl_find:
        # callee save routine
        pushl %ebp
        movl %esp, %ebp # move current stack position into base pointer
        pushl %ebx
        pushl %esi
        pushl %edi

        movl $-1, %eax
        # callee teardown
        popl %edi
        popl %esi
        popl %ebx
        leave
        ret

# int mp1_ioctl_sync (unsigned long arg)
# Interface: Register-based arguments
#       Inputs: 2 two-byte unsigned short integers
#       whose first integer represents the location of the
#       first blinking character, and second is for the second
#       character to be synced with the first
#       Outputs: The "I/M" blinks sync with the rest of the background
#       after some frames. Returns 0 on success or -1 on failure.
# Registers: Clobbers EDX, ECX
#            Return Value in EAX 
mp1_ioctl_sync:

        # callee save
        pushl %ebp
        movl %esp, %ebp # move current stack position into base pointer
        pushl %ebx
        pushl %esi
        pushl %edi

        # clear registers for use
        andl $0, %edx
        andl $0, %ebx

        # extract upper and lower bits
        movw 8(%ebp), %dx
        movw 10(%ebp), %bx

        # look for errors
        cmpw $0, %dx
        je ERR
        cmpw $0, %bx
        je ERR

        # call linked list search function for EBX
        pushl %edx
        pushl %ebx
        call mp1_linked_list
        popl %ebx
        popl %edx

        # ensure proper return value
        cmpl $-1, %eax
        je ERR
        movl %eax, %esi # if successful, store lower bit value into temp reg

        xorl %eax, %eax # clear EAX for next call

        # call linked list search function for EDX
        pushl %edx
        call mp1_linked_list
        popl %edx

        # ensure proper return value
        cmpl $-1, %eax
        je ERR
        movl %eax, %ecx # if successful, store upper bit value into temp reg

        # clear EDI register for use
        # copy old timing information using ESI (lower bits value)
        # into new timing information using ECX (upper bits value)
        # for each on_length, off_length, countdown, status fields
        xorl %edi, %edi
        movw ON_LENGTH(%esi), %di
        movw %di, ON_LENGTH(%ecx)

        xorl %edi, %edi
        movw OFF_LENGTH(%esi), %di
        movw %di, OFF_LENGTH(%ecx)

        xorl %edi, %edi
        movw COUNTDOWN(%esi), %di
        movw %di, COUNTDOWN(%ecx)

        xorl %edi, %edi
        movw STATUS(%esi), %di
        movw %di, STATUS(%ecx)

        # check status register to update correct character
        cmpl $0, STATUS(%ecx)
        je PRINT1
        cmpl $1, STATUS(%ecx)
        je PRINT2

        
        xorl %eax, %eax
        jmp ERR

PRINT1: # print on_char if status is 0
        # copied function call to mp1_poke
        # set on_char into %cl, location to %ax, left shift %eax by 1
        xorl %ecx, %ecx
        movb ON_CHAR(%edi), %cl 
        cmpw $1999, LOCATION(%edi) # location out of bounds if greater than 80*25-1=1999
        ja RET
        cmpw $0, LOCATION(%edi) # location out of bounds if less than 0
        jl RET
        xorl %eax, %eax
        movw LOCATION(%edi), %ax 
        shlw $1, %ax 
        call mp1_poke
        xorl %eax, %eax

        # set return value to 0 to indicate success
        movl $0, %eax

        # general register teardown
        popl %ebx
        popl %esi
        popl %edi
        leave
        ret

PRINT2: # print off_char if status is 1
        # copied function call to mp1_poke
        # set off_char into %cl, location to %ax, left shift %eax by 1
        xorl %ecx, %ecx
        movb OFF_CHAR(%edi), %cl 
        cmpw $1999, LOCATION(%edi) # location out of bounds if greater than 80*25-1=1999
        ja RET
        cmpw $0, LOCATION(%edi) # location out of bounds if less than 0
        jl RET
        xorl %eax, %eax
        movw LOCATION(%edi), %ax 
        shlw $1, %ax 
        call mp1_poke 
        xorl %eax, %eax

        # set return value to 0 to indicate success
        movl $0, %eax

        # general register teardown
        popl %ebx
        popl %esi
        popl %edi
        leave
        ret

# int mp1_linked list(uin16_t arg)
# Interface: Register-based arguments
#       Inputs: 16-bit integer location
#       Outputs: Location value if match found in mp1_head_list, else -1
#       Registers: Clobbers ECX, EDX
#                  Return Value in EAX         
mp1_linked_list:

        # callee save
        pushl %ebp
        movl %esp, %ebp # move current stack position into base pointer
        pushl %ebx
        pushl %esi
        pushl %edi

        # load in integer location
        xorl %ecx, %ecx
        movw 8(%ebp), %cx

        # move head pointer into temporary register
        movl mp1_list_head, %ebx 
        # check for errors
        cmpl $0, %ebx 
        je ERR
        jmp LIST_LOOP # enter iterative loop
LIST_LOOP: # loops through mp1_list_head
        # loads in location value
        movw LOCATION(%ebx), %di
        # location bounds check
        cmpw $0, %di # location out of bounds if less than 80*25-1=1999
        jl ERR 
        cmpw $1999, %di # location out of bounds if greater than 80*25-1=1999
        ja ERR 
        # check to see if equal to passed location, if so found
        cmpw %di, %cx 
        je FOUND
       
        movl %ebx, %edx # save previous value
        movl NEXT(%ebx), %ebx
        
        cmpl $0, %ebx
        je ERR
        # recursive call
        jmp LIST_LOOP

FOUND: # case if location found
        # copy matching location into return value
        movl %ebx, %eax

        # callee teardown routine and return
        popl %edi
        popl %esi
        popl %ebx
        leave
        ret
.end
