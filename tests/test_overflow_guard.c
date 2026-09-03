/*
   The overflow this guards against (see new_value() in json.c, the
   json_array and json_object cases) only wraps around on a platform
   where size_t is 32 bits wide, and only once an array or object has
   grown past roughly a billion entries. Reproducing that for real would
   need a 32-bit build and a multi-gigabyte input just to get past the
   first pass, which isn't practical to run here or in CI.

   Instead, this simulates an ILP32-style platform directly: uint32_t
   standing in for both `unsigned int` (the type of json_value's length
   fields) and `size_t` (the type of the allocation size), which is
   exactly the situation the original report described. It checks that
   the same guard formula used in json.c both (a) lets ordinary, safe
   lengths through and (b) catches the boundary value that would
   otherwise wrap the multiplication and hand an undersized allocation
   to code that's about to write well past the end of it.
*/

#include <stdio.h>
#include <stdint.h>

#define ELEMENT_SIZE 16u /* stand-in for sizeof (json_value *) on a 32-bit build */

/* Same shape as the check added to new_value() in json.c, just typed for
   the simulated 32-bit platform instead of the host's real size_t. */
static int would_overflow (uint32_t length, uint32_t element_size)
{
   return length > ((uint32_t) -1) / element_size;
}

int main (void)
{
   int failures = 0;

   /* A modest, entirely realistic length must never be rejected. */
   if (would_overflow (1000, ELEMENT_SIZE))
   {
      fprintf (stderr, "FAIL: a small, safe length was rejected\n");
      failures++;
   }

   /* The largest length new_value()'s own increment guard ever allows
      through (UINT_MAX - 8) is exactly the case the original report was
      about: on a 32-bit build this is large enough that length * element_size
      wraps around unless it's caught first. */
   {
      uint32_t max_allowed_length = ((uint32_t) -1) - 8;

      if (! would_overflow (max_allowed_length, ELEMENT_SIZE))
      {
         fprintf (stderr, "FAIL: a length known to overflow the multiplication was accepted\n");
         failures++;
      }

      /* Demonstrate what the unguarded code actually did: the raw
         multiplication wraps to something far smaller than the real size
         needed, which is the undersized allocation at the root of the bug. */
      {
         uint32_t wrapped = max_allowed_length * ELEMENT_SIZE;
         uint64_t real_size = (uint64_t) max_allowed_length * ELEMENT_SIZE;

         if (wrapped == (uint32_t) real_size && real_size > (uint32_t) -1)
         {
            printf ("confirmed: unguarded 32-bit multiplication wraps to %u bytes "
                    "instead of the %llu bytes actually needed\n",
                    wrapped, (unsigned long long) real_size);
         }
         else
         {
            fprintf (stderr, "FAIL: expected the raw multiplication to wrap on a 32-bit platform\n");
            failures++;
         }
      }
   }

   /* The exact boundary: one element fewer should just barely fit. */
   {
      uint32_t boundary_length = ((uint32_t) -1) / ELEMENT_SIZE;

      if (would_overflow (boundary_length, ELEMENT_SIZE))
      {
         fprintf (stderr, "FAIL: the exact boundary length was rejected\n");
         failures++;
      }

      if (! would_overflow (boundary_length + 1, ELEMENT_SIZE))
      {
         fprintf (stderr, "FAIL: one past the boundary length was accepted\n");
         failures++;
      }
   }

   if (failures)
   {
      fprintf (stderr, "%d check(s) failed\n", failures);
      return 1;
   }

   printf ("all overflow guard checks passed\n");
   return 0;
}
