#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>

void print_uid(char *str, int ret)
{
    uid_t ruid;
    uid_t euid;
    uid_t suid;
    getresuid(&ruid, &euid, &suid);
    
    printf("%s ret:%d\n"
           "Real:%4d  Effective:%4d  Saved:%4d\n",
           str, ret, ruid, euid, suid);
}

int main(void)
{
    int ret = 0;

    print_uid("init", ret);            /* Real:1000  Effective:   0  Saved:   0 */

    ret = seteuid(600);
    print_uid("seteuid(600)", ret);    /* Real:1000  Effective: 600  Saved:   0 */

    ret = setuid(1000);
    print_uid("setuid(1000)", ret);    /* Real:1000  Effective:1000  Saved:   0 */

    ret = setuid(0);
    print_uid("setuid(0)", ret);       /* Real:1000  Effective:   0  Saved:   0 */

    ret = setuid(1000);
    print_uid("setuid(1000)", ret);    /* Real:1000  Effective:1000  Saved:1000 */

    ret = setuid(0);
    print_uid("setuid(0)", ret);       /* Real:1000  Effective:1000  Saved:1000 */

    return 0 ;       
}
