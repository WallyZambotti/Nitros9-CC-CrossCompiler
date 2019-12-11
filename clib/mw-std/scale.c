
/* **************************************************************** *
 * scale.c -                                                        * 
 * **************************************************************** */

extern double _flacc;

static double atoftbl[] = {
    0.5, 10.0, 100.0, 1000.0,
    10000.0, 100000.0, 1000000.0, 10000000.0,
    100000000.0, 1000000000.0, 10000000000.0,
    1.000000e+20, 1.000000e+30
};

/*char atoftbl[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
    0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x84,
    0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87,
    0x7a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8a,
    0x1c, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8e,
    0x43, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x91,
    0x74, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x94,
    0x18, 0x96, 0x80, 0x00, 0x00, 0x00, 0x00, 0x98,
    0x3e, 0xbc, 0x20, 0x00, 0x00, 0x00, 0x00, 0x9b,
    0x6e, 0x6b, 0x28, 0x00, 0x00, 0x00, 0x00, 0x9e,
    0x15, 0x02, 0xf9, 0x00, 0x00, 0x00, 0x00, 0xa2,
    0x2d, 0x78, 0xeb, 0xc5, 0xac, 0x62, 0x00, 0xc3,
    0x49, 0xf2, 0xc9, 0xcd, 0x04, 0x67, 0x4f, 0xe4
};*/

static double
L0000 (_dblar, _decpl, pos_expon)
    double _dblar;
    int _decpl;
    int pos_expon;
{
    /*return ((_decpl != 0 ?
            (pos_expon != 0 ? _dblar * atoftbl[_decpl] : _dblar / atoftbl[_decpl])
            : _dblar));*/
    if ( _decpl != 0)
    {
        if (pos_expon)
        {
            return (_dblar * atoftbl[_decpl]);
        }
        else
        {
            return (_dblar / atoftbl[_decpl]);
        }
    }
    else
    {
        return _dblar;
    }
}

scale (dblarr, decplaces, pos_expon)
   /*    +4       +12      +14 */
    double dblarr;
    int decplaces;
    int pos_expon;
    /*double *dbladdr;*/
{
    if (decplaces > 9)          /* else L0079 */
    {
        dblarr = L0000 (dblarr, (decplaces/10) + 9 , pos_expon);
/*#asm
        leax 4,s    &dblarr
        pshs x

        ldd 16,s    expol
        pshs d

        ldd 16,s    decplaces
        pshs d
        ldd #10
        lbsr ccdiv
        add #9
        pshs d

        leax 10,s
        lbsr _dstack
        lbsr L0000
        leas 12,s
        lbsr dmove */
    }

    L0000 (dblarr, decplaces % 10, pos_expon);
/*#asm
    ldd 14,s
    pshs d

    ldd 14,s
    pshs d
    ldd #10
    lbsr ccmod
    pshs d

    leax 8,s
    lbsr _dstack
    lbsr L0000
    leas 12,s*/
}
