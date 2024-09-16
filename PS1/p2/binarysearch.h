/*
 * tab:2
 *
 * search.h - Search function & helper function declarations
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
 * Creation Date: Fri Aug 30 2020
 * Filename:      search.c
 * History:
 *    AS    1    Fri Aug 30 2020
 *        First written.
 */

#ifndef BINARYSEARCH_H
#define BINARYSEARCH_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// binarySearch
//   searches for an element in the BST recursively
//   Returns:
//     1 - found element
//     -1 - element not found
extern int32_t binarySearch(int32_t* arr, int32_t md, int32_t low, int32_t high);

extern int32_t binarySearch_asm(int32_t* arr, int32_t md, int32_t low, int32_t high);

#endif