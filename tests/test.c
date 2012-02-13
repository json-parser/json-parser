
/* vim: set et ts=3 sw=3 ft=c:
 *
 * Copyright (C) 2012 James McLaughlin et al.  All rights reserved.
 * https://github.com/udp/json-parser
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "../json.c"

int main (int argc, char * argv [])
{
    int valid = 1, succeeded = 0, failed = 0;
    json_settings settings;

    memset (&settings, 0, sizeof (settings));

    while (valid >= 0)
    {
        int i;

        for (i = 1 ;; ++ i)
        {
            json_value * value;
            char filename [16];
            FILE * file;
            char * buf;
            unsigned int length;
            char error [256];

            sprintf (filename, valid ? "valid-%04d.json" : "invalid-%04d.json", i);

            if (! (file = fopen (filename, "rb")))
                break;

            fseek (file, 0, SEEK_END);
            length = ftell (file);
            fseek (file, 0, SEEK_SET);
            buf = malloc (length + 1);
            fread (buf, 1, length, file);
            fclose (file);

            buf [length] = 0;

            if (value = json_parse_ex (&settings, buf, error))
            {
                if (valid)
                {   
                    printf ("%s succeeded (type = %d)\n", filename, value->type);
                    ++ succeeded;
                }
                else
                {
                    printf ("%s failed (type = %d, should have been error)\n",
                                    filename, value->type);

                    ++ failed;
                }

                json_value_free (value);
            }
            else
            {
                if (!valid)
                {
                    printf ("%s succeeded : %s\n", filename, error);
                    ++ succeeded;
                }
                else
                {
                    printf ("%s failed : %s\n", filename, error);
                    ++ failed;
                }

                json_value_free (value);
            }
        }
        
        -- valid;
    }

    printf ("*** %d tests executed, of which %d succeeded and %d failed\n",
                    succeeded + failed, succeeded, failed);
}



