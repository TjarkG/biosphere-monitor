/*
 * itoa.h
 *
 * Int to String function
 *
 *  Author: Brian W. Kernighan / Dennis M. Ritchie
 *  from "The C Programming Language" Page 62 and 64
 */

#ifndef itoa_H
#define itoa_H

#include <string.h>

/* reverse:  reverse string s in place */
 void reverse(char s[])
 {
     int i, j;
     char c;

     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
} 
 
 /* itoa:  convert n to characters in s */
 void _itoa(long n, char s[])
 {
     int i, sign;

     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % 10 + '0';   /* get next digit */
     } while ((n /= 10) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
}

#endif /*itoa_H*/