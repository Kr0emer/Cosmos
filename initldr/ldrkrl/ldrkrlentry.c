#include "cmctl.h"
void ldrkrl_entry()
{
    init_curs();
    close_curs();
    clear_screen(VGADP_DFVL);
    init_bstartparm();
    return;
}


void kerror(char_t* kestr)
{
    kprint("INITKLDR DIE ERROR:%s\n",kestr);
    for(;;);
    return;
}