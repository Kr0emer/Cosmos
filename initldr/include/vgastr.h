#ifndef _VGASTR_H
#define _VGASTR_H

void kprint(const char_t *fmt, ...);
char_t *numberk(char_t *str, uint_t n, sint_t base);
char_t *strcopy(char_t *buf, char_t *str_s);

#endif