#ifndef	__SPRINTF_DOT_H
#define	__SPRINTF_DOT_H

#include "sci.h"

/*************************** INTERFACE FOR TYPICAL PUBLIC 'FUNCTIONS': ******************************
Basic functions: all other formatting functions in this file call these: */

#define         _sltoa(value,str,radix)   my_sltoa((long         )(value),str,radix)
#define         _sstoa(value,str,radix)   my_sltoa((long         )(value),str,radix)
#define         _sctoa(value,str,radix)   my_sltoa((long         )(value),str,radix)
#define         _itoa(value,str,radix)    my_sltoa((long         )(value),str,radix)
#define         _ultoa(value,str,radix)   my_ultoa((unsigned long)(value),str,radix)
#define         _ustoa(value,str,radix)   my_ultoa((unsigned long)(value),str,radix)
#define         _uctoa(value,str,radix)   my_ultoa((unsigned long)(value),str,radix)
#define         _utoa(value,str,radix)    my_ultoa((unsigned long)(value),str,radix)

// Common-usage "functions", implemented as another group of macros:
#define         _ltoa(value,str,radix)    _sltoa(value,str,radix) // signed   long
#define         _stoa(value,str,radix)    _sstoa(value,str,radix) // signed   short
#define         _sitoa(value,str,radix)   _itoa(value,str,radix)  // signed   int
#define         _uitoa(value,str,radix)   _utoa(value,str,radix)  // unsigned int

// These are not typically used..
char           *my_sltoa(         long value, char* str, int radix); // signed   long
char           *my_ultoa(unsigned long value, char* str, int radix); // unsigned long

char *strrev(char *string);
int strlpad(char *string, byte width, char pad);
int str_ins(char *string, byte position, char insert);
char* UnsignedShortToString(unsigned short num, char* str, int base);
void reverse(char str[], int length);

/*****************************************************************************
_sprintf()      Simplified & customised implementation of sprintf()

Input:  'format'        Format string based on ANSI usage, but modified.
        ...             Values to be converted
Modify: 'buffer'        Destination for output string
Return: Number of bytes output

Notes:
        Format specifier:
        %[flags][width][size_modifier]type_char

        flags:  not currently supported

        width:
                decimal string { 0n | n }
                n => minimum width
                0 => pad with leading '0' else pad with leading ' '
                * => variable width read from parameter before value

        size_modifier:
                l => long int

        type_char:
                c       character (char)
                d       decimal (int)
                p       pointer (same as hexadecimal)
                s       string, NUL terminated (char *)
                x,X     hexadecimal (int) uppercase
                f	'float' interpretation of long (NOT CURRENTLY SUPPORTED)

Rev:
0.00    18-JAN-2002     Creation
0.10    31-JAN-2002     _sprintf() handles unknown type_char with '?'
                        added ver info and commenting
*****************************************************************************/
int _sprintf(char *buffer, const char *format, ...);


#endif /* SPRINTF_H_ */
