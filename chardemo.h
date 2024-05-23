#ifndef _CHARDEMO_H_
#define _CHARDEMO_H_

#define _IO(type, nr) (((0U) << (((0 + 8) + 8) + 14)) | ((type) << (0 + 8)) | ((nr) << 0) | ((0) << ((0 + 8) + 8)))

#define CHARDEMO_MAGIC 239U
#define CHARDEMO_MEM_CLEAR _IO(CHARDEMO_MAGIC, 0)

#endif
