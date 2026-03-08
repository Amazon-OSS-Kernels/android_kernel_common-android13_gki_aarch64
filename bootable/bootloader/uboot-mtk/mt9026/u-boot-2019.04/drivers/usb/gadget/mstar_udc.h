#ifndef __UDC_MSTAR_H__
#define __UDC_MSTAR_H__

#define MS_BIT0		0x01
#define MS_BIT1		0x02
#define MS_BIT2		0x04
#define MS_BIT3		0x08
#define MS_BIT4		0x10
#define MS_BIT5		0x20
#define MS_BIT6		0x40
#define MS_BIT7		0x80

/* virtual address to physical address translation */
#define ENABLE_USB_NEW_MIU_SEL	1
#define USB_MIU_SEL0	0xF0	/* UMA */
#define USB_MIU_SEL1	0xEF
#define USB_MIU_SEL2	0xEF
#define USB_MIU_SEL3	0xEF
#define MIU0_BUS_BASE_ADDR	0x20000000UL
#define MIU0_PHY_BASE_ADDR	0x00000000UL

#define MAX_USB_DMA_CHANNEL  2

static const char ep0name [] = "ep0";
static const char *const ep_name[] = {
	ep0name,                                /* everyone has ep0 */
	"ep1in-bulk", "ep2out-bulk", "ep3in-int", "ep4out-bulk", "ep5in-int", "ep6out-bulk", "ep7in-bulk"
};
#define MSB250X_ENDPOINTS ARRAY_SIZE(ep_name)

//------ UTMI disconnect level parameters ---------------------------------
// disc: bit[7:4] 0x00: 550mv, 0x20: 575, 0x40: 600, 0x60: 625
// squelch: bit[3:0] 4'b0010 => 100mv
#define UTMI_DISCON_LEVEL_2A	(0x60 | 0x0e)

//------ UTMI eye diagram parameters ---------------------------------
// for 40nm after Agate, use 55nm setting1, the default
#define UTMI_EYE_SETTING_2C (0x10)
#define UTMI_EYE_SETTING_2D (0x02)
#define UTMI_EYE_SETTING_2E (0x00)
#define UTMI_EYE_SETTING_2F (0x81)
#define UTMI_ALL_EYE_SETTING	(0x81000210)

#endif /* __UDC_MSTAR_H__ */
