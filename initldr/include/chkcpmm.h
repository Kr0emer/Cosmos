#ifndef _CHKCPMM_H
#define _CHKCPMM_H
#define CLI_HALT() __asm__ __volatile__("cli; hlt": : :"memory")
u32_t acpi_get_bios_ebda();
void mmap(e820map_t **retemp, u32_t *retemnr);
int chk_cpuid();
int chk_cpu_longmode();
void init_chkcpu(machbstart_t *mbsp);
void init_krlinitstack(machbstart_t *mbsp);
void init_mem(machbstart_t *mbsp);
e820map_t *chk_memsize(e820map_t *e8p, u32_t enr, u64_t sadr, u64_t size);
u64_t get_memsize(e820map_t *e8p, u32_t enr);
PUBLIC void init_acpi(machbstart_t *mbsp);
PUBLIC mrsdp_t *find_acpi_rsdp();
mrsdp_t *findacpi_rsdp_core(void *findstart, u32_t findlen);
mrsdp_t *acpi_rsdp_isok(mrsdp_t *rdp);
int acpi_checksum(unsigned char *ap, s32_t len);
void init_meme820(machbstart_t *mbsp);
void init_bstartpages(machbstart_t *mbsp);
void init_chkmm();
void out_char(char *c);
void ldr_createpage_and_open();
#endif 
