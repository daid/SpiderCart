#include <tusb.h>
#include <bsp/board_api.h>
#include "pico/unique_id.h"


#define USBD_VID (0x2E8A) // Raspberry Pi
#define USBD_PID (0x000a) // Raspberry Pi Pico SDK CDC for RP2040


tusb_desc_device_t const device_desc = {
    .bLength = sizeof(tusb_desc_device_t), //length of this descriptor (in bytes)
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200, //usb specification version number in BCD. here it is USB 2.0

    .bDeviceClass = TUSB_CLASS_MISC, //CDC Class falls under MISC class
    .bDeviceSubClass = MISC_SUBCLASS_COMMON, //CDC uses common subclass
    .bDeviceProtocol = MISC_PROTOCOL_IAD, //this is a protocol code (assigned by the USB-IF)
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor =  USBD_VID, //we use the prototype VID as the vendor id
    .idProduct = USBD_PID, //the fixed product ID we set before
    .bcdDevice = 0x0100, //device release number in BCD

    .iManufacturer = 0x1, //index of the Manufaturer field in the string_dec_arr
    .iProduct = 0x02, //index of the product field in the string desc_arr
    .iSerialNumber = 0x03, //index of the serial number string
    .bNumConfigurations = 0x01
};

static char usbd_serial_str[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1];
//to store the device descriptors that come as strings, we use a pointer to 
char const *string_desc_arr[] = {
    (const char []) {0x09, 0x04}, // 0 : supported language
    "Daid",           // 1 : Manufacturer of the product
    "SpiderCart",     // 2 : Product     
    usbd_serial_str,  // 3  : serial number of the product 
    "SpiderCartMSC"
};

//USB hosts identify interfaces by numbers (0, 1, 2) and the CDC device we make need two interfaces
// one for communication the other for the data interface
//we use this enum structure to tell the TinyUSB's Macro to use them
enum {
    ITF_NUM_CDC = 0,
    ITF_NUM_CDC_DATA,
    ITF_NUM_MSC,
    ITF_NUM_TOTAL
};

//CDC needs three endpoints
//Endpoint : 0x81   (IN)    Interript
//Endpoint : 0x02   (OUT)   Data Out
//Endpoint : 0x82   (IN)    Data IN

#define EPNUM_CDC_NOTIF 0x81 
#define EPNUM_CDC_OUT 0x02
#define EPNUM_CDC_IN 0x82

#define EPNUM_MSC_OUT     0x03
#define EPNUM_MSC_IN      0x83


#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_MSC_DESC_LEN)

uint8_t config_desc[] = {
    TUD_CONFIG_DESCRIPTOR(
        1,ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x80,100
    ),

    TUD_CDC_DESCRIPTOR(
        ITF_NUM_CDC,
        0,
        EPNUM_CDC_NOTIF,
        8,
        EPNUM_CDC_OUT,
        EPNUM_CDC_IN,
        64
    ),
    
    TUD_MSC_DESCRIPTOR(ITF_NUM_MSC, 4, EPNUM_MSC_OUT, EPNUM_MSC_IN, 64),
};


//callback functions
uint8_t const *tud_descriptor_device_cb(void) {
    return (uint8_t const *)&device_desc;
};

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
    (void) index;  //we avoid the unused variable error while keeping the function's signature intact
    return config_desc;
}

// buffer to hold the string descriptor during the request | plus 1 for the null terminator
static uint16_t _desc_str[32 + 1];

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void) langid;

    // Assign the SN using the unique flash id
    if (!usbd_serial_str[0]) {
        pico_get_unique_board_id_string(usbd_serial_str, sizeof(usbd_serial_str));
    }

    if (index == 0) {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 + 2);
        return _desc_str;
    }

    const char *str = string_desc_arr[index];
    size_t len = strlen(str);

    for (size_t i = 0; i < len; i++) {
        _desc_str[1 + i] = str[i];
    }

    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 + len * 2);
    return _desc_str;
}