/* Copyright (C) 2017, Ward Jaradat
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <os/config.h>
#include <os/startup.h>
#include <os/console.h>
#include <os/sched.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>

#ifdef ENABLE_FS
#include <fs/osal.h>
#include <fs/fs-tests.h>
#endif

#ifdef ENABLE_PTE_TESTS
#include <pte/test.h>
#endif

#ifdef ENABLE_PTE_TESTS
void pte_tests_thread(void *p)
{
	pthread_t r;
	pthread_create(&r, NULL, pte_test_main, NULL);
}
#endif

#ifdef ENABLE_FS
void fs_test_thread(void *p)
{
    if(init_disk())
    {
		int file_len;
		fs_read_test(&file_len);
	}
}
#endif

int fib(int n)
{
  int  i, Fnew, Fold, temp,ans;

    Fnew = 1;  Fold = 0;
    for ( i = 2;
	  i <= n;          /* apsim_loop 1 0 */
	  i++ )
    {
      temp = Fnew;
      Fnew = Fnew + Fold;
      Fold = temp;
    }
    ans = Fnew;
  return ans;
}

unsigned long long fibonacci(unsigned int n)
{
        if (n == 0)
        {
                return 0;
        }
        else if (n == 1)
        {
                return 1;
        }
        else
        {
                return fibonacci(n - 1) + fibonacci(n - 2);
        }
}


int is_prime(int n) {
    if (n <= 1) return 0; // zero and one are not prime numbers
    if (n <= 3) return 1; // 2 and 3 are prime numbers

    // eliminate multiples of 2 and 3
    if (n % 2 == 0 || n % 3 == 0) return 0;

    int i = 5;
    while (i * i <= n) {
        if (n % i == 0 || n % (i + 2) == 0) return 0;
        i += 6;
    }
    return 1;
}

void compute_primes(int max) {
    for (int i = 0; i < max; i++) {
        if (is_prime(i)) {
            // Uncomment the below line if you want to print the prime numbers
            // printf("%d ", i);
        }
    }
}


void fn(void *arg)
{
    compute_primes(1000900);
}

__attribute__((weak)) int app_main(struct app_main_args *aargs)
{

        //create_thread_with_priority("Thread", &fn, UKERNEL_FLAG, NULL, 1);
        //create_thread_with_priority("Thread", &fn, UKERNEL_FLAG, NULL, 4);
    for (int i = 0; i < 5760; i++)
    {
        create_thread_with_priority("Thread", &fn, UKERNEL_FLAG, NULL, i % 128);
    }

    /*
    for (int i = 0; i < 300; i++)
    {
        create_thread_with_priority("Thread", &fn, UKERNEL_FLAG, NULL, i % 100);
    }
    */

    /* This function can be overwritten to permit the kernel to be linked with
       an external application that defines the same function. Currently, in this
       function we provide a list of default tests.
     */

#ifdef ENABLE_PTE_TESTS
    /* Example for running the pthread tests. Please note that these tests 
       have been ported and adapted from PTE for experimentation purposes and
       in order to run them, for now, you need to:

       Enable the commented out OBJS+= directive for lib/pte/tests/*.c in the
       makefile, and define ENABLE_PTE_TESTS in os/config.h header file.

       Kindly note that these tests need to be revisited as long as the work
       on the optimisation of the scheduler continues.
     */
    create_thread("pte_tests_thread", pte_tests_thread, UKERNEL_FLAG, NULL);
#endif

#ifdef ENABLE_FS
    create_thread("fs_test_thread", fs_test_thread, UKERNEL_FLAG, NULL);
#endif 
    return 0;
}
