

#include <stdio.h>
#include "radar01_io.h"

struct radar01_io_info_t *iwr1642_dss_blk;
struct radar01_io_info_t *iwr1642_mss_blk;
int main(int UNUSED(argc), char const UNUSED(*argv[]))
{
    int rc = 0;
    /* code */
    // rc = radar01_io_init("/dev/ttyAMC1", (void *) &iwr1642_dss_blk);
    return rc;
}
