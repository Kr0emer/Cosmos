#include "cmctl.h"
void ldrkrl_entry()
{
    init_bstartparm();
    return;
}


void kerror(char_t* kestr)
{
    kprint("INITKLDR DIE ERROR:%s\n",kestr);
    for(;;);
    return;
}