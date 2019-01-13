#include "fwriteLookups.h"
#include <errno.h>
#include <unistd.h>    // for access()
#include <fcntl.h>
#include <stdbool.h>   // true and false
#include <stdint.h>    // INT32_MIN
#include <math.h>      // isfinite, isnan
#include <stdlib.h>    // abs
#include <string.h>    // strlen, strerror

#ifdef WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#define WRITE _write
#define CLOSE _close
#else
#define WRITE write
#define CLOSE close
#endif

#include "zlib.h"      // for writing gzip file
#include "myomp.h"
#include "fwrite.h"

#define NUM_SF   15
#define SIZE_SF  1000000000000000ULL  // 10^NUM_SF

// Globals for this file only. Written once to hold parameters passed from R level.
static const char *na;                 // by default "" or if set (not recommended) then usually "NA"
static char sep;                       // comma in .csv files
static char sep2;                      // '|' within list columns. Used here to know if field should be quoted and in freadR.c to write sep2 in list columns
static char dec;                       // the '.' in the number 3.1416. In Europe often: 3,1416
static int8_t doQuote=INT8_MIN;        // whether to surround fields with double quote ". NA means 'auto' (default)
static bool qmethodEscape=false;       // when quoting fields, how to escape double quotes in the field contents (default false means to add another double quote)
static bool squashDateTime=false;      // 0=ISO(yyyy-mm-dd) 1=squash(yyyymmdd)

extern const char *getString(void *, int);
extern const char *getCategString(void *, int);
extern double wallclock(void);

inline void write_chars(const char *x, char **pch)
{
  // similar to C's strcpy but i) doesn't include trailing \0 and ii) moves destination along
  char *ch = *pch;
  while (*x) *ch++=*x++;
  *pch = ch;
}

void writeBool8(int8_t *col, int64_t row, char **pch)
{
  int8_t x = col[row];
  char *ch = *pch;
  *ch++ = '0'+(x==1);
  *pch = ch-(x==INT8_MIN);  // if NA then step back, to save a branch
}

void writeBool32(int32_t *col, int64_t row, char **pch)
{
  int32_t x = col[row];
  char *ch = *pch;
  if (x==INT32_MIN) {  // TODO: when na=='\0' as recommended, use a branchless writer
    write_chars(na, &ch);
  } else {
    *ch++ = '0'+x;
  }
  *pch = ch;
}

void writeBool32AsString(int32_t *col, int64_t row, char **pch)
{
  int32_t x = col[row];
  char *ch = *pch;
  if (x == INT32_MIN) {
    write_chars(na, &ch);
  } else if (x) {
    *ch++='T'; *ch++='R'; *ch++='U'; *ch++='E';
  } else {
    *ch++='F'; *ch++='A'; *ch++='L'; *ch++='S'; *ch++='E';
  }
  *pch = ch;
}

static inline void reverse(char *upp, char *low)
{
  upp--;
  while (upp>low) {
    char tmp = *upp;
    *upp = *low;
    *low = tmp;
    upp--;
    low++;
  }
}

void writeInt32(int32_t *col, int64_t row, char **pch)
{
  char *ch = *pch;
  int32_t x = col[row];
  if (x == INT32_MIN) {
    write_chars(na, &ch);
  } else {
    if (x<0) { *ch++ = '-'; x=-x; }
    // Avoid log() for speed. Write backwards then reverse when we know how long.
    char *low = ch;
    do { *ch++ = '0'+x%10; x/=10; } while (x>0);
    reverse(ch, low);
  }
  *pch = ch;
}

void writeInt64(int64_t *col, int64_t row, char **pch)
{
  char *ch = *pch;
  int64_t x = col[row];
  if (x == INT64_MIN) {
    write_chars(na, &ch);
  } else {
    if (x<0) { *ch++ = '-'; x=-x; }
    char *low = ch;
    do { *ch++ = '0'+x%10; x/=10; } while (x>0);
    reverse(ch, low);
  }
  *pch = ch;
}

/*
 * Generate fwriteLookup.h which defines sigparts, expsig and exppow that writeNumeric() that follows uses.
 * It was run once a long time ago in dev and we don't need to generate it again unless we change it.
 * Commented out and left here in the file where its result is used, in case we need it in future.
 * Reason: ldexpl may not be available on all platforms and is slower than a direct lookup when it is.
 *
void genLookups() {
  FILE *f = fopen("/tmp/fwriteLookups.h", "w");
  fprintf(f, "//\n\
// Generated by fwrite.c:genLookups()\n\
//\n\
// 3 vectors: sigparts, expsig and exppow\n\
// Includes precision higher than double; leave this compiler on this machine\n\
// to parse the literals at reduced precision.\n\
// 2^(-1023:1024) is held more accurately than double provides by storing its\n\
// exponent separately (expsig and exppow)\n\
// We don't want to depend on 'long double' (>64bit) availability to generate\n\
// these at runtime; libraries and hardware vary.\n\
// These small lookup tables are used for speed.\n\
//\n\n");
  fprintf(f, "const double sigparts[53] = {\n0.0,\n");
  for (int i=1; i<=52; i++) {
    fprintf(f, "%.40Le%s\n",ldexpl(1.0L,-i), i==52?"":",");
  }
  fprintf(f, "};\n\nconst double expsig[2048] = {\n");
  char x[2048][60];
  for (int i=0; i<2048; i++) {
    sprintf(x[i], "%.40Le", ldexpl(1.0L, i-1023));
    fprintf(f, "%.*s%s\n", (int)(strchr(x[i],'e')-x[i]), x[i], (i==2047?"":",") );
  }
  fprintf(f, "};\n\nconst int exppow[2048] = {\n");
  for (int i=0; i<2048; i++) {
    fprintf(f, "%d%s", atoi(strchr(x[i],'e')+1), (i==2047?"":",") );
  }
  fprintf(f, "};\n\n");
  fclose(f);
  return R_NilValue;
}
*/

