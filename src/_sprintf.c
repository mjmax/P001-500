/*****************************************************************************
Module:	_sprintf.c	(c) 2002        Ranger Instruments
Aim:	simplified & customised implementation of sprintf()
Product:	general library

Author:	Robert J. Dixon

NOTE:
Rev:
0.00    18-JAN-2002     RJD	Creation
0.10    31-JAN-2002     RJD	_sprintf() handles unknown type_char with '?'
                        RJD	added ver info and commenting
0.20	08-FEB-2002	RJD	added '*' for varaible field width
			RJD	added %[0][width]f for "float" as a long
			RJD	began to add [.prec] but not completed yet
0.21	10-MAY-2002	RJD	corrected "%c" handling (see comments)
0.22	06-JUN-2002	RJD	Add DEBUG_SPRINTF for testing.  Tidy comments
0.30	11-JUL-2002	RJD	major rework for code size and "%f" support

*****************************************************************************/

#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include "_sprintf.h"
#include "sci.h"
//#include "_str_lib.h"
//#include "_ltoa.h"
#include <stdlib.h>

#define NUL ('\0')

#ifndef	FALSE
#define	FALSE	(0)
#endif

#ifndef	TRUE
#define	TRUE	(!FALSE)
#endif


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
0.00    18-JAN-2002     RJD	Creation
0.10    31-JAN-2002     RJD	_sprintf() handles unknown type_char with '?'
                        	added ver info and commenting
0.20	08-FEB-2002	RJD	added '*
0.20	08-FEB-2002	RJD	added '*' for varaible field width
				added %[0][width]f for "float" as a long
				began to add [.prec] but not completed yet
0.21	10-MAY-2002	RJD	corrected "%c" handling (see comments)
*****************************************************************************/

typedef signed char flagBit;

typedef struct {
   flagBit bLeadingZero : 1;
   flagBit bConverted : 1;
   flagBit bIsLong : 1;
   flagBit bIsFloat : 1;
   flagBit bPrec : 1;
   flagBit bConvertingNumeric : 1;
   flagBit bNegative : 1;
} sprintf_flags_t;

const sprintf_flags_t clr_flags = {
   FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE
};


int _sprintf(char *buffer, const char *format, ...)
{
   va_list arglist;

   long temp_long;
   char *bj= buffer;
   char *fi = (char *) format;		//relax 'const'
   char *cp;

   unsigned char width, prec, length = 0, numeric, radix = 0;
   char pad;
   sprintf_flags_t flags = clr_flags;

   va_start(arglist, format);		// init va_args

   if ((buffer != NULL) && (format != NULL))
   {
      strcpy(buffer, "?FMT?");		// unknown or no format
      while (*fi != NUL)
      {
         if (*fi != '%')
            *(bj++) = *(fi++);  /* nothing special, copy through */
         else
         {
            switch (*(fi+1))        /* decode after '%' */
            {
               case NUL:		/* Bad format.  End now */
                  return 0;
//                  *(bj++) = *(fi++);
                  break;

               case '%':
                  *(bj++) = *(fi++);	/* copy % */
                  fi++;			/* skip %% */
                  break;

               default:
                  flags = clr_flags;
                  prec = 0;
                  width = 0;
                  numeric = 0;
                  fi++;			// skip '%'

                  do {
                     if ((!flags.bPrec) && (*fi == '0') && (!flags.bConvertingNumeric))
                        flags.bLeadingZero = TRUE;
                     else
                     {
                        if (isdigit(*fi))
                        {
                           numeric = (unsigned char)((numeric * 10) + (*fi - '0'));
                           flags.bConvertingNumeric = TRUE;
                        }
                        else
                        {
                           if (!flags.bPrec)
                              width = numeric;
                           else
                              prec = numeric;

                           radix = 10;	//assume "%d"
                           length = 0;

                           switch (*fi)	// decode type_char and convert
                           {
                              case NUL:
                                 flags.bConverted = TRUE;
                                 break;

                              case '.':
                                 numeric = 0;
                                 flags.bPrec = TRUE;
                                 break;

                              case '*':
                                 numeric = (signed char) va_arg(arglist, int);
                                 break;

                              case 'c':
                              		// According to notes on <stdarg.h>
                              		// CANNOT use va_arg(arglist, char)
                              		// due to type conversion
                                 *(bj++) = (char) va_arg(arglist, int);
                                 *bj = NUL;
                                 flags.bConverted = TRUE;
                                 length = 1;
                                 break;

                              case 'f': // *** NON-STANDARD USE OF %f ***
                                 flags.bIsFloat = TRUE;
                                 temp_long = va_arg(arglist, long);
                                 _ltoa(labs(temp_long), bj, 10);
                                 flags.bNegative =  temp_long < 0;

                                 cp = bj;

                                 while (*bj != NUL)
				    bj++;
                                 length = (byte)(bj - cp);

                                 if (width < length)
                                    width = length;

                                 if ((flags.bNegative) && (width > prec) && (prec > 0))
                                    width--;		//leave room for '-'

                                 if (prec > 0)
				 {
                                    if (prec >= length)	//'0' around '.'
                                    {
                                       length = (byte)strlpad(cp, (byte)(prec+1), (char)'0');
                                    }

                                    pad = (flags.bLeadingZero ? '0' : ' ');
                                    length = (byte)strlpad(cp, (byte)(width-1), pad);

                                    length = (byte)str_ins(cp, (byte)(length - prec), '.');

                                    bj = cp + length;
                                 } // if

                                 if (flags.bNegative)
                                 {
                                    length = (byte)str_ins(cp, 0, '-');
                                    bj = cp + length;
                                 }
                                 flags.bConverted = TRUE;
                                 break;

                              case 'l':
                                 flags.bIsLong = TRUE;
                                 break;

                              case 's':
                                 strcpy(bj, va_arg(arglist, char *));
                                 length = 0;
                                 while (*bj != NUL)
                                 {
                                    bj++;
                                    length++;
                                 }
                                 flags.bConverted = TRUE;
                                 break;

                              case 'p':
                                 radix = 0x10;	//force radix=HEX
                              case 'd':	//assume radix=10
                                 if (flags.bIsLong)
                                    _ltoa(va_arg(arglist, long int), bj, (int)radix);
                                 else
                                    _itoa(va_arg(arglist, int), bj, (int)radix);

                                 flags.bNegative =  (*bj == '-');

                                 for (length=0; (*bj != NUL); bj++)	//find end
                                    length++;
                                 flags.bConverted = TRUE;
                                 break;

                              case 'x':
                              case 'X':
                                 radix = 0x10;	//force radix=HEX
                              case 'u':
                                 if (flags.bIsLong)
                                    _ultoa(va_arg(arglist, unsigned long int), bj, (int)radix);
                                 else
                                    _ultoa((unsigned long int)(va_arg(arglist, unsigned int)), bj, (int)radix);

                                 flags.bNegative =  (*bj == '-');

                                 for (length=0; (*bj != NUL); bj++)	//find end
                                    length++;
                                 flags.bConverted = TRUE;
                                 break;

                              default:  // catch undefined type_char
                                 *bj++ = '?';
                                 *bj++ = NUL;
                                 flags.bConverted = TRUE;
                                 break;
                           } // switch
                        } // else if !isdigit()
                     } // else
                     fi++;
                  } while (!flags.bConverted);

                  bj -= length;

                  if (flags.bNegative)	//skip '-'
                  {
                     bj++;
                     if (width > 0)
		        width--;
                  }

   	          pad = (flags.bLeadingZero ? '0' : ' ');
   	          strlpad(bj, width, pad);

                  while (*bj != NUL)
                     bj++;

                  break;
            } /* switch */
         } /* else */
      } /* while */
      *bj = NUL;

   } /* if  */
   va_end(arglist);			// cleanup va_args

   return (int) (bj - buffer);
} // _sprintf()


