#ifndef _BSTARTPARM_H
#define _BSTARTPARM_H

void init_bstartparm();
void machbstart_t_init(machbstart_t* initp);
int adrzone_is_ok(u64_t sadr,u64_t slen,u64_t kadr,u64_t klen);

#endif // BSTARTPARM_H