# Hardware

TODO


## V1 mistakes

I took quite a risk by ordering 5 boards of a completely untested design. And, with any PCB design you will make mistakes. It was good fortune that every single mistake was fixable. All these mistakes have been fixed in the repository design files.

### Oops 1: Swapped USB lines

During early development of the hardware, I copied the Raspberry Pi reference design for the RP2350B chip. But, I replaced the custom symbols with default KiCad symbols where possible. In this process a mistake creeped in.

The reference schematic has the following layout for the USB pins.
![USB connection on PI reference schematic](./img/hw/usb-pi.png)

I copied this, but the KiCad symbol has the USB pins on the other side. So I mirrored it and be done.
![USB connection on SpiderCart V1](./img/hw/usb-sc.png.png)

See what I missed? It's easy to miss. The DM and DP pins are swapped. This was fixed by removing the R7 and R8 resistors and adding two small wire bridges to swap the lines.

> TODO: Insert photo of the patch.

