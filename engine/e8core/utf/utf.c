
#include "utf.h"
#include <string.h>
#include <malloc.h>
#include <ctype.h>

#define aa 0x0430
#define bb 0x0431
#define vv 0x0432
#define gg 0x0433
#define dd 0x0434
#define ye 0x0435
#define eo 0x0451
#define zh 0x0436
#define zz 0x0437
#define ii 0x0438
#define jj 0x0439
#define kk 0x043A
#define ll 0x043B
#define mm 0x043C
#define nn 0x043D
#define oo 0x043E
#define pp 0x043F
#define rr 0x0440
#define ss 0x0441
#define tt 0x0442
#define uu 0x0443
#define ff 0x0444
#define hh 0x0445
#define cc 0x0446
#define ch 0x0447
#define sh 0x0448
#define sc 0x0449
#define er 0x044A
#define yi 0x044B
#define ir 0x044C
#define ee 0x044D
#define yu 0x044E
#define ya 0x044F


#define AA 0x0410
#define BB 0x0411
#define VV 0x0412
#define GG 0x0413
#define DD 0x0414
#define YE 0x0415
#define EO 0x0401
#define ZH 0x0416
#define ZZ 0x0417
#define II 0x0418
#define JJ 0x0419
#define KK 0x041A
#define LL 0x041B
#define MM 0x041C
#define NN 0x041D
#define OO 0x041E
#define PP 0x041F
#define RR 0x0420
#define SS 0x0421
#define TT 0x0422
#define UU 0x0423
#define FF 0x0424
#define HH 0x0425
#define CC 0x0426
#define CH 0x0427
#define SH 0x0428
#define SC 0x0429
#define ER 0x042A
#define YI 0x042B
#define IR 0x042C
#define EE 0x042D
#define YU 0x042E
#define YA 0x042F

unsigned short __utf_base[128] = {
//  00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  0E  0F
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, /* 80 */
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, /* 90 */
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, /* A0 */
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, /* B0 */
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, /* C0 */
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, /* D0 */
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, /* E0 */
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, /* F0 */
};

unsigned short cp1251[128] = {
//  00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  0E  0F
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, /* 80 */
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, /* 90 */
    00, 00, 00, 00, 00, 00, 00, 00, EO, 00, 00, 00, 00, 00, 00, 00, /* A0 */
    00, 00, 00, 00, 00, 00, 00, 00, eo, 00, 00, 00, 00, 00, 00, 00, /* B0 */
    AA, BB, VV, GG, DD, YE, ZH, ZZ, II, JJ, KK, LL, MM, NN, OO, PP, /* C0 */
    RR, SS, TT, UU, FF, HH, CC, CH, SH, SC, ER, YI, IR, EE, YU, YA, /* D0 */
    aa, bb, vv, gg, dd, ye, zh, zz, ii, jj, kk, ll, mm, nn, oo, pp, /* E0 */
    rr, ss, tt, uu, ff, hh, cc, ch, sh, sc, er, yi, ir, ee, yu, ya, /* F0 */
};

unsigned short cp866[128] = {
//  00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  0E  0F
    AA, BB, VV, GG, DD, YE, ZH, ZZ, II, JJ, KK, LL, MM, NN, OO, PP, /* 80 */
    RR, SS, TT, UU, FF, HH, CC, CH, SH, SC, ER, YI, IR, EE, YU, YA, /* 90 */
    aa, bb, vv, gg, dd, ye, zh, zz, ii, jj, kk, ll, mm, nn, oo, pp, /* A0 */
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, /* B0 */
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, /* C0 */
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, /* D0 */
    rr, ss, tt, uu, ff, hh, cc, ch, sh, sc, er, yi, ir, ee, yu, ya, /* E0 */
    EO, eo, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, /* F0 */
};


unsigned short koi8r[128] = {
//  00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  0E  0F
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, /* 80 */
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, /* 90 */
    00, 00, 00, eo, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, /* A0 */
    00, 00, 00, EO, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, /* B0 */
    yu, aa, bb, cc, dd, ee, ff, gg, hh, ii, jj, kk, ll, mm, nn, oo, /* C0 */
    pp, ya, rr, ss, tt, uu, zh, vv, ir, yi, zz, sh, ee, sc, ch, er, /* D0 */
    YU, AA, BB, CC, DD, EE, FF, GG, HH, II, JJ, KK, LL, MM, NN, OO, /* E0 */
    PP, YA, RR, SS, TT, UU, ZH, VV, IR, YI, ZZ, SH, EE, SC, CH, ER, /* F0 */
};


