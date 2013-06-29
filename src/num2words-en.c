#include "num2words-en.h"
#include "string.h"
//#include <stdio.h>

static const char* const ONES[] = {
  "noll",
  "ett",
  "två",
  "tre",
  "fyra",
  "fem",
  "sex",
  "sju",
  "åtta",
  "nio"
};

static const char* const TEENS[] ={
  "-",
  "elva",
  "tolv",
  "tretton",
  "fjorton",
  "femton",
  "sexton",
  "sjutton",
  "arton",
  "nitton"
};

static const char* const TENS[] = {
  "",
  "tio",
  "tjugo",
  "trettio",
  "fyrtio",
  "femtio",
  "sextio",
  "sjuttio",
  "åttio",
  "nittio"
};

#define QUARTER "kvart"
#define HALF "halv"

#define PAST "över"
#define TO "i"

#define MAX_LEN 7

static size_t append_number(char* words, int num) {
  int tens_val = num / 10 % 10;
  int ones_val = num % 10;

  size_t len = 0;

  if (tens_val > 0) {
    if (tens_val == 1 && num != 10) {
      strcat(words, TEENS[ones_val]);
      return strlen(TEENS[ones_val]);
    }
    strcat(words, TENS[tens_val]);
    len += strlen(TENS[tens_val]);
  }

  if (ones_val > 0 || num == 0) {
    strcat(words, ONES[ones_val]);
    len += strlen(ONES[ones_val]);
  }

  if (len > MAX_LEN)
  {
    int fullSize = strlen(words);
    words[fullSize - (len - MAX_LEN)] = 0;
    len = MAX_LEN;
  }

  return len;
}

static size_t append_string(char* buffer, const size_t length, const char* str) {
  strncat(buffer, str, length);

  size_t written = strlen(str);
  return (length > written) ? written : length;
}


void time_to_words(int hours, int minutes, char* words, size_t length) {

  size_t remaining = length;
  memset(words, 0, length);

  // Fuzzy time
  minutes = (minutes + 3) / 5 * 5;

  // Handle minute wrapping
  if (minutes > 55)
  {
    minutes -= 60;
    hours++;
  }

  switch (minutes)
  {
    case 0:
      remaining -= append_string(words, remaining, " ");
      break;
    case 5:
    case 10:
    case 20:
    case 25:
      remaining -= append_number(words, minutes);
      remaining -= append_string(words, remaining, " ");
      remaining -= append_string(words, remaining, PAST);
      break;
    case 15:
      remaining -= append_string(words, remaining, QUARTER);
      remaining -= append_string(words, remaining, " ");
      remaining -= append_string(words, remaining, PAST);
      break;
    case 30:
      remaining -= append_string(words, remaining, HALF);
      hours++;
      break;
    case 35:
    case 40:
    case 50:
    case 55:
      remaining -= append_number(words, 60 - minutes);
      remaining -= append_string(words, remaining, " ");
      remaining -= append_string(words, remaining, TO);
      hours++;
      break;
    case 45:
      remaining -= append_string(words, remaining, QUARTER);
      remaining -= append_string(words, remaining, " ");
      remaining -= append_string(words, remaining, TO);
      hours++;
      break;
  }

  remaining = append_string(words, remaining, " ");

  // Handle hour wrapping
  hours += 12; // If hours == 0
  while (hours > 12)
  {
    hours -= 12;
  }

  remaining -= append_number(words, hours);
  remaining -= append_string(words, remaining, " ");
}

void time_to_3words(int hours, int minutes, char *line1, char *line2, char *line3, size_t length)
{
	char value[length];
	time_to_words(hours, minutes, value, length);
	
	memset(line1, 0, length);
	memset(line2, 0, length);
	memset(line3, 0, length);
	
	char *start = value;
	char *pch = strstr (start, " ");
	while (pch != NULL) {
		if (line1[0] == 0) {
			memcpy(line1, start, pch-start);
		}  else if (line2[0] == 0) {
			memcpy(line2, start, pch-start);
		} else if (line3[0] == 0) {
			memcpy(line3, start, pch-start);
		}
		start += pch-start+1;
		pch = strstr(start, " ");
	}
	
}