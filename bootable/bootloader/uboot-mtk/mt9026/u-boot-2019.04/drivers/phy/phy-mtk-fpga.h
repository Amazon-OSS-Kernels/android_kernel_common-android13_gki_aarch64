#ifndef _MTU3_PHY_H_
#define _MTU3_PHY_H_
#include <generic-phy.h>

#define SSUSB_IPPC_LEN				0x100
#define SSUSB_FPGA_I2C_PORT_SIZE		0x8
#define SSUSB_FPGA_I2C_PORT_OFFSET(x)	\
	(0xd0 + (SSUSB_FPGA_I2C_PORT_SIZE) * (x))

#define SSUSB_PHY_VERSION_BANK	0x20
#define SSUSB_PHY_VERSION_ADDR	0xe4

#define PHY_TRUE 1
#define PHY_FALSE 0

#define SSUSB_FPGA_I2C_OUT	0x0
#define SSUSB_FPGA_I2C_SDA_OUT	BIT(0)
#define SSUSB_FPGA_I2C_SDA_OEN	BIT(1)
#define SSUSB_FPGA_I2C_SCL_OUT	BIT(2)
#define SSUSB_FPGA_I2C_SCL_OEN	BIT(3)

#define SSUSB_FPGA_I2C_IN	0x4
#define SSUSB_FPGA_I2C_SDA_IN	BIT(0)
#define SSUSB_FPGA_I2C_SCL_IN	BIT(1)

#define I2C_DELAY 1

enum i2c_dir {
	I2C_INPUT = 0,
	I2C_OUTPUT,
};

enum i2c_pin {
	I2C_SDA = 0,
	I2C_SCL,
};

#define PHY_TEST_CHIP_A60931 0xa60931a
#define PHY_TEST_CHIP_A60810 0xa60810a
#define PHY_TEST_CHIP_A60855 0xa60855a
/* some test chip got default name without changed to correct one*/
#define PHY_TEST_CHIP_NONAME 0xd60802a

int fpga_phy_set_pclk(struct phy *phy, int pclk);
int phy_writeb(void __iomem *port, u8 i2c_addr, u8 addr, u8 value);
u8 phy_readb(void __iomem *port, u8 i2c_addr,  u8 addr);

#endif
