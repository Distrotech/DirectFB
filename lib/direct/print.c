/*
   (c) Copyright 2001-2008  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>

#include <direct/mem.h>
#include <direct/print.h>
#include <direct/system.h>
#include <direct/trace.h>


/**********************************************************************************************************************/

/* Decimal conversion is by far the most typical, and is used
 * for /proc and /sys data. This directly impacts e.g. top performance
 * with many processes running. We optimize it for speed
 * using code from
 * http://www.cs.uiowa.edu/~jones/bcd/decimal.html
 * (with permission from the author, Douglas W. Jones). */

/* Formats correctly any integer in [0,99999].
 * Outputs from one to five digits depending on input.
 * On i386 gcc 4.1.2 -O2: ~250 bytes of code. */
static char* put_dec_trunc(char *buf, unsigned q)
{
     unsigned d3, d2, d1, d0;
     d1 = (q>>4) & 0xf;
     d2 = (q>>8) & 0xf;
     d3 = (q>>12);

     d0 = 6*(d3 + d2 + d1) + (q & 0xf);
     q = (d0 * 0xcd) >> 11;
     d0 = d0 - 10*q;
     *buf++ = d0 + '0'; /* least significant digit */
     d1 = q + 9*d3 + 5*d2 + d1;
     if (d1 != 0) {
          q = (d1 * 0xcd) >> 11;
          d1 = d1 - 10*q;
          *buf++ = d1 + '0'; /* next digit */

          d2 = q + 2*d2;
          if ((d2 != 0) || (d3 != 0)) {
               q = (d2 * 0xd) >> 7;
               d2 = d2 - 10*q;
               *buf++ = d2 + '0'; /* next digit */

               d3 = q + 4*d3;
               if (d3 != 0) {
                    q = (d3 * 0xcd) >> 11;
                    d3 = d3 - 10*q;
                    *buf++ = d3 + '0';  /* next digit */
                    if (q != 0)
                         *buf++ = q + '0';  /* most sign. digit */
               }
          }
     }
     return buf;
}
/* Same with if's removed. Always emits five digits */
static char* put_dec_full(char *buf, unsigned q)
{
     /* BTW, if q is in [0,9999], 8-bit ints will be enough, */
     /* but anyway, gcc produces better code with full-sized ints */
     unsigned d3, d2, d1, d0;
     d1 = (q>>4) & 0xf;
     d2 = (q>>8) & 0xf;
     d3 = (q>>12);

     /* Possible ways to approx. divide by 10 */
     /* gcc -O2 replaces multiply with shifts and adds */
     // (x * 0xcd) >> 11: 11001101 - shorter code than * 0x67 (on i386)
     // (x * 0x67) >> 10:  1100111
     // (x * 0x34) >> 9:    110100 - same
     // (x * 0x1a) >> 8:     11010 - same
     // (x * 0x0d) >> 7:      1101 - same, shortest code (on i386)

     d0 = 6*(d3 + d2 + d1) + (q & 0xf);
     q = (d0 * 0xcd) >> 11;
     d0 = d0 - 10*q;
     *buf++ = d0 + '0';
     d1 = q + 9*d3 + 5*d2 + d1;
     q = (d1 * 0xcd) >> 11;
     d1 = d1 - 10*q;
     *buf++ = d1 + '0';

     d2 = q + 2*d2;
     q = (d2 * 0xd) >> 7;
     d2 = d2 - 10*q;
     *buf++ = d2 + '0';

     d3 = q + 4*d3;
     q = (d3 * 0xcd) >> 11; /* - shorter code */
     /* q = (d3 * 0x67) >> 10; - would also work */
     d3 = d3 - 10*q;
     *buf++ = d3 + '0';
     *buf++ = q + '0';
     return buf;
}

/* No inlining helps gcc to use registers better */
static char* put_dec(char *buf, unsigned long long num)
{
     while (1) {
          unsigned rem;
          if (num < 100000)
               return put_dec_trunc(buf, num);
          rem = num % 100000;
          num /= 100000;
          buf = put_dec_full(buf, rem);
     }
}

#define ZEROPAD	1		/* pad with zero */
#define SIGNED	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SMALL	32		/* Must be 32 == 0x20 */
#define SPECIAL_	64		/* 0x */

