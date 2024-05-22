#ifndef _FS_H
#define _FS_H
u64_t get_wt_imgfilesz(machbstart_t *mbsp);
int move_krlimg(machbstart_t *mbsp, u64_t cpyadr, u64_t cpysz);
#endif // FS_H
