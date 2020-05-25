# Mach4-Serial-Pendant
CNC Control Pendant for Mach4, connected through USB Serial
​There are surprisingly few Pendants (CNC Machine, remote controls) for Mach4 and even fewer DIY projects.  Pretty much the only one I found is on Instructables here: https://www.instructables.com/id/Mach4-Pendant/

While it looks OK - it doesn't fully work.  The method they've used to send data to Mach4 (Keystroke Injection) does not allow any return data.

This Pendant just plugs into a USB port.  It uses an Arduino Nano which the PC recognises as a Serial device.  It sends & receives the Pendant data to a script running inside Mach4.

The Pendant sends, for example, a button press, to Mach4.  Mach4 then sends the state of the thing that should have changed back to the pendant for display.  That way, you know that if the button on the penant has changed state, Mach4 has acted on the instruction.  That applies to the MPG Wheel too - the X, Y & Z positions shown are from Mach4.

The Pendant I linked above has another, more subtle problem.  It uses a Resistive Touch Screen.  If you've ever tried to use a Resistive screen with wet fingers, you'll know it doesn't work!  The moisture in your fingerprint continues to register as a finger!

The answer is to use a Capacitive Touch Screen.  Unfortunately, there are not many small, arduino friendly screens available!  The one I eventually bought is called an H35B-IC.  There are a few posts on line from people failing to make it work through poor documentation, librarys & software but with some help from the vendor on eBay, I did make it work.

Bill of Materials:

MPG Wheel.
Mine is a Fanuc A860-0203-T001 which often come up on eBay new & used for about £25.  There are plenty of other after market MPG wheels available.  Which type doesn't matter too much so long as it has Quadrature A B outputs and can run from 5V.

Arduino Nano.
Mine is a clone - and only cost £2 on eBay.  It doesn't seem to make much difference (other than supporting Arduino) whether it's genuine or a clone - I've not found one that didn't work as expected.

Capacitive Touch Screen H35B-IC
AliExpress: https://www.aliexpress.com/item/4000159575099.html?spm=a2g0o.productlist.0.0.3e5434e8tctHuh&algo_pvid=5334b10a-cfdb-43ac-b2d4-843669a1f964&algo_expid=5334b10a-cfdb-43ac-b2d4-843669a1f964-2&btsid=0ab6f83915904324836991389e70b1&ws_ab_test=searchweb0_0,searchweb201602_,searchweb201603_

Make sure you buy the capacitive 3.5" 480*320 Serial I2C version.  There's also resistive, UART and different size versions available.

The vendor will send you a link to the software & documentation.  The link they sent me is below, however I dout it will persist there very long.  If the vendor won't cooperate, message me & I'll send a copy if I still have it.

https://www.adrive.com/public/gtyuYs/0128058%202.8%203.5%204.3%205inch%20display%20%20i2c_package_english_v1.2.zip

Mini USB Lead.
One may well come with your Arduino Nano, but you will need a longer lead - probably 2-3m

How do I exchange Serial Data with Mach4?
For a different project, I wanted to be able to send Serial data from
Mach4 to a Servo Drive.  A guy, Mike, on the Mach Support forum provided the answer - and a great piece of example code: https://www.machsupport.com/forum/index.php?topic=42084.msg274524#msg274524

This worked like a charm!  (Thanks Mike, if you read this!). The first part of this puzzle was to see if I could send the positions of the Axis to a remote 'DRO' display.  That was surprisingly easy using Mike's example.


Setting up the H35B LCD Display
I think the display was intended as a 'poor-mans' HMI (Human Machine Interface) for controlling Machinery.

Unlike most other Arduino LCD Displays, this one has a processor on board which can support some basic functions such as displaying BMP image files, on screen buttons, gauges & stuff like that.  It comes pre-loaded with an example which shows off some of it's capabilities.  Unfortunately, when you use the Arduino Library supplied to interface with it, many of the functions just don't work!  One example is updating numbers, labels & text fields on screen - they don't update!  I worked around the issues and have attached the project file you need to upload to the LCD using the Visual LCD Studio software supplied.  It comes with an OK manual, so I'll not bore you with how to use it.

I've included the documentation and software for the H35B-IC display in the repository



