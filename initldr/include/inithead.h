#ifndef _INITHEAD_H
#define _INITHEAD_H

void inithead_entry();
void write_realintsvefile();
void write_ldrkrlfile();
fhdsc_t *find_file(char_t *fname);
sint_t strcmpl(const char *str1, const char *str2);
void error(char_t *estr);
#endif //_INITHEAD_H