#include "variant.h"
#include <string.h>

void e8_date_add(e8_date *date, long delta)
{
    *date += delta;
}
void e8_date_sub_long(e8_date *date, long delta)
{
    *date -= delta;
}
long e8_date_sub_date(e8_date a, e8_date b)
{
    return a - b;
}

static const int DAY = 24*60*60;
static int m_c[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static int is_leap(int year)
{
    return (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0) );
}

static inline int loff(int y, int m)
{
    return is_leap(y) && m > 2 ? 1 : 0;
}

static inline int mc(int y, int m)
{
    return m_c[m] + (is_leap(y) && (m == 1) ? 1 : 0);
}

static e8_date date_base(int year, int month, int m_day)
{
    int vkc = year / 4 - year / 100 + year / 400;
    int mkc = 0;
    int i;
    for (i = 0; i < month - 1; i++)
        mkc += m_c[i];

    mkc += loff(year, month);

    e8_date r = (year*365ll + vkc) + mkc + m_day - 1;

    return r;
}
static long days_before(int year)
{
    return date_base(year, 1, 1);
}


static void parse(long day, int *year, int *month, int *mday)
{
    int min_y = day / 366;
    int max_y = day / 365;
    *year = min_y;
    int y;
    for (y = min_y; y <= max_y; ++y)
        if (days_before(y) > day) {
            *year = y - 1;
            break;
        }
    long md = day - days_before(*year);
    *month = 1;
    while (md >= mc(*year, *month-1)) {
        md -= mc(*year, *month-1);
        ++(*month);
    }
    *mday = md + 1;
}

static int get_pyear(long day)
{
    int y, m, d;
    parse(day, &y, &m, &d);
    return y;
}
static int get_pmonth(long day)
{
    int y, m, d;
    parse(day, &y, &m, &d);
    return m;
}
static int get_pday(long day)
{
    int y, m, d;
    parse(day, &y, &m, &d);
    return d;
}

int e8_date_get_month(e8_date m_data)
{
    return get_pmonth(m_data / DAY);
}
int e8_date_get_year(e8_date m_data)
{
    return get_pyear(m_data / DAY);
}
int e8_date_get_day(e8_date m_data)
{
    return get_pday(m_data / DAY);
}
int e8_date_get_hour(e8_date m_data)
{
    return (m_data / 3600) % 24;
}
int e8_date_get_min(e8_date m_data)
{
    return (m_data / 60) % 60;
}
int e8_date_get_sec(e8_date m_data)
{
    return m_data % 60;
}

int e8_date_construct(int Y, int M, int D, int hh, int mm, int ss, e8_date *d)
{
    *d = date_base(Y, M, D) *24*60*60 + hh*60*60 + mm * 60 + ss;
    return 0;
}

static inline int __cc(char c)
{
    return (c - '0');
}

int e8_date_from_string(const char *s, e8_date *d)
{
    int l = strlen(s);
    if (l != 8 && l != 14)
        return 1;

    int yyyy = (s[0] - '0') * 1000 + (s[1] - '0') * 100 + (s[2] - '0') * 10 + (s[3] - '0') * 1;
    int MM = (s[4] - '0') * 10 + (s[5] - '0') * 1;
    int dd = (s[6] - '0') * 10 + (s[7] - '0') * 1;
    int HH = 0, mm = 0, ss = 0;
    if (l > 8) {
        HH = (s[8] - '0') * 10 + (s[9] - '0') * 1;
        mm = (s[10] - '0') * 10 + (s[11] - '0') * 1;
        ss = (s[12] - '0') * 10 + (s[13] - '0') * 1;
    }

    return e8_date_construct(yyyy, MM, dd, HH, mm, ss, d);
}
