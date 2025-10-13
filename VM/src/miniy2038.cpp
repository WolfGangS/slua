// ============================================================================
// Extracted from y2038 library (https://github.com/evalEmpire/y2038)
// Copyright (c) 2007-2010 Michael G Schwern
// Licensed under MIT License
// Note that this is a significantly stripped version, with a lot of macros expanded / removed.
// It's placed somewhat haphazardly because I do not love it. I will be happy to get
// rid of it entirely once x86-32 is no longer a thing. In all likelihood this is totally
// unnecessary, but I feel weird about knowingly exposing APIs with different behavior
// on different platforms.
// ============================================================================

#include "lcommon.h"

#include "miniy2038.h"

#if defined(_WIN32)
static tm* gmtime_r(const time_t* timep, tm* result)
{
    return gmtime_s(result, timep) == 0 ? result : NULL;
}

static tm* localtime_r(const time_t* timep, tm* result)
{
    return localtime_s(result, timep) == 0 ? result : NULL;
}
#endif

#define MAX_SAFE_YEAR 2037
#define MIN_SAFE_YEAR 1971
#define SOLAR_CYCLE_LENGTH 28

static const short julian_days_by_month[2][12] = {
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
    {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335},
};
static const char days_in_month[2][12] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
};

/* Year cycle from MAX_SAFE_YEAR down. */
static const short safe_years_high[SOLAR_CYCLE_LENGTH] = {
    2016, 2017, 2018, 2019,
    2020, 2021, 2022, 2023,
    2024, 2025, 2026, 2027,
    2028, 2029, 2030, 2031,
    2032, 2033, 2034, 2035,
    2036, 2037, 2010, 2011,
    2012, 2013, 2014, 2015
};

/* Year cycle from MIN_SAFE_YEAR up */
static const int safe_years_low[SOLAR_CYCLE_LENGTH] = {
    1996, 1997, 1998, 1971,
    1972, 1973, 1974, 1975,
    1976, 1977, 1978, 1979,
    1980, 1981, 1982, 1983,
    1984, 1985, 1986, 1987,
    1988, 1989, 1990, 1991,
    1992, 1993, 1994, 1995,
};

static const short length_of_year[2] = { 365, 366 };

static const int64_t years_in_gregorian_cycle   = 400;
static const int64_t days_in_gregorian_cycle    = ((365 * 400) + 100 - 4 + 1);

/* Let's assume people are going to be looking for dates in the future.
   Let's provide some cheats so you can skip ahead.
   This has a 4x speed boost when near 2008.
*/
/* Number of days since epoch on Jan 1st, 2008 GMT */
#define CHEAT_DAYS  (1199145600 / 24 / 60 / 60)
#define CHEAT_YEARS 108

/* IS_LEAP is used all over the place to index on arrays, so make sure it always returns 0 or 1. */
#define IS_LEAP(n)  ( (!(((n) + 1900) % 400) || (!(((n) + 1900) % 4) && (((n) + 1900) % 100))) ? 1 : 0 )

/* Allegedly, some <termios.h> define a macro called WRAP, so use a longer name like WRAP_TIME64. */
#define WRAP_TIME64(a,b,m)     ((a) = ((a) <  0  ) ? ((b)--, (a) + (m)) : (a))

static int is_exception_century(int64_t year)
{
    return ((year % 100 == 0) && !(year % 400 == 0));
}

static int64_t cycle_offset(int64_t year)
{
    const int64_t start_year = 2000;
    int64_t year_diff  = year - start_year;
    int64_t exceptions;

    if( year > start_year )
        year_diff--;

    exceptions  = year_diff / 100;
    exceptions -= year_diff / 400;

    return exceptions * 16;
}

static int safe_year(const int64_t year)
{
    int safe_year_result;
    int64_t year_cycle;

    if( year >= MIN_SAFE_YEAR && year <= MAX_SAFE_YEAR ) {
        return (int)year;
    }

    year_cycle = year + cycle_offset(year);

    if( year < MIN_SAFE_YEAR )
        year_cycle -= 8;

    if( is_exception_century(year) )
        year_cycle += 11;

    if( is_exception_century(year - 1) )
        year_cycle += 17;

    year_cycle %= SOLAR_CYCLE_LENGTH;
    if( year_cycle < 0 )
        year_cycle = SOLAR_CYCLE_LENGTH + year_cycle;

    if( year < MIN_SAFE_YEAR )
        safe_year_result = safe_years_low[year_cycle];
    else if( year > MAX_SAFE_YEAR )
        safe_year_result = safe_years_high[year_cycle];
    else
        safe_year_result = (int)year;

    return safe_year_result;
}

static lua_time_t timegm64_y2038(const struct tm *date) {
    int64_t days    = 0;
    int64_t seconds = 0;
    int64_t year;
    int64_t orig_year = (int64_t)date->tm_year;
    int64_t cycles  = 0;

    if( orig_year > 100 ) {
        cycles = (orig_year - 100) / 400;
        orig_year -= cycles * 400;
        days      += cycles * days_in_gregorian_cycle;
    }
    else if( orig_year < -300 ) {
        cycles = (orig_year - 100) / 400;
        orig_year -= cycles * 400;
        days      += cycles * days_in_gregorian_cycle;
    }

    if( orig_year > 70 ) {
        year = 70;
        while( year < orig_year ) {
            days += length_of_year[IS_LEAP(year)];
            year++;
        }
    }
    else if ( orig_year < 70 ) {
        year = 69;
        do {
            days -= length_of_year[IS_LEAP(year)];
            year--;
        } while( year >= orig_year );
    }

    days += julian_days_by_month[IS_LEAP(orig_year)][date->tm_mon];
    days += date->tm_mday - 1;

    seconds = days * 60 * 60 * 24;

    seconds += date->tm_hour * 60 * 60;
    seconds += date->tm_min * 60;
    seconds += date->tm_sec;

    return (lua_time_t)seconds;
}

