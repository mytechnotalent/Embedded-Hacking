![image](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded%20Hacking.png?raw=true)

<br>

---
**LEGAL DISCLAIMER:**
The information, tools, and code provided in this repository and course are strictly for educational, research, and defensive purposes only. 

You are explicitly prohibited from using any materials contained herein to access, test, modify, or exploit any device, network, or system that you do not own 100% or for which you do not have explicit, documented, and legally binding authorization to interact with.

By using this repository and course, you acknowledge and agree that:
1. Any illegal, unauthorized, or malicious use of this information is solely your responsibility.
2. The author(s) and contributor(s) of this repository and course shall not be held liable for any damages, legal repercussions, criminal charges, or unauthorized actions resulting from the use, misuse, or abuse of the contents herein.
3. You will comply with all applicable local, state, national, and international laws regarding cybersecurity and computer fraud.

**IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS REPOSITORY AND COURSE.**
---

<br>

## FREE Reverse Engineering Self-Study Course [HERE](https://github.com/mytechnotalent/Reverse-Engineering-Tutorial)

<br>

# Today's Tutorial [August 16, 2026]
## Lesson 271: Hacking Embedded Rust Course w/ microbit (Chapter 5: Exploitation and Security Assessment)
This chapter covers the exploitation and security assessment for an embedded Rust microbit.

