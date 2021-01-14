# NextionX
A new, alternative and universal library to interact with Nextion HMI displays from Arduino and compatible MCUs.
## Foreword
This library is not in concurrence with the [official Nextion library](https://github.com/itead/ITEADLIB_Arduino_Nextion). It has been created and is still growing as a result of me teaching the basics of interaction between Arduino and compatible MCUs in my Nextion Sunday Blog. The following articles allow you to follow the story from its beginning:
* [The Sunday Blog: Talking to your Nextion HMI – Part 7: Time to wrap things in classes](https://nextion.tech/2020/12/14/the-sunday-blog-talking-to-your-nextion-hmi-part-7-time-to-wrap-things-in-classes/)
* [The Sunday Blog: Talking to your Nextion HMI – Part 8: A practical example with the new class](https://nextion.tech/2020/12/21/the-sunday-blog-talking-to-your-nextion-hmi-part-8-a-practical-example-with-the-new-class/)
* [The Sunday Blog: Talking to your Nextion HMI – Part 9: Let’s create a universal component class](https://nextion.tech/2020/12/28/the-sunday-blog-talking-to-your-nextion-hmi-part-9-lets-create-a-universal-component-class/)
* more to come...
## General information
To be most universal, this library allows (in opposite to the official library) the use of multiple Nextion HMI displays connected to the same MCU under the condition to have enough hardware or software emulated serial ports (UARTs). 

On an Arduino MEGA, you could for example use the Serial1, Serial2 and Serial3 ports to connect up to 3 Nextion HMIs, while keeping the default Serial port free for debugging in the Serial Monitor of the Arduino IDE. On an Arduino UNO, you might use either the Nextion on Serial and no debugging or the Nextion on a SoftwareSerial port and use Serial for debugging.

Compared to the published code in the Blog posts, the usage of the library has again be simplified due to heavy code reworking before publishing it here.
## Technical information
As you can read in the aforementionned Blog posts, using different types of serial ports (hardware, software or even virtual USB emulated) requires either fiddling with #define tags in the library header files (as in the official library) or the heavy use of template classes (what this library does). But don't be afraid, as you can see in the reference below, you'll not have to declare objects with template parameters! Although the Arduino toolchain uses still the C/C++11 compiler which does not yet allow to type classes as "auto", a few small but efficient function macros have been added which do all the typedef-ing and type identifying with decltype() for you.
## Versions
The current version is 0.5.0 which means it is about half ready. It does things actually completely, but one-way, which means that you can send commands to the Nextion and change all attributes of all components. With the time and further Blog posts, the other way round, listening to Nextion return codes and querying component attributes will be added which will make the version number increase consecutively.
## Reference
Everything will start with including the library `#include <NextionX.h>`. Then, one or more NexComm objects need be created, one for each connected HMI display. Finally, you'll have to declare the GUI components your MCU code needs to interact with.
### The NexCom object
... is the direct link between your Arduino code and your Nextion HMI. Talking to your Nextion in code means interacting with the NexCom object which is a wrapper for all the communication over the serial port to which your Nextion HMI is hooked up. No debug info will be sent. Declaring and instantiating it is somewhat tricky because of the template parameters, that's why there is the following `initComm()` function makro which does all the work for you as follows:
#### initComm(nex_port, tag) *function macro*
Creates a single Nextion connexion on the selected serial port, named 'tagComm' and declares the 'tagComp' component class for future use. Example for Arduino UNO: `initComm(Serial, Nex1)` will thus create the connexion object *Nex1*Comm (wired to Serial) and create the object type *Nex1*Comp for associated components which can be declared afterwards. initComm must be called before `setup()` and before declaring associated GUI components in your Arduino Sketch, but in case you want to use a software emulated serial port, that one has logically to be declared before.
#### NexCom.begin(nex_baudrate) *method*
Initializes the nex_port with nex_baudrate and clears all buffers. If nex_baudrate is omitted, it will default to 9600 baud. Example: `Nex1Comm.begin(115200)` - please make sure that the bauds or baud system variables of the Nextion HMI match this value.
#### NexCom.cmdWrite(nex_commandString) *method*
Sends the nex_commandString to the HMI "as is" and adds the 3 byte 0xFF terminator at the end. Example: `Nex1Comm.cmdWrite("sys0=4711")`
****
Now, if you want the same but with debugging information over a second serial port, you use the following object instead. Because of the additional functionality, it has a slightly bigger memory footprint. This object is primarily intended for developing and testing. Afterwards, it should be replaced by the NexCom object without debugging named with an identical tag. You'll then just need to adapt the parameters in `.begin()` and remove all code calls to `.dbgEnable()`. The derived NexComp objects remain unchanged.
### The NexComDbg object
... is the direct link between your Arduino code, your Nextion HMI and a serial monitor on a second serial port for debugging. Talking to your Nextion in code means interacting with the NexCom object which is a wrapper for all the communication over the serial port to which your Nextion HMI is hooked up. In parallel, debug info will be sent over the second port. Declaring and instantiating it is somewhat tricky because of the template parameters, that's why there is the following `initCommDbg()` function makro which does all the work for you as follows:
#### initCommDbg(nex_port, debug_port, tag) *function macro*
Creates a single Nextion connexion on the selected port, named 'tagComm' with debugging ability over debug_port and declares the 'tagComp' component class for future use. Example for Arduino MEGA: `initCommDbg(Serial1, Serial, Nex1)` will create the connexion object *Nex1*Comm (wired to Serial1) and create the type *Nex1*Comp for associated components which can be declared afterwards. Debug information is sent over Serial. If more than one Nextion HMI are hooked up to your MCU, these may share the same debug port since each debug message is preceded by the corresponding tag. Thus, a second Nextion on an Arduino MEGA would be declared as `initCommDbg(Serial2, Serial, Nex2)`.
#### NexComDbg.begin(nex_baudrate, debug_baudrate) *method*
Initializes the nex_port with nex_baudrate, the debug_port with debug_baudrate, and clears all buffers. If nex_baudrate or debug_baudrate is omitted, it will default to 9600 baud. Example: `Nex1Comm.begin(115200, 57600)` - please make sure that the baudrates of the Nextion HMI and the serial monitor match the corresponding values. After the serial ports will be initialized, you should see two lines, `Nex1    Dbg ready` and `Nex1    Com ready` in the serial debug monitor.
#### NexComDbg.cmdWrite(nex_commandString) *method*
Sends the nex_commandString to the HMI "as is" and adds the 3 byte 0xFF terminator at the end. Example: `Nex1Comm.cmdWrite("sys0=4711")`. In the serial debug monitor, you'll then find a line `Nex1    Cmd sent: sys0=4711`.
#### NexComDbg.dbgEnable(true/false) *method*
Debug messages are by default enabled for the NexComDbg object. To disable the debug messages temporarily, use `Nex1Comm.dbgEnable(false)` and to re-enable `Nex1Comm.dbgEnable(true)`
****
### The NexComp object
Creating a NexCom or a NexComDbg object (to taste) includes automatically the definition of the associated NexComp object type which will be named with the same tag. For example, `initComm(Serial, myNex)` creates not only the *myNex*Comm object, but declares also alreay the *myNex*Comp object type, so that GUI components can immediately be instantiated as follows and are automatically bound to to their corresponding *myNex*Comm object. 
#### NexComp(page_id, obj_id) *constructor*
Declaring GUI components is as easy as writing `myNexComp myNum(0, 1)` which addresses the component with the id 1 on page 0. Each declared component eats a few bytes of the MCU memory, thus please take care to declare only those components which need to communicate in one or the other way with the MCU.
#### NexComp.setAttr(attribute, value) *method*
The setAttr() method is the universal method to modify the content or value of a GUI component's whatever (writable at runtime) attribute. Changing the displayed value of a number field is as easy as `myNum.setAttr("val", 4711)` or, to change the color, `myNum.setAttr("pco", 1024)`. This works not only for numeric values, but also for text: `myTxt.setAttr("txt", "Ready!")` will change the displayed text of a Text component (within the limits of the txt_maxlength attribute). 
## Example
### Preparation of your Nextion HMI
In the Nextion Editor, create a new HMI project, add a font, and place a Number component named n0 on the first (single) page. Compile and and upload the generated tft file to your Nextion HMI
### The example sketch
```c++
#include <NextionX.h> // include the library

initComm(Serial, myNex); // create a NexCom object on the default Serial port (pins 0 and 1 on Arduino)

myNexComp myNum(0, 1); // declare the number component with the id 1 on page 0

uint8_t counter = 0; // declare and initialize a counter variable

void setup() {
  myNexComm.begin(9600); // initialize the communication with the Nextion HMI
}

void loop() {
  myNum.setAttr("val", counter); // display the current counter value on the Nextion HMI
  counter++; // increase the counter by one
  if(counter > 100) // check if we reached 100
  {
    counter = 0;  // reset the counter
  }
  delay(100); // wait a little before moving on
}
```
