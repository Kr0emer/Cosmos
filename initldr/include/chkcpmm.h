#ifndef _CHKCPMM_H
#define _CHKCPMM_H
#define CLI_HALT() __asm__ __volatile__("cli; hlt": : :"memory")


void init_chkcpu(machbstart_t *mbsp);
void init_mem(machbstart_t *mbsp);

void init_krlinitstack(machbstart_t *mbsp);
e820map_t *chk_memsize(e820map_t *e8p, u32_t enr, u64_t sadr, u64_t size);

#endif 
