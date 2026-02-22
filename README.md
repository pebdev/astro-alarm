# Astro Alarm

## Presentation

**Goal of the Astro Alarm :**
- Include a silent alarm : Astro Alarm Server send data to the Astro Alarm Client which monitor the alarm status
- Inclinometer : provide mount angles to help user during installation phase

**Astro Alarm Server** and **Astro Alarm Client** are based on the same hardware and the same software. The only difference, is the inclinometer sensor connected to the **Astro Alarm Server** but not on the **Astro Alarm Client**, and the Buzzer on the **Astro Alarm Client**.

We need this material :
- (x2) LilyGo T-Display S3
- (x2) 3.7V Lipo battery (optional)
- (x1) HWT906 sensor (**Astro Alarm Server** only)
- (x1) Buzzer (**Astro Alarm Client** only)


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
|   Type   | Pin |
|:--------:|:---:|
| Buuzer + | 21  |

---
## Arduino IDE

### Settings
**Board package** : esp32 par Espressif  
**Board** : LilyGo T-Display-S3

### Packages
You need these libraries (to add with the IDE library manager):
- Hardware Buttons
- TFT_eSPI  
  *Note : because there is a problem with the TFT library, you must replace the **User_Setup_Select.h** file in the TFT_eSPI library folder by the file provided with the Astro Alarm repo.  
  If the library is updated, you must do it again.*

### Programmation
If you have an error when flashing the board, just unplug USB-C, push the bottom button and plug USB-C.


---
## Arduino Software

### Board mode
Server or Client mode is automatically selected by the software, the software is the same for both board.

### TFT Auto shutdown
Server board has a TFT auto shutdown mechanism after 10 minutes.  
Client board has a TFT auto shutdown mechanisl too, but only only when the alarm is enabled, after 2 minutes.  

TFT will be automatically switch to ON in these conditions :
- if the connection with the router or the server is lost
- if alarm is triggered (on Client board only)

### Button usage
#### Server side
- **short push** : switch the TFT blacklight state
- **long push** : memory function for angular values. Will update the memory at each short push

#### Client side
- **short push** : switch the TFT blacklight state
- **long push** : switch the alarm mode (on -> off | stop-alert -> off -> …)
