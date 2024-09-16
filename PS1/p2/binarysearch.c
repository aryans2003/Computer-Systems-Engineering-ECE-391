/*
 * tab:2
 *
 * search.c - Implementation of C Recursive Depth First Search
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

#include "binarysearch.h"


int32_t binarySearch(int32_t* arr, int32_t md, int32_t low, int32_t high)
{
    int mid = 0;
    if (low <= high) 
    {
        mid = (low + high) / 2;
        if (md == arr[mid]) 
        {
            return 1;
        }
        else if (md < arr[mid]) 
        {
            return binarySearch(arr, md, low, mid - 1);
        }
        else
            return binarySearch(arr, md, mid + 1, high);
    }
    else
        return -1;
}