inline char _mtoupper(const char ca)
{
    if (ca >= 'a' && ca <= 'z')
        return ca - 'a' + 'A';
    return ca;
}

static inline int
__str1cmp(char a, char b)
{
    unsigned char ua = (unsigned char)a;
    unsigned char ub = (unsigned char)b;
    return ua - ub;
}

int stricmp(const char *a, const char *b)
{
    int r = __str1cmp(*a, *b);
    while (!r && *a && *b) {
        r = __str1cmp(*++a, *++b);
    }
    return r;
}


int e8_utf_encoding_support(const char *encoding)
{
    if (stricmp(encoding, "cp866") == 0)
        return 1;
    if (stricmp(encoding, "cp1251") == 0)
        return 1;
    if (stricmp(encoding, "koi8r") == 0)
        return 1;
    if (stricmp(encoding, "utf-8") == 0)
        return 1;
    if (stricmp(encoding, "ru_ru.utf-8") == 0)
        return 1;
    return 0;
}

int e8_utf_collate_index_ci(const e8_uchar c)
{
    if (c < 0x80)
        return tolower(c);

    if (c == EO)
        return ye + 1;
    if (c == eo)
        return ye + 1;

    #define DELTA  (ya - YA)

    if (c <= YE)
        return c + DELTA;
    if (c <= YA)
        return c + 1 + DELTA;

    #undef DELTA

    if (c <= ye)
        return c;

    if (c <= ya)
        return c + 1;

    return c;
}

int e8_utf_char_collate_index(const e8_uchar c)
{
    if (c < 0x80)
        return c;

    if (c == EO)
        return YE + 1;
    if (c == eo)
        return ye + 1;

    if (c <= YE)
        return c;
    if (c <= YA)
        return c + 1;

    if (c <= ye)
        return c;
    if (c <= ya)
        return c + 1;
    return c;
}

int e8_utf_compare(const e8_uchar a, const e8_uchar b)
{
    int ia = e8_utf_char_collate_index(a);
    int ib = e8_utf_char_collate_index(b);
    if (ia < ib)
        return -1;
    if (ia > ib)
        return +1;
    return 0;
}

int e8_utf_compare_ci(const e8_uchar a, const e8_uchar b)
{
    int ia = e8_utf_collate_index_ci(a);
    int ib = e8_utf_collate_index_ci(b);
    if (ia < ib)
        return -1;
    if (ia > ib)
        return +1;
    return 0;
}



size_t e8_utf_strlen(const e8_uchar *utf)
{
    if (!utf)
        return 0;

    size_t r = 0;
    const e8_uchar *c = utf;
    while (*c++)
        ++r;
    return r;
}


unsigned short *table(const char *encoding)
{
    if (stricmp(encoding, "cp866") == 0)
        return cp866;
    if (stricmp(encoding, "cp1251") == 0)
        return cp1251;
    if (stricmp(encoding, "koi8r") == 0)
        return koi8r;
    return 0;
}

e8_uchar e8_utf_make_from_8bit(const char ascii, const char *encoding)
{
    unsigned char uascii = (unsigned char)ascii;
    if (uascii < 0x80)
        return uascii;

    unsigned short *t = table(encoding);
    if (!t)
        return 0;
    return t[uascii & 0x7F];

}
e8_uchar e8_utf_make_from_8bit_table(const char ascii, const unsigned short *t)
{
    unsigned char uascii = (unsigned char)ascii;
    if (uascii < 0x80)
        return uascii;

    if (!t)
        return 0;
    return t[uascii & 0x7F];
}


void e8_utf_from_7bit(const char *ascii, e8_uchar **utf)
{
    size_t len = strlen(ascii);

    if (!*utf)
        *utf = malloc(sizeof(e8_uchar) * (len + 1));

    const char *c = ascii;
    e8_uchar *uc = *utf;
    while (*c) {
        *uc = *c;
        ++c;
        ++uc;
    }
    *uc = 0;
}

static
unsigned length(const unsigned char *buf)
{
    if (*buf < 0x80)
        return 1;
    if (*buf >> 5 == 6)
        return 2;
    if (*buf >> 4 == 14)
        return 3;
    if (*buf >> 3 == 30)
        return 4;
    if (*buf >> 2 == 62)
        return 5;
    if (*buf >> 1 == 126)
        return 6;

    return 0;
}

