#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
   /* C99 might give us uintptr_t and UINTPTR_MAX but they also might not be provided */
   #include <stdint.h>
#endif

#ifdef UINTPTR_MAX
typedef uintptr_t json_uintptr_t;
#else
typedef size_t json_uintptr_t;
#endif

enum json_test_result
{
   json_test_parsed, /* success */
   json_test_rejected, /* task failed successfully */
   json_test_missing, /* fopen failed */
   json_test_error /* unusual test error such as file seek failure or memory allocation failure */
};

struct file_content_result
{
   char * content;
   long content_size;
   enum json_test_result error;
};

static struct file_content_result get_file_content(const char * filename_buffer)
{
   FILE * input_json;
   int seek_end_result;
   long tell_result;
   int seek_set_result;
   char * file_content_buffer;
   size_t read_result;
   struct file_content_result ret = {NULL, -1, json_test_error};

   input_json = fopen(filename_buffer, "rb");
   if(input_json == NULL)
   {
      ret.error = json_test_missing;
      return ret;
   }

   seek_end_result = fseek(input_json, 0, SEEK_END);
   if(seek_end_result != 0)
   {
      fprintf(stderr, "fseek end error, %i\n", seek_end_result);
      fclose(input_json);
      return ret;
   }

   tell_result = ftell(input_json);
   if(tell_result < 0)
   {
      fprintf(stderr, "ftell error, %li\n", tell_result);
      fclose(input_json);
      return ret;
   }

   seek_set_result = fseek(input_json, 0, SEEK_SET);
   if(seek_set_result != 0)
   {
      fprintf(stderr, "fseek set error, %i\n", seek_set_result);
      fclose(input_json);
      return ret;
   }

   if(tell_result == 0)
   {
      file_content_buffer = (char *)calloc(1, 1);
   }
   else
   {
      file_content_buffer = (char *)malloc(tell_result);
   }
   if(file_content_buffer == NULL)
   {
      fprintf(stderr, "memory allocation failed, %li bytes\n", tell_result);
      fclose(input_json);
      return ret;
   }

   read_result = fread(file_content_buffer, 1, tell_result, input_json);
   fclose(input_json);
   if(read_result < (size_t)tell_result)
   {
      fprintf(stderr, "fread error, %lu\n", (unsigned long)read_result);
      free(file_content_buffer);
      return ret;
   }

   ret.content = file_content_buffer;
   ret.content_size = tell_result;
   return ret;
}

static enum json_test_result json_test(struct file_content_result file_content, json_settings * settings, char * error_buffer)
{
   json_value * parsed_json = json_parse_ex(settings, file_content.content, file_content.content_size, error_buffer);
   if(parsed_json == NULL)
   {
      return json_test_rejected;
   }
   if(settings->mem_free)
   {
      json_value_free_ex(settings, parsed_json);
   }
   else
   {
      json_value_free(parsed_json);
   }
   return json_test_parsed;
}

static void * noisy_alloc(size_t count, int zero, void * user_data)
{
   void * ret;
   (void)user_data; /* silence unused param warning */
   if(zero)
   {
      fprintf(stderr, "calloc %lu bytes: ", (unsigned long)count);
      ret = calloc(count, 1);
   }
   else
   {
      fprintf(stderr, "malloc %lu bytes: ", (unsigned long)count);
      ret = malloc(count);
   }
   fprintf(stderr, "%p\n", ret);
   return ret;
}
static void noisy_free(void * ptr, void * user_data)
{
   (void)user_data; /* silence unused param warning */
   fprintf(stderr, "free %p\n", ptr);
   free(ptr);
}

