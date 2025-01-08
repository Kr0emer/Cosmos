#ifndef _INITHEAD_H
#define _INITHEAD_H

void inithead_entry();
void write_realintsvefile();
void write_ldrkrlfile();
fhdsc_t *find_file(char_t *fname);
int strcmpl(const char *a, const char *b);
void error(char_t *estr);
#endif //_INITHEAD_H