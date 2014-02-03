#include "num2words.h"
#include "strings-en.h"
#include "strings-sv.h"
#include "string.h"

size_t min(const size_t a, const size_t b) {
  return a < b ? a : b;
}

static size_t append_string(char* buffer, const size_t length, const char* str) {
  strncat(buffer, str, length);

  size_t written = strlen(str);
  return (length > written) ? written : length;
}

static size_t interpolate_and_append(char* buffer, const size_t length,
    const char* parent_str, const char* placeholder_str) {
  char *insert_ptr = strstr(parent_str, "%");
  size_t parent_len = strlen(parent_str);
  size_t insert_offset = insert_ptr ? (size_t) insert_ptr - (size_t) parent_str : parent_len;

  size_t remaining = length;

  remaining -= append_string(buffer, min(insert_offset, remaining), parent_str);
  remaining -= append_string(buffer, remaining, placeholder_str);
  if (insert_ptr) {
    remaining -= append_string(buffer, remaining, insert_ptr + 1);
  }

  return remaining;
}

void time_to_words(Language lang, int hours, int minutes, int seconds, char* words, size_t buffer_size) {

  size_t remaining = buffer_size;
  memset(words, 0, buffer_size);

  // We want to operate with a resolution of 30 seconds.  So multiply
  // minutes and seconds by 2.  Then divide by (2 * 5) to carve the hour
  // into five minute intervals.
  int half_mins  = (2 * minutes) + (seconds / 30);
  int rel_index  = ((half_mins + 5) / (2 * 5)) % 12;
  int hour_index;

  if (rel_index > 6 || (rel_index == 0 && minutes > 30)) {
    hour_index = (hours + 1) % 24;
  }
  else {
    hour_index = hours % 24;
  }

  const char* hour = lang == EN ? HOURS_EN[hour_index] : HOURS_SV[hour_index];
  const char* rel  = lang == EN ? RELS_EN[rel_index]   : RELS_SV[rel_index];

  remaining -= interpolate_and_append(words, remaining, rel, hour);

  // Leave one space at the end
  remaining -= append_string(words, remaining, " ");

}
