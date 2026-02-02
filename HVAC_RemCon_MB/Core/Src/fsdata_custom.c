//-- fsdata_custom.c --//
#include "lwip/def.h"
#include "fs.h"

#define file_NULL (struct fsdata_file *) NULL

/* No actual files defined → FS_ROOT effectively NULL */
const struct fsdata_file *FS_ROOT = file_NULL;
