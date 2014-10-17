                                 INTRODUCTION
                                 ------------


Slidcom is a PM "Slider" application that uses the IOCTL commands
to communicate via COM1, 2, 3, or 4, to a dumb terminal or similar
device. For example: The user hooks up a dumb terminal to one of the
COM  ports.  Sets the dumb terminal to 9600 baud, 8 bit, no parity, 1 stop
bit. Next bring up Slidcom from an OS2 prompt. Using the pull down menus,
select a  COM port and then initialize the line settings to match the
dumb terminal. A null modem may/may not be needed. Next, click on the
slider. Moving the slider arm will cause ASCII characters to be displayed on
the  dumb terminal. Entering a value between 0 and 100 followed by <cr> at
the  dumb terminal,  will move the slider arm to the corresponding
position.


APPLICATION SPECIFIC NOTES:

1. Make the sure the line settings on the dumb terminal match that of
   the  slider.

2. A null modem may or may not be required.

3. I noticed that this application runs somewhat differently depending
   whether or not the OS2 service pack updates have been applied to
   your  system.  Suggest you install these updates.

4. Debug messages were left in. These will typically occur when trying
   to  select an invalid COM port. The RC number displayed is the return
   code  of the API call that failed. This code is displayed in Appendix B
   of the Control Program Programming Reference.


USAGE:

  The slider has most of the "slider type features". The slider arm can
  be moved by clicking on the "indents", the arrows under the  numbers.
  The slider arm can also be moved by placing the mouse on  the slider
  shaft on either side of the slider indent and clicking. It can also be
  moved by clicking on the direction arrows at the right end of the slider
  shaft. The left and right arrow keyboard keys can also be used. The 
  home and end keys also work, however make sure the slider arm has the
  focus. You can do this by using the up or down arrow keys.

  You will notice a slight delay for the ASCII numbers that appear at
  the   dumb terminal when moving the slider arm. This delay was deliberately
  put in to keep the user from flooding the IOCTL commands with output
  by rapidly and constantly dragging the slider arm back and forth.


                           DEVELOPER NOTES:

OVERVIEW
--------

There are three task for this application. The main task runs all the
window stuff. The two remaining task are a "transmit" and "receive"
task used for sending and receiving data via the COM port and dumb
terminal.


SLIDCOM Module
--------------

This module contains the basic code to bring up and display the
window. It handles all the necessary window functions required. It 
also contains a number of common subroutines used by the other
modules.


SLIDDLG Module
--------------

This module contains all the code to handle the Dialog boxes used for
setting up the COM port, line initialization and the slider itself. Each
of these have its own dialog procedure.

The slider dialog code collects the slider position number and displays
it in the side window.  A  window timer function comes around every 750
msecs to pickup the latest value and sends it to the dumb terminal driver
task via a function call. 



PMCOMDRV  Module
----------------

This  module contains the I/O driver code and tasks for sending and
receiving data via COM port and dumb terminal.

The receive task collects a character at a time and upon receipt of <CR>
sends the data via  the "WinPostMsg" API. This is then picked up in the
Slider dialog code, (SLIDDLG module) via WM_TERMINAL_MSG and  displayed
at the slider.

For the transmit task, when the timer in SLIDDLG module comes around it
collects the data from the slider and via function call PrintTerm(void *ptr)
sends the message to the transmit task. The PrintTerm functions allocates
some shared memory, copies the data over and then does a DosWriteQueue.
The transmit task is pending on this queue. When it gets a message off
the queue it then sends it to the dumb terminal and deallocates the message
buffer.


This application was developed using the OS2 toolkit and C Set/2 tools


Regards,

Dale G. Herrig
72740,467

