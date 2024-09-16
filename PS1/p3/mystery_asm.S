
/*
 * tab:2
 *
 * mystery_asm.S - Assembly mystery
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE,
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:        Aamir Hasan
 * Version:       1
 * Creation Date: Sun Aug 30 2020
 * Filename:      mystery_asm.S
 * History:
 *    AH    1    Sun Aug 30 2020
 *        First written.
 */


# mystery (Assembly)
# 
# Registers:
#   eax - Return Value
#   ebx - x
#   ecx - y
#   esi - opcode

.GLOBAL mystery_asm

mystery_asm:
    pushl   %ebp
    movl    %esp, %ebp
    pushl   %ebx
    pushl   %esi
    pushl   %edi
    movl    8(%ebp), %esi
    movl    12(%ebp), %ebx
    movl    16(%ebp), %ecx
    cmpl    $2, %esi
    jg      DEFAULT
    cmpl    $0, %esi
    jl      DEFAULT
    jmp     *jumptable(,%esi,4)
OP1:
    movl    (%ebx, %ecx, 4), %eax
    jmp     FINISH
OP2:
    leal    (%ebx, %ecx, 8), %eax
    subl    %ecx, %eax
    jmp     FINISH
OP3:
    movl    %ebx, %eax          
    xorl    %edx, %edx
    divl    %ecx       
    movl    %edx, %eax
    jmp     FINISH

DEFAULT:
    movl    $0xBAD, %eax
FINISH:
    popl    %edi
    popl    %esi
    popl    %ebx
    leave
    ret

jumptable:
    .long OP1, OP2, OP3