//........................SUPPROT FUNCTIONS.............................


/*****************************************************************************
strupper()      Force a string to UPPERCASE in situ.
Modify: string[].
Return: pointer to string[].
E.G:    strupper("string") => "STRING"
*****************************************************************************/

int str_ins(char *string, byte position, char insert)
{
   char *cp;
   for (cp = string; (*cp != NUL); cp++)	//find NUL at end
      ;

   while (cp - string >= position)
   {
      *(cp+1) = *cp;
      cp--;
   }
   *(cp+1) = insert;

   for (cp = string; (*cp != NUL); cp++)	//find NUL at end
      ;

   return (int)(string - cp);
}

int strlpad(char *string, byte width, char pad)
{
   char *cp;
   byte length=0;

   for (cp = string; (*cp != NUL); cp++)	//find NUL at end
      length++;

   while (length < width)
   {
      while (cp >= string)		//work back to start
      {
         *(cp+1) = *cp;
         cp--;
      }
      *string = pad;
      length++;

      cp = string + length;		//start at NUL at end
   }
   return (int)length;
}

// #define swap (a,b) ((a)^=(b), (b)^=(a),(a)^=(b))

char *strrev(char *string)
{
   char *cp, *end;
   char temp;

   if (string != NULL)
   {
      cp = string;
      end = string;

      while (*end != NUL)		// find NUL at end of string
         end++;
      end--;				// move back to last char

      while (cp < end)		// while not meeting in middle
      {
         temp = *cp;		// swap *cp and *end
         *cp++ = *end;
         *end-- = temp;
      } // while
   } // if != NULL

   return string;
} // strrev()

const char szLookup_ltoa[] = "0123456789ABCDEF";

char *my_sltoa(long value, char* string, int radix)
{
  char *d;
  unsigned long ul;

  if ( string != NULL )
  {
    d = string;
    if ((radix == 10) && (value < 0 ))
    {
      *d++ = '-';
      ul = -value;
    }
    else
      ul = value;
    my_ultoa(ul, d, radix);
  }
  return string;
}
char *my_ultoa(unsigned long value, char* string, int radix)
{
  int i;

  if ( radix == 0 )
    radix = 10;
  if ( string == NULL || radix < 2 || radix > 16 )
    return NULL;
  i = 0;
  do
  {
    string[i++] = szLookup_ltoa[value % (unsigned long)radix];
  }
  while ( ( value /= radix ) > 0 );

  string[i] = '\0';
  strrev(string);
  return string;
}


char* UnsignedShortToString(unsigned short num, char* str, int base)
{
	int i = 0;

	if (num == 0)
	{
		str[i++] = '0';
		str[i] = '\0';
		return str;
	}
	while (num != 0)
	{
		int rem = num % base;
		str[i++] = (rem > 9)? ((rem-10) + 'a') : (rem + '0');
		num = num/base;
	}
	str[i] = '\0';
	reverse(str, i);
	return str;
}

// built function for covert number to string
void reverse(char str[], int length)
{
    int start = 0;
    int end = length -1;
    char ivalue = 0;
    while (start < end)
    {
       // swap(*(str+start), *(str+end));
        ivalue = *(str+end);
        *(str+end) = *(str+start);
        *(str+start) = ivalue;
        start++;
        end--;
    }
}





