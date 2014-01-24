#include "num2words-en.h"
#include "string.h"

static const char* const HOURS[] = {
  "midnight",
  "one",
  "two",
  "three",
  "four",
  "five",
  "six",
  "seven",
  "eight",
  "nine",
  "ten",
  "eleven",
  "noon"
};

static const char* const RELS[] = {
  " o'clock",
  "five after ",
  "ten after ",
  "quarter after ",
  "twenty after ",
  "twenty five after ",
  "half past ",
  "twenty five to ",
  "twenty to ",
  "quarter to ",
  "ten to ",
  "five to "
};

static size_t append_string(char* buffer, const size_t length, const char* str) {
  strncat(buffer, str, length);

  size_t written = strlen(str);
  return (length > written) ? written : length;
}

void time_to_words(int hours, int minutes, int seconds, char* words, size_t buffer_size) {

  size_t remaining = buffer_size;
  memset(words, 0, buffer_size);

  // We want to operate with a resolution of 30 seconds.  So multiply
  // minutes and seconds by 2.  Then divide by (2 * 5) to carve the hour
  // into five minute intervals.
  int half_mins = (2 * minutes) + (seconds / 30);
  int index = ((half_mins + 5) / (2 * 5)) % 12;
  int hour = hours % 12;

  if (index == 0 && hour == 0) {
    // The "*" makes the hour bold.
    remaining -= append_string(words, remaining, "*");
    // It seems awkward to write "noon o'clock" or "midnight o'clock".
    remaining -= append_string(words, remaining, HOURS[hours /* no modulus! */]);
  }
  else if (index == 0) {
    remaining -= append_string(words, remaining, "*");
    remaining -= append_string(words, remaining, HOURS[hour]);
    remaining -= append_string(words, remaining, RELS[index]);
  }
  else {
    remaining -= append_string(words, remaining, RELS[index]);
    remaining -= append_string(words, remaining, "*");
    remaining -= append_string(words, remaining, HOURS[hour]);
  }

  // Leave one space at the end
  remaining -= append_string(words, remaining, " ");

}

