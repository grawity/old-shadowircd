#if defined(USE_SSL)
#include <openssl/md5.h>
#elif !defined(_MD5_H)
#define _MD5_H

/* Any 32-bit or wider unsigned integer data type will do */
typedef unsigned long My_MD5_u32plus;

typedef struct {
	My_MD5_u32plus lo, hi;
	My_MD5_u32plus a, b, c, d;
	unsigned char buffer[64];
	My_MD5_u32plus block[16];
} My_MD5_CTX;

extern void My_MD5_Init(My_MD5_CTX *ctx);
extern void My_MD5_Update(My_MD5_CTX *ctx, void *data, unsigned long size);
extern void My_MD5_Final(unsigned char *result, My_MD5_CTX *ctx);

#endif
