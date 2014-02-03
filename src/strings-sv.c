#include "strings-sv.h"

const char* const HOURS_SV[] = {
  // AM hours
  "tolv",
  "ett",
  "två",
  "tre",
  "fyra",
  "fem",
  "sex",
  "sju",
  "åtta",
  "nio",
  "tio",
  "elva",

  // PM hours
  "tolv",
  "ett",
  "två",
  "tre",
  "fyra",
  "fem",
  "sex",
  "sju",
  "åtta",
  "nio",
  "tio",
  "elva"
};

// The "%" character will be replaced with the hour.  A "*" before
// a word makes it bold.
const char* const RELS_SV[] = {
  "%",
  "fem över *%",
  "tio över *%",
  "kvart över *%",
  "tjugo över *%",
  "tjugo fem över *%",
  "halv över *%",
  "tjugo fem i *%",
  "tjugo i *%",
  "kvart i *%",
  "tio i *%",
  "fem i *%"
};
