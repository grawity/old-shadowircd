001 /*      $KAME: md5.c,v 1.5 2000/11/08 06:13:08 itojun Exp $     */
002 /*
003  * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
004  * All rights reserved.
005  *
006  * Redistribution and use in source and binary forms, with or without
007  * modification, are permitted provided that the following conditions
008  * are met:
009  * 1. Redistributions of source code must retain the above copyright
010  *    notice, this list of conditions and the following disclaimer.
011  * 2. Redistributions in binary form must reproduce the above copyright
 12  *    notice, this list of conditions and the following disclaimer in the
 13  *    documentation and/or other materials provided with the distribution.
 14  * 3. Neither the name of the project nor the names of its contributors
 15  *    may be used to endorse or promote products derived from this software
 16  *    without specific prior written permission.
 17  *
 18  * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 19  * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 20  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 21  * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 22  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 23  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 24  * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 25  * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 26  * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 27  * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 28  * SUCH DAMAGE.
 29  */
 30 
 31 #include <sys/cdefs.h>
 32 __FBSDID("$FreeBSD: src/sys/crypto/md5.c,v 1.7 2003/06/10 21:36:57 obrien Exp $");
 33 
 34 #include <sys/types.h>
 35 #include <sys/cdefs.h>
 36 #include <sys/time.h>
 37 #include <sys/systm.h>
 38 #include <crypto/md5.h>
 39 
 40 #define SHIFT(X, s) (((X) << (s)) | ((X) >> (32 - (s))))
 41 
 42 #define F(X, Y, Z) (((X) & (Y)) | ((~X) & (Z)))
 43 #define G(X, Y, Z) (((X) & (Z)) | ((Y) & (~Z)))
 44 #define H(X, Y, Z) ((X) ^ (Y) ^ (Z))
 45 #define I(X, Y, Z) ((Y) ^ ((X) | (~Z)))
 46 
 47 #define ROUND1(a, b, c, d, k, s, i) { \
 48         (a) = (a) + F((b), (c), (d)) + X[(k)] + T[(i)]; \
 49         (a) = SHIFT((a), (s)); \
 50         (a) = (b) + (a); \
 51 }
 52 
 53 #define ROUND2(a, b, c, d, k, s, i) { \
 54         (a) = (a) + G((b), (c), (d)) + X[(k)] + T[(i)]; \
 55         (a) = SHIFT((a), (s)); \
 56         (a) = (b) + (a); \
 57 }
 58 
 59 #define ROUND3(a, b, c, d, k, s, i) { \
 60         (a) = (a) + H((b), (c), (d)) + X[(k)] + T[(i)]; \
 61         (a) = SHIFT((a), (s)); \
 62         (a) = (b) + (a); \
 63 }
 64 
 65 #define ROUND4(a, b, c, d, k, s, i) { \
 66         (a) = (a) + I((b), (c), (d)) + X[(k)] + T[(i)]; \
 67         (a) = SHIFT((a), (s)); \
 68         (a) = (b) + (a); \
 69 }
 70 
 71 #define Sa       7
 72 #define Sb      12
 73 #define Sc      17
 74 #define Sd      22
 75 
 76 #define Se       5
 77 #define Sf       9
 78 #define Sg      14
 79 #define Sh      20
 80 
 81 #define Si       4
 82 #define Sj      11
 83 #define Sk      16
 84 #define Sl      23
 85 
 86 #define Sm       6
 87 #define Sn      10
 88 #define So      15
 89 #define Sp      21
 90 
 91 #define MD5_A0  0x67452301
 92 #define MD5_B0  0xefcdab89
 93 #define MD5_C0  0x98badcfe
 94 #define MD5_D0  0x10325476
 95 
 96 /* Integer part of 4294967296 times abs(sin(i)), where i is in radians. */
 97 static const u_int32_t T[65] = {
 98         0,
 99         0xd76aa478,     0xe8c7b756,     0x242070db,     0xc1bdceee,
100         0xf57c0faf,     0x4787c62a,     0xa8304613,     0xfd469501,
101         0x698098d8,     0x8b44f7af,     0xffff5bb1,     0x895cd7be,
102         0x6b901122,     0xfd987193,     0xa679438e,     0x49b40821,
103 
104         0xf61e2562,     0xc040b340,     0x265e5a51,     0xe9b6c7aa,
105         0xd62f105d,     0x2441453,      0xd8a1e681,     0xe7d3fbc8,
106         0x21e1cde6,     0xc33707d6,     0xf4d50d87,     0x455a14ed,
107         0xa9e3e905,     0xfcefa3f8,     0x676f02d9,     0x8d2a4c8a,
108 
109         0xfffa3942,     0x8771f681,     0x6d9d6122,     0xfde5380c,
110         0xa4beea44,     0x4bdecfa9,     0xf6bb4b60,     0xbebfbc70,
111         0x289b7ec6,     0xeaa127fa,     0xd4ef3085,     0x4881d05,
112         0xd9d4d039,     0xe6db99e5,     0x1fa27cf8,     0xc4ac5665,
113 
114         0xf4292244,     0x432aff97,     0xab9423a7,     0xfc93a039,
115         0x655b59c3,     0x8f0ccc92,     0xffeff47d,     0x85845dd1,
116         0x6fa87e4f,     0xfe2ce6e0,     0xa3014314,     0x4e0811a1,
117         0xf7537e82,     0xbd3af235,     0x2ad7d2bb,     0xeb86d391,
118 };
119 
120 static const u_int8_t md5_paddat[MD5_BUFLEN] = {
121         0x80,   0,      0,      0,      0,      0,      0,      0,
122         0,      0,      0,      0,      0,      0,      0,      0,
123         0,      0,      0,      0,      0,      0,      0,      0,
124         0,      0,      0,      0,      0,      0,      0,      0,
125         0,      0,      0,      0,      0,      0,      0,      0,
126         0,      0,      0,      0,      0,      0,      0,      0,
127         0,      0,      0,      0,      0,      0,      0,      0,
128         0,      0,      0,      0,      0,      0,      0,      0,      
129 };
130 
131 static void md5_calc(u_int8_t *, md5_ctxt *);
132 
133 void md5_init(ctxt)
134         md5_ctxt *ctxt;
135 {
136         ctxt->md5_n = 0;
137         ctxt->md5_i = 0;
138         ctxt->md5_sta = MD5_A0;
139         ctxt->md5_stb = MD5_B0;
140         ctxt->md5_stc = MD5_C0;
141         ctxt->md5_std = MD5_D0;
142         bzero(ctxt->md5_buf, sizeof(ctxt->md5_buf));
143 }
144 
145 void md5_loop(ctxt, input, len)
146         md5_ctxt *ctxt;
147         u_int8_t *input;
148         u_int len; /* number of bytes */
149 {
150         u_int gap, i;
151 
152         ctxt->md5_n += len * 8; /* byte to bit */
153         gap = MD5_BUFLEN - ctxt->md5_i;
154 
155         if (len >= gap) {
156                 bcopy((void *)input, (void *)(ctxt->md5_buf + ctxt->md5_i),
157                         gap);
158                 md5_calc(ctxt->md5_buf, ctxt);
159 
160                 for (i = gap; i + MD5_BUFLEN <= len; i += MD5_BUFLEN) {
161                         md5_calc((u_int8_t *)(input + i), ctxt);
162                 }
163                 
164                 ctxt->md5_i = len - i;
165                 bcopy((void *)(input + i), (void *)ctxt->md5_buf, ctxt->md5_i);
166         } else {
167                 bcopy((void *)input, (void *)(ctxt->md5_buf + ctxt->md5_i),
168                         len);
169                 ctxt->md5_i += len;
170         }
171 }
172 
173 void md5_pad(ctxt)
174         md5_ctxt *ctxt;
175 {
176         u_int gap;
177 
178         /* Don't count up padding. Keep md5_n. */       
179         gap = MD5_BUFLEN - ctxt->md5_i;
180         if (gap > 8) {
181                 bcopy(md5_paddat,
182                       (void *)(ctxt->md5_buf + ctxt->md5_i),
183                       gap - sizeof(ctxt->md5_n));
184         } else {
185                 /* including gap == 8 */
186                 bcopy(md5_paddat, (void *)(ctxt->md5_buf + ctxt->md5_i),
187                         gap);
188                 md5_calc(ctxt->md5_buf, ctxt);
189                 bcopy((md5_paddat + gap),
190                       (void *)ctxt->md5_buf,
191                       MD5_BUFLEN - sizeof(ctxt->md5_n));
192         }
193 
194         /* 8 byte word */       
195 #if BYTE_ORDER == LITTLE_ENDIAN
196         bcopy(&ctxt->md5_n8[0], &ctxt->md5_buf[56], 8);
197 #endif
198 #if BYTE_ORDER == BIG_ENDIAN
199         ctxt->md5_buf[56] = ctxt->md5_n8[7];
200         ctxt->md5_buf[57] = ctxt->md5_n8[6];
201         ctxt->md5_buf[58] = ctxt->md5_n8[5];
202         ctxt->md5_buf[59] = ctxt->md5_n8[4];
203         ctxt->md5_buf[60] = ctxt->md5_n8[3];
204         ctxt->md5_buf[61] = ctxt->md5_n8[2];
205         ctxt->md5_buf[62] = ctxt->md5_n8[1];
206         ctxt->md5_buf[63] = ctxt->md5_n8[0];
207 #endif
208 
209         md5_calc(ctxt->md5_buf, ctxt);
210 }
211 
212 void md5_result(digest, ctxt)
213         u_int8_t *digest;
214         md5_ctxt *ctxt;
215 {
216         /* 4 byte words */
217 #if BYTE_ORDER == LITTLE_ENDIAN
218         bcopy(&ctxt->md5_st8[0], digest, 16);
219 #endif
220 #if BYTE_ORDER == BIG_ENDIAN
221         digest[ 0] = ctxt->md5_st8[ 3]; digest[ 1] = ctxt->md5_st8[ 2];
222         digest[ 2] = ctxt->md5_st8[ 1]; digest[ 3] = ctxt->md5_st8[ 0];
223         digest[ 4] = ctxt->md5_st8[ 7]; digest[ 5] = ctxt->md5_st8[ 6];
224         digest[ 6] = ctxt->md5_st8[ 5]; digest[ 7] = ctxt->md5_st8[ 4];
225         digest[ 8] = ctxt->md5_st8[11]; digest[ 9] = ctxt->md5_st8[10];
226         digest[10] = ctxt->md5_st8[ 9]; digest[11] = ctxt->md5_st8[ 8];
227         digest[12] = ctxt->md5_st8[15]; digest[13] = ctxt->md5_st8[14];
228         digest[14] = ctxt->md5_st8[13]; digest[15] = ctxt->md5_st8[12];
229 #endif
230 }
231 
232 #if BYTE_ORDER == BIG_ENDIAN
233 u_int32_t X[16];
234 #endif
235 
236 static void md5_calc(b64, ctxt)
237         u_int8_t *b64;
238         md5_ctxt *ctxt;
239 {
240         u_int32_t A = ctxt->md5_sta;
241         u_int32_t B = ctxt->md5_stb;
242         u_int32_t C = ctxt->md5_stc;
243         u_int32_t D = ctxt->md5_std;
244 #if BYTE_ORDER == LITTLE_ENDIAN
245         u_int32_t *X = (u_int32_t *)b64;
246 #endif  
247 #if BYTE_ORDER == BIG_ENDIAN
248         /* 4 byte words */
249         /* what a brute force but fast! */
250         u_int8_t *y = (u_int8_t *)X;
251         y[ 0] = b64[ 3]; y[ 1] = b64[ 2]; y[ 2] = b64[ 1]; y[ 3] = b64[ 0];
252         y[ 4] = b64[ 7]; y[ 5] = b64[ 6]; y[ 6] = b64[ 5]; y[ 7] = b64[ 4];
253         y[ 8] = b64[11]; y[ 9] = b64[10]; y[10] = b64[ 9]; y[11] = b64[ 8];
254         y[12] = b64[15]; y[13] = b64[14]; y[14] = b64[13]; y[15] = b64[12];
255         y[16] = b64[19]; y[17] = b64[18]; y[18] = b64[17]; y[19] = b64[16];
256         y[20] = b64[23]; y[21] = b64[22]; y[22] = b64[21]; y[23] = b64[20];
257         y[24] = b64[27]; y[25] = b64[26]; y[26] = b64[25]; y[27] = b64[24];
258         y[28] = b64[31]; y[29] = b64[30]; y[30] = b64[29]; y[31] = b64[28];
259         y[32] = b64[35]; y[33] = b64[34]; y[34] = b64[33]; y[35] = b64[32];
260         y[36] = b64[39]; y[37] = b64[38]; y[38] = b64[37]; y[39] = b64[36];
261         y[40] = b64[43]; y[41] = b64[42]; y[42] = b64[41]; y[43] = b64[40];
262         y[44] = b64[47]; y[45] = b64[46]; y[46] = b64[45]; y[47] = b64[44];
263         y[48] = b64[51]; y[49] = b64[50]; y[50] = b64[49]; y[51] = b64[48];
264         y[52] = b64[55]; y[53] = b64[54]; y[54] = b64[53]; y[55] = b64[52];
265         y[56] = b64[59]; y[57] = b64[58]; y[58] = b64[57]; y[59] = b64[56];
266         y[60] = b64[63]; y[61] = b64[62]; y[62] = b64[61]; y[63] = b64[60];
267 #endif
268 
269         ROUND1(A, B, C, D,  0, Sa,  1); ROUND1(D, A, B, C,  1, Sb,  2);
270         ROUND1(C, D, A, B,  2, Sc,  3); ROUND1(B, C, D, A,  3, Sd,  4);
271         ROUND1(A, B, C, D,  4, Sa,  5); ROUND1(D, A, B, C,  5, Sb,  6);
272         ROUND1(C, D, A, B,  6, Sc,  7); ROUND1(B, C, D, A,  7, Sd,  8);
273         ROUND1(A, B, C, D,  8, Sa,  9); ROUND1(D, A, B, C,  9, Sb, 10);
274         ROUND1(C, D, A, B, 10, Sc, 11); ROUND1(B, C, D, A, 11, Sd, 12);
275         ROUND1(A, B, C, D, 12, Sa, 13); ROUND1(D, A, B, C, 13, Sb, 14);
276         ROUND1(C, D, A, B, 14, Sc, 15); ROUND1(B, C, D, A, 15, Sd, 16);
277         
278         ROUND2(A, B, C, D,  1, Se, 17); ROUND2(D, A, B, C,  6, Sf, 18);
279         ROUND2(C, D, A, B, 11, Sg, 19); ROUND2(B, C, D, A,  0, Sh, 20);
280         ROUND2(A, B, C, D,  5, Se, 21); ROUND2(D, A, B, C, 10, Sf, 22);
281         ROUND2(C, D, A, B, 15, Sg, 23); ROUND2(B, C, D, A,  4, Sh, 24);
282         ROUND2(A, B, C, D,  9, Se, 25); ROUND2(D, A, B, C, 14, Sf, 26);
283         ROUND2(C, D, A, B,  3, Sg, 27); ROUND2(B, C, D, A,  8, Sh, 28);
284         ROUND2(A, B, C, D, 13, Se, 29); ROUND2(D, A, B, C,  2, Sf, 30);
285         ROUND2(C, D, A, B,  7, Sg, 31); ROUND2(B, C, D, A, 12, Sh, 32);
286 
287         ROUND3(A, B, C, D,  5, Si, 33); ROUND3(D, A, B, C,  8, Sj, 34);
288         ROUND3(C, D, A, B, 11, Sk, 35); ROUND3(B, C, D, A, 14, Sl, 36);
289         ROUND3(A, B, C, D,  1, Si, 37); ROUND3(D, A, B, C,  4, Sj, 38);
290         ROUND3(C, D, A, B,  7, Sk, 39); ROUND3(B, C, D, A, 10, Sl, 40);
291         ROUND3(A, B, C, D, 13, Si, 41); ROUND3(D, A, B, C,  0, Sj, 42);
292         ROUND3(C, D, A, B,  3, Sk, 43); ROUND3(B, C, D, A,  6, Sl, 44);
293         ROUND3(A, B, C, D,  9, Si, 45); ROUND3(D, A, B, C, 12, Sj, 46);
294         ROUND3(C, D, A, B, 15, Sk, 47); ROUND3(B, C, D, A,  2, Sl, 48);
295         
296         ROUND4(A, B, C, D,  0, Sm, 49); ROUND4(D, A, B, C,  7, Sn, 50); 
297         ROUND4(C, D, A, B, 14, So, 51); ROUND4(B, C, D, A,  5, Sp, 52); 
298         ROUND4(A, B, C, D, 12, Sm, 53); ROUND4(D, A, B, C,  3, Sn, 54); 
299         ROUND4(C, D, A, B, 10, So, 55); ROUND4(B, C, D, A,  1, Sp, 56); 
300         ROUND4(A, B, C, D,  8, Sm, 57); ROUND4(D, A, B, C, 15, Sn, 58); 
301         ROUND4(C, D, A, B,  6, So, 59); ROUND4(B, C, D, A, 13, Sp, 60); 
302         ROUND4(A, B, C, D,  4, Sm, 61); ROUND4(D, A, B, C, 11, Sn, 62); 
            ROUND4(C, D, A, B,  2, So, 63); ROUND4(B, C, D, A,  9, Sp, 64);
   
            ctxt->md5_sta += A;
            ctxt->md5_stb += B;
            ctxt->md5_stc += C;
            ctxt->md5_std += D;
    }
