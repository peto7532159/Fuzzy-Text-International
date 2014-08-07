#include "strings-en_GB.h"

const char* const HOURS_EN_GB[] = {
  // AM hours
  "dvanásť",
  "jedna",
  "dve",
  "tri",
  "štyri",
  "päť",
  "šesť",
  "sedem",
  "osem",
  "deväť",
  "desať",
  "jedenásť",

  // PM hours
  "dvanásť",
  "jedna",
  "dve",
  "tri",
  "štyri",
  "päť",
  "šesť",
  "sedem",
  "osem",
  "deväť",
  "desať",
  "jedenásť",
};

/**
 * The string "$1" will be replaced with the current hour (e.g., "three"
 * at 3:45).  The string "$2" will be replaced with the *next* hour
 * (e.g., "four" at 3:45).
 *
 * A "*" character before a word makes that word bold.
 */
const char* const RELS_EN_GB[] = {
  "*$1 hodín",
  "*$1 päť",
  "*$1 desať",
  "štvrť na *$2",
  "*$1 dvadsať",
  "*$1 dvadsaťpäť",
  "pol *$2",
  "*$1 tridsaťpäť",
  "*$1 štyridsať",
  "trištvrte na *$2",
  "*$1 päťdesiat",
  "*$1 päťdesiatpäť"
};