struct arena_user_data
{
   void* start;
   size_t size;
   size_t next;
   char sentinel;
};
static void * arena_alloc(size_t count, int zero, void * user_data)
{
   struct arena_user_data * arena = (struct arena_user_data *)user_data;
   char * ret = NULL;
   size_t count_original = count;
   size_t i = 0;
   if(count_original < 1)
   {
      return NULL;
   }
   count += sizeof(count_original);
   if(arena->next >= arena->size || count > arena->size - arena->next)
   {
      fprintf(stderr, "arena exhausted, size = %lu, next = %lu, count = %lu\n", (unsigned long)arena->size, (unsigned long)arena->next, (unsigned long)count);
      return NULL;
   }
   ret = (char *)arena->start + arena->next;
   for(i = 0; i < count; ++i)
   {
      if(ret[i] != arena->sentinel)
      {
         fprintf(stderr, "arena corruption detected during arena_alloc\n");
         abort();
      }
   }
   if(zero)
   {
      memset(ret, 0, count);
   }
   memcpy(ret, &count_original, sizeof(count_original));
   ret += sizeof(count_original);
   arena->next += count;
   return ret;
}
static void arena_free(void * ptr, void * user_data)
{
   struct arena_user_data * arena = (struct arena_user_data *)user_data;
   json_uintptr_t start = (json_uintptr_t)arena->start;
   json_uintptr_t end = (json_uintptr_t)arena->start + arena->next;
   json_uintptr_t where = (json_uintptr_t)ptr;
   size_t count = 0;
   if(ptr == NULL)
   {
      return;
   }
   if(where < start + sizeof(count) || where >= end)
   {
      fprintf(stderr, "invalid arena_free call\n");
      abort();
   }
   memcpy(&count, (char *)ptr - sizeof(count), sizeof(count));
   if(count < 1)
   {
      fprintf(stderr, "double arena_free call\n");
      abort();
   }
   if(where + count > end)
   {
      fprintf(stderr, "corrupt arena_free call\n");
      abort();
   }
   memset(ptr, arena->sentinel, count);
   memset((char *)ptr - sizeof(count), arena->sentinel, sizeof(count));
}
static char large_arena[1024UL*1024UL];

static int json_verify(const char * filename_format, unsigned highest_file_num, int extensions, int expect_failure)
{
   int result = 0;
   unsigned test_num;
   char filename_buffer[32];
   int filename_buffer_result;
   json_settings settings = {0};
   char error_buffer[json_error_max];
   struct arena_user_data arena = {large_arena};
   struct file_content_result file_content = {NULL, -1, json_test_error};

   if(extensions == 1)
   {
      settings.settings = json_enable_comments;
   }

   for(test_num = 0; ; ++test_num)
   {
      filename_buffer_result = sprintf(filename_buffer, filename_format, test_num);
      if(filename_buffer_result < 0 || filename_buffer_result > 30)
      {
         result = 1;
         fprintf(stderr, "sprintf error, test_num = %u, filename_format = %s\n", test_num, filename_format);
         continue;
      }
      fprintf(stderr, "\n\n%s\n", filename_buffer);
      file_content = get_file_content(filename_buffer);
      if(file_content.content == NULL || file_content.content_size < 0)
      {
         switch(file_content.error)
         {
         case json_test_missing:
            if(test_num != highest_file_num + 1)
            {
               result = 1;
               fprintf(stderr, "mismatched number of test input files for %s, highest expected was %u but file %u was missing\n", filename_format, highest_file_num, test_num);
            }
            return result;
         default:
            result = 1;
            fprintf(stderr, "failed to read `%s` into memory\n", filename_buffer);
            continue;
         }
      }
      /* fuzz various max_memory values in the full arena size */
      arena.size = sizeof(large_arena)/sizeof(char);
      arena.sentinel = 123;
      settings.mem_alloc = arena_alloc;
      settings.mem_free = arena_free;
      settings.user_data = &arena;
      for(settings.max_memory = 0; settings.max_memory < 1024UL*1024UL; settings.max_memory += (settings.max_memory < 1024UL*16UL? 7 : 1007))
      {
         memset(arena.start, arena.sentinel, arena.size);
         arena.next = 0;
         (void)json_test(file_content, &settings, error_buffer); /* just check that it doesn't crash */
      }
      /* fuzz various arena sizes with the unlimited max_memory mode */
      settings.max_memory = 0;
      for(arena.size = 0; arena.size < sizeof(large_arena)/sizeof(char); arena.size += 1)
      {
         memset(arena.start, arena.sentinel, arena.size);
         arena.next = 0;
         (void)json_test(file_content, &settings, error_buffer); /* just check that it doesn't crash */
         if(arena.next + 128 < arena.size)
         {
            fprintf(stderr, "arena high water mark: %lu\n", (unsigned long)arena.next);
            break;
         }
      }
      /* do the normal test */
      settings.max_memory = 0;
      settings.mem_alloc = noisy_alloc;
      settings.mem_free = noisy_free;
      settings.user_data = filename_buffer; /* for debugging */
      switch(json_test(file_content, &settings, error_buffer))
      {
      case json_test_parsed:
         if(expect_failure == 1)
         {
            result = 1;
            fprintf(stderr, "unexpected acceptance for %s\n", filename_buffer);
         }
         break;
      case json_test_rejected:
         if(expect_failure != 1)
         {
            result = 1;
            fprintf(stderr, "unexpected rejection for %s, given error: %s\n", filename_buffer, error_buffer);
         }
         break;
      case json_test_missing:
      case json_test_error:
         result = 1;
         break;
      }
      free(file_content.content);
   }
   /* unreachable */
   return result;
}

