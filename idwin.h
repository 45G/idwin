#ifndef IDWIN_H
#define IDWIN_H

#include <stdint.h>

struct idwin
{
	uint32_t winsize;
	uint32_t lo_wat;
	uint32_t hi_wat;
	
	uint32_t base;
	uint32_t offset;
	
	uint8_t bitmap[0];
};

enum idwin_result
{
	IDWIN_RESULT_NEW,
	IDWIN_RESULT_DUP,
	INDWI_RESULT_NOTINWIN,
};

struct idwin *idwin_init(uint32_t winsize, uint32_t lo_wat, uint32_t hi_wat, uint32_t base);

enum idwin_result idwin_hit(struct idwin *ctx, uint32_t num);

void idwin_destroy(struct idwin *ctx);


#endif // IDWIN_H
