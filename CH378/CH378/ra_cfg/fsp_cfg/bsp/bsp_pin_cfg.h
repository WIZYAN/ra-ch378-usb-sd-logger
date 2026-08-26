/* generated configuration header file - do not edit */
#ifndef BSP_PIN_CFG_H_
#define BSP_PIN_CFG_H_
#include "r_ioport.h"

/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_HEADER

#define DIS_POWER (BSP_IO_PORT_01_PIN_15) /*  DIS_POWER */
#define D0 (BSP_IO_PORT_05_PIN_00)
#define D1 (BSP_IO_PORT_05_PIN_01)
#define D2 (BSP_IO_PORT_05_PIN_02)
#define D3 (BSP_IO_PORT_05_PIN_03)
#define D4 (BSP_IO_PORT_05_PIN_04)
#define D5 (BSP_IO_PORT_05_PIN_05)
#define D6 (BSP_IO_PORT_05_PIN_06)
#define D7 (BSP_IO_PORT_05_PIN_07)
#define DIS_EN (BSP_IO_PORT_06_PIN_08) /* DIS_EN */
#define PCS (BSP_IO_PORT_07_PIN_00)
#define RST (BSP_IO_PORT_07_PIN_01)
#define WR (BSP_IO_PORT_07_PIN_02)
#define RD (BSP_IO_PORT_07_PIN_03)
#define AO (BSP_IO_PORT_07_PIN_04)
#define INT (BSP_IO_PORT_07_PIN_09)
extern const ioport_cfg_t g_bsp_pin_cfg; /* R7FA6M4AF3CFB.pincfg */

void BSP_PinConfigSecurityInit();

/* Common macro for FSP header files. There is also a corresponding FSP_HEADER macro at the top of this file. */
FSP_FOOTER

#endif /* BSP_PIN_CFG_H_ */