static char *number(char *buf, char *end, unsigned long long num, int base, int size, int precision, int type)
{
     /* we are called with base 8, 10 or 16, only, thus don't need "G..."  */
     static const char digits[16] = "0123456789ABCDEF"; /* "GHIJKLMNOPQRSTUVWXYZ"; */

     char tmp[66];
     char sign;
     char locase;
     int need_pfx = ((type & SPECIAL_) && base != 10);
     int i;

     /* locase = 0 or 0x20. ORing digits or letters with 'locase'
      * produces same digits or (maybe lowercased) letters */
     locase = (type & SMALL);
     if (type & LEFT)
          type &= ~ZEROPAD;
     sign = 0;
     if (type & SIGNED) {
          if ((signed long long) num < 0) {
               sign = '-';
               num = - (signed long long) num;
               size--;
          }
          else if (type & PLUS) {
               sign = '+';
               size--;
          }
          else if (type & SPACE) {
               sign = ' ';
               size--;
          }
     }
     if (need_pfx) {
          size--;
          if (base == 16)
               size--;
     }

     /* generate full string in tmp[], in reverse order */
     i = 0;
     if (num == 0)
          tmp[i++] = '0';
     /* Generic code, for any base:
     else do {
          tmp[i++] = (digits[do_div(num,base)] | locase);
     } while (num != 0);
     */
     else if (base != 10) { /* 8 or 16 */
          int mask = base - 1;
          int shift = 3;
          if (base == 16) shift = 4;
          do {
               tmp[i++] = (digits[((unsigned char)num) & mask] | locase);
               num >>= shift;
          } while (num);
     }
     else { /* base 10 */
          i = put_dec(tmp, num) - tmp;
     }

     /* printing 100 using %2d gives "100", not "00" */
     if (i > precision)
          precision = i;
     /* leading space padding */
     size -= precision;
     if (!(type & (ZEROPAD+LEFT))) {
          while (--size >= 0) {
               if (buf < end)
                    *buf = ' ';
               ++buf;
          }
     }
     /* sign */
     if (sign) {
          if (buf < end)
               *buf = sign;
          ++buf;
     }
     /* "0x" / "0" prefix */
     if (need_pfx) {
          if (buf < end)
               *buf = '0';
          ++buf;
          if (base == 16) {
               if (buf < end)
                    *buf = ('X' | locase);
               ++buf;
          }
     }
     /* zero or space padding */
     if (!(type & LEFT)) {
          char c = (type & ZEROPAD) ? '0' : ' ';
          while (--size >= 0) {
               if (buf < end)
                    *buf = c;
               ++buf;
          }
     }
     /* hmm even more zero padding? */
     while (i <= --precision) {
          if (buf < end)
               *buf = '0';
          ++buf;
     }
     /* actual digits of result */
     while (--i >= 0) {
          if (buf < end)
               *buf = tmp[i];
          ++buf;
     }
     /* trailing space padding */
     while (--size >= 0) {
          if (buf < end)
               *buf = ' ';
          ++buf;
     }
     return buf;
}

static int skip_atoi(const char **s)
{
     int i=0;

     while (isdigit(**s))
          i = i*10 + *((*s)++) - '0';
     return i;
}

static char *string(char *buf, char *end, char *s, int field_width, int precision, int flags)
{
     int len, i;

     if ((unsigned long)s < direct_pagesize())
          s = "<NULL>";

     len = strnlen(s, precision);

     if (!(flags & LEFT)) {
          while (len < field_width--) {
               if (buf < end)
                    *buf = ' ';
               ++buf;
          }
     }
     for (i = 0; i < len; ++i) {
          if (buf < end)
               *buf = *s;
          ++buf; ++s;
     }
     while (len < field_width--) {
          if (buf < end)
               *buf = ' ';
          ++buf;
     }
     return buf;
}

static char *print_string(char *buf, char *end, void *ptr, int field_width, int precision, int flags)
{
#if 0 //FIXME
     char tmp[200];

     extern int direct_type_print( void *ptr, char *buf, size_t len );

     if (direct_type_print( ptr, tmp, sizeof(tmp) ) == DR_OK)
          return string(buf, end, tmp, field_width, precision, flags);
#endif

     return string(buf, end, "ERROR", field_width, precision, flags);
}

static char *symbol_string(char *buf, char *end, void *ptr, int field_width, int precision, int flags)
{
     unsigned long value = (unsigned long) ptr;

     const char *symbol = direct_trace_lookup_symbol_at( ptr );

     if (symbol)
          return string(buf, end, (char*) symbol, field_width, precision, flags);

     field_width = 2*sizeof(void *);
     flags |= SPECIAL_ | SMALL | ZEROPAD;
     return number(buf, end, value, 16, field_width, precision, flags);
}


/*
 * Show a '%p' thing.  A kernel extension is that the '%p' is followed
 * by an extra set of alphanumeric characters that are extended format
 * specifiers.
 *
 * Right now we handle:
 *
 * - 'F' For symbolic function descriptor pointers
 * - 'S' For symbolic direct pointers
 * - 'R' For a struct resource pointer, it prints the range of
 *       addresses (not the name nor the flags)
 * - 'M' For a 6-byte MAC address, it prints the address in the
 *       usual colon-separated hex notation
 * - 'I' [46] for IPv4/IPv6 addresses printed in the usual way (dot-separated
 *       decimal for v4 and colon separated network-order 16 bit hex for v6)
 * - 'i' [46] for 'raw' IPv4/IPv6 addresses, IPv6 omits the colons, IPv4 is
 *       currently the same
 *
 * Note: The difference between 'S' and 'F' is that on ia64 and ppc64
 * function pointers are really function descriptors, which contain a
 * pointer to the real address.
 */
