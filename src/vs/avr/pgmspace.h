#pragma once

#define PROGMEM

#define pgm_read_byte(a)	*(a)
#define pgm_read_word(a)	*((size_t*)a)