static
void shift(unsigned char **in, e8_uchar **out)
{
    unsigned l = length(*in);

    switch (l) {
        case 1: {
            **out = **in;
            ++*in;
            ++*out;
            break;
        }
        case 2: {
            e8_uchar cp = **in;
            ++*in;
            cp = ((cp << 6) & 0x7ff) + ((**in) & 0x3f);
            ++*in;
            **out = cp;
            ++*out;
            break;
        }
        default: {
            *in += l; // пропускаем неведомый знак
            return;
        }
    }
}

static
void e8_utf_from_utf8(const char *s, e8_uchar **utf)
{
    unsigned char *buf = (unsigned char *)s;
    if (!*utf) {

        /* Считаем размер строки */
        int prelength = 0;
        while (*buf) {
            unsigned l = length(buf);
            prelength += l;
            buf += l;
        }
        *utf = e8_utf_malloc(prelength);
    }

    buf = (unsigned char *)s;
    e8_uchar *res = *utf;
    while (*buf) {
        shift(&buf, &res);
    }
    *res = 0;
}

void e8_utf_from_8bit(const char *ascii, e8_uchar **utf, const char *encoding)
{

    if (stricmp(encoding, "utf-8") == 0) {
        e8_utf_from_utf8(ascii, utf);
        return;
    }

    if (!*utf) {
        size_t len = strlen(ascii);
        *utf = e8_utf_malloc(len);
    }
    const char *c = ascii;
    e8_uchar *uc = *utf;
    unsigned short *t = table(encoding);
    while (*c) {
        *uc = e8_utf_make_from_8bit_table(*c, t);
        ++c;
        ++uc;
    }
    *uc = 0;
}
char e8_deutf(e8_uchar uc, const unsigned short *t)
{
    if (uc < 0x80)
        return uc;
    if (!t)
        return 0;

    unsigned short i;
    for (i = 0; i < 0x80; ++i)
        if (t[i] == uc)
            return i | 0x80;

    return 0;
}


size_t e8_utf_get_utf8_length(const e8_uchar *utf)
{
    size_t len = e8_utf_strlen(utf);
    size_t prelen = 0;
    size_t i = 0;
    for (; i < len; ++i) {
        if (utf[i] < 0x80)
            prelen += 1;
        else if (utf[i] < 0x800)
            prelen += 2;
        else if (utf[i] < 0x10000)
            prelen += 3;
        else if (utf[i] < 0x20000)
            prelen += 4;
        else if (utf[i] < 0x40000)
            prelen += 5;
        else
            prelen += 6;
    } // for i
    return prelen;
}


static
void e8_utf_to_utf8(const e8_uchar *utf, char **out)
{
    if (!*out) {
        size_t prelen = e8_utf_get_utf8_length(utf);
        *out = malloc(prelen + 1);
    }
    size_t len = e8_utf_strlen(utf);

    char *res = *out;
    size_t i;
    for (i = 0; i < len; ++i) {
        e8_uchar cp = utf[i];
        if (cp < 0x80) {
            *res = cp;
            ++res;
        } else if (cp < 0x800) {

            *res = ((cp >> 6) & 0x1F) | 0xC0;
            ++res;

            *res = (cp & 0x3F) | 0x80;
            ++res;

        } /* 0x10000 0x20000 0x40000*/
        /* TODO: [E8::Core::UTF-8] сделать обработку кодовых позиций более 2 байт*/
        else {

        }
    } // for i
    *res = 0;
}

void e8_utf_to_8bit(const e8_uchar *utf, char **ascii, const char *encoding)
{
    if (stricmp(encoding, "utf-8") == 0) {
        e8_utf_to_utf8(utf, ascii);
        return;
    }
    unsigned short *t = table(encoding);
    const e8_uchar *uc = utf;
    if (!*ascii) {
        size_t len = e8_utf_strlen(utf);
        *ascii = malloc(sizeof(char) * (len + 1));
    }
    char *c = *ascii;
    while (*uc) {
        switch (*uc) {

        case cyr_EAT:
            *c = e8_deutf(YE, t);
            break;

        case cyr_eat:
            *c = e8_deutf(ye, t);
            break;

        case cyr_I:
            *c = e8_deutf(II, t);
            break;

        case cyr_i:
            *c = e8_deutf(ii, t);
            break;

        case cyr_THETA:
            *c = e8_deutf(FF, t);
            break;

        case cyr_theta:
            *c = e8_deutf(ff, t);
            break;
        default:
            *c = e8_deutf(*uc, t);
        }
        ++uc;
        if (*c)
            ++c;
    }
    *c = 0;
}