void writeFloat64(double *col, int64_t row, char **pch)
{
  // hand-rolled / specialized for speed
  // *pch is safely the output destination with enough space (ensured via calculating maxLineLen up front)
  // technique similar to base R (format.c:formatReal and printutils.c:EncodeReal0)
  // differences/tricks :
  //   i) no buffers. writes straight to the final file buffer passed to write()
  //  ii) no C libary calls such as sprintf() where the fmt string has to be interpretted over and over
  // iii) no need to return variables or flags.  Just writes.
  //  iv) shorter, easier to read and reason with in one self contained place.
  double x = col[row];
  char *ch = *pch;
  if (!isfinite(x)) {
    if (isnan(x)) {
      write_chars(na, &ch);
    } else {
      if (x<0) *ch++ = '-';
      *ch++ = 'I'; *ch++ = 'n'; *ch++ = 'f';
    }
  } else if (x == 0.0) {
    *ch++ = '0';   // and we're done.  so much easier rather than passing back special cases
  } else {
    if (x < 0.0) { *ch++ = '-'; x = -x; }  // and we're done on sign, already written. no need to pass back sign
    union { double d; uint64_t l; } u;
    u.d = x;
    uint64_t fraction = u.l & 0xFFFFFFFFFFFFF;           // (1<<52)-1;
    uint32_t exponent = (int32_t)((u.l>>52) & 0x7FF);    // [0,2047]

    // Now sum the appropriate powers 2^-(1:52) of the fraction
    // Important for accuracy to start with the smallest first; i.e. 2^-52
    // Exact powers of 2 (1.0, 2.0, 4.0, etc) are represented precisely with fraction==0
    // Skip over tailing zeros for exactly representable numbers such 0.5, 0.75
    // Underflow here (0u-1u = all 1s) is on an unsigned type which is ok by C standards
    // sigparts[0] arranged to be 0.0 in genLookups() to enable branch free loop here
    double acc = 0;  // 'long double' not needed
    int i = 52;
    if (fraction) {
      while ((fraction & 0xFF) == 0) { fraction >>= 8; i-=8; }
      while (fraction) {
        acc += sigparts[(((fraction & 1u)^1u)-1u) & i];
        i--;
        fraction >>= 1;
      }
    }
    // 1.0+acc is in range [1.5,2.0) by IEEE754
    // expsig is in range [1.0,10.0) by design of fwriteLookups.h
    // Therefore y in range [1.5,20.0)
    // Avoids (potentially inaccurate and potentially slow) log10/log10l, pow/powl, ldexp/ldexpl
    // By design we can just lookup the power from the tables
    double y = (1.0+acc) * expsig[exponent];  // low magnitude mult
    int exp = exppow[exponent];
    if (y>=9.99999999999999) { y /= 10; exp++; }
    uint64_t l = y * SIZE_SF;  // low magnitude mult 10^NUM_SF
    // l now contains NUM_SF+1 digits as integer where repeated /10 below is accurate

    // if (verbose) Rprintf("\nTRACE: acc=%.20Le ; y=%.20Le ; l=%llu ; e=%d     ", acc, y, l, exp);

    if (l%10 >= 5) l+=10; // use the last digit to round
    l /= 10;
    if (l == 0) {
      if (*(ch-1)=='-') ch--;
      *ch++ = '0';
    } else {
      // Count trailing zeros and therefore s.f. present in l
      int trailZero = 0;
      while (l%10 == 0) { l /= 10; trailZero++; }
      int sf = NUM_SF - trailZero;
      if (sf==0) {sf=1; exp++;}  // e.g. l was 9999999[5-9] rounded to 10000000 which added 1 digit

      // l is now an unsigned long that doesn't start or end with 0
      // sf is the number of digits now in l
      // exp is e<exp> were l to be written with the decimal sep after the first digit
      int dr = sf-exp-1; // how many characters to print to the right of the decimal place
      int width=0;       // field width were it written decimal format. Used to decide whether to or not.
      int dl0=0;         // how many 0's to add to the left of the decimal place before starting l
      if (dr<=0) { dl0=-dr; dr=0; width=sf+dl0; }  // 1, 10, 100, 99000
      else {
        if (sf>dr) width=sf+1;                     // 1.234 and 123.4
        else { dl0=1; width=dr+1+dl0; }            // 0.1234, 0.0001234
      }
      // So:  3.1416 => l=31416, sf=5, exp=0     dr=4; dl0=0; width=6
      //      30460  => l=3046, sf=4, exp=4      dr=0; dl0=1; width=5
      //      0.0072 => l=72, sf=2, exp=-3       dr=4; dl0=1; width=6
      if (width <= sf + (sf>1) + 2 + (abs(exp)>99?3:2)) {
         //              ^^^^ to not include 1 char for dec in -7e-04 where sf==1
         //                      ^ 2 for 'e+'/'e-'
         // decimal format ...
         ch += width-1;
         if (dr) {
           while (dr && sf) { *ch--='0'+l%10; l/=10; dr--; sf--; }
           while (dr) { *ch--='0'; dr--; }
           *ch-- = dec;
         }
         while (dl0) { *ch--='0'; dl0--; }
         while (sf) { *ch--='0'+l%10; l/=10; sf--; }
         // ch is now 1 before the first char of the field so position it afterward again, and done
         ch += width+1;
      } else {
        // scientific ...
        ch += sf;  // sf-1 + 1 for dec
        for (int i=sf; i>1; i--) {
          *ch-- = '0' + l%10;
          l /= 10;
        }
        if (sf == 1) ch--; else *ch-- = dec;
        *ch = '0' + l;
        ch += sf + (sf>1);
        *ch++ = 'e';  // lower case e to match base::write.csv
        if (exp < 0) { *ch++ = '-'; exp=-exp; }
        else { *ch++ = '+'; }  // to match base::write.csv
        if (exp < 100) {
          *ch++ = '0' + (exp / 10);
          *ch++ = '0' + (exp % 10);
        } else {
          *ch++ = '0' + (exp / 100);
          *ch++ = '0' + (exp / 10) % 10;
          *ch++ = '0' + (exp % 10);
        }
      }
    }
  }
  *pch = ch;
}

