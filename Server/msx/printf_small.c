/*-------------------------------------------------------------------------
   printf_small.c - source file for reduced version of printf

   Copyright (C) 1999, Sandeep Dutta <sandeep.dutta AT ieee.org>
   Modified for pic16 port, by Vangelis Rokas, 2004 <vrokas AT otenet.gr>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License 
   along with this library; see the file COPYING. If not, write to the
   Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
   MA 02110-1301, USA.

   As a special exception, if you link this library with other files,
   some of which are compiled with SDCC, to produce an executable,
   this library does not by itself cause the resulting executable to
   be covered by the GNU General Public License. This exception does
   not however invalidate any other reasons why the executable file
   might be covered by the GNU General Public License.
-------------------------------------------------------------------------*/

/* This function uses function putchar() to dump a character
 * to standard output. putchar() is defined in libc18f.lib
 * as dummy function, which will be linked if no putchar()
 * function is provided by the user.
 * The user can write his own putchar() function and link it
 * with the source *BEFORE* the libc18f.lib library. This way
 * the linker will link the first function (i.e. the user's function) */

/* following formats are supported :-
   format     output type       argument-type
     %d        decimal             int
     %ld       decimal             long
     %hd       decimal             char
     %x        hexadecimal         int
     %lx       hexadecimal         long
     %hx       hexadecimal         char
     %o        octal               int
     %lo       octal               long
     %ho       octal               char
     %c        character           char
     %s        character           generic pointer

     %u, %lu
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void _ultoa(long val, char* buffer, char radix);
void _uitoa(int val, char* buffer, char radix);

static void format_string_small (char* buf, const char *fmt, va_list ap);

void printf_small(const char *fmt, ...)
{
  va_list arg;
  va_start (arg, fmt);
  format_string_small(0, fmt, arg);
}

void sprintf_small(const char* buf, const char* fmt, ...)
{
  va_list arg;
  va_start (arg, fmt);
  format_string_small(buf, fmt, arg);
}

static void do_char(const char* buf, char c) __naked
{
  __asm

  ld hl,#4
  add hl,sp
  ld e,(hl)

  dec hl
  ld a,(hl)
  dec hl
  ld l,(hl)
  ld h,a
  or l

  jp z,_putchar_rr_dbs

  ld (hl),e
  ret

  __endasm;
}

#define do_char_inc(buf, c) {do_char(buf,c); if(buf) buf++;}

static void format_string_small (char* buf, const char *fmt, va_list ap)
{
  const char *ch;
  char radix;
  char flong;
  char fstr;
  char fchar;
  char funsigned;
  char *str;
  char *str1;
  long val;
  static char buffer[16];

  ch = fmt;

  while (*ch)
  {
    if (*ch == '%')
    {
      flong = fstr = fchar = funsigned = 0;
      radix = 0;
      ++ch;

      if (*ch == 'l')
        {
          flong = 1;
          ++ch;
        }
      else if (*ch == 'h')
        {
          fchar = 1;
          ++ch;
        }

      if (*ch == 's')
        fstr = 1;
      else if (*ch == 'd')
        radix = 10;
      else if (*ch == 'x')
        radix = 16;
      else if (*ch == 'c')
        radix = 0;
      else if (*ch == 'o')
        radix = 8;
      else if (*ch == 'u') {
        radix = 10;
        funsigned = 1;
      }

      if (fstr)
        {
          str = va_arg (ap, char *);
          while (*str)
            do_char_inc (buf, *str++);
        }
      else
        {
          if (flong)
            val = va_arg (ap, long);
          else if (fchar)
            val = (char) va_arg (ap, int);  // FIXME: SDCC casts char arguments into ints
          else
            {
              val = va_arg (ap, int);
            }

          if (radix)
            {
              if(funsigned)
                _ultoa (val, buffer, radix);
              else
                _ltoa (val, buffer, radix);

              str1 = buffer;
              while (*str1)
                {
                  do_char_inc (buf,*str1++);
                }
            }
          else
            do_char_inc (buf, (char) val);
        }
    }
    else
      do_char_inc (buf,*ch);

    ++ch;
  }
  if(buf) *buf = '\0';
}