static char *pointer(const char *fmt, char *buf, char *end, void *ptr, int field_width, int precision, int flags)
{
     if (!ptr)
          return string(buf, end, "(null)", field_width, precision, flags);

     switch (*fmt) {
          case 'p':
               return print_string(buf, end, ptr, field_width, precision, flags);
          case 'F':
               /* Fallthrough */
          case 'S':
               return symbol_string(buf, end, ptr, field_width, precision, flags);
          case 'R':
               return NULL;//resource_string(buf, end, ptr, field_width, precision, flags);
          case 'm':
               flags |= SPECIAL_;
               /* Fallthrough */
          case 'M':
               return NULL;//mac_address_string(buf, end, ptr, field_width, precision, flags);
          case 'i':
               flags |= SPECIAL_;
               /* Fallthrough */
          case 'I':
               if (fmt[1] == '6')
                    return NULL;//ip6_addr_string(buf, end, ptr, field_width, precision, flags);
               if (fmt[1] == '4')
                    return NULL;//ip4_addr_string(buf, end, ptr, field_width, precision, flags);
               flags &= ~SPECIAL_;
               break;
     }
     flags |= SMALL;
     if (field_width == -1) {
          field_width = 2*sizeof(void *);
          flags |= ZEROPAD;
     }
     return number(buf, end, (unsigned long) ptr, 16, field_width, precision, flags);
}

/**
 * vsnprintf - Format a string and place it in a buffer
 * @buf: The buffer to place the result into
 * @size: The size of the buffer, including the trailing null space
 * @fmt: The format string to use
 * @args: Arguments for the format string
 *
 * This function follows C99 vsnprintf, but has some extensions:
 * %pS output the name of a text symbol
 * %pF output the name of a function pointer
 * %pR output the address range in a struct resource
 *
 * The return value is the number of characters which would
 * be generated for the given input, excluding the trailing
 * '\0', as per ISO C99. If you want to have the exact
 * number of characters written into @buf as return value
 * (not including the trailing '\0'), use vscnprintf(). If the
 * return is greater than or equal to @size, the resulting
 * string is truncated.
 *
 * Call this function if you are already dealing with a va_list.
 * You probably want snprintf() instead.
 */
