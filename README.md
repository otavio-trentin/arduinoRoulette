# arduinoRoulette

The whole project was based in the video https://www.youtube.com/watch?v=oU-7Vgl9Ins

An arduino code that control 8 LEDs for a roulette system. Each LED will give a prize to the player. There is an inventory control the prize. At each prize given it is subtracted from the inventory, the probability of each LED to be selected is based on the inventory. I can update my inventory by a bluetooth serial connection.

In the eletronic project the arduino used is the Nano, the bluetooth module is HC05 (only works with Android, not iOS), the LED are 12V so an external power supply and transistors are used. (will be updated soon)

To update the inventory the app used in the Android is ....

The issues we faced in the usage is that after some time the LEDs brightness decrase, not sure if its the LED used or some ground problem.

The improvements can be:
  Save the inventory to EEPROM, so at restart it is not lost.
  Modify the bluetooth module to work with iOS.
  Use MUX to activate mode LEDs and use less Digital Output.
  Add a buzzer/sound, we remove from the project because it was annoying.