void e8_utf_to_7bit()
{

}

void e8_utf_strcpy(e8_uchar *dst, const e8_uchar *src)
{
    if (src)
        while (*src) {
            *dst = *src;
            ++dst;
            ++src;
        }
    *dst = 0;
}
void e8_utf_strncpy(e8_uchar *dst, const e8_uchar *src, int n)
{
    if (src)
        while (*src && n) {
            *dst = *src;
            ++dst;
            ++src;
            --n;
        }
    if (!n)
        *dst = 0;
}

void e8_utf_strcat(e8_uchar *dst, const e8_uchar *src)
{
    e8_utf_strcpy(dst + e8_utf_strlen(dst), src);
}

int e8_utf_strcmp(const e8_uchar *a, const e8_uchar *b)
{
    size_t i = 0;
    int r = e8_utf_compare(a[i], b[i]);
    while (!r && a[i] && b[i]) {
        ++i;
        r = e8_utf_compare(a[i], b[i]);
    }
    return r;
}
int e8_utf_stricmp(const e8_uchar *a, const e8_uchar *b)
{

    size_t i = 0;
    int r = e8_utf_compare_ci(a[i], b[i]);
    while (!r && a[i] && b[i]) {
        ++i;
        r = e8_utf_compare_ci(a[i], b[i]);
    }

    return r;
}
int e8_utf_strncmp(const e8_uchar *a, const e8_uchar *b, int n)
{
    size_t i = 0;
    int r = e8_utf_compare(a[i], b[i]);
    while (!r && a[i] && b[i]) {

        if (!--n)
            break;

        ++i;

        r = e8_utf_compare(a[i], b[i]);

    }
    return r;
}

e8_uchar e8_upcase(e8_uchar uc)
{
    if (uc >= 'a' && uc <= 'z')
        return uc - 'a' + 'A';
    if (uc < 0x80)
        return uc;
    if (uc == eo)
        return EO;
    if (uc == cyr_eat)
        return cyr_EAT;
    if (uc == cyr_i)
        return cyr_I;
    if (uc == cyr_theta)
        return cyr_THETA;
    if (uc >= aa && uc <= ya)
        return uc - aa + AA;
    return uc;
}
e8_uchar e8_locase(e8_uchar uc)
{
    if (uc >= 'A' && uc <= 'Z')
        return uc - 'A' + 'a';
    if (uc < 0x80)
        return uc;
    if (uc == EO)
        return eo;
    if (uc == cyr_EAT)
        return cyr_eat;
    if (uc == cyr_I)
        return cyr_i;
    if (uc == cyr_THETA)
        return cyr_theta;
    if (uc >= AA && uc <= YA)
        return uc - AA + aa;
    return uc;
}

void e8_utf_upper_case(e8_uchar *us)
{
    while (*us) {
        *us = e8_upcase(*us);
        ++us;
    }
}
void e8_utf_lower_case(e8_uchar *us)
{
    while (*us) {
        *us = e8_locase(*us);
        ++us;
    }
}

e8_uchar *e8_utf_malloc(size_t len)
{
    return malloc(sizeof(e8_uchar) * (len + 1));
}
e8_uchar *e8_utf_realloc(e8_uchar *c, size_t len)
{
    return realloc(c, sizeof(e8_uchar) * (len + 1));
}

static void e8_shift_multybyte(e8_uchar **dst, const char **ref, int size)
{
    **dst = 0;
    int i;
    for (i = 0; i < size; ++i)
        **dst += (*ref)[i] << (i*8);
    *ref += size;
    ++(*dst);
}

e8_uchar *e8_utf_from_mbstring(const void *mbs, int length, int size)
{
    e8_uchar *r = e8_utf_malloc(length), *t = r;
    char const *c = mbs;
    int i;
    for (i = 0; i < length; ++i)
        e8_shift_multybyte(&t, &c, size);
    *t = 0;
    return r;
}

void e8_utf_free(e8_uchar *us)
{
    if (us)
        free(us);
}

e8_uchar *e8_utf_strdup(const e8_uchar *src)
{
    e8_uchar *r = e8_utf_malloc(e8_utf_strlen(src));
    e8_utf_strcpy(r, src);
    return r;
}

e8_uchar *e8_utf_strcdup(const char *src, const char *encoding)
{
    e8_uchar *r = 0;
    e8_utf_from_8bit(src, &r, encoding);
    return r;
}
