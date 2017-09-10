/* gunzip for emerald mine caves */

#define DICSIZE 32768
#define STRSIZE 288

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

struct unzip {
	int state;

/* strings */

	uchar dictionary[DICSIZE + STRSIZE * 3];

	uchar *dicptr;
	uchar *strptr;

	ulong size;
	ulong blocksize;

	uint blocktype;

/* huffman */

	ulong huf_len[2048];
	ulong huf_ptr[512];
	uchar huf_bit[320];

	uint len_size;
	uint ptr_size;
	uint hdr_size;

	uint rep;

/* bitstream */

	ulong code;
	uint shift;

	ulong huf, len, ptr;
	uint bit;

/* metadata */

#ifndef NO_META
	ulong crc;
	uchar *crcptr;

	uchar meta[10];
	uint cnt;
#endif
};

#include <string.h>

/* static huffman */

#define INDIRECT 0x80000000

static uint rev(uint a)
{
	a = (a & 0x00ff) << 8 | (a >> 8 & 0x00ff);
	a = (a & 0x0f0f) << 4 | (a >> 4 & 0x0f0f);
	a = (a & 0x3333) << 2 | (a >> 2 & 0x3333);
	a = (a & 0x5555) << 1 | (a >> 1 & 0x5555);
	return(a);
}

static const char *table(ulong *table, uint table_size, uchar *length, uint size, uint pow, char reverse)
{
	ulong avbl = 1, code = 0, table_alloc = 1 << pow;

	uint cnt, bit;

/* radix sort the lengths */

	uint radix[33] = { 0 };
	ushort sort[288];
	cnt = 0;
	for(bit = 0; bit < size; bit++) { radix[length[bit]]++; }
	for(bit = 0; bit <= 32; bit++) { uint sum = radix[bit]; radix[bit] = cnt; cnt += sum; }
	for(bit = 0; bit < size; bit++) { sort[radix[length[bit]]++] = bit; }

	cnt = 0;
	for(bit = 0; bit < 32; bit++) {
		for(; cnt < radix[bit]; cnt++) {
			ulong huf = (sort[cnt] << 8) + bit; /* pack the code and length into a single word */
			uint base = 0;

/* make indirect hash tables */

			ulong c = code;
			uint b = bit, p = pow;
			while(b > p) {
				b -= p;
				if(reverse) { base += rev(c >> b) >> (16 - p); } else { base += c >> b; }
				c &= (1 << b) - 1;
				p = 2;
				if(c == 0) {
					table[base] = table_alloc + INDIRECT;
					table_alloc += 1 << p;
					if(table_alloc > table_size) return "*** BUG: huffman table";
				}
				base = table[base] - INDIRECT;
			}
			p -= b;
			if(reverse) { base += rev(c) >> (16 - b); } else { base += c << p; }

/* fill in hash tables */

			if(reverse) {
				uint inc = 1 << b;
				uint rep = 1 << p;
				while(rep > 0) { table[base] = huf; base += inc; rep--; }
			} else {
				uint rep = 1 << p;
				while(rep > 0) { table[base++] = huf; rep--; }
			}

			if(avbl == 0) return "huffman tree overflow";
			avbl--; code++;
		}
		avbl *= 2; code *= 2;
	}
	if(avbl != 0) return "huffman tree underflow";

	return(0);
}

/* crc-32-ieee 802.3 */

static ulong crc_table[256];

static void crc_calc(ulong *indcrc, uchar *in, uint size)
{
	ulong crc = *indcrc;
	crc = ~crc & 0xffffffff;
	while(size > 0) {
		crc = crc_table[*in++ ^ (crc & 255)] ^ (crc >> 8);
		size--;
	}
	crc = ~crc & 0xffffffff;
	*indcrc = crc;
}

static void crc_init(void)
{
static char build;
if(build) return;
{
	uint a, b;
	for(a = 0; a < 256; a++) {
		ulong crc = a;
		for(b = 0; b < 8; b++) {
			crc = ((crc & 1) == 0 ? 0 : 0xedb88320) ^ (crc >> 1);
		}
		crc_table[a] = crc;
	}
}
build = 1;
}

static const uchar hdr_order[19] = { 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15, };

#define IN(st) { case st: if(in == inend) { if(inend != 0) { S->state = st; goto ret; } else return "end of file"; } }
#define OUT(st) { case st: if(out == outend) { S->state = st; goto ret; } }

#define RPEEK(st) { while(S->shift <= 24) { IN(st); S->code += *in++ << S->shift; S->shift += 8; } }
#define RSHIFT(x) { S->code >>= (x); S->shift -= (x); }

