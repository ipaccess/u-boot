/*
 * Copyright (c) 2014, ip.access Ltd. All Rights Reserved.
 */
#include "damm.h"




/*
 * Damm check digit calculation/validation
 * For more information see http://en.wikipedia.org/wiki/Damm_algorithm
 * The array could be substituted with another totally anti-symmetric quasigroup
 * This would affect the check digit.
 */
static char taq(const char * number)
{
    static const char taqDhmd111rr[] = "0317598642"
                                       "7092154863"
                                       "4206871359"
                                       "1750983426"
                                       "6123045978"
                                       "3674209581"
                                       "5869720134"
                                       "8945362017"
                                       "9438617205"
                                       "2581436790";
    char interim = '0';
    const char * p = number;

    for (; *p != '\0'; ++p)
    {
        if ((unsigned char)(*p-'0') > 9)
        {
            /* minus sign indicates an error: character is not a digit */
            return '-';
        }

        interim = taqDhmd111rr[(*p-'0') + (interim-'0')*10];
    }

    return interim;
}




/**
 * Calculates a DAMM check digit from an input character sequence
 *
 * @param numberWithoutCheckDigit A textual representation of the number to calculate a check digit for.  This is expected to be non-NULL and NUL terminated.
 * @return The DAMM check digit for the provided input.
 */
char dammCalculateCheckDigit(const char * numberWithoutCheckDigit)
{
    return taq(numberWithoutCheckDigit);
}




/**
 * Verify a input character sequence with a trailing DAMM check digit
 *
 * @param numberWithCheckDigit A textual representation of the number to verify.  This is expected to be non-NULL and NUL terminated.  The last digit in the sequence is expected to be the DAMM check digit.
 * @return Zero for failure, non-zero for success.
 */
int dammValidCheckDigit(const char * numberWithCheckDigit)
{
    return taq(numberWithCheckDigit) == '0';
}