int
direct_vsnprintf( char       *buffer,
                  size_t      bytes,
                  const char *format,
                  va_list     args )
{
     unsigned long long num;
     int base;
     char *str, *end, c;

     int flags;          /* flags to number() */

     int field_width;    /* width of output field */
     int precision;      /* min. # of digits for integers; max
                       number of chars for from string */
     int qualifier;      /* 'h', 'l', or 'L' for integer fields */
     /* 'z' support added 23/7/1999 S.H.    */
     /* 'z' changed to 'Z' --davidm 1/25/99 */
     /* 't' added for ptrdiff_t */


#if DIRECT_BUILD_OSTYPE == DIRECT_OS_LINUX_GNU_LIBC
     return vsnprintf( buffer, bytes, format, args );
#endif

     /* Reject out-of-range values early.  Large positive sizes are
        used for unknown buffer sizes. */
     if (D_UNLIKELY((int) bytes < 0)) {
          D_ONCE( "bytes < 0" );
          return 0;
     }

     str = buffer;
     end = buffer + bytes;

     /* Make sure end is always >= buffer */
     if (end < buffer) {
          end = ((void *)-1);
          bytes = end - buffer;
     }

     for (; *format ; ++format) {
          if (*format != '%') {
               if (str < end)
                    *str = *format;
               ++str;
               continue;
          }

          /* process flags */
          flags = 0;
          repeat:
          ++format;      /* this also skips first '%' */
          switch (*format) {
               case '-': flags |= LEFT; goto repeat;
               case '+': flags |= PLUS; goto repeat;
               case ' ': flags |= SPACE; goto repeat;
               case '#': flags |= SPECIAL_; goto repeat;
               case '0': flags |= ZEROPAD; goto repeat;
          }

          /* get field width */
          field_width = -1;
          if (isdigit(*format))
               field_width = skip_atoi(&format);
          else if (*format == '*') {
               ++format;
               /* it's the next argument */
               field_width = va_arg(args, int);
               if (field_width < 0) {
                    field_width = -field_width;
                    flags |= LEFT;
               }
          }

          /* get the precision */
          precision = -1;
          if (*format == '.') {
               ++format;
               if (isdigit(*format))
                    precision = skip_atoi(&format);
               else if (*format == '*') {
                    ++format;
                    /* it's the next argument */
                    precision = va_arg(args, int);
               }
               if (precision < 0)
                    precision = 0;
          }

          /* get the conversion qualifier */
          qualifier = -1;
          if (*format == 'h' || *format == 'l' || *format == 'L' ||
              *format =='Z' || *format == 'z' || *format == 't') {
               qualifier = *format;
               ++format;
               if (qualifier == 'l' && *format == 'l') {
                    qualifier = 'L';
                    ++format;
               }
          }

          /* default base */
          base = 10;

          switch (*format) {
               case 'c':
                    if (!(flags & LEFT)) {
                         while (--field_width > 0) {
                              if (str < end)
                                   *str = ' ';
                              ++str;
                         }
                    }
                    c = (unsigned char) va_arg(args, int);
                    if (str < end)
                         *str = c;
                    ++str;
                    while (--field_width > 0) {
                         if (str < end)
                              *str = ' ';
                         ++str;
                    }
                    continue;

               case 's':
                    str = string(str, end, va_arg(args, char *), field_width, precision, flags);
                    continue;

               case 'p':
                    str = pointer(format+1, str, end,
                                  va_arg(args, void *),
                                  field_width, precision, flags);
                    /* Skip all alphanumeric pointer suffixes */
                    while (isalnum(format[1]))
                         format++;
                    continue;

               case 'n':
                    /* FIXME:
                    * What does C99 say about the overflow case here? */
                    if (qualifier == 'l') {
                         long * ip = va_arg(args, long *);
                         *ip = (str - buffer);
                    }
                    else if (qualifier == 'Z' || qualifier == 'z') {
                         size_t * ip = va_arg(args, size_t *);
                         *ip = (str - buffer);
                    }
                    else {
                         int * ip = va_arg(args, int *);
                         *ip = (str - buffer);
                    }
                    continue;

               case '%':
                    if (str < end)
                         *str = '%';
                    ++str;
                    continue;

                    /* integer number formats - set up the flags and "break" */
               case 'o':
                    base = 8;
                    break;

               case 'x':
                    flags |= SMALL;
               case 'X':
                    base = 16;
                    break;

               case 'd':
               case 'i':
                    flags |= SIGNED;
               case 'u':
                    break;

               default:
                    if (str < end)
                         *str = '%';
                    ++str;
                    if (*format) {
                         if (str < end)
                              *str = *format;
                         ++str;
                    }
                    else {
                         --format;
                    }
                    continue;
          }
          if (qualifier == 'L')
               num = va_arg(args, long long);
          else if (qualifier == 'l') {
               num = va_arg(args, unsigned long);
               if (flags & SIGNED)
                    num = (signed long) num;
          }
          else if (qualifier == 'Z' || qualifier == 'z') {
               num = va_arg(args, size_t);
          }
          else if (qualifier == 't') {
               num = va_arg(args, ptrdiff_t);
          }
          else if (qualifier == 'h') {
               num = (unsigned short) va_arg(args, int);
               if (flags & SIGNED)
                    num = (signed short) num;
          }
          else {
               num = va_arg(args, unsigned int);
               if (flags & SIGNED)
                    num = (signed int) num;
          }
          str = number(str, end, num, base,
                       field_width, precision, flags);
     }
     if (bytes > 0) {
          if (str < end)
               *str = '\0';
          else
               end[-1] = '\0';
     }
     /* the trailing null byte doesn't count towards the total */
     return str-buffer;
}

/**********************************************************************************************************************/

int
direct_snprintf( char       *buffer,
                 size_t      bytes,
                 const char *format,
                 ... )
{
     int     ret;
     va_list args;

     va_start( args, format );
     ret = direct_vsnprintf( buffer, bytes, format, args );
     va_end( args );

     return ret;
}

/**********************************************************************************************************************/
/**********************************************************************************************************************/

DirectResult
direct_print( char        *buf,
              size_t       size,
              const char  *format,
              va_list      args,
              char       **ret_ptr )
{
     int len = 1;

     *ret_ptr = buf;

     if (buf) {
          va_list args_copy;

          buf[0] = 0;

          va_copy( args_copy, args );
          len = direct_vsnprintf( buf, size, format, args_copy );
          va_end( args_copy );

          if (len < 0)
               return DR_FAILURE;
     }
     else
          size = 0;


     if (len >= size) {
          char *ptr = buf;

          ptr = direct_malloc( len+1 );
          if (!ptr)
               return DR_NOLOCALMEMORY;

          len = direct_vsnprintf( ptr, len+1, format, args );
          if (len < 0) {
               direct_free( ptr );
               return DR_FAILURE;
          }

          *ret_ptr = ptr;
     }

     return DR_OK;
}

