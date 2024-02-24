/*********************************************************************************************************************
 * Project : Astro Alarm
 * Author  : PEB <pebdev@lavache.com> 
 * Date    : 2024.01.18
 *********************************************************************************************************************
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *********************************************************************************************************************/


/** I N C L U D E S **************************************************************************************************/
#include <driver/ledc.h>


/** D E F I N E S ****************************************************************************************************/
// Gpio
#define GPIO_OUT_LCD_BACKLIGHT        (38)

// TFT state
#define TFT_STATE_NO_AUTO_SHUTDOWN    (-1)
#define TFT_STATE_OFF                 (0)
#define TFT_STATE_ON                  (1)

// Settings
#define TFT_BRIGHTNESS                (20)  //1-255


/** S O U N D ********************************************************************************************************/
class TftManager
{
private:
  uint8_t lcdState;
  unsigned long timeoutOffScreen_ms;
  unsigned long timerOffScreen_ms;


public:
  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Constructor
  /*-------------------------------------------------------------------------------------------------------------------*/
  TftManager (void)
  {
    // Turn on display power (needed when board can be powered with a battery)
    pinMode(15, OUTPUT);
    digitalWrite(15, HIGH);

    // Reduce brightness
    ledcSetup(1, 10000, 8);
    ledcAttachPin(GPIO_OUT_LCD_BACKLIGHT, 1);
    ledcWrite(1, TFT_BRIGHTNESS);

    lcdState = TFT_STATE_OFF;
    this->timeoutOffScreen_ms = 100000;
    this->timerOffScreen_ms   = TFT_STATE_NO_AUTO_SHUTDOWN;
  }
  
  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Enable the auto shutdown mode
  // @param _off_screen_timeout_ms : timeout to switch off the screen.
  /*-------------------------------------------------------------------------------------------------------------------*/
  void set_auto_shutdown_timeout (unsigned long _off_screen_timeout_ms)
  {
    this->timeoutOffScreen_ms = _off_screen_timeout_ms;
    Serial.println("TFT : set auto shutdown timeout ("+String(this->timeoutOffScreen_ms)+")");
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Enable the auto shutdown mode
  // @param _off_screen_timeout_ms : timeout to switch off the screen. 0 to keep the current value.
  /*-------------------------------------------------------------------------------------------------------------------*/
  void enable_auto_shutdown (unsigned long _off_screen_timeout_ms = 0)
  {
    if (_off_screen_timeout_ms != 0)
      this->set_auto_shutdown_timeout(_off_screen_timeout_ms);

    if (this->timerOffScreen_ms == TFT_STATE_NO_AUTO_SHUTDOWN)
    {
      this->timerOffScreen_ms = millis();
      Serial.print("TFT : enable auto shutdown (");
      Serial.print(this->timeoutOffScreen_ms/1000);
      Serial.println("sec)");
    }
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Disable the auto shutdown mode
  /*-------------------------------------------------------------------------------------------------------------------*/
  void disable_auto_shutdown (void)
  {
    if (this->timerOffScreen_ms != TFT_STATE_NO_AUTO_SHUTDOWN)
    {
      this->timerOffScreen_ms = TFT_STATE_NO_AUTO_SHUTDOWN;
      Serial.println("TFT : disabled auto shutdown");
    }
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Enable TFT backlight
  /*-------------------------------------------------------------------------------------------------------------------*/
  void enable (void)
  {
    if (lcdState == TFT_STATE_OFF)
    {
      lcdState = TFT_STATE_ON;
      digitalWrite(GPIO_OUT_LCD_BACKLIGHT, TFT_STATE_ON);
      ledcWrite(1, TFT_BRIGHTNESS);
      Serial.println("TFT : enabled");

      if (this->timerOffScreen_ms != TFT_STATE_NO_AUTO_SHUTDOWN)
      {
        this->timerOffScreen_ms = millis();
        Serial.println("TFT : reset off screen timer");
      }
    }
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Disable TFT backlight
  /*-------------------------------------------------------------------------------------------------------------------*/
  void disable (void)
  {
    if (lcdState == TFT_STATE_ON)
    {
      lcdState = TFT_STATE_OFF;
      digitalWrite(GPIO_OUT_LCD_BACKLIGHT, TFT_STATE_OFF);
      Serial.println("TFT : disabled");
    }
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Switch state of the TFT backlight
  /*-------------------------------------------------------------------------------------------------------------------*/
  void switch_state (void)
  {
    // Current TFT state : OFF
    if (digitalRead(GPIO_OUT_LCD_BACKLIGHT) == TFT_STATE_OFF)
    {
      lcdState = TFT_STATE_ON;
      digitalWrite(GPIO_OUT_LCD_BACKLIGHT, TFT_STATE_ON);
      ledcWrite(1, TFT_BRIGHTNESS);
      Serial.println("TFT : switch state to ON");

      if (this->timerOffScreen_ms != TFT_STATE_NO_AUTO_SHUTDOWN)
        this->timerOffScreen_ms = millis();
    }

    // Current TFT state : ON
    else
    {
      lcdState = TFT_STATE_OFF;
      digitalWrite(GPIO_OUT_LCD_BACKLIGHT, TFT_STATE_OFF);
      Serial.println("TFT : switch state to OFF");
    }
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Update state of the TFT backlight
  /*-------------------------------------------------------------------------------------------------------------------*/
  void update (void)
  {
    if (this->timerOffScreen_ms != TFT_STATE_NO_AUTO_SHUTDOWN)
    {
      // Power off screen if the timeout was reached
      if ((millis()-this->timerOffScreen_ms) > this->timeoutOffScreen_ms)
      {
        lcdState = TFT_STATE_OFF;
        digitalWrite(GPIO_OUT_LCD_BACKLIGHT, TFT_STATE_OFF);
      }
    }
  }
};
