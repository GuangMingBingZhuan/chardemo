#ifndef _CHARDEMO_H_
#define _CHARDEMO_H_

#include <uapi/asm-generic/ioctl.h>

#define CHARDEMO_MAGIC 239U
#define CHARDEMO_MEM_CLEAR _IO(CHARDEMO_MAGIC, 0)

#endif
