#pragma once

#define CFG_TUD_ENABLED (1) //enables tinyusb device mode
#define CFG_TUD_CDC (1)
#define CFG_TUD_MSC (0)

// Legacy RHPORT configuration
// Tells TinyUSB that RHPORT0 is active and should run in:
//  - DEVICE mode (not HOST mode)
//  - FULL SPEED (12 Mbps) USB operation.
#define CFG_TUSB_RHPORT0_MODE   (OPT_MODE_DEVICE | OPT_MODE_FULL_SPEED)
#define BOARD_TUD_RHPORT        (0)
// end legacy RHPORT


#define CFG_TUD_CDC_RX_BUFSIZE (64)   //sets the receiving buffer size to 64 bytes
#define CFG_TUD_CDC_TX_BUFSIZE (64)   // sets the sending buffer size to 64 bytes
#define CFG_TUD_CDC_EP_BUFSIZE (64)   // endpoint buffer size for CDC endpoints.

// max packet size for endpoint 0 (control endpoint).
#define CFG_TUD_ENDPOINT0_SIZE (64) 

#define CFG_TUD_MSC_EP_BUFSIZE (512)