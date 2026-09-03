#define JSON_OBJECT_ELEMENTS_MAX 3
#define JSON_ARRAY_ELEMENTS_MAX 4
#define JSON_STRING_LENGTH_MAX 7
#define JSON_OBJECT_NAMES_COMBINED_LENGTH_MAX 6

#include <stdio.h>
#include <stdlib.h>

#include "json.h"
#include "json.c"

static const char pass0[] = "[4,3,2,1]";
static const char fail0[] = "[4,3,2,1,0]";

static const char pass1[] = "{\"\":3,\"\":2,\"\":1,}";
static const char fail1[] = "{\"\":3,\"\":2,\"\":1,\"\":0}";

static const char pass2[] = "\"7654321\"";
static const char fail2[] = "\"76543210\"";

static const char pass3[] = "{\"ab\":0,\"cd\":0}";
static const char fail3[] = "{\"ab\":0,\"cde\":0}";

static const char pass4[] = "{\"abcd\":0,\"\":0}";
static const char fail4[] = "{\"abcde\":0,\"\":0}";

static int exit_code = EXIT_SUCCESS;

static void expect_pass(const char str[], size_t len_plus_null)
{
   json_value * temp = json_parse(str, len_plus_null - 1);
   json_value_free(temp);
   if(temp) return;
   exit_code = EXIT_FAILURE;
   fprintf(stderr, "unexpected fail: %.*s\n", ((int) (len_plus_null - 1)), str);
}
static void expect_fail(const char str[], size_t len_plus_null)
{
   json_value * temp = json_parse(str, len_plus_null - 1);
   json_value_free(temp);
   if(!temp) return;
   exit_code = EXIT_FAILURE;
   fprintf(stderr, "unexpected pass: %.*s\n", ((int) (len_plus_null - 1)), str);
}

int main(void)
{
   expect_pass(pass0, sizeof(pass0));
   expect_fail(fail0, sizeof(fail0));
   expect_pass(pass1, sizeof(pass1));
   expect_fail(fail1, sizeof(fail1));
   expect_pass(pass2, sizeof(pass2));
   expect_fail(fail2, sizeof(fail2));
   expect_pass(pass3, sizeof(pass3));
   expect_fail(fail3, sizeof(fail3));
   expect_pass(pass4, sizeof(pass4));
   expect_fail(fail4, sizeof(fail4));
   return exit_code;
}
