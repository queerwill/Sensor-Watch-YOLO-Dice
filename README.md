Controls
--------

There are three main game screens 


| **BUTTON (HOLD)** | **LIGHT**      | **MODE**                 | **ALARM**      |
| :-----:           | :---------:    | :-------:                | :-----------:  |
| **Roll Screen**   | Move Selection | Roll Dice (Jump to Menu) | Hold/Unhold (Jump to Score) |
| **Score Screen**  | Move Selection | Go Back (Jump to Menu)   | Select |
| **Menu Screen**   | Go Home        | Go Back                  | (Hold to reset) |



RULES
-----

The game consists of a number of rounds. In each round, a player gets three rolls of the dice, although they can choose to end their turn after one or two rolls. After the first roll the player can save any dice they want and re-roll the other dice. This procedure is repeated after the second roll. The player has complete choice as to which dice to roll. It is possible to re-roll both dice that were or were not rolled before.

YOLO contains 13 different category boxes and in each round, after the third roll, the player must choose one of these categories. The score entered in the box depends on how well the five dice match the scoring rule for the category. Details of the scoring rules for each category are given below. As an example, one of the categories is called Set of Three. The scoring rule for this category means that a player only scores if at least three of the five dice are the same value. The game is completed after 13 rounds by each player, with each of the 13 boxes filled. The total score is calculated by summing all thirteen boxes, together with any bonuses.

YOLO contains 13 scoring categorie divided into upper and lower sections.

Upper section
-------------

In the upper section there are six categories. The score in each of these categories is determined by adding the total number of dice matching that category.

| **Category** | **Description** | **Score**                         | **Example**    | 
| :-----:      | :---------:     | :-------------:                   | :-----------:  |
| **ACES**     | Any combination | The sum of dice with the number 1 | 11134 scores 3 |
| **TWOS**     | Any combination | The sum of dice with the number 2 | 22256 scores 6 |
| **THREES**   | Any combination | The sum of dice with the number 3 | 33334 scores 12| 
| **FOURS**    | Any combination | The sum of dice with the number 4 | 44555 scores 8 |
| **FIVES**    | Any combination | The sum of dice with the number 5 | 11225 scores 5 |
| **SIXES**    | Any combination | The sum of dice with the number 6 | 33666 scores 18| 

If a player scores a total of 63 or more points in these six categories, a bonus of 24 is added to the bonus score and the player earns a bonus round at the end of the game.

Lower section
-------------

| **Category**     | **Description**              | **Score**                         | **Example**    | 
| :-----:          | :---------:                  | :-------------:                   | :-----------:  |
| **SET OF THREE** | At least three dice the same | Sum of all dice                   | 23444 scores 17 |
| **SET OF FOUR**  | At least four dice the same  | Sum of all dice                   | 45555 scores 24 |
| **FULL HOUSE**   | Three of one number and two of another | 25                      | 22555 scores 25| 
| **SHORT RUN**    | Four sequential dice         | 30                                | 13456 scores 30 |
| **LONG RUN**     | Five sequential dice         | 40                                | 12345 scores 40 |
| **CHOICE**       | All five dice the same       | Sum of all dice                   | 11345 scores 14 | 
| **YOLO**         | Any combination              | 50 | 33333 scores 50              | 


If a category is chosen but the dice do not match the requirements of the category the player scores 0 in that category. 

If the player rolls a YOLO and selects the YOLO category, they will earn a bonus roll at the end of the game. 50 points and a bonus round will be added for each additional category selected with a YOLO.

Bonus Rounds
------------

In each bonus round, the player gets three rolls of the dice, the same as is the standard rounds above. The player earns points for the highest scoring category (excluding Choice) that can be claimed with the rolled combination.


Try out the demo on the emulator:

[PLAY YOLO](https://queerwill.github.io/)



The Sensor Watch
================

The [Sensor Watch](https://www.sensorwatch.net) is a board replacement for the classic Casio F-91W wristwatch. It is powered by a Microchip SAM L22 microcontroller with built-in segment LCD controller. You can write your own programs for the watch using the provided watch library, program the watch over USB using the built-in UF2 bootloader, and then install the board in your existing watch case to run your own software on your wrist.

![image](/images/sensor-watch.jpg)

Features:
* ARM Cortex M0+ microcontroller
* 32KHz crystal for real-time clock with alarm function
* Ten digit segment LCD, plus five indicator segments
* Three interrupt capable buttons
* Red / green PWM’able LED backlight
* Optional piezo buzzer (requires some light soldering)
* On-board USB Micro B connector
* Reset button with double-tap UF2 bootloader
* Nine-pin flex PCB connector

![image](/images/sensor-board.png)

You may have noticed that there are no sensors on this board. That is by design: rather than pick sensors for you, the goal is to add a tiny flexible PCB with the sensors YOU want, and interface them over the nine-pin connector. The connector provides the following options for power and connectivity:

* 3V power (nominal voltage from a CR2016 coin cell, can drop to ~2.7V)
* An I²C interface with built-in pull-up resistors
* Five general purpose IO pins, which can be configured as:
    * Five analog inputs
    * Five interrupt-capable digital inputs, with internal pull-up or pull-down resistors
    * Five digital outputs
    * SPI controller (with one spare analog / GPIO pin leftover)
    * One UART TX/RX pair (with three GPIO leftover)
    * Up to four PWM pins on two independent TC instances
    * Two external wake inputs that can wake from the ultra-low-power BACKUP mode

| **Pin** | **Digital** | **Interrupt**   | **Analog**    | **I2C**             | **SPI**              | **UART**                 | **PWM**  | **Ext. Wake** |
| :-----: | :---------: | :-------------: | :-----------: | :-----------------: | :------------------: | :----------------------: | :------: | :-----------: |
| **A0**  | PB04        | EIC/EXTINT\[4\] | ADC/AIN\[12\] | —                   | —                    | —                        | —        | —             |
| **SCL** | —           | —               | —             | SCL<br>SERCOM1\[1\] | —                    | —                        | —        | —             |
| **SDA** | —           | —               | —             | SDA<br>SERCOM1\[0\] | —                    | —                        | —        | —             |
| **A1**  | PB01        | EIC/EXTINT\[1\] | ADC/AIN\[9\]  | —                   | SCK<br>SERCOM3\[3\]  | RX<br>SERCOM3\[3\]       | TC3\[1\] | —             |
| **A2**  | PB02        | EIC/EXTINT\[2\] | ADC/AIN\[10\] | —                   | MOSI<br>SERCOM3\[0\] | TX or RX<br>SERCOM3\[0\] | TC2\[0\] | RTC/IN\[1\]   |
| **A3**  | PB03        | EIC/EXTINT\[3\] | ADC/AIN\[11\] | —                   | CS<br>SERCOM3\[1\]   | RX<br>SERCOM3\[1\]       | TC2\[1\] | —             |
| **A4**  | PB00        | EIC/EXTINT\[0\] | ADC/AIN\[8\]  | —                   | MISO<br>SERCOM3\[2\] | TX or RX<br>SERCOM3\[2\] | TC3\[0\] | RTC/IN\[0\]   |

These tiny “sensor boards” have a set outline, and the available area for your electronics is quite small (5.7 × 5.7 × 1 mm). Still, this is plenty of room for an environmental sensor, MEMS accelerometer or magnetometer and a couple of decoupling capacitors. Note that you will likely be limited to QFN and LGA type parts; SOICs are too large, and even SSOP packages are generally too thick. You can find reference designs for several sensor boards in the `PCB/Sensor Boards` directory within this repository.

Getting code on the watch
-------------------------
The watch library in this repository is very work-in-progress, but it should allow you to get started. To create a new project, copy the “starter-project” folder in the apps folder, and write your code in the app.c file.

You will need to install [the GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads/) to build projects for the watch. The watch library has been tested with the `9-2019-q4-major` version and the `10.3-2021.07` versions. If you're using Debian or Ubuntu, it should be sufficient to `apt install gcc-arm-none-eabi`.

To build your project, open your terminal and navigate to the project's `make` folder, then type `make`.

To install the project onto your Sensor Watch board, plug the watch into your USB port and double tap the tiny Reset button on the back of the board. You should see the LED light up red and begin pulsing. (If it does not, make sure you didn’t plug the board in upside down). Once you see the “WATCHBOOT” drive appear on your desktop, type `make install`. This will convert your compiled program to a UF2 file, and copy it over to the watch.

Using the Movement framework
----------------------------
If you just want to make minor modifications and use existing code, start with the `movement` directory. You can build the default watch firmware with:

```
cd movement/make
make
```

Then copy `movement/make/build/watch.uf2` to your watch. If you'd like to modify which faces are built, see `movement_config.h`.

You may want to test out changes in the emulator first. To do this, you'll need to install [emscripten](https://emscripten.org/), then run:

```
cd movement/make
emmake make
python3 -m http.server -d build-sim
```

Finally, visit [watch.html](http://localhost:8000/watch.html) to see your work.

License
-------
Different components of the project are licensed differently, see [LICENSE.md](https://github.com/joeycastillo/Sensor-Watch/blob/main/LICENSE.md).
