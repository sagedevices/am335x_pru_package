Updates to the BeagleBone PRU examples

Getting Started
---------------

0. Log into your beaglebone and have git installed (opkg install git)
1. git clone git@github.com:sagedevices/am335x_pru_package.git
2. cd pru_sw/app_loader/interface
3. make
4. cd ../../example_apps
5. make
6. modprobe uio_pruss
7. Run some of the examples in bin/

References
----------

* http://blog.boxysean.com/2012/08/12/first-steps-with-the-beaglebone-pru/
* http://processors.wiki.ti.com/index.php/Programmable_Realtime_Unit
* http://processors.wiki.ti.com/index.php/PRU_Assembly_Instructions
* http://processors.wiki.ti.com/index.php/PRU_Linux-based_Example_Code
* http://caxapa.ru/thumbs/319913/PRUSS_Training_Slides.pdf


Original TI Readme
-------------------------------------------------------------

AM335x PRU PACKAGE

The hardware / software modules and descriptions referred 
to in this package are *NOT SUPPORTED* by Texas Instruments 
(www.ti.com / e2e.ti.com). 

These materials are intended for do-it-yourself (DIY) users 
who want to use the PRU at their own risk without TI support.  
"Community" support is offered at BeagleBoard.org/discuss.
 

-------------------------------------------------------------

The AM335x PRU Package includes:

Documentation:
   AM335x PRU-ICSS overview slides
   AM18x to AM335x PRU software migration guide
   AM335x PRU Linux Application Driver documentation
   CCS AM335x PRU Debugger training slides
   AM335x_PRU_ICSS gel file for use with CCS AM335x PRU Debugger

PRU software (pru_sw):
   Utils:
      PASM (PRU assembler) binary
      PASM Source code

   Linux PRU userspace driver (app_loader):
      prussdrv.c

   Example applications:
      PRU_memAccess_DDR_PRUsharedRAM
      PRU_memAccessPRUDataRam
      PRU_PRUtoPRUInterrupt   
