
#include <common.h>
#include <command.h>
#include <time.h>

#include "debug_impl.h"
#include "cm4_impl.h"

#define VAD_DEVNODE "/vad_rproc"
#define VAD_MAXARGS (2)
#define VAD_REP (1)
static bool bCm4Init=0;

int do_boot_vad (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    unsigned int start = 0, end = 0;
    int ret = 0;

    if (bCm4Init)
    {
        UBOOT_ERROR("do command: boot_vad already init\n");
    }
    else
    {
        start = get_timer(0);
        ret = cm4_loader(VAD_DEVNODE);
        end = get_timer(0);
        bCm4Init = 1;
        UBOOT_INFO("\n[boot_vad][start:%u][end:%u][total time:%u].\n", start, end, end - start);
    }
    return ret;
}

U_BOOT_CMD(
    boot_vad, VAD_MAXARGS, VAD_REP, do_boot_vad,
    "boot coprocessor with vad\n",
    NULL
);

int do_stop_vad(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    unsigned int start = 0, end = 0;
    int ret = 0;

    UBOOT_TRACE("do command: stop_vad.\n");
    start = get_timer(0);
    ret = cm4_stop(VAD_DEVNODE);
    end = get_timer(0);
    bCm4Init = 0;
    UBOOT_INFO("\n[stop_vad][start:%u][end:%u][total time:%u].\n", start, end, end - start);
    return ret;
}

U_BOOT_CMD(
    stop_vad, VAD_MAXARGS, VAD_REP, do_stop_vad,
    "stop coprocessor with vad\n",
    NULL
);
