# AVRC-EMBEDDED

**About**

This was an undergraduate research project to prove whether a low-cost localization system for indoor robots could be created, functional and enduring.

AVR-C code for Infrared Beacon Localization project. To view code, open a chosen folder, open the following folder with the same chosen name and then view the main.c file.

## Project Devices

**RECEIVER platform integrated with Raspberry Pi3 via USB2.0:**

(ATMEGA328P) Receiver uses an TSOP IR sensor and decodes 38khz IR pulses by the period of the pulses, discerned 0's and 1's. With smart filtering by measuring the period of "off" time between transmitted pulses, can then forward the information of which beacon I.D. code was received to a Raspberry Pi3 which will run a python mapping algorithm combined with an "estimated" velocity from the platform that this setup is mounted on to create a probability map and localize the platform.


**Transmitter and Synchronizater BEACON platform:**

(ATTINY13A) Transmitters uses a continouslly running timer interrupts to generate a 38kHz signal that is then connected/disconnected with the output pins to the IR led's. This device transmits controlled time length IR pulses to send a "SYNC" pulse and then it's hardcoded Identification code as a 2-bit binary sequence. The transmission is in a timed "charge up" sequence that is controlled though the Sychronizer device.


(ATTINY13A) Sychronizer uses an TSOP IR receiving sensor to decode 38khz IR pulses for their "SYNC" pulse and outputs a signal to the Transmitter device to have it modify it's timing sequence when a "SYNC" pulse is received from any additioaly BEACON platforms in this BEACON's interference range. By using IAF (Integrate and Fire), the BEACON's can synchronize their pulses as to not overlap signals and transmit a cohereant Identification code without interference from other BEACON's.


## Reasons for key decisions behind key functions
  
**Transmission of bit's through InfraRed 38khz Signal**

Using the same idea as Sony remotes. A binary "0" is represented as a pulse with the period of T. A binary "1" is represented as a pulse with a period of 2T. A "SYNC" pulse is represented as a pulse of period "4T".

**Need for Synchronization of BEACON's**

Because the TSOP IR sensors are the chosen cost-effective sensors, the possibility of using multiple channels of Infrared was not an option. To avoid interference, a simple solution was developed inspired by the sychronization of firefly species Pteroptyx Cribellata (Phase-delay sychronization) and Photinus Pyralis (Phase-advance entrainment model). The solution was to have all BEACON's sychronize pulse sequences if another BEACON's transmission was received by a BEACON, which meant they were both in interference range of eachother. If not BEACON's are in range, then the BEACON fires it's Identification sequence without sychronizing.

