#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

char* strip_colour(char* string)
{
  char *c = string;
  char *c2 = string;
  char *last_non_space = NULL;
  /* c is source, c2 is target */
  for (; c && *c; c++)
    switch(*c)
      {
      case 3:
        if (isdigit(c[1]))
          {
            c++;
            if (isdigit(c[1]))
              c++;
            if (c[1] == ',' && isdigit(c[2]))
              {
                c+=2;
                if (isdigit(c[1]))
                  c++;
              }
          }
        break;
      case 2: case 6: case 7: case 22: case 23: case 27: case 31:
        break;
      case 32:
        *c2++ = *c;
        break;
      default:
        *c2++ = *c;
        last_non_space = c2;
        break;
      }
  *c2 = '\0';
  if (last_non_space)
    *last_non_space = '\0';
  return string;
}