// DATE/TIME

static inline void write_time(int32_t x, char **pch)
// just a helper called below by the real writers (time-only and datetime)
{
  char *ch = *pch;
  if (x<0) {  // <0 covers NA_INTEGER too (==INT_MIN checked in init.c)
    write_chars(na, &ch);
  } else {
    int hh = x/3600;
    int mm = (x - hh*3600) / 60;
    int ss = x%60;
    *ch++ = '0'+hh/10;
    *ch++ = '0'+hh%10;
    *ch++ = ':';
    ch -= squashDateTime;
    *ch++ = '0'+mm/10;
    *ch++ = '0'+mm%10;
    *ch++ = ':';
    ch -= squashDateTime;
    *ch++ = '0'+ss/10;
    *ch++ = '0'+ss%10;
  }
  *pch = ch;
}

void writeITime(int32_t *col, int64_t row, char **pch) {
  write_time(col[row], pch);
}

static inline void write_date(int32_t x, char **pch)
// just a helper called below by the two real writers (date-only and datetime)
{
  // From base ?Date :
  //  "  Dates are represented as the number of days since 1970-01-01, with negative values
  // for earlier dates. They are always printed following the rules of the current Gregorian calendar,
  // even though that calendar was not in use long ago (it was adopted in 1752 in Great Britain and its
  // colonies)  "

  // The algorithm here in data.table::fwrite was taken from civil_from_days() here :
  //   http://howardhinnant.github.io/date_algorithms.html
  // which was donated to the public domain thanks to Howard Hinnant, 2013.
  // The rebase to 1 March 0000 is inspired: avoids needing isleap() at all.
  // The only small modifications here are :
  //   1) no need for era
  //   2) impose date range of [0000-03-01, 9999-12-31]. All 3,652,365 dates tested in test 1739
  //   3) use direct lookup for mmdd rather than the math using 153, 2 and 5
  //   4) use true/false value (md/100)<3 rather than ?: branch
  // The end result is 5 lines of simple branch free integer math with no library calls.
  // as.integer(as.Date(c("0000-03-01","9999-12-31"))) == c(-719468,+2932896)

  char *ch = *pch;
  if (x< -719468 || x>2932896) {
    // NA_INTEGER<(-719468) (==INT_MIN checked in init.c)
    write_chars(na, &ch);
  } else {
    x += 719468;  // convert days from 1970-01-01 to days from 0000-03-01 (the day after 29 Feb 0000)
    int y = (x - x/1461 + x/36525 - x/146097) / 365;  // year of the preceeding March 1st
    int z =  x - y*365 - y/4 + y/100 - y/400 + 1;     // days from March 1st in year y
    int md = monthday[z];  // See fwriteLookups.h for how the 366 item lookup 'monthday' is arranged
    y += z && (md/100)<3;  // The +1 above turned z=-1 to 0 (meaning Feb29 of year y not Jan or Feb of y+1)

    ch += 7 + 2*!squashDateTime;
    *ch-- = '0'+md%10; md/=10;
    *ch-- = '0'+md%10; md/=10;
    *ch-- = '-';
    ch += squashDateTime;
    *ch-- = '0'+md%10; md/=10;
    *ch-- = '0'+md%10; md/=10;
    *ch-- = '-';
    ch += squashDateTime;
    *ch-- = '0'+y%10; y/=10;
    *ch-- = '0'+y%10; y/=10;
    *ch-- = '0'+y%10; y/=10;
    *ch   = '0'+y%10; y/=10;
    ch += 8 + 2*!squashDateTime;
  }
  *pch = ch;
}

void writeDateInt32(int32_t *col, int64_t row, char **pch) {
  write_date(col[row], pch);
}

void writeDateFloat64(double *col, int64_t row, char **pch) {
  write_date(isfinite(col[row]) ? (int)(col[row]) : INT32_MIN, pch);
}