const char *unzip(struct unzip *S, uchar **indin, uchar **indout, uchar *inend, uchar *outend)
{
	uchar *in = *indin;
	uchar *out = *indout;

	if(S == 0) return "failed to allocate state";

switch(S->state) {
default: if(in == inend) break;

	S->dicptr = S->dictionary;
	S->size = 0;

#ifndef NO_META
	S->crc = 0;
	S->crcptr = S->dicptr;

	for(S->cnt = 0; S->cnt < 10; S->cnt++) {
		IN(13); S->meta[S->cnt] = *in++;
	}
	if(S->meta[0] != 31 || S->meta[1] != 139 || S->meta[2] != 8) return "not a deflated gzip file";

	if(S->meta[3] & 4) { /* FEXTRA */
		for(S->cnt = 0; S->cnt < 2; S->cnt++) {
			IN(14); S->meta[S->cnt] = *in++;
		}
		for(S->cnt = S->meta[0] | S->meta[1] << 8; S->cnt > 0; S->cnt--) {
			IN(15); S->meta[0] = *in++;
		}
	}
	if(S->meta[3] & 8) { /* FNAME */
		for(S->cnt = 4095; S->cnt > 0; S->cnt--) {
			IN(16); S->meta[0] = *in++;
			if(S->meta[0] == 0) break;
		}
		if(S->cnt == 0) return "name too long";
	}
	if(S->meta[3] & 16) { /* FCOMMENT */
		for(S->cnt = 65535; S->cnt > 0; S->cnt--) {
			IN(17); S->meta[0] = *in++;
			if(S->meta[0] == 0) break;
		}
		if(S->cnt == 0) return "comment too long";
	}
	if(S->meta[3] & 2) { /* FHCRC */
		for(S->cnt = 0; S->cnt < 2; S->cnt++) {
			IN(18); S->meta[S->cnt] = *in++;
		}
	}
	if(S->meta[3] & 1) { /* FTEXT */
	}
#endif

	S->code = 0; S->shift = 0;

	S->blocktype = 0;
	while((S->blocktype & 1) == 0) {
		RPEEK(1);
		S->blocktype = S->code & 7; RSHIFT(3);

		if((S->blocktype & 6) == 4) { /* dynamic */

			S->len_size = (S->code & 31) + 257; RSHIFT(5);
			S->ptr_size = (S->code & 31) + 1; RSHIFT(5);
			S->hdr_size = (S->code & 15) + 4; RSHIFT(4);

			memset(S->huf_bit, 32, 19);
			for(S->len = 0; S->len < S->hdr_size; S->len++) {
				RPEEK(2);
				S->rep = S->code & 7; RSHIFT(3);
				if(S->rep) S->huf_bit[hdr_order[S->len]] = S->rep;
			}
			if(table(S->huf_len, 2048, S->huf_bit, 19, 7, 1)) return "bad header tree";

			memset(S->huf_bit, 32, 320);
			S->rep = 32;
			for(S->len = 0; S->len < S->len_size + S->ptr_size; ) {
				RPEEK(3); S->huf = S->huf_len[S->code & 127];
				S->bit = S->huf & 255; RSHIFT(S->bit); S->huf >>= 8;

				switch(S->huf) {
				case 18: S->ptr = (S->code & 127) + 11; RSHIFT(7); S->rep = 0; break; /* zero 11-138 */
				case 17: S->ptr = (S->code & 7) + 3; RSHIFT(3); S->rep = 0; break; /* zero 3-10 */
				case 16: S->ptr = (S->code & 3) + 3; RSHIFT(2); break; /* repeat 3-6 */
				default: S->ptr = 1; S->rep = S->huf; break;
				}

				if(S->rep > 15) return "bad tree code";
				if(S->len + S->ptr > S->len_size + S->ptr_size) return "bad tree length";
				for(; S->ptr > 0; S->ptr--) {
					if(S->rep) S->huf_bit[S->len] = S->rep;
					S->len++;
				}
			}

			if(S->ptr_size == 1) S->huf_bit[S->len_size] = 0; /* special case */ /* no support for incomplete codes rubbish */

			if(table(S->huf_len, 2048, S->huf_bit, S->len_size, 10, 1)) return "bad character tree";
			if(table(S->huf_ptr, 512, S->huf_bit + S->len_size, S->ptr_size, 8, 1)) return "bad string tree";

			S->blocksize = -1;

		} else if((S->blocktype & 6) == 2) { /* fixed */

			memset(S->huf_bit + 0, 8, 144);
			memset(S->huf_bit + 144, 9, 112);
			memset(S->huf_bit + 256, 7, 24);
			memset(S->huf_bit + 280, 8, 8);
			memset(S->huf_bit + 288, 5, 32);

			if(table(S->huf_len, 2048, S->huf_bit, 288, 10, 1)) return "*** BUG: fixed character tree";
			if(table(S->huf_ptr, 512, S->huf_bit + 288, 32, 8, 1)) return "*** BUG: fixed string tree";

			S->blocksize = -1;

		} else if((S->blocktype & 6) == 0) { /* stored */

			S->bit = S->shift & 7; RSHIFT(S->bit);

			for(S->len = 0; S->len < 256; S->len++) {
				S->huf = (S->len << 8) + 8;
				for(S->ptr = S->len; S->ptr < 1024; S->ptr += 256) S->huf_len[S->ptr] = S->huf;
			}

			RPEEK(4);
			S->huf = S->code & 65535; RSHIFT(16);
			RSHIFT(16);

			S->blocksize = S->size + S->huf;

		} else return "bad block type";

/* read block */

		while(S->size < S->blocksize) {

			RPEEK(5); S->huf = S->huf_len[S->code & 1023];
			if(S->huf >= INDIRECT) {
				S->huf = S->huf_len[S->huf + (S->code >> 10 & 3) - INDIRECT];
				if(S->huf >= INDIRECT) {
					S->huf = S->huf_len[S->huf + (S->code >> 12 & 3) - INDIRECT];
					if(S->huf >= INDIRECT) {
						S->huf = S->huf_len[S->huf + (S->code >> 14 & 3) - INDIRECT];
					}
				}
			}
			S->bit = S->huf & 255; RSHIFT(S->bit); S->huf >>= 8;

			if(S->huf > 256) {

				S->len = S->huf - 257;
				if(S->len >= 8) {
					if(S->len < 28) {
						RPEEK(6);
						S->bit = (S->len >> 2) - 1;
						S->len = (4 + (S->len & 3)) << S->bit;
						S->len += S->code & ((1 << S->bit) - 1); RSHIFT(S->bit);
					} else if(S->len == 28) {
						S->len = 255;
					} else return "bad string length code";
				}
				S->len += 3;

				RPEEK(7); S->huf = S->huf_ptr[S->code & 255];
				if(S->huf >= INDIRECT) {
					S->huf = S->huf_ptr[S->huf + (S->code >> 8 & 3) - INDIRECT];
					if(S->huf >= INDIRECT) {
						S->huf = S->huf_ptr[S->huf + (S->code >> 10 & 3) - INDIRECT];
						if(S->huf >= INDIRECT) {
							S->huf = S->huf_ptr[S->huf + (S->code >> 12 & 3) - INDIRECT];
							if(S->huf >= INDIRECT) {
								S->huf = S->huf_ptr[S->huf + (S->code >> 14 & 3) - INDIRECT];
							}
						}
					}
				}
				S->bit = S->huf & 255; RSHIFT(S->bit); S->huf >>= 8;

				S->ptr = S->huf;
				if(S->ptr >= 4) {
					if(S->ptr < 30) {
						RPEEK(8);
						S->bit = (S->ptr >> 1) - 1;
						S->ptr = (2 + (S->ptr & 1)) << S->bit;
						S->ptr += S->code & ((1 << S->bit) - 1); RSHIFT(S->bit);
					} else return "bad string pointer code";
				}
				S->ptr += 1;

				if(S->ptr > S->size) return "bad string pointer";
				S->strptr = S->dicptr - S->ptr + (S->dicptr < S->dictionary + S->ptr ? DICSIZE + STRSIZE : 0);

				S->size += S->len;
				for(; S->len > 0; S->len--) {
					OUT(9);
					S->huf = *S->strptr++;
					*S->dicptr++ = S->huf;
					*out++ = S->huf;
				}
			} else if(S->huf < 256) {
				OUT(10);
				*S->dicptr++ = S->huf;
				*out++ = S->huf;
				S->size++;
			} else {
				S->blocksize = S->size;
			}

			if(S->dicptr >= S->dictionary + DICSIZE + STRSIZE * 2) {
#ifndef NO_META
				crc_calc(&S->crc, S->crcptr, S->dictionary + DICSIZE + STRSIZE - S->crcptr);
				S->crcptr = S->dictionary;
#endif
				memcpy(S->dictionary, S->dictionary + DICSIZE + STRSIZE, STRSIZE * 2);
				S->dicptr -= DICSIZE + STRSIZE;
			}
		}
	}

	S->bit = S->shift & 7; RSHIFT(S->bit);
	RPEEK(11);

#ifndef NO_META
	crc_calc(&S->crc, S->crcptr, S->dicptr - S->crcptr);

	if(S->code != S->crc) return "bad checksum";

	for(S->cnt = 0; S->cnt < 4; S->cnt++) {
		IN(12); S->meta[S->cnt] = *in++;
	}
	S->code = S->meta[0] | S->meta[1] << 8 | S->meta[2] << 16 | S->meta[3] << 24;

	S->crc = S->size & 0xffffffff;
	if(S->code != S->crc) return "bad file length";
#endif
}
S->state = 0;

ret:
	*indin = in;
	*indout = out;
	return(0);
}

static struct unzip malloc_unzip;
void unzip_init(struct unzip **indS) { if(*indS == 0) (*indS) = &malloc_unzip; if(*indS) (*indS)->state = 0; crc_init(); }
void unzip_free(struct unzip **indS) { (*indS) = 0; }
