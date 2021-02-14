# NextionX
A new, alternative and universal library to interact with Nextion HMI displays from Arduino and compatible MCUs.
## Foreword
This library is not in concurrence with the [official Nextion library](https://github.com/itead/ITEADLIB_Arduino_Nextion). It has been created and is still growing as a result of me teaching the basics of interaction between Arduino and compatible MCUs in my Nextion Sunday Blog. The following articles allow you to follow the story from its beginning:
* [The Sunday Blog: Talking to your Nextion HMI – Part 7: Time to wrap things in classes](https://nextion.tech/2020/12/14/the-sunday-blog-talking-to-your-nextion-hmi-part-7-time-to-wrap-things-in-classes/)
* [The Sunday Blog: Talking to your Nextion HMI – Part 8: A practical example with the new class](https://nextion.tech/2020/12/21/the-sunday-blog-talking-to-your-nextion-hmi-part-8-a-practical-example-with-the-new-class/)
* [The Sunday Blog: Talking to your Nextion HMI – Part 9: Let’s create a universal component class](https://nextion.tech/2020/12/28/the-sunday-blog-talking-to-your-nextion-hmi-part-9-lets-create-a-universal-component-class/)
For new examples using version v0.7.0:
* [The Sunday Blog: The alternative NextionX library does now listen to your Nextion HMI](https://nextion.tech/2021/02/15/the-sunday-blog-the-alternative-nextionx-library-does-now-listen-to-your-nextion-hmi/)
* more to come...
## General information
To be most universal, this library allows (in opposite to the official library) the use of multiple Nextion HMI displays connected to the same MCU under the condition to have enough hardware or software emulated serial ports (UARTs). 

On an Arduino MEGA, you could for example use the Serial1, Serial2 and Serial3 ports to connect up to 3 Nextion HMIs, while keeping the default Serial port free for debugging in the Serial Monitor of the Arduino IDE. On an Arduino UNO, you might use either the Nextion on Serial and no debugging or the Nextion on a SoftwareSerial port and use Serial for debugging.

Compared to the published code in the above Blog posts, the usage of the library has again be simplified due to heavy code reworking before publishing it here.
## Technical information
The newest release, v.0.7.0 brings some **elementary API changes** to ease the use. Function macros and separate objects for use with or without debugging as in v0.5.0 are not longer needed! Please refer to the updated reference, below.
## Versions
### v.0.7.0
* This version does now listen to the return data from the Nextion. This translates in detailed feedback if debugging is enabled and in the new functionality which permits adding onTouch and onRelease callback handlers for individual components.
* There is no longer a separate object for debugging. The same **NexComm** object can output debug, error and status messages when the `.addDebug()` method is invoked after `.begin()` (see Reference below)
* No separate event listener list (as in the official Nextion library) to declare. As soon as you invoke either `setOnTouch()` and/or `.setOnRelease()`methods on a **NexComp** object, it is automatically added to the (internal) listener list of the corresponding **NexComm** object, if it isn't already there. As a compromise between flexibility and memory usage, the internal listener list of each **NexComm** object can hold 16 active components in the form of a ring buffer. The latter means that if you add for example a 17th component, it will deactivate the very first declared by overwriting it. This allows to dynamically update the listener list at runtime. In this release, the number of 16 active components at a time is graved in stone. A further relase will allow a user defined size.
### v.0.5.0
It does things actually completely, but one-way, which means that you can send commands to the Nextion and change all attributes of all components. With the time and further Blog posts, the other way round, listening to Nextion return codes and querying component attributes will be added which will make the version number increase consecutively.
## Reference
Everything will start with including the library `#include <NextionX.h>`. Then, one or more NexComm objects need be created, one for each connected HMI display. Finally, you'll have to declare the GUI components your MCU code needs to interact with.
### NexComm() *constructor*
... is the direct link between your Arduino code and your Nextion HMI. It is declared before the components and before `setup()` just by adding `NexComm objName();` for each *Nextion HMI display* connected to one of the serial ports of your MCU. Example: `NexComm Nex1();`.
#### NexComm.begin(nex_port, nex_baudrate) *method*
Initializes the nex_port with nex_baudrate and clears all buffers. If nex_baudrate is omitted, it will default to 9600 baud. It is recommended to leave the default Serial port free for debugging and to use other (preferrably hardware) UART(s) of your MCU to connect your Nextion(s).  Example for Arduino Mega: `Nex1.begin(Serial1, 115200)` - please make sure that the bauds or baud system variables of the Nextion HMI match this value. The "bkcmd=0" command is sent to the Nextion to minimize the traffic on the serial lines.
#### NexComm.addDebug(dbg_port, dbg_baudrate) *method*
Enables the debug mode of the **NexComm** object (after the .begin() method had been invoked), and initializes the dbg_port with dbg_baudrate. If nex_baudrate is omitted, it will default to 9600 baud. Since enabling the debug mode of a **NexComm** object will consume about ~1kb more flash memory, you should only use it during development and testing, but not in your final code. The debug mode is NOT required for normal event listening! Example for Arduino Mega: `Nex1.addDebug(Serial, 57600)`. The "bkcmd=3" command is sent to the Nextion to obtain the most qualified status and error returns.
#### NexComm.cmdWrite(nex_commandString) *method*
Sends the nex_commandString to the HMI "as is" and adds the 3 byte 0xFF terminator at the end. Example: `Nex1.cmdWrite("sys0=4711")`. If the debug mode is enabled, the command String will be echoed of the debug port, followed by either "Success" or a qualified error code, corresponding to the return codes documented in the Nextion Instruction Set, like "Error 1a" which means "Invalid variable".
#### NexComm.loop() *method*
For each declared **NexComm** object, you'll have to add its `.loop()` method at the beginning of your main `loop()` to handle incoming events and - if debug mode is active - error and status returns from your Nextion HMI. 
****
### NexComp(NexComm_obj, page_id, obj_id) *constructor*
Declaring GUI components is as easy as writing `myNexComp myNum(Nex1, 0, 1)` which addresses the component with the id 1 on page 0 of the HMI connected to the Nex1 **NexComm** object. Each declared component eats a few bytes of the MCU memory, thus, please take care to declare only those components which need to interact in one or the other way with the MCU.
#### NexComp.setAttr(attribute, value) *method*
The `.setAttr()` method is the universal method to modify the content or value of a GUI component's whatever (writable at runtime) attribute. Changing the displayed value of a number field is as easy as `myNum.setAttr("val", 4711)` or, to change the color, `myNum.setAttr("pco", 1024)`. This works not only for numeric values, but also for text: `myTxt.setAttr("txt", "Ready!")` will change the displayed text of a Text component (within the limits of the txt_maxlength attribute).
#### NexComp.setOnTouch(touch_callback_function) *method*
Attaches the touch_callback_function (has to be declared before in your sketch) to the internal touch event handler and adds the component - if it isn't already there - to the internal listen list. Example: `b0.setOnTouch(LED_on);`.
#### NexComp.setOnRelease(release_callback_function) *method*
Attaches the release_callback_function (has to be declared before in your sketch) to the internal release event handler and adds the component - if it isn't already there - to the internal listen list. Example: `b0.setOnRelease(LED_ff);`. 
**Note:** During runtime, the `setOnTouch()` and `setOnRelease()` methods might be invoked repeatedly, either to attach a different callback function to a component, or with empty parameter to detach the previous callbck function. 

## Example(s)
For example code and the corresponding HMI project, please refer to this blog article: [The Sunday Blog: The alternative NextionX library does now listen to your Nextion HMI](https://nextion.tech/2021/02/15/the-sunday-blog-the-alternative-nextionx-library-does-now-listen-to-your-nextion-hmi/).
