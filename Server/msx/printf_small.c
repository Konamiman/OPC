/*-------------------------------------------------------------------------
   printf_small.c - source file for reduced version of printf

   Copyright(C) 1999, Sandeep Dutta <sandeep.dutta AT ieee.org>
   Modified for pic16 port, by Vangelis Rokas, 2004 <vrokas AT otenet.gr>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or(at your option) any
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
 * the linker will link the first function(i.e. the user's function) */

/* following formats are supported :-
   format     output type       argument-type
     %d        decimal             int
     %ld       decimal             long
     %x        hexadecimal         int
     %lx       hexadecimal         long
     %o        octal               int
     %lo       octal               long
     %c        character           char
     %s        character           generic pointer

     %u, %lu
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void _ultoa(long val, char* buffer, char radix);
void _uitoa(int val, char* buffer, char radix);

static int format_string(const char* buf, const char *fmt, va_list ap);

int printf(const char *fmt, ...)
{
  va_list arg;
  va_start(arg, fmt);
  return format_string(0, fmt, arg);
}

int sprintf(const char* buf, const char* fmt, ...)
{
  va_list arg;
  va_start(arg, fmt);
  return format_string(buf, fmt, arg);
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

  ld(hl),e
  ret

  __endasm;
}

#define do_char_inc(c) {do_char(bufPnt,c); if(bufPnt) { bufPnt++; } count++;}

static int format_string(const char* buf, const char *fmt, va_list ap)
{
  char *fmtPnt;
  char *bufPnt;
  char radix;
  char isLong;
  char isUnsigned;
  char *str;
  long val;
  static char buffer[16];
  char theChar;
  int count=0;

  fmtPnt = fmt;
  bufPnt = buf;

  while((theChar = *fmtPnt)!=0)
  {
    isLong = isUnsigned = 0;
    radix = 0;

    fmtPnt++;

    if(theChar != '%') {
      do_char_inc(theChar);
      continue;
    }

    theChar = *fmtPnt;
    fmtPnt++;

    if(theChar == 's')
    {
      str = va_arg(ap, char *);
      while(*str)
        do_char_inc(*str++);

      continue;
    } 

    if(theChar == 'c')
    {
      val = va_arg(ap, int);
      do_char_inc((char) val);

      continue;
    } 

    if(theChar == 'l')
    {
      isLong = 1;
      theChar = *fmtPnt;
      fmtPnt++;
    }

    if(theChar == 'd')
      radix = 10;
    else if(theChar == 'x')
      radix = 16;
    else if(theChar == 'o')
      radix = 8;
    else if(theChar == 'u') {
      radix = 10;
      isUnsigned = 1;
    }

    if(!radix) {
      do_char_inc(theChar);
      continue;
    }

    if(isLong)
      val = va_arg(ap, long);
    else
      val = va_arg(ap, int);

    if(isUnsigned)
      _ultoa(val, buffer, radix);
    else
      _ltoa(val, buffer, radix);

    str = buffer;
    while((theChar = *str++) != 0) 
      do_char_inc(theChar);
  }

  if(bufPnt) *bufPnt = '\0';

  return count;
}
