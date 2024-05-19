# Custom transmission frame decode on PIC microcontroller using Special Function Register (SFR)
## Microcontrollers that can be used
- PIC16F877
- PIC16F877A
- PIC16F887
## Project task
- Send a custom transmission frame using RS232 communication, decode it, and send it back to the terminal.
- The transmission frame has this structure: "a xxxxx b xxxxx c xxx d xxx e". For example: "a123.4b5.678c200d150e"
- The numbers between 'a' & 'b' and 'b' & 'c' are 4-digit floats, and the numbers between 'c' & 'd' and 'd' & 'e' are 1-to-3 digit integers.
- The last number is used as the setpoint (desire speed) to control the speed of the motor with PID controller integrated into PIC microcontroller.  
- Do not use built-in function, only SFRs are allowed.
## Features
- Control signal: 10-bit PWM value
- Calculating feedback speed using encoder pulse frequency
- Counting sampling time with Timer 0, reading encoder pulse with Timer 1
- Implementing PID control algorithm 

## Program used
- Hercules SETUP utility v3.2.8
- Proteus v8.12
- PIC CCS C v5.115 / MPLAB X IDE v5.45
- Virtual Serial Port Driver v6.9
## How to use
- Create a Proteus project and wire as shown in the image.
- Use a RS232 terminal software, such as Hercules SETUP utility, Arduino IDE serial monitor, MATLAB, etc. to send a transmission frame with the above structure.
- To connect the terminal with Proteus, create virtual COM ports using Virtual Serial Port Driver to connect as well as pair them.
- Use the .hex file to input to the PIC microcontroller in Proteus for quick simulation.
- If modification is needed, create a PIC CCS C project or a MPLAB X IDE project (if MPLAB X IDE is used, please remove the register address declaration lines)
- Generate .hex file for the above project to put into the microcontroller in Proteus. 
## Project simulation video
- https://drive.google.com/file/d/1-sbAnhIMAV6_CeADsyaDe60QoZeW19HL/view?usp=sharing
## Reference
- Matab: https://www.mathworks.com/products/new_products/release2021b.html
- PIC CCS C: https://www.ccsinfo.com/compilers.php
- Proteus: https://www.labcenter.com/
- Hercules SETUP utility: https://www.hw-group.com/software/hercules-setup-utility
- VSPD: https://www.eltima.com/products/vspdxp/

