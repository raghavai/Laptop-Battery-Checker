Laptop-Battery-Status-Check
===========================
This project gives a little insight into the design and use of ACPI and kernel module development.
I have written a linux kernel module that checks the battery status every second and reports any of the 
following results:
  1. Battery begins to discharge and how much percentage left.
  2. Battery begins to charge and how much percentage charged.
  3. Battery has reached full charge.
  4. Battery level has become low.
  
A basic understanding of kernel programming, loadind and unloading modules is required.
You will need help to find the name of the method your computer uses to access the battery information.
Its similar to \_SB_.BATO._BIF. Use the ACPI Specification pdf at http://www.acpi.info/DOWNLOADS/ACPIspec50.pdf for more help.
  
