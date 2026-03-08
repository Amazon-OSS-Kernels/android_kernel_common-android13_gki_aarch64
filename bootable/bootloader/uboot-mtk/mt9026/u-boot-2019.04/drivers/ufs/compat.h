#ifndef _UFS_COMPAT_H_
#define _UFS_COMPAT_H_

#define SG_ALL 64//128

#define platform_device udevice

struct completion {
	unsigned int done;
};
#define init_completion(x) memset((void *)(x), 0, sizeof(struct completion ))
#define complete(x) (x)->done = 1

typedef ulong ktime_t;
#define ktime_to_us(x) (unsigned long long)x
#define ktime_get timer_get_boot_us
#define ktime_set(a, b) (a*1000+b)

#define msleep(a) udelay(a * 1000)
#define usleep_range(a, b) udelay(a)

#define jiffies timer_get_boot_us()
#define msecs_to_jiffies(x) (x*1000)

#define test_and_set_bit_lock test_and_set_bit
#define clear_bit_unlock __clear_bit

#define down_read_trylock(...) 1

#define wmb()

#define dev_WARN_ONCE(dev, condition, fmt, args...) printf(fmt, ##args)
#define dev_err_ratelimited dev_err
#define dev_WARN dev_err

typedef unsigned short wchar_t;
typedef u32 unicode_t;

#define ktime_sub(a, b) (a-b)

#define trace_ufshcd_profile_hibern8(...)
#define trace_ufshcd_auto_bkops_state(...)
#define trace_ufshcd_init(...)

/* Byte order for UTF-16 strings */
enum utf16_endian {
	UTF16_HOST_ENDIAN,
	UTF16_LITTLE_ENDIAN,
	UTF16_BIG_ENDIAN
};

#define UNICODE_MAX	0x0010ffff
#define PLANE_SIZE	0x00010000

#define SURROGATE_MASK	0xfffff800
#define SURROGATE_PAIR	0x0000d800
#define SURROGATE_LOW	0x00000400
#define SURROGATE_BITS	0x000003ff

#endif
