#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"

enum json_test_result
{
   json_test_parsed, /* success */
   json_test_rejected, /* task failed successfully */
   json_test_missing, /* fopen failed */
   json_test_error /* unusual test error such as file seek failure or memory allocation failure */
};

static enum json_test_result json_test(const char * filename_buffer, json_settings * settings, char * error_buffer)
{
   FILE * input_json;
   int seek_end_result;
   long tell_result;
   int seek_set_result;
   char * input_json_buffer;
   size_t read_result;
   json_value * parsed_json;

   input_json = fopen(filename_buffer, "rb");
   if(input_json == NULL)
   {
      return json_test_missing;
   }

   seek_end_result = fseek(input_json, 0, SEEK_END);
   if(seek_end_result != 0)
   {
      fprintf(stderr, "fseek end error, %i\n", seek_end_result);
      fclose(input_json);
      return json_test_error;
   }

   tell_result = ftell(input_json);
   if(tell_result < 0)
   {
      fprintf(stderr, "ftell error, %li\n", tell_result);
      fclose(input_json);
      return json_test_error;
   }

   seek_set_result = fseek(input_json, 0, SEEK_SET);
   if(seek_set_result != 0)
   {
      fprintf(stderr, "fseek set error, %i\n", seek_set_result);
      fclose(input_json);
      return json_test_error;
   }

   if(tell_result == 0)
   {
      input_json_buffer = (char *)calloc(1, 1);
   }
   else
   {
      input_json_buffer = (char *)malloc(tell_result);
   }
   if(input_json_buffer == NULL)
   {
      fprintf(stderr, "memory allocation failed, %li bytes\n", tell_result);
      fclose(input_json);
      return json_test_error;
   }

   read_result = fread(input_json_buffer, 1, tell_result, input_json);
   fclose(input_json);
   if(read_result < (size_t)tell_result)
   {
      fprintf(stderr, "fread error, %lu\n", (unsigned long)read_result);
      free(input_json_buffer);
      return json_test_error;
   }

   parsed_json = json_parse_ex(settings, input_json_buffer, tell_result, error_buffer);
   free(input_json_buffer);
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

static int json_verify(const char * filename_format, unsigned highest_file_num, int extensions, int expect_failure)
{
   int result = 0;
   unsigned test_num;
   char filename_buffer[32];
   int filename_buffer_result;
   json_settings settings =
   {
      /*  max_memory */ 0,
      /*    settings */ 0,
      /*   mem_alloc */ noisy_alloc,
      /*   mem_free  */ noisy_free,
      /*   user_data */ NULL,
      /* value_extra */ 0
   };
   char error_buffer[json_error_max];

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
      settings.user_data = filename_buffer;
      switch(json_test(filename_buffer, &settings, error_buffer))
      {
      case json_test_parsed:
         if(expect_failure == 1)
         {
            result = 1;
            fprintf(stderr, "unexpected acceptance for %s\n", filename_buffer);
         }
         continue;
      case json_test_rejected:
         if(expect_failure != 1)
         {
            result = 1;
            fprintf(stderr, "unexpected rejection for %s, given error: %s\n", filename_buffer, error_buffer);
         }
         continue;
      case json_test_missing:
         if(test_num != highest_file_num + 1)
         {
            result = 1;
            fprintf(stderr, "mismatched number of test input files for %s, highest expected was %u but file %u was missing\n", filename_format, highest_file_num, test_num);
         }
         return result;
      case json_test_error:
         result = 1;
         continue;
      }
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