void writePOSIXct(double *col, int64_t row, char **pch)
{
  // Write ISO8601 UTC by default to encourage ISO standards, stymie ambiguity and for speed.
  // R internally represents POSIX datetime in UTC always. Its 'tzone' attribute can be ignored.
  // R's representation ignores leap seconds too which is POSIX compliant, convenient and fast.
  // Aside: an often overlooked option for users is to start R in UTC: $ TZ='UTC' R
  // All positive integers up to 2^53 (9e15) are exactly representable by double which is relied
  // on in the ops here; number of seconds since epoch.

  double x = col[row];
  char *ch = *pch;
  if (!isfinite(x)) {
    write_chars(na, &ch);
  } else {
    int64_t xi, d, t;
    if (x>=0) {
      xi = floor(x);
      d = xi / 86400;
      t = xi % 86400;
    } else {
      // before 1970-01-01T00:00:00Z
      xi = floor(x);
      d = (xi+1)/86400 - 1;
      t = xi - d*86400;  // xi and d are both negative here; t becomes the positive number of seconds into the day
    }
    int m = ((x-xi)*10000000); // 7th digit used to round up if 9
    m += (m%10);  // 9 is numerical accuracy, 8 or less then we truncate to last microsecond
    m /= 10;
    write_date(d, &ch);
    *ch++ = 'T';
    ch -= squashDateTime;
    write_time(t, &ch);
    if (squashDateTime || (m && m%1000==0)) {
      // when squashDateTime always write 3 digits of milliseconds even if 000, for consistent scale of squash integer64
      // don't use writeInteger() because it doesn't 0 pad which we need here
      // integer64 is big enough for squash with milli but not micro; trunc (not round) micro when squash
      m /= 1000;
      *ch++ = '.';
      ch -= squashDateTime;
      *(ch+2) = '0'+m%10; m/=10;
      *(ch+1) = '0'+m%10; m/=10;
      *ch     = '0'+m;
      ch += 3;
    } else if (m) {
      // microseconds are present and !squashDateTime
      *ch++ = '.';
      *(ch+5) = '0'+m%10; m/=10;
      *(ch+4) = '0'+m%10; m/=10;
      *(ch+3) = '0'+m%10; m/=10;
      *(ch+2) = '0'+m%10; m/=10;
      *(ch+1) = '0'+m%10; m/=10;
      *ch     = '0'+m;
      ch += 6;
    }
    *ch++ = 'Z';
    ch -= squashDateTime;
  }
  *pch = ch;
}

void writeNanotime(int64_t *col, int64_t row, char **pch)
{
  int64_t x = col[row];
  char *ch = *pch;
  if (x == INT64_MIN) {
    write_chars(na, &ch);
  } else {
    int d/*days*/, s/*secs*/, n/*nanos*/;
    n = x % 1000000000;
    x /= 1000000000;
    if (x>=0 && n>=0) {
      d = x / 86400;
      s = x % 86400;
    } else {
      // before 1970-01-01T00:00:00.000000000Z
      if (n) { x--; n += 1000000000; }
      d = (x+1)/86400 - 1;
      s = x - d*86400;  // x and d are both negative here; secs becomes the positive number of seconds into the day
    }
    write_date(d, &ch);
    *ch++ = 'T';
    ch -= squashDateTime;
    write_time(s, &ch);
    *ch++ = '.';
    ch -= squashDateTime;
    for (int i=8; i>=0; i--) { *(ch+i) = '0'+n%10; n/=10; }  // always 9 digits for nanoseconds
    ch += 9;
    *ch++ = 'Z';
    ch -= squashDateTime;
  }
  *pch = ch;
}

static inline void write_string(const char *x, char **pch)
{
  char *ch = *pch;
  if (x == NULL) {
    // NA is not quoted even when quote=TRUE to distinguish from quoted "NA" value.  But going forward: ,,==NA and ,"",==empty string
    write_chars(na, &ch);
  } else {
    int8_t q = doQuote;
    if (q==INT8_MIN) { // NA means quote="auto"
      const char *tt = x;
      if (*tt=='\0') {
        // Empty strings are always quoted to distinguish from ,,==NA
        *ch++='"'; *ch++='"';   // test 1732.7 covers this (confirmed in gdb) so it's unknown why codecov claims no coverage
        *pch = ch;
        return;
      }
      while (*tt!='\0' && *tt!=sep && *tt!=sep2 && *tt!='\n' && *tt!='\r' && *tt!='"') *ch++ = *tt++;
      // Windows includes \n in its \r\n so looking for \n only is sufficient
      // sep2 is set to '\0' when no list columns are present
      if (*tt=='\0') {
        // most common case: no sep, newline or " contained in string
        *pch = ch;  // advance caller over the field already written
        return;
      }
      ch = *pch; // rewind the field written since it needs to be quoted
      q = true;
    }
    if (q==false) {
      write_chars(x, &ch);
    } else {
      *ch++ = '"';
      const char *tt = x;
      if (qmethodEscape) {
        while (*tt!='\0') {
          if (*tt=='"' || *tt=='\\') *ch++ = '\\';
          *ch++ = *tt++;
        }
      } else {
        // qmethod='double'
        while (*tt!='\0') {
          if (*tt=='"') *ch++ = '"';
          *ch++ = *tt++;
        }
      }
      *ch++ = '"';
    }
  }
  *pch = ch;
}

