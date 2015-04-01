==============================================================================
Readme - Software for the PNEV512B Blueboard and CLEV663B Blueboard
==============================================================================

1. Document purpose

This document describes the content of the source package as well as known
problems and restrictions.
A separate User Manual describes the first steps how to use the software in
combination with the Hardware.

2. Known problems and restrictions

* Restrictions
The Software is restricted to the PNEV512B Blueboard and CLEV663B Blueboard. The API is intended for
the NXP contactless/contact reader ICs, as such the API is not to be ported to any technology from
any other vendor.
The Software is running on a Cortex M0 LPC1227/301 as well as on a Cortex M3 LPC1769 controller
from NXP.
NXP will not support porting to any other vendor platform. 

This project is intended to be used together with a
LPC MCU project and an application project.

* Known Problems

  - none
	
3. Content

NXP-Reader-Library (Eclipse project) content:
+---comps
¦   +---phacDiscLoop
¦   ¦   +---src
¦   ¦       +---Sw
¦   +---phalFelica
¦   ¦   +---src
¦   ¦       +---Sw
¦   +---phalMfc
¦   ¦   +---src
¦   ¦       +---Sw
¦   +---phalMfdf
¦   ¦   +---src
¦   ¦       +---Sw
¦   +---phalMful
¦   ¦   +---src
¦   ¦       +---Sw
¦   +---phalT1T
¦   ¦   +---src
¦   ¦       +---Sw
¦   +---phalTop
¦   ¦   +---src
¦   ¦       +---Sw
¦   +---phbalReg
¦   ¦   +---src
¦   ¦       +---Stub
¦   +---phCidManager
¦   ¦   +---src
¦   ¦       +---Sw
¦   +---phhalHw
¦   ¦   +---src
¦   ¦       +---Callback
¦   ¦       +---Rc523
¦   ¦       +---Rc663
¦   +---phKeyStore
¦   ¦   +---src
¦   ¦       +---Rc663
¦   ¦       +---Sw
¦   +---phlnLlcp
¦   ¦   +---src
¦   ¦       +---Fri
¦   +---phLog
¦   ¦   +---src
¦   +---phnpSnep
¦   ¦   +---src
¦   ¦       +---Fri
¦   +---phOsal
¦   ¦   +---src
¦   ¦       +---Stub
¦   +---phpalFelica
¦   ¦   +---src
¦   ¦       +---Sw
¦   +---phpalI14443p3a
¦   ¦   +---src
¦   ¦       +---Sw
¦   +---phpalI14443p3b
¦   ¦   +---src
¦   ¦       +---Sw
¦   +---phpalI14443p4
¦   ¦   +---src
¦   ¦       +---Sw
¦   +---phpalI14443p4a
¦   ¦   +---src
¦   ¦       +---Sw
¦   +---phpalI18092mPI
¦   ¦   +---src
¦   ¦       +---Sw
¦   +---phpalI18092mT
¦   ¦   +---src
¦   ¦       +---Sw
¦   +---phpalMifare
¦   ¦   +---src
¦   ¦       +---Sw
¦   +---phTools
¦       +---src
+---intfs
+---types

4. Mandatory material, not included

* LPCXpresso IDE as described on the web site:
	http://www.lpcware.com/lpcxpresso
* LPCXpresso LPC1227 or LPC1769 development board

5. Revision History

V1.0:
* This package contains the NFC Reader version 3.010.00.001407
* First packaged version.