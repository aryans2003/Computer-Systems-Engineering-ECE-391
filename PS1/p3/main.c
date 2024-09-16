/*
 * tab:2
 *
 * main.c - I/O and P3 setup
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
 * Filename:      main.c
 * History:
 *    AH    1    Sun Aug 30 2020
 *        First written.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "mystery.h"

#define MAX_LENGTH 500

uint32_t import(char* infile, uint32_t* op, uint32_t* x, uint32_t* y) {
  FILE* f = fopen(infile, "r");
  if (f == NULL) return -1;
  fscanf(f, "%d %d %d\n", op, x, y);
  fclose(f);
  return 0;
}



int main(int argc, char** argv) {
  if(argc < 2) {
    printf("Usage: ./mystery <input file>\n");
    return -1;
  }
  uint32_t x = 0; 
  uint32_t y = 0;
  uint32_t opcode = 0;
  uint32_t out = 0;

  import(argv[1], &opcode, &x, &y);
  if(opcode ==0 ) {
    if (y > MAX_LENGTH) {
      printf("Error: x is too large\n");
      return -1;
    }
    uint32_t* ptr = malloc(sizeof(uint32_t) * MAX_LENGTH);
    for(int i=0; i<MAX_LENGTH; i++) {
      ptr[i] = i;
    }
    x = (uint32_t)ptr;
  }

  printf("Running C Code\n");
  out = mystery_c(opcode,x, y);
  printf("Value= %d\n ",  out);

  printf("Running ASM Code\n");

  out = mystery_asm(opcode, x, y);
  printf("Value= %d\n ",  out);

  // free(y);
  return 0;
}
