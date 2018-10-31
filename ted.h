#include "linked-list.h"
#include "fileops.h"
//#include "str-utils/str-utils.h"

#define HLINES 80
#define VLINES 24
#define LF_FLAG HLINES + 1
#define DECREMENT(x) (x - 1 < 0 ? 0 : x - 1)
#define INCREMENT(x, max) (x > max ? max : x + 1) //increment value with max threshold

typedef node_t line_t;
