
#define NOP 0x00     ///< No-op register
#define SWRESET 0x01 ///< Software reset register
#define RDDID 0x04   ///< Read display identification information
#define RDDST 0x09   ///< Read Display Status

#define SLPIN 0x10  ///< Enter Sleep Mode
#define SLPOUT 0x11 ///< Sleep Out
#define PTLON 0x12  ///< Partial Mode ON
#define NORON 0x13  ///< Normal Display Mode ON

#define RDMODE 0x0A     ///< Read Display Power Mode
#define RDMADCTL 0x0B   ///< Read Display MADCTL
#define RDPIXFMT 0x0C   ///< Read Display Pixel Format
#define RDIMGFMT 0x0D   ///< Read Display Image Format
#define RDSELFDIAG 0x0F ///< Read Display Self-Diagnostic Result

#define INVOFF 0x20   ///< Display Inversion OFF
#define INVON 0x21    ///< Display Inversion ON
#define GAMMASET 0x26 ///< Gamma Set
#define DISPOFF 0x28  ///< Display OFF
#define DISPON 0x29   ///< Display ON

#define CASET 0x2A ///< Column Address Set
#define PASET 0x2B ///< Row Address Set
#define RAMWR 0x2C ///< Memory Write
#define RAMRD 0x2E ///< Memory Read

#define PTLAR 0x30    ///< Partial Area
#define VSCRDEF 0x33  ///< Vertical Scrolling Definition
#define MADCTL 0x36   ///< Memory Access Control
#define VSCRSADD 0x37 ///< Vertical Scrolling Start Address
#define PIXFMT 0x3A   ///< COLMOD: Pixel Format Set

#define FRMCTR1 0xB1 ///< Frame Rate Control (In Normal Mode/Full Colors)
#define FRMCTR2 0xB2 ///< Frame Rate Control (In Idle Mode/8 colors)
#define FRMCTR3 0xB3 ///< Frame Rate control (In Partial Mode/Full Colors)
#define INVCTR 0xB4  ///< Display Inversion Control
#define DFUNCTR 0xB6 ///< Display Function Control

#define PWCTR1 0xC0 ///< Power Control 1
#define PWCTR2 0xC1 ///< Power Control 2
#define PWCTR3 0xC2 ///< Power Control 3
#define PWCTR4 0xC3 ///< Power Control 4
#define PWCTR5 0xC4 ///< Power Control 5
#define VMCTR1 0xC5 ///< VCOM Control 1
#define VMCTR2 0xC7 ///< VCOM Control 2

#define RDID1 0xDA ///< Read ID 1
#define RDID2 0xDB ///< Read ID 2
#define RDID3 0xDC ///< Read ID 3
#define RDID4 0xDD ///< Read ID 4

#define GMCTRP1 0xE0 ///< Positive Gamma Correction
#define GMCTRN1 0xE1 ///< Negative Gamma Correction
//#define PWCTR6     0xFC