void writeString(void *col, int64_t row, char **pch)
{
  write_string(getString(col, row), pch);
}

void writeCategString(void *col, int64_t row, char **pch)
{
  write_string(getCategString(col, row), pch);
}


static int failed = 0;
static int rowsPerBatch;

static inline void checkBuffer(
  char **buffer,       // this thread's buffer
  size_t *myAlloc,     // the size of this buffer
  char **ch,           // the end of the last line written to the buffer by this thread
  size_t myMaxLineLen  // the longest line seen so far by this thread
  // Initial size for the thread's buffer is twice as big as needed for rowsPerBatch based on
  // maxLineLen from the sample; i.e. only 50% of the buffer should be used.
  // If we get to 75% used, we'll realloc.
  // i.e. very cautious and grateful to the OS for not fetching untouched pages of buffer.
  // Plus, more caution ... myMaxLineLine is tracked and if that grows we'll realloc too.
  // Very long lines are caught up front and rowsPerBatch is set to 1 in that case.
  // This checkBuffer() is called after every line.
) {
  if (failed) return;  // another thread already failed. Fall through and error().
  size_t thresh = 0.75*(*myAlloc);
  if ((*ch > (*buffer)+thresh) ||
      (rowsPerBatch*myMaxLineLen > thresh )) {
    size_t off = *ch-*buffer;
    *myAlloc = 1.5*(*myAlloc);
    *buffer = realloc(*buffer, *myAlloc);
    if (*buffer==NULL) {
      failed = -errno;    // - for malloc/realloc errno, + for write errno
    } else {
      *ch = *buffer+off;  // in case realloc moved the allocation
    }
  }
}

