# Astro Alarm

## Presentation

Goal of the Astro Alarm :
- Include a silent alarm, connected to a Astro AlarmClient which receive notification if the mount is moved
- Provide mount angles to help during installation phase

**Astro Alarm Server** and **Astro Alarm Client** are based on the same hardware and the same software. The only difference, is the inclinometer sensor connected to the **Astro Alarm Server** but not on the **Astro Alarm Client**, and the Buzzer on the **Astro Alarm Client**.

We need this material :
- LilyGo T-Display S3
- 3.7V Lipo battery (optional)
- HWT906 sensor (**Astro Alarm Server** only)
- Buzzer (**Astro Alarm Client** only)


---
## Hardware
### Astro Alarm Server
![](README/image.png)

Cable connection :
| Type | Pin  | Color |
|:----:|:----:|:-----:|
| 3.3V | 3.3V | brown |
| GND  | GND  | black |
| RX   | 18   | blue  |
### Astro Alarm Client
![](README/image%202.png)

Buzzer connection :
| Type | Pin |
|:----:|:---:|
| +    | 16  |

---
## Arduino IDE

### Settings
**Board package** : esp32 par Espressif  
**Board** : LilyGo T-Display-S3

### Packages
You need these libraries :
- Hardware Buttons (with the IDE)
- TFT_eSPI (manual installation)
  **1-** Download official board documents : [link](https://github.com/Xinyuan-LilyGO/T-Display-S3)  
  **2-** Do a zip file of the **lib/TFT_eSPI** folder  
  **3-** Add to the Arduino libraries  
  **4-** Because there is a problem with the TFT library, you must replace the **User_Setup_Select.h** file in the library folder by the file provided with the Astro Alarm repo.

### Programmation
If you have an error when flashing the board, just unplug USB-C, push the bottom button and plug USB-C.


---
## Arduino Software

Before to build the software, you must choose if you would like to build the software in **server** or **client** mode.
To do that, just set the variable boardMode in the setup function :

```
  //------------------------------------------------------------------------------------
  // TO SET BEFORE TO COMPILE THE SOFWARE : BOARD_MODE_SERVER | BOARD_MODE_CLIENT
  //------------------------------------------------------------------------------------
  boardMode = BOARD_MODE_SERVER;
```

### Button usage
On the server side, buttons are not used
On the client side, you can use the top button like this :
- Long push : switch the alarm mode (on -> off | stop-alert -> on -> …)