static int json_compare_string(const char * input_json, size_t input_json_len, const char * expected, size_t expected_len)
{
   int ret = 2;
   json_value * parsed_json = json_parse(input_json, input_json_len);
   if(parsed_json != NULL)
   {
      ret = 1;
      if(parsed_json->type == json_string
      && parsed_json->u.string.length == expected_len
      && 0 == memcmp(parsed_json->u.string.ptr, expected, expected_len))
      {
         ret = 0;
      }
      json_value_free(parsed_json);
   }
   return ret;
}

int main(void)
{
   int exit_code = EXIT_SUCCESS;

   #define JSON_COMPARE_STRING(r, j, s) \
   if(r != json_compare_string("\"" j "\"", (sizeof(j)/sizeof(j[0])) + 1, s, (sizeof(s)/sizeof(s[0])) - 1))\
   {\
      exit_code = EXIT_FAILURE;\
      fprintf(stderr, "string comparison on line %i failed\n", __LINE__);\
   }\
   else\
   {\
      fprintf(stderr, "string comparison on line %i succeeded\n", __LINE__);\
   }
   JSON_COMPARE_STRING(0, "", "");
   JSON_COMPARE_STRING(1, "a", "");
   JSON_COMPARE_STRING(1, "", "a");
   JSON_COMPARE_STRING(0, "a", "a");
   JSON_COMPARE_STRING(0, "\n", "\n");
   JSON_COMPARE_STRING(0, "\r\n", "\r\n");
   JSON_COMPARE_STRING(0, "\\n", "\n");
   JSON_COMPARE_STRING(0, "\\r\\n", "\r\n");
   JSON_COMPARE_STRING(2, "abc \0 123", "abc \0 123"); /* TODO: should this really be disallowed? */
   JSON_COMPARE_STRING(0, "abc \\u0000 123", "abc \0 123");
   JSON_COMPARE_STRING(1, "\\ud841\\udf31", "𠜱"); /* TODO: this should actually succeed after PR #58 is merged */

   if(0 != json_verify(      "valid-%04u.json", 13, 0, 0)){ exit_code = EXIT_FAILURE; }
   if(0 != json_verify(    "invalid-%04u.json", 10, 0, 1)){ exit_code = EXIT_FAILURE; }
   if(0 != json_verify(  "ext-valid-%04u.json",  3, 1, 0)){ exit_code = EXIT_FAILURE; }
   if(0 != json_verify("ext-invalid-%04u.json",  2, 1, 1)){ exit_code = EXIT_FAILURE; }

   return exit_code;
}