void fwriteMain(fwriteMainArgs args)
{
  double startTime = wallclock();
  double nextTime = startTime+2; // start printing progress meter in 2 sec if not completed by then
  double t0 = startTime;

  na = args.na;
  sep = args.sep;
  sep2 = args.sep2;
  dec = args.dec;
  doQuote = args.doQuote;

  // When NA is a non-empty string, then we must quote all string fields in case they contain the na string
  // na is recommended to be empty, though
  if (na[0]!='\0' && doQuote==INT8_MIN) doQuote = true;

  qmethodEscape = args.qmethodEscape;
  squashDateTime = args.squashDateTime;

  // Estimate max line length of a 1000 row sample (100 rows in 10 places).
  // 'Estimate' even of this sample because quote='auto' may add quotes and escape embedded quotes.
  // Buffers will be resized later if there are too many line lengths outside the sample, anyway.
  // maxLineLen is required to determine a reasonable rowsPerBatch.


  // alloc one buffMB here.  Keep rewriting each field to it, to sum up the size.  Restriction: one field can't be
  // greater that minimumum buffMB (1MB = 1 million characters).  Otherwise unbounded overwrite. Possible with very
  // very long single strings, or very long list column values.
  // The caller guarantees no field with be longer than this. If so, it can set buffMB larger. It might know
  // due to some stats it has maintained on each column or in the environment generally.
  // However, a single field being longer than 1 million characters is considered a very reasonable restriction.
  // Once we have a good line length estimate, we may increase the buffer size a lot anyway.
  // The default buffMB is 8MB,  so it's really 8 million character limit by default. 1MB is because user might set
  // buffMB to 1, say if they have 512 CPUs or more, perhaps.

  // Cold section as only 1,000 rows. Speed not an issue issue here.
  // Overestimating line length is ok.
  int eolLen = strlen(args.eol);
  if (eolLen<=0) STOP("eol must be 1 or more bytes (usually either \\n or \\r\\n) but is length %d", eolLen);

  int buffMB = args.buffMB;
  if (buffMB<1 || buffMB>1024) STOP("buffMB=%d outside [1,1024]", buffMB);
  size_t buffSize = (size_t)1024*1024*buffMB;
  char *buff = malloc(buffSize);
  if (!buff) STOP("Unable to allocate %dMB for line length estimation: %s", buffMB, strerror(errno));

  if (args.verbose) {
    DTPRINT("Column writers: ");
    if (args.ncol<=50) {
      for (int j=0; j<args.ncol; j++) DTPRINT("%d ", args.whichFun[j]);
    } else {
      for (int j=0; j<30; j++) DTPRINT("%d ", args.whichFun[j]);
      DTPRINT("... ");
      for (int j=args.ncol-10; j<args.ncol; j++) DTPRINT("%d ", args.whichFun[j]);
    }
    DTPRINT("\n");
  }

  int maxLineLen = 0;
  int step = args.nrow<1000 ? 100 : args.nrow/10;
  for (int64_t start=0; start<args.nrow; start+=step) {
    int64_t end = (args.nrow-start)<100 ? args.nrow : start+100;
    for (int64_t i=start; i<end; i++) {
      int thisLineLen=0;
      if (args.doRowNames) {
        if (args.rowNames) {
          char *ch = buff;
          writeString(args.rowNames, i, &ch);
          thisLineLen += (int)(ch-buff);     // see comments above about restrictions/guarantees/contracts
        } else {
          thisLineLen += 1+(int)log10(args.nrow);  // the width of the row number
        }
        thisLineLen += 2*(doQuote!=0/*NA('auto') or true*/) + 1/*sep*/;
      }
      for (int j=0; j<args.ncol; j++) {
        char *ch = buff;                // overwrite each field at the beginning of buff to be more robust to single fields > 1 million bytes
        args.funs[args.whichFun[j]]( args.columns[j], i, &ch );
        thisLineLen += (int)(ch-buff) + 1/*sep*/;        // see comments above about restrictions/guarantees/contracts
      }
      if (thisLineLen > maxLineLen) maxLineLen = thisLineLen;
    }
  }
  maxLineLen += eolLen;
  if (args.verbose) DTPRINT("maxLineLen=%d from sample. Found in %.3fs\n", maxLineLen, 1.0*(wallclock()-t0));

  int f=0;
  gzFile zf=NULL;
  int err;
  if (*args.filename=='\0') {
    f=-1;  // file="" means write to standard output
    args.is_gzip = false; // gzip is only for file
    // eol = "\n";  // We'll use DTPRINT which converts \n to \r\n inside it on Windows
  } else if (!args.is_gzip) {
#ifdef WIN32
    f = _open(args.filename, _O_WRONLY | _O_BINARY | _O_CREAT | (args.append ? _O_APPEND : _O_TRUNC), _S_IWRITE);
    // O_BINARY rather than O_TEXT for explicit control and speed since it seems that write() has a branch inside it
    // to convert \n to \r\n on Windows when in text mode not not when in binary mode.
#else
    f = open(args.filename, O_WRONLY | O_CREAT | (args.append ? O_APPEND : O_TRUNC), 0666);
    // There is no binary/text mode distinction on Linux and Mac
#endif
    if (f == -1) {
      int erropen = errno;
      STOP(access( args.filename, F_OK ) != -1 ?
           "%s: '%s'. Failed to open existing file for writing. Do you have write permission to it? Is this Windows and does another process such as Excel have it open?" :
           "%s: '%s'. Unable to create new file for writing (it does not exist already). Do you have permission to write here, is there space on the disk and does the path exist?",
           strerror(erropen), args.filename);
    }
  } else {
    zf = gzopen(args.filename, "wb");
    if (zf == NULL) {
      int erropen = errno;
      STOP(access( args.filename, F_OK ) != -1 ?
           "%s: '%s'. Failed to open existing file for writing. Do you have write permission to it? Is this Windows and does another process such as Excel have it open?" :
           "%s: '%s'. Unable to create new file for writing (it does not exist already). Do you have permission to write here, is there space on the disk and does the path exist?",
           strerror(erropen), args.filename);
    }
    // alloc gzip buffer : buff + 10% + 16
    size_t buffzSize = (size_t)(1024*1024*buffMB + 1024*1024*buffMB / 10 + 16);
    if (gzbuffer(zf, buffzSize)) {
      STOP("Error allocate buffer for gzip file");
    }
  }
    
  t0=wallclock();

  if (args.verbose) {
    DTPRINT("Writing column names ... ");
    if (f==-1) DTPRINT("\n");
  }
  if (args.colNames) {
    // We don't know how long this line will be.
    // It could be (much) longer than the data row line lengths
    // To keep things simple we'll reuse the same buffer used above for each field, and write each column name separately to the file.
    // If multiple calls to write() is ever an issue, we'll come back to this. But very unlikely.
    char *ch = buff;
    if (args.doRowNames) {
      // Unusual: the extra blank column name when row_names are added as the first column
      if (doQuote!=0/*'auto'(NA) or true*/) { *ch++='"'; *ch++='"'; } // to match write.csv
      *ch++ = sep;
    }
    for (int j=0; j<args.ncol; j++) {
      writeString(args.colNames, j, &ch);
      if(!args.is_gzip) {
        if (f==-1) {
          *ch = '\0';
          DTPRINT(buff);
        } else if (WRITE(f, buff, (int)(ch-buff)) == -1) {  // TODO: move error check inside WRITE
          int errwrite=errno;  // capture write errno now incase close fails with a different errno
          CLOSE(f);
          free(buff);
          STOP("%s: '%s'", strerror(errwrite), args.filename);
        }
      } else {
        if ((!gzwrite(zf, buff, (int)(ch-buff)))) {
          int errwrite=gzclose(zf);
          free(buff);
          STOP("Error gzwrite %d: %s", errwrite, args.filename);
        }
      }
          
      ch = buff;  // overwrite column names at the start in case they are > 1 million bytes long
      *ch++ = args.sep;  // this sep after the last column name won't be written to the file
    }
    if (f==-1) {
      DTPRINT(args.eol);
    } else if (!args.is_gzip && WRITE(f, args.eol, eolLen)==-1) {
      int errwrite=errno;
      CLOSE(f);
      free(buff);
      STOP("%s: '%s'", strerror(errwrite), args.filename);
    } else if (args.is_gzip && (!gzwrite(zf, args.eol, eolLen))) {
      int errwrite=gzclose(zf);
      free(buff);
      STOP("Error gzwrite %d: %s", errwrite, args.filename);
    }
      
  }
  free(buff);  // TODO: also to be free'd in cleanup when there's an error opening file above
  if (args.verbose) DTPRINT("done in %.3fs\n", 1.0*(wallclock()-t0));
  if (args.nrow == 0) {
    if (args.verbose) DTPRINT("No data rows present (nrow==0)\n");
    if (args.is_gzip) {
      if ( (err = gzclose(zf)) ) STOP("gzclose error %d: '%s'", err, args.filename);
    } else {
      if (f!=-1 && CLOSE(f)) STOP("%s: '%s'", strerror(errno), args.filename);
    }
    return;
  }

  // Decide buffer size and rowsPerBatch for each thread
  // Once rowsPerBatch is decided it can't be changed, but we can increase buffer size if the lines
  // turn out to be longer than estimated from the sample.
  // buffSize large enough to fit many lines to i) reduce calls to write() and ii) reduce thread sync points
  // It doesn't need to be small in cache because it's written contiguously.
  // If we don't use all the buffer for any reasons that's ok as OS will only getch the cache lines touched.
  // So, generally the larger the better up to max filesize/nth to use all the threads. A few times
  //   smaller than that though, to achieve some load balancing across threads since schedule(dynamic).
  if (maxLineLen > buffSize) buffSize=2*maxLineLen;  // A very long line; at least 1,048,576 characters (since min(buffMB)==1)
  rowsPerBatch =
    (10*maxLineLen > buffSize) ? 1 :  // very very long lines (100,000 characters+) each thread will just do one row at a time.
    0.5 * buffSize/maxLineLen;        // Aim for 50% buffer usage. See checkBuffer for comments.
  if (rowsPerBatch > args.nrow) rowsPerBatch = args.nrow;
  int numBatches = (args.nrow-1)/rowsPerBatch + 1;
  int nth = args.nth;
  if (numBatches < nth) nth = numBatches;
  if (args.verbose) {
    DTPRINT("Writing %d rows in %d batches of %d rows (each buffer size %dMB, showProgress=%d, nth=%d) ... ",
            args.nrow, numBatches, rowsPerBatch, args.buffMB, args.showProgress, nth);
    if (f==-1) DTPRINT("\n");
  }
  t0 = wallclock();

  failed=0;  // static global so checkBuffer can set it. -errno for malloc or realloc fails, +errno for write fail
  bool hasPrinted=false;
  bool anyBufferGrown=false;
  int maxBuffUsedPC=0;

  #pragma omp parallel num_threads(nth)
  {
    char *ch, *myBuff;               // local to each thread
    ch = myBuff = malloc(buffSize);  // each thread has its own buffer. malloc and errno are thread-safe.
    if (myBuff==NULL) {failed=-errno;}
    // Do not rely on availability of '#omp cancel' new in OpenMP v4.0 (July 2013).
    // OpenMP v4.0 is in gcc 4.9+ (https://gcc.gnu.org/wiki/openmp) but
    // not yet in clang as of v3.8 (http://openmp.llvm.org/)
    // If not-me failed, I'll see shared 'failed', fall through loop, free my buffer
    // and after parallel section, single thread will call STOP() safely.

    size_t myAlloc = buffSize;
    size_t myMaxLineLen = maxLineLen;
    // so we can realloc(). Should only be needed if there are very long lines that are
    // much longer than occurred in the sample for maxLineLen; e.g. unusally long string values
    // that didn't occur in the sample, or list columns with some very long vectors in some cells.

    #pragma omp single
    {
      nth = omp_get_num_threads();  // update nth with the actual nth (might be different than requested)
    }
    int me = omp_get_thread_num();

    #pragma omp for ordered schedule(dynamic)
    for(int64_t start=0; start<args.nrow; start+=rowsPerBatch) {
      if (failed) continue;  // Not break. See comments above about #omp cancel
      int64_t end = ((args.nrow - start)<rowsPerBatch) ? args.nrow : start + rowsPerBatch;
      for (int64_t i=start; i<end; i++) {
        char *lineStart = ch;
        // Tepid starts here (once at beginning of each per line)
        if (args.doRowNames) {
          if (args.rowNames==NULL) {
            if (doQuote!=0/*NA'auto' or true*/) *ch++='"';
            int64_t rn = i+1;
            writeInt64(&rn, 0, &ch);
            if (doQuote!=0) *ch++='"';
          } else {
            writeString(args.rowNames, i, &ch);
          }
          *ch++=sep;
        }
        // Hot loop
        for (int j=0; j<args.ncol; j++) {
          //printf("j=%d args.ncol=%d myBuff='%.*s' ch=%p\n", j, args.ncol, 20, myBuff, ch);
          (args.funs[args.whichFun[j]])(args.columns[j], i, &ch);
          //printf("  j=%d args.ncol=%d myBuff='%.*s' ch=%p\n", j, args.ncol, 20, myBuff, ch);
          *ch++ = sep;
          //printf("  j=%d args.ncol=%d myBuff='%.*s' ch=%p\n", j, args.ncol, 20, myBuff, ch);
        }
        // Tepid again (once at the end of each line)
        ch--;  // backup onto the last sep after the last column. ncol>=1 because 0-columns was caught earlier.
        write_chars(args.eol, &ch);  // overwrite last sep with eol instead

        // Track longest line seen so far. If we start to see longer lines than we saw in the
        // sample, we'll realloc the buffer. The rowsPerBatch chosen based on the (very good) sample,
        // must fit in the buffer. Can't early write and reset buffer because the
        // file output would be out-of-order. Can't change rowsPerBatch after the 'parallel for' started.
        size_t thisLineLen = ch-lineStart;
        if (thisLineLen > myMaxLineLen) myMaxLineLen=thisLineLen;
        checkBuffer(&myBuff, &myAlloc, &ch, myMaxLineLen);
        if (failed) break; // this thread stop writing rows; fall through to clear up and STOP() below
      }
      #pragma omp ordered
      {
        if (!failed) { // a thread ahead of me could have failed below while I was working or waiting above
          if (f==-1) {
            *ch='\0';  // standard C string end marker so DTPRINT knows where to stop
            DTPRINT(myBuff);
            // nth==1 at this point since when file=="" (f==-1 here) fwrite.R calls setDTthreads(1)
            // Although this ordered section is one-at-a-time it seems that calling Rprintf() here, even with a
            // R_FlushConsole() too, causes corruptions on Windows but not on Linux. At least, as observed so
            // far using capture.output(). Perhaps Rprintf() updates some state or allocation that cannot be done
            // by slave threads, even when one-at-a-time. Anyway, made this single-threaded when output to console
            // to be safe (setDTthreads(1) in fwrite.R) since output to console doesn't need to be fast.
          } else {
            if (!args.is_gzip && WRITE(f, myBuff, (int)(ch-myBuff)) == -1) {
              failed=errno;
            } else if (args.is_gzip && (!gzwrite(zf, myBuff, (int)(ch-myBuff)))) {
              gzerror(zf, &failed);
            }
            if (myAlloc > buffSize) anyBufferGrown = true;
            int used = 100*((double)(ch-myBuff))/buffSize;  // percentage of original buffMB
            if (used > maxBuffUsedPC) maxBuffUsedPC = used;
            double now;
            if (me==0 && args.showProgress && (now=wallclock())>=nextTime && !failed) {
              // See comments above inside the f==-1 clause.
              // Not only is this ordered section one-at-a-time but we'll also Rprintf() here only from the
              // master thread (me==0) and hopefully this will work on Windows. If not, user should set
              // showProgress=FALSE until this can be fixed or removed.
              int ETA = (int)((args.nrow-end)*((now-startTime)/end));
              if (hasPrinted || ETA >= 2) {
                if (args.verbose && !hasPrinted) DTPRINT("\n");
                DTPRINT("\rWritten %.1f%% of %d rows in %d secs using %d thread%s. "
                        "anyBufferGrown=%s; maxBuffUsed=%d%%. ETA %d secs.      ",
                         (100.0*end)/args.nrow, args.nrow, (int)(now-startTime), nth, nth==1?"":"s",
                         anyBufferGrown?"yes":"no", maxBuffUsedPC, ETA);
                // TODO: use progress() as in fread
                nextTime = now+1;
                hasPrinted = true;
              }
            }
            // May be possible for master thread (me==0) to call R_CheckUserInterrupt() here.
            // Something like:
            // if (me==0) {
            //   failed = TRUE;  // inside ordered here; the slaves are before ordered and not looking at 'failed'
            //   R_CheckUserInterrupt();
            //   failed = FALSE; // no user interrupt so return state
            // }
            // But I fear the slaves will hang waiting for the master (me==0) to complete the ordered
            // section which may not happen if the master thread has been interrupted. Rather than
            // seeing failed=TRUE and falling through to free() and close() as intended.
            // Could register a finalizer to free() and close() perhaps :
            // [r-devel] http://r.789695.n4.nabble.com/checking-user-interrupts-in-C-code-tp2717528p2717722.html
            // Conclusion for now: do not provide ability to interrupt.
            // write() errors and malloc() fails will be caught and cleaned up properly, however.
          }
          ch = myBuff;  // back to the start of my buffer ready to fill it up again
        }
      }
    }
    free(myBuff);
    // all threads will call this free on their buffer, even if one or more threads had malloc
    // or realloc fail. If the initial malloc failed, free(NULL) is ok and does nothing.
  }
  // Finished parallel region and can call R API safely now.
  if (hasPrinted) {
    if (!failed) {
      // clear the progress meter
      DTPRINT("\r                                                                       "
              "                                                              \r");
    } else {
      // unless failed as we'd like to see anyBufferGrown and maxBuffUsedPC
      DTPRINT("\n");
    }
  }
  
  if (!args.is_gzip) {
    if (f!=-1 && CLOSE(f) && !failed)
      STOP("%s: '%s'", strerror(errno), args.filename);
  } else {
    if ( (err=gzclose(zf)) ) {
      STOP("gzclose error %d: '%s'", err, args.filename);
    }
  }
  // quoted '%s' in case of trailing spaces in the filename
  // If a write failed, the line above tries close() to clean up, but that might fail as well. So the
  // '&& !failed' is to not report the error as just 'closing file' but the next line for more detail
  // from the original error.
  if (failed<0) {
    STOP("%s. One or more threads failed to malloc or realloc their private buffer. nThread=%d and initial buffMB per thread was %d.\n",
         strerror(-failed), nth, args.buffMB);
  } else if (failed>0) {
    STOP("%s: '%s'", strerror(failed), args.filename);
  }
  if (args.verbose) DTPRINT("done (actual nth=%d, anyBufferGrown=%s, maxBuffUsed=%d%%)\n",
                            nth, anyBufferGrown?"yes":"no", maxBuffUsedPC);
  return;
}

