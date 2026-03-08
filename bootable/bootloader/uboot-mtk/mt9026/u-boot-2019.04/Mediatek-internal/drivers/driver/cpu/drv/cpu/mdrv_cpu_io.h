#ifndef __MDRV_CPU_IO_H__
#define __MDRV_CPU_IO_H__

// -----------------------------------------------------------------------------
// Linux mdrv_cpu_io.h define start
// -----------------------------------------------------------------------------
/* Use 'N' as magic number */
#define MDRV_CPU_IOC_MAGIC                'N'

#define MDRV_CPU_IOC_QUERY_LIT          _IOWR(MDRV_CPU_IOC_MAGIC, 0x00, MDRV_CPU_PARM)
#define MDRV_CPU_IOC_QUERY_BIG          _IOWR(MDRV_CPU_IOC_MAGIC, 0x01, MDRV_CPU_PARM)

#define MDRV_EVENT_BUF_SIZE                 (128)
#define MDRV_EVENT_DEV_NAME                 "/dev/cpu"

#define MDRV_CPU_DEBUG(fmt, args...)  printk(KERN_ERR "[%s][%d] " fmt, __func__, __LINE__, ## args)

//-------------------------------------------------------------------------------------------------
//  Structure and Enum
//-------------------------------------------------------------------------------------------------
typedef struct _MDRV_CPU_PARM
{
    unsigned int para[10];
} MDRV_CPU_PARM;

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
#endif
