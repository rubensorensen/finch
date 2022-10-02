# Loggy formatting:

## General

- %%:  A literal % character

- %Gl: Logging level (INFO | WARN | ERROR | FATAL)

- %Gm: Log message (Used in format specifier to insert actual log message)

## Color

- %Ck: Black

- %Cr: Red

- %Cg: Green

- %Co: Orange

- %Cb: Blue

- %Cp: Purple

- %Cc: Cyan

- %Cl: Light gray

- %Cn: Normal

## File:

- %Fn: Path to current source file

- %Fl: Line number of current source file

## Varargs

- %Vd: Signed decimal number

- %Vu: Unsigned decimal number

- %Vf: 64-bit floating point number

- %Vc: Character

- %Vs: String

- %Vp: Pointer

## Time:

- %Ta: The abbreviated name of the day of the week according to the current locale. (Calculated from tm_wday.) (The specific names used in the current locale can be obtained by calling nl_langinfo(3) with ABDAY_{1–7} as an argument.)

- %TA: The full name of the day of the week according to the current locale. (Calculated from tm_wday.) (The specific names used in the current locale can be obtained by calling nl_langinfo(3) with DAY_{1–7} as an argument.)

- %Tb: The abbreviated month name according to the current locale. (Calculated from tm_mon.) (The specific names used in the current locale can be obtained by calling nl_langinfo(3) with ABMON_{1–12} as an argument.)

- %TB: The full month name according to the current locale. (Calculated from tm_mon.) (The specific names used in the current lo‐ cale can be obtained by calling nl_langinfo(3) with MON_{1–12} as an argument.)

- %Tc: The preferred date and time representation for the current locale. (The specific format used in the current locale can be obtained by calling nl_langinfo(3) with D_T_FMT as an argument for the %c conversion specification, and with ERA_D_T_FMT for the %Ec conversion specification.) (In the POSIX locale this is equivalent to %a %b %e %H:%M:%S %Y.)

- %TC: The century number (year/100) as a 2-digit integer. (SU) (The %EC conversion specification corresponds to the name of the era.) (Calculated from tm_year.)

- %Td: The day of the month as a decimal number (range 01 to 31). (Calculated from tm_mday.)

- %TD: Equivalent to %m/%d/%y. (Yecch—for Americans only. Americans should note that in other countries %d/%m/%y is rather common. This means that in international context this format is ambiguous and should not be used.) (SU)

- %Te: Like %d, the day of the month as a decimal number, but a leading zero is replaced by a space. (SU) (Calculated from tm_mday.)

- %TE: Modifier: use alternative ("era-based") format, see below. (SU)

- %TF: Equivalent to %Y-%m-%d (the ISO 8601 date format). (C99)

- %TG: The ISO 8601 week-based year (see NOTES) with century as a decimal number. The 4-digit year corresponding to the ISO week number (see %V). This has the same format and value as %Y, except that if the ISO week number belongs to the previous or next year, that year is used instead. (TZ) (Calculated from tm_year, tm_yday, and tm_wday.)

- %Tg: Like %G, but without century, that is, with a 2-digit year (00–99). (TZ) (Calculated from tm_year, tm_yday, and tm_wday.)

- %Th: Equivalent to %b. (SU)

- %TH: The hour as a decimal number using a 24-hour clock (range 00 to 23). (Calculated from tm_hour.)

- %TI: The hour as a decimal number using a 12-hour clock (range 01 to 12). (Calculated from tm_hour.)

- %Tj: The day of the year as a decimal number (range 001 to 366). (Calculated from tm_yday.)

- %Tk: The hour (24-hour clock) as a decimal number (range 0 to 23); single digits are preceded by a blank. (See also %H.) (Calculated from tm_hour.) (TZ)

- %Tl: The hour (12-hour clock) as a decimal number (range 1 to 12); single digits are preceded by a blank. (See also %I.) (Calculated from tm_hour.) (TZ)

- %Tm: The month as a decimal number (range 01 to 12). (Calculated from tm_mon.)

- %TM: The minute as a decimal number (range 00 to 59). (Calculated from tm_min.)

- %Tn: A newline character. (SU)

- %TO: Modifier: use alternative numeric symbols, see below. (SU)

- %Tp: Either "AM" or "PM" according to the given time value, or the corresponding strings for the current locale. Noon is treated as "PM" and midnight as "AM". (Calculated from tm_hour.) (The specific string representations used for "AM" and "PM" in the current locale can be obtained by calling nl_langinfo(3) with AM_STR and PM_STR, respectively.)

- %TP: Like %p but in lowercase: "am" or "pm" or a corresponding string for the current locale. (Calculated from tm_hour.) (GNU)

- %Tr: The time in a.m. or p.m. notation. (SU) (The specific format used in the current locale can be obtained by calling nl_lang info(3) with T_FMT_AMPM as an argument.) (In the POSIX locale this is equivalent to %I:%M:%S %p.)

- %TR: The time in 24-hour notation (%H:%M). (SU) For a version including the seconds, see - %T :below.

- %Ts: The number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC). (TZ) (Calculated from mktime(tm).)

- %TS: The second as a decimal number (range 00 to 60). (The range is up to 60 to allow for occasional leap seconds.) (Calculated from tm_sec.)

- %Tt: A tab character. (SU)

- %TT: The time in 24-hour notation (%H:%M:%S). (SU)

- %Tu: The day of the week as a decimal, range 1 to 7, Monday being 1. See also %w. (Calculated from tm_wday.) (SU)

- %TU: The week number of the current year as a decimal number, range 00 to 53, starting with the first Sunday as the first day of week 01. See also %V and %W. (Calculated from tm_yday and tm_wday.)

- %TV: The ISO 8601 week number (see NOTES) of the current year as a decimal number, range 01 to 53, where week 1 is the first week that has at least 4 days in the new year. See also %U and %W. (Calculated from tm_year, tm_yday, and tm_wday.) (SU)

- %Tw: The day of the week as a decimal, range 0 to 6, Sunday being 0. See also %u. (Calculated from tm_wday.)

- %TW: The week number of the current year as a decimal number, range 00 to 53, starting with the first Monday as the first day of week 01. (Calculated from tm_yday and tm_wday.)

- %Tx: The preferred date representation for the current locale without the time. (The specific format used in the current locale can be obtained by calling nl_langinfo(3) with D_FMT as an argument for the %x conversion specification, and with ERA_D_FMT for the %Ex conversion specification.) (In the POSIX locale this is equivalent to %m/%d/%y.)

- %TX: The preferred time representation for the current locale without the date. (The specific format used in the current locale can be obtained by calling nl_langinfo(3) with T_FMT as an argument for the %X conversion specification, and with ERA_T_FMT for the %EX conversion specification.) (In the POSIX locale this is equivalent to %H:%M:%S.)

- %Ty: The year as a decimal number without a century (range 00 to 99). (The %Ey conversion specification corresponds to the year since the beginning of the era denoted by the %EC conversion specification.) (Calculated from tm_year)

- %TY: The year as a decimal number including the century. (The %EY conversion specification corresponds to the full alternative year representation.) (Calculated from tm_year)

- %Tz: The +hhmm or -hhmm numeric timezone (that is, the hour and minute offset from UTC). (SU)

- %TZ: The timezone name or abbreviation.

- %T+: The date and time in date(1) format. (TZ) (Not supported in
 glibc2.)