-> Click [HERE](https://github.com/mytechnotalent/rust_embassy_microbit_project/blob/main/Hacking_Embedded_Rust_w_microbit.pdf) to read the FREE pdf book.

<br>

# Embedded Hacking
A FREE comprehensive step-by-step embedded hacking course covering Embedded Software Development to Reverse Engineering.

VIDEO PROMO [HERE](https://www.youtube.com/watch?v=aD7X9sXirF8)

<br>

# FREE Book [Download](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf)

<br>

# Required Skills
Students must have a working understanding of the following items:

- **Intermediate C Programming:** In particular, you must understand how pointers reference memory addresses which we will use to perform live variable hijacking.
  - Training Link [HERE](https://www.learn-c.org)
- **Bare-Metal Embedded Systems Assembly Basics:** You must have experience with bare-metal assembly as we will be using this extensively throughout the entire course specifically with GDB and Ghidra.
  - Training Link [HERE](https://azeria-labs.com/writing-arm-assembly-part-1)
- **Computer Architecture Basics (Registers & Stack):** You need a working mental model of how a CPU/MCU uses registers and the stack to follow the boot process and function calls.
  - Training Link [HERE](https://www.geeksforgeeks.org/computer-science-fundamentals/what-is-register-memory)
  - Training Link [HERE](https://www.geeksforgeeks.org/computer-organization-architecture/memory-stack-organization-in-computer-architecture)
- **Command Line Interface (CLI) Proficiency:** You must be comfortable navigating directories and executing commands in a terminal to run the necessary OpenOCD and GDB tools.
  - Training Link [HERE](https://www.freecodecamp.org/news/command-line-commands-cli-tutorial)
- **Breadboarding Proficiency:** You must be comfortable demonstrating breadboard proficiency by efficiently prototyping circuits and integrating diverse peripherals, sensors, and microcontrollers.
  - Training Video [HERE](https://www.youtube.com/watch?v=fq6U5Y14oM4)

<br>

# Hardware [View Full Parts List](PARTS.md)
## Complete Component Kit for Raspberry Pi [BUY](https://www.pishop.us/product/custom-kit-for-pitt-edu)
### 10% PiShop DISCOUNT CODE - UPKT_HS320548_10PC
## 5mm LEDs (1 Red, 1 Green, 1 Yellow) [BUY](https://www.amazon.com/s?k=5mm+led+kit)
## 100, 220 or 330 Ohm Resistors (for LEDs) [BUY](https://www.amazon.com/s?k=resistor+assortment+kit)
## 1x SG90 Servo Motor [BUY](https://www.amazon.com/s?k=sg90+micro+servo+motor)
## 10pc 25v 1000uF Capacitor [BUY](https://www.amazon.com/Cionyce-Capacitor-Electrolytic-CapacitorsMicrowave/dp/B0B63CCQ2N?th=1)

<br><br>

# Breadboard Design
![image](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/EHP2_bb.png?raw=true)

<br>

# Syllabus

## Week 1
Introduction and Overview of Embedded Reverse Engineering: Ethics, Scoping, and Basic Concepts

### Week 1 Slides [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK01/WEEK01-SLIDES.pdf)

### Week 1 Notebook [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK01/WEEK01.md)

### Chapter 1: hello, world
This chapter covers the basics of setting up a dev environment and basic template firmware for the Pico 2 MCU in addition to printing hello, world.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 2: Debugging hello, world
This chapter covers the debugging of our firmware for the Pico 2 MCU hello, world program.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

## Week 2
Hello, World - Debugging and Hacking Basics: Debugging and Hacking a Basic Program for the Pico 2

### Week 2 Slides [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK02/WEEK02-SLIDES.pdf)

### Week 2 Notebook [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK02/WEEK02.md)

### Chapter 3: Hacking hello, world
This chapter covers the hacking of our firmware for the Pico 2 MCU hello, world program.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

## Week 3
Embedded System Analysis: Understanding the RP2350 Architecture w/ Comprehensive Firmware Analysis

### Week 3 Slides [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK03/WEEK03-SLIDES.pdf)

### Week 3 Notebook [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK03/WEEK03.md)

### Chapter 4: Embedded System Analysis
This chapter covers a comprehensive embedded system analysis reviewing parts of the RP2350 datasheet and helpful firmware analysis tools.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

## Week 4
Variables in Embedded Systems: Debugging and Hacking Variables w/ GPIO Output Basics

### Week 4 Slides [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK04/WEEK04-SLIDES.pdf)

### Week 4 Notebook [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK04/WEEK04.md)

### Chapter 5: Intro To Variables
This chapter covers an introduction to variables as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 6: Debugging Intro To Variables
This chapter covers debugging an introduction to variables as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 7: Hacking Intro To Variables
This chapter covers hacking an introduction to variables as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 8: Uninitialized Variables
This chapter covers uninitialized variables as well as an intro to GPIO outputs as we blink an LED as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 9: Debugging Uninitialized Variables
This chapter covers debugging uninitialized variables as well as an intro to GPIO outputs as we blink an LED as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 10: Hacking Uninitialized Variables
This chapter covers hacking uninitialized variables as well as an intro to GPIO outputs as we blink an LED as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

## Week 5
Integers and Floats in Embedded Systems: Debugging and Hacking Integers and Floats w/ Intermediate GPIO Output Assembler Analysis

### Week 5 Slides [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK05/WEEK05-SLIDES.pdf)

### Week 5 Notebook [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK05/WEEK05.md)

### Chapter 11: Integer Data Type
This chapter covers the integer data type in addition to a deeper assembler dive into GPIO outputs as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 12: Debugging Integer Data Type
This chapter covers debugging the integer data type in addition to a deeper assembler dive into GPIO outputs as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 13: Hacking Integer Data Type
This chapter covers hacking the integer data type in addition to a deeper assembler dive into GPIO outputs as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 14: Floating-Point Data Type
This chapter covers the floating-point data type as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 15: Debugging Floating-Point Data Type
This chapter covers debugging the floating-point data type as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 16: Hacking Floating-Point Data Type
This chapter covers hacking the floating-point data type as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 17: Double Floating-Point Data Type
This chapter covers the double floating-point data type as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 18: Debugging Double Floating-Point Data Type
This chapter covers debugging the double floating-point data type as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 19: Hacking Double Floating-Point Data Type
This chapter covers hacking the double floating-point data type as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

## Week 6
Static Variables in Embedded Systems: Debugging and Hacking Static Variables w/ GPIO Input Basics

### Week 6 Slides [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK06/WEEK06-SLIDES.pdf)

### Week 6 Notebook [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK06/WEEK06.md)

### Chapter 20: Static Variables
This chapter covers static variables as well as an intro to GPIO inputs as we work with push buttons as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 21: Debugging Static Variables
This chapter covers debugging static variables as well as an intro to GPIO inputs as we work with push buttons as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 22: Hacking Static Variables
This chapter covers hacking static variables as well as an intro to GPIO inputs as we work with push buttons as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

## Week 7
Constants in Embedded Systems: Debugging and Hacking Constants w/ 1602 LCD I2C Basics

### Week 7 Slides [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK07/WEEK07-SLIDES.pdf)

### Week 7 Notebook [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK07/WEEK07.md)

### Chapter 23: Constants
This chapter covers constants as well as an intro to I2C as we work a 1602 LCD as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 24: Debugging Constants
This chapter covers debugging constants as well as an intro to I2C as we work a 1602 LCD as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 25: Hacking Constants
This chapter covers hacking constants as well as an intro to I2C as we work a 1602 LCD as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

## Week 8
### Midterm Exam

## Week 9
Operators in Embedded Systems: Debugging and Hacking Operators w/ DHT11 Temperature & Humidity Sensor Single-Wire Protocol Basics

### Week 9 Slides [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK09/WEEK09-SLIDES.pdf)

### Week 9 Notebook [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK09/WEEK09.md)

### Chapter 26: Operators
This chapter covers operators as well as an intro to single-wire protocol as we work a DHT11 temperature and humidity sensor as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 27: Debugging Operators
This chapter covers debugging operators as well as an intro to single-wire protocol as we work a DHT11 temperature and humidity sensor as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 28: Hacking Operators
This chapter covers hacking operators as well as an intro to single-wire protocol as we work a DHT11 temperature and humidity sensor as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

## Week 10
Conditionals in Embedded Systems: Debugging and Hacking Static & Dynamic Conditionals w/ SG90 Servo Motor PWM Basics

### Week 10 Slides [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK10/WEEK10-SLIDES.pdf)

### Week 10 Notebook [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK10/WEEK10.md)

### Chapter 29: Static Conditionals
This chapter covers static conditionals as well as an intro to PWM as we work a SG90 servo motor as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 30: Debugging Static Conditionals
This chapter covers debugging static conditionals as well as an intro to PWM as we work a SG90 servo motor as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 31: Hacking Static Conditionals
This chapter covers hacking static conditionals as well as an intro to PWM as we work a SG90 servo motor as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 32: Dynamic Conditionals
This chapter covers dynamic conditionals as well as additional PWM examples as we work a SG90 servo motor as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 33: Debugging Dynamic Conditionals
This chapter covers debugging dynamic conditionals as well as additional PWM examples as we work a SG90 servo motor as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 34: Hacking Dynamic Conditionals
This chapter covers hacking dynamic conditionals as well as additional PWM examples as we work a SG90 servo motor as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

## Week 11
Structures and Functions in Embedded Systems: Debugging and Hacking w/ IR Remote Control and NEC Protocol Basics

### Week 11 Slides [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK11/WEEK11-SLIDES.pdf)

### Week 11 Notebook [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK11/WEEK11.md)

### Chapter 35: Structures
This chapter covers structures as well as an intro to infrared basics as we work a infrared receiver and infrared remote controller as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 36: Debugging Structures
This chapter covers debugging structures as well as an intro to infrared basics as we work a infrared receiver and infrared remote controller as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 37: Hacking Structures
This chapter covers hacking structures as well as an intro to infrared basics as we work a infrared receiver and infrared remote controller as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 38: Functions, w/ Param, w/ Return
This chapter covers functions, w/ params and w/ a return value as well as additional infrared examples as we work a infrared receiver and infrared remote controller it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 39: Debugging Functions, w/ Param, w/ Return
This chapter covers debugging functions, w/ params and w/ a return value as well as additional infrared examples as we work a infrared receiver and infrared remote controller as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 40: Hacking Functions, w/ Param, w/ Return
This chapter covers hacking functions, w/ params and w/ a return value as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded-Hacking.pdf) to read the FREE pdf book.

## Week 12
Unknown Firmware Debugging and Hacking

## Week 13
Final Review – Embedded Debugging and Hacking Techniques w/ Advanced Firmware Analysis Q&A

## Week 14
### Final
### Final Project Option 1: The InfuSafe Pro Incident
In the aftermath of a catastrophic medical device failure, you are thrust into the role of an FDA forensic investigator facing an impossible crisis: 23 patients dead, 100 million recalled insulin pumps sitting in warehouses worldwide, and 2.3 million lives hanging in the balance all while the only evidence remaining is raw binary firmware after a rogue engineer destroyed every line of source code before fleeing to Montenegro. Armed only with GDB, Ghidra, and the reverse engineering skills honed over the first seven weeks of this course, you must excavate the truth from machine code, identify the lethal bugs spawned by an AI code generator called "OopsieGPT," and determine whether these devices can be salvaged to save millions in underserved communities or if $4.7 billion in humanitarian medical technology must be incinerated. This is not a simulation; this is triage at the intersection of embedded systems security and human survival.

### Final Project Option 2: Operation Dark Eclipse
Forty-two stories beneath frozen tundra, a shadow intelligence alliance called Dark Eyes operates centrifuges enriching weapons-grade material for a first strike against Washington, D.C. and Agent NIGHTINGALE gave her life to extract the single firmware file that now sits before you. Conventional warfare cannot reach this fortress buried beneath rock and concrete, but you can: as the architect of a precision cyber weapon in the tradition of Stuxnet, you must reverse engineer the RP2350-based centrifuge controller, craft binary patches that double the spin speed while falsifying every sensor readout to show nominal operation, and execute the sabotage that will cascade-destroy their enrichment program and set their nuclear ambitions back a decade. Every skill from the entire semester ARM assembly, Ghidra analysis, IEEE-754 floating-point manipulation, branch modification, log desynchronization converges in this final mission. Agent NIGHTINGALE's seven-year-old daughter still watches the driveway, waiting for a mother who will never return. Honor that sacrifice. Complete the mission. Do not fail.

<br>

## RP2350 ARM Assembler Drivers

### An RP2350 UART driver written entirely in ARM Assembler. [HERE](https://github.com/mytechnotalent/RP2350_UART_Driver)

### An RP2350 blink driver written entirely in ARM Assembler. [HERE](https://github.com/mytechnotalent/RP2350_Blink_Driver)

### An RP2350 button driver written entirely in ARM Assembler. [HERE](https://github.com/mytechnotalent/RP2350_Button_Driver)

<br>

## RP2350 RISC-V Assembler Drivers

### An RP2350 UART driver written entirely in RISC-V Assembler. [HERE](https://github.com/mytechnotalent/RP2350_UART_Driver_RISCV)

### An RP2350 blink driver written entirely in RISC-V Assembler. [HERE](https://github.com/mytechnotalent/RP2350_Blink_Driver_RISCV)

### An RP2350 button driver written entirely in RISC-V Assembler. [HERE](https://github.com/mytechnotalent/RP2350_Button_Driver_RISCV)

<br>

# License
[Apache License, Version 2.0](https://www.apache.org/licenses/LICENSE-2.0)
