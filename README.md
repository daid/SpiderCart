# SpiderCart

SpiderCart is a new gameboy and gameboy color flash cartridge. Which will have many legs over the alternatives.

Currently, it is still in development. But it uses a different setup compared to other flash carts. Other carts use FPGAs or CPLDs. Instead the SpiderCart uses a single RP2354B microcontroller to function as the MBC and handle all other functionalities required. This offers greater flexiblity at lower costs.

The SpiderCart is fully open source MIT license. And can be produced and copied by anyone. It also has all the flexibility to be modified by anyone.

## Terminology

This repository documents the technical side of the SpiderCart. Because of this it uses a lot of terminology. This list ensures that we all speak the same language.

* DMG: The origonal grey brick gameboy. With 4 "green" colors.
* GBC: The Gameboy Color. Upgraded version of the grey brick with color support, double CPU speed and a few more things.
* MCU: Microcontroller. In this case the RP2354B from Raspberry Pi. The RP2354B is a RP2350 with internal flash, so references to RP2350 are also refering to this chip.
* SRAM: "Save" ram. This is also called "external ram" from the gameboy perspective, and contained within the cartridge. Optionally this is preserved between power cycles. Not to be confused with "Static RAM" which is a name of a specific type of RAM chips (to which we just refer as RAM chip)
* RTC: A clock. The gameboy documentation usually refers to this as the Timer. It keep "clock"/"wall" time. Even when the gameboy is off. This requires a battery to preserve state.
* Firmware: Code installed on the MCU that facilitates the operation of the flash cart
* Loader: The code running on the gameboy that allows selecting of rom files and potentially do other things.

## Features

The following features are working:

* Support running DMG and GBC roms up to 2MB in size (V1 limitation, design supports up to 8MB)
* Saves backed up to SD card automatically
* Menu to load ROMs from SD card
* Directly loading ROMs through the USB port for quick ROM debugging/testing
* Optional quickboot to start a rom without resetting and running the bootrom (not compatible with all roms)

The following features are in development:

* Pico-8 support
* USB port direct access to SD card for easy file updating without taking out the SD card
* Full support for RTC (Harvest Moon / Pokemon / Homebrew)
* 3 axis accelerometer (Kirby's Tilt 'n' Tumble / Homebrew)
* Rumble support

The following features are ideas that could work but not actively being developed right now:

* Customizable loader to make rom selection look how you want it, current loader is ugly but functional
* SD file access for homebrew
* USB networking
* NES emulation
* Doom

## Implementation

The SpiderCart is implemented by connecting the MCU and RAM chip directly to the gameboy cartridge bus. The RAM chip is loaded with whatever ROM needs to run. The rom is loaded by the MCU into the RAM chip, this can be done while the gameboy is kept in reset.

While the gameboy is running, the MCU needs to act like the MBC, controlling when the RAM chip is enabled, as well as simulating the SRAM. It will only need of the two CPU cores this MCU has, so 1 will be available for other functionality.

On the 150Mhz RP2350 Core1 is constantly running as MBC, while core0 handles things like saving SRAM and communicating with the RTC/Accelerometer/USB.