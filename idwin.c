#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "idwin.h"

#define BITS_PER_BYTE 8

struct idwin *idwin_init(uint32_t winsize, uint32_t lo_wat, uint32_t hi_wat, uint32_t base)
{
	if (
			winsize % BITS_PER_BYTE != 0 ||
			winsize == 0 ||
			hi_wat >= winsize ||
			lo_wat >= hi_wat)
	{
		errno = EINVAL;
		return NULL;
	}
	
	size_t bitmap_size = winsize / BITS_PER_BYTE;
	
	struct idwin *ret = malloc(sizeof(struct idwin) + bitmap_size);
	if (!ret)
	{
		errno = ENOMEM;
		return NULL;
	}
	
	ret->winsize = winsize,
	ret->lo_wat = lo_wat,
	ret->hi_wat = hi_wat,
	ret->base = base,
	ret->offset = 0,
	memset(ret->bitmap, 0, bitmap_size);
	
	return ret;
}

static int idwin_in_window(struct idwin *ctx, uint32_t num)
{
	uint32_t head = ctx->base;
	uint32_t tail = head + ctx->winsize;
	int overflow = tail < head;
	
	if (overflow)
		return (num >= head || num < tail);
	else
		return (num >= head && num < tail);
}

void bitmap_zero(uint8_t *bmp, uint32_t size, uint32_t offset, uint32_t count)
{
	if (offset % BITS_PER_BYTE)
	{
		uint32_t byte = offset / BITS_PER_BYTE;
		uint32_t bit_offset = offset % BITS_PER_BYTE;
		uint32_t max_bit_count = BITS_PER_BYTE - bit_offset;
		uint32_t bit_count = count > max_bit_count ? max_bit_count : count;
		uint8_t mask = (1 << bit_offset) - 1;
		
		bmp[byte] &= mask;
		offset = (offset + bit_count) % size;
		count -= bit_count;
	}
	
	while (count % BITS_PER_BYTE)
	{
		uint32_t byte = offset / BITS_PER_BYTE;
		
		bmp[byte] = 0;
		offset = (offset + BITS_PER_BYTE) % size;
		count -= BITS_PER_BYTE;
	}
	
	if (count)
	{
		uint32_t byte = offset / BITS_PER_BYTE;
		uint8_t mask = ~(1 << count);
		
		bmp[byte] &= mask;
	}
}

static int bitmap_set_bit(uint8_t *bmp, uint32_t num)
{
	uint32_t byte = num / BITS_PER_BYTE;
	uint32_t bit = num % BITS_PER_BYTE;
	uint8_t mask = 1 << bit;
	int ret = (bmp[byte] & mask) == 0;
	
	bmp[byte] &= mask;
	
	return ret;
}

static void idwin_enforce_wat(struct idwin *ctx, uint32_t num)
{
	if (num - ctx->base < ctx->hi_wat)
		return;
	
	uint32_t shift = num - ctx->base - ctx->lo_wat;
	
	bitmap_zero(ctx->bitmap, ctx->winsize, ctx->offset, shift);
	ctx->base += shift;
	ctx->offset = ((uint64_t)ctx->offset + shift) % ctx->winsize;
}

enum idwin_result idwin_hit(struct idwin *ctx, uint32_t num)
{
	if (!idwin_in_window(ctx, num))
		return INDWI_RESULT_NOTINWIN;
	
	idwin_enforce_wat(ctx, num);
	
	if (bitmap_set_bit(ctx->bitmap, (num - ctx->base + ctx->offset) % ctx->winsize))
		return IDWIN_RESULT_NEW;
	
	return IDWIN_RESULT_DUP;
}

void idwin_destroy(struct idwin *ctx)
{
	free(ctx);
}
