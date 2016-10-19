# Smart-Fan-Controller
Smart Fan Controller C Code for the Smart Fan project proposed by Fisher &amp; Paykel Appliances.

Using the ATtinyY841 Atmel Microcontroller.

# Build
You need to use Atmel Studio 7 in order to build and program the ATtiny. 
  1. Create a new project with the device set as ATtiny 841
  2. Remove and delete the main.c file from the project
  3. Add all the file in this repository into the projecy
  4. Set the compiler optimisation to -Os (Project -> Toolchain -> Optimisation Level: -OS)
  6. Clear the CLKDIV8 fuse bit of the ATtiny 841 in programming settings
  5. Build the project and then program the ATtiny  using the Atmel ISP programmer