struct tm * gmtime64_r_y2038(const lua_time_t *in_time, struct tm *p)
{
    int v_tm_sec, v_tm_min, v_tm_hour, v_tm_mon, v_tm_wday;
    lua_time_t v_tm_tday;
    int leap;
    lua_time_t m;
    lua_time_t time = *in_time;
    int64_t year = 70;
    int64_t cycles = 0;

    p->tm_isdst  = 0;

    v_tm_sec =  (int)(time % 60);
    time /= 60;
    v_tm_min =  (int)(time % 60);
    time /= 60;
    v_tm_hour = (int)(time % 24);
    time /= 24;
    v_tm_tday = time;

    WRAP_TIME64 (v_tm_sec, v_tm_min, 60);
    WRAP_TIME64 (v_tm_min, v_tm_hour, 60);
    WRAP_TIME64 (v_tm_hour, v_tm_tday, 24);

    v_tm_wday = (int)((v_tm_tday + 4) % 7);
    if (v_tm_wday < 0)
        v_tm_wday += 7;
    m = v_tm_tday;

    if (m >= CHEAT_DAYS) {
        year = CHEAT_YEARS;
        m -= CHEAT_DAYS;
    }

    if (m >= 0) {
        /* Gregorian cycles, this is huge optimization for distant times */
        cycles = m / (lua_time_t) days_in_gregorian_cycle;
        if( cycles ) {
            m -= cycles * (lua_time_t) days_in_gregorian_cycle;
            year += cycles * years_in_gregorian_cycle;
        }

        /* Years */
        leap = IS_LEAP (year);
        while (m >= (lua_time_t) length_of_year[leap]) {
            m -= (lua_time_t) length_of_year[leap];
            year++;
            leap = IS_LEAP (year);
        }

        /* Months */
        v_tm_mon = 0;
        while (m >= (lua_time_t) days_in_month[leap][v_tm_mon]) {
            m -= (lua_time_t) days_in_month[leap][v_tm_mon];
            v_tm_mon++;
        }
    } else {
        year--;

        /* Gregorian cycles */
        cycles = (m / (lua_time_t) days_in_gregorian_cycle) + 1;
        if( cycles ) {
            m -= cycles * (lua_time_t) days_in_gregorian_cycle;
            year += cycles * years_in_gregorian_cycle;
        }

        /* Years */
        leap = IS_LEAP (year);
        while (m < (lua_time_t) -length_of_year[leap]) {
            m += (lua_time_t) length_of_year[leap];
            year--;
            leap = IS_LEAP (year);
        }

        /* Months */
        v_tm_mon = 11;
        while (m < (lua_time_t) -days_in_month[leap][v_tm_mon]) {
            m += (lua_time_t) days_in_month[leap][v_tm_mon];
            v_tm_mon--;
        }
        m += (lua_time_t) days_in_month[leap][v_tm_mon];
    }

    p->tm_year = (int)year;

    if( p->tm_year != year ) {
        // Year overflow
        return NULL;
    }

    /* At this point m is less than a year so casting to an int is safe */
    p->tm_mday = (int) m + 1;
    p->tm_yday = julian_days_by_month[leap][v_tm_mon] + (int)m;
    p->tm_sec  = v_tm_sec;
    p->tm_min  = v_tm_min;
    p->tm_hour = v_tm_hour;
    p->tm_mon  = v_tm_mon;
    p->tm_wday = v_tm_wday;

    return p;
}

struct tm * localtime64_r_y2038(const lua_time_t *time, struct tm *local_tm)
{
    struct tm gm_tm;
    int64_t orig_year;
    int month_diff;

    if(gmtime64_r_y2038(time, &gm_tm) == NULL ) {
        return NULL;
    }

    orig_year = gm_tm.tm_year;

    if (gm_tm.tm_year > (2037 - 1900) ||
        gm_tm.tm_year < (1970 - 1900)
    )
    {
        gm_tm.tm_year = safe_year((int64_t)(gm_tm.tm_year + 1900)) - 1900;
    }

    time_t safe_time = (time_t)timegm64_y2038(&gm_tm);
    if( localtime_r(&safe_time, local_tm) == NULL ) {
        return NULL;
    }

    local_tm->tm_year = (int)orig_year;

    if( local_tm->tm_year != orig_year ) {
        // Year overflow
        return NULL;
    }

    month_diff = local_tm->tm_mon - gm_tm.tm_mon;

    /*  When localtime is Dec 31st previous year and
        gmtime is Jan 1st next year.
    */
    if( month_diff == 11 ) {
        local_tm->tm_year--;
    }

    /*  When localtime is Jan 1st, next year and
        gmtime is Dec 31st, previous year.
    */
    if( month_diff == -11 ) {
        local_tm->tm_year++;
    }

    /* GMT is Jan 1st, xx01 year, but localtime is still Dec 31st
       in a non-leap xx00.  There is one point in the cycle
       we can't account for which the safe xx00 year is a leap
       year.  So we need to correct for Dec 31st coming out as
       the 366th day of the year.
    */
    if( !IS_LEAP(local_tm->tm_year) && local_tm->tm_yday == 365 )
        local_tm->tm_yday--;

    return local_tm;
}
