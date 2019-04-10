# kvitochka
The smart pot for your plant

## A short  description
We created a smart spot, which can control the soil humidity by watering the plant if needed. Also, it maintains the lighting around the plant at the same level using the led strip. It allows you to change the color of light so that you can set up the one you like the most.

## Structure
Its body consists of several independent parts. They were soldered together to form a stable system. The main controller is STM32F411.
    - It gathers the data from photoresistor and soil humidity sensor using ADC. In the general case, the data from such sensors is unstable, so we need to normalize it. In our program, we take an array of values, then cast the highest and smallest ones and find the middle value of others.
    - It also measures the water level in the tank using the rangefinder, and when the level is close to zero, it can notify the user.
    - It sends all this data to ESP8266, on which we run a web server. The data is sent via the UART protocol and shown to the user on a web page.
    - We have a separate 12V power supply block for our lighting. It has four optocouplers (three of them to control the color and the last one for PWM). PWM is the pulse width modulation from STM32 which is dependent on the data from the photoresistor. It allows us to change the LED strip brightness gradually according to the lighting level around the pot.
    - Also, the data is shown on the LCD, which we connected via the I2C protocol to the STM32. It updates the screen one time per second. After the user turns on the pot, the display shows the IP-address of the esp module for ten seconds, so the user can type it in the browser and open the webpage.
    - The water pump is fully isolated inside the pot. When the ground is dry, the pump waters it for a few seconds and then stm32 measures the ground humidity again. If it is on the appropriate level, the pump turns off.

## Security system.
It has an independent safety system, which controls how long the water pump has been working. The separate controller (Attiny13) measures that time and controls three relay modules. If the measured period was too long, it turns off everything. Such a system guarantees that even if something went wrong, our pot would not flood itself.
Also, we set up the watchdog timer on stm32. The main principle of its work is to update (drop to 0) its value in the system every 2 seconds. If the system is stuck, the timer will not reset, and watchdog will reload stm32. It may be useful when some lines of data transfer protocols get temporarily disconnected.


## Some pictures of our team.

![Team members](https://i.ibb.co/v3n1vKJ/image.jpg)

![Marusia](https://i.ibb.co/TBt0T03/1.jpg)

![Display](https://i.ibb.co/xmZVWZw/2.jpg)

![Shemas](https://i.ibb.co/0CfDMM1/3.jpg)
