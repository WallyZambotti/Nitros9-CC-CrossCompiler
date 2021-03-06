/* *********************************** *
 * atof                                *
 * *********************************** */

#include <ctype.h>

extern double _dnorm();
extern double scale();
extern double _flacc;
/*extern char _chcodes[];*/

double
atof (str)
    register char *str;
{
    union {
        double dbl;
        char ch[8];
    } _mydbl;

    int _exponent;
    int _isneg;
    int _negexpon;
    int _decplaces;
    char _curchr;
    char *_dblptr;

    while ((*str == ' ') || (*str == '\t'))
    {
        ++str;
    }

    if (*str == '-')
    {
        _isneg = 1;
    }
    else
    {
        _isneg = 0;
    }

    if ((*str == '-') || (*str == '+'))
    {
        ++str;
    }

    _mydbl.dbl = 0;    /* Initialize double */

    while (isdigit (_curchr = *str))
    {
        L0178 (&_mydbl.dbl, _curchr);    /* _13 */
        ++str;
    }

    _decplaces = 0;

    if (*str == '.')        /* else _17 */
    {
        ++str;
        
        while (isdigit (_curchr = *str))
        {
            L0178 (&(_mydbl.dbl), _curchr);    /* _19 */
            ++str;
            ++_decplaces;
        }
    }

    _dblptr = &_mydbl;
    _dblptr[7] = 184;
    _mydbl.dbl = _dnorm (&_mydbl);
    
    if (((_curchr = *str) == 'e') || (_curchr == 'E'))    /* else _21 */
    {
        _negexpon = 1;
        ++str;

        if (*str == '+')
        {
            ++str;
        }
        else
        {
            if (*str == '-')
            {
                ++str;
                _negexpon = 0;
            }
        }

        _exponent = 0;     /* _25 */

        while (isdigit (_curchr = *(str++)))
        {
            _exponent = (((_exponent * 10) + _curchr) - '0');
        }

        _decplaces += (_negexpon ? -_exponent : _exponent);
    }

    if (_decplaces < 0)     /* _21 */
    {
        _decplaces = -_decplaces;
        _negexpon = 1;
    }
    else
    {
        _negexpon = 0;
    }

    _mydbl.dbl = scale (_mydbl.dbl, _decplaces, _negexpon);

    if (_isneg)
    {
        return - _mydbl.dbl;
    }
    else
    {
        return _mydbl.dbl;
    }
}

#asm
L0178 ldx   2,s 
 leas  -8,s 
 ldd   5,x 
 lslb   
 rola   
 std   5,x 
 std   5,s 
 ldd   3,x 
 rolb   
 rola   
 std   3,x 
 std   3,s 
 ldd   1,x 
 rolb   
 rola   
 std   1,x 
 std   1,s 
 lda   ,x 
 rola   
 sta   ,x 
 sta   ,s 
 asl   6,x 
 rol   5,x 
 rol   4,x 
 rol   3,x 
 rol   2,x 
 rol   1,x 
 rol   ,x 
 lblo  L0205 
 asl   6,x 
 rol   5,x 
 rol   4,x 
 rol   3,x 
 rol   2,x 
 rol   1,x 
 rol   ,x 
 lblo  L0205 
 ldd   5,x 
 addd  5,s 
 std   5,x 
 ldd   3,x 
 adcb  4,s 
 adca  3,s 
 std   3,x 
 ldd   1,x 
 adcb  2,s 
 adca  1,s 
 std   1,x 
 ldb   ,x 
 adcb  ,s 
 stb   ,x 
 bcs   L0205 
 ldb   13,s 
 andb  #$0f 
 clra   
 addd  5,x 
 std   5,x 
 ldd   #0 
 adcb  4,x 
 adca  3,x 
 std   3,x 
 ldd   #0 
 adcb  2,x 
 adca  1,x 
 std   1,x 
 lda   #0 
 adca  ,x 
 sta   ,x 
 bcs   L0205 
 leas  8,s 
 clra   
 clrb   
 rts    
L0205 ldd   #1 
 leas  8,s 
 rts    

 ends  

#endasm

