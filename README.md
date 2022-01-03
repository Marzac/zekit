# ZeKit - DIY Paraphonic Synth KIT 
**(c) Fred's Lab - Frédéric Meslin**  
**fred@fredslab.net**  
**2021 - 2022**  

This is the **official public repository** for the ZeKit project.  

/Instructions/  
/Firmware/  
/Schematics/  

#### GitHub Pull Requests are not accepted!

## 1- Get the source code
First make sure git is correctly installed on your system and second, clone the project repository typing the following command in your terminal of choice:

``` shell
git clone git@github.com:Marzac/zekit.git
```
or:

``` shell
git clone https://github.com/Marzac/zekit.git
```

## 2- Compile the project
1. Install the latest *Microchip MPLAB X IDE*  
Get it from: https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide#tabs  

2. Install the latest Microchip XC16 compiler (the free version)  
Get it from: https://www.microchip.com/en-us/tools-resources/develop/mplab-xc-compilers#tabs  

3. Open the "Firmware" folder of the ZeKit repository using the *Open Project* command in the *File menu* of MPLAB X IDE.

## 3- Flash, Run & Debug

To flash, run and debug the ZeKit firmware you will need an ICSP programming probe. A PICkit 3 or any newer compatible probe is highly recommend. Here is a link to the official toolkit from Microchip:

https://www.microchip.com/en-us/development-tool/PG164140  

To keep this project simple, the ZeKit firmware does not come with a MIDI bootloader.  

The ICSP programming header is located at the bottom left of the ZeKit PCB. The ICSP pin N°1 is labelled RST on the board.  

## About Open Source

I decided to open up some of **Fred's Lab** software, to offer the users the option to customize their software, to ensure long term interoperability & serviceability of the bought gear and finally, in the hope that the present sources be of some pedagogical value.

However, I have no concrete intention, nor the time, to maintain a contribution based open-source project and I want to keep ownership on Fred's Lab made content.
Therefore, please simply consider this repository as reference material = just a place to start.

Feel free to fork the repository and create your own **free alternate versions**, within the conditions of the *GPLv3 Software License.*

https://www.gnu.org/licenses/gpl-3.0.en.html

## To contribute

The best way to contribute to the ZeKit project is by proudly and loudly sharing your derivate work and all ZeKit related content with the world. It helps bringing attention to the machine and to your production.

If you are convinced a new feature or a bug fix must land into this reference repository and that it will benefit all ZeKit users, instead of submitting a Pull Request (not accepted), please *contact me* per e-mail (fred@fredslab.net) and let us discuss first about it.

Best greetings from Germany,  
Frédéric
