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


/** D E F I N E S ****************************************************************************************************/
// Gpio
#define GPIO_OUT_LCD_BACKLIGHT        (38)

// TFT state
#define TFT_STATE_NO_AUTO_SHUTDOWN    (-1)
#define TFT_STATE_OFF                 (0)
#define TFT_STATE_ON                  (1)

// Timeout
#define TIMER_OFF_SCREEN_TIMEOUT_MS   (120000)


/** S O U N D ********************************************************************************************************/
class TftManager
{
private:
  unsigned long timerOffScreen_ms;


public:
  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Constructor
  /*-------------------------------------------------------------------------------------------------------------------*/
  TftManager (void)
  {
    this->timerOffScreen_ms = TFT_STATE_NO_AUTO_SHUTDOWN;
  }
  
  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Enable the auto shutdown mode
  /*-------------------------------------------------------------------------------------------------------------------*/
  void enable_auto_shutdown (void)
  {
    if (this->timerOffScreen_ms == TFT_STATE_NO_AUTO_SHUTDOWN)
    {
      this->timerOffScreen_ms = millis();
      Serial.print("TFT : enable auto shutdown (");
      Serial.print(TIMER_OFF_SCREEN_TIMEOUT_MS/1000);
      Serial.println("sec)");
    }
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Disable the auto shutdown mode
  /*-------------------------------------------------------------------------------------------------------------------*/
  void disable_auto_shutdown (void)
  {
    this->timerOffScreen_ms = TFT_STATE_NO_AUTO_SHUTDOWN;
    Serial.println("TFT : disabled auto shutdown");
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Enable TFT backlight
  /*-------------------------------------------------------------------------------------------------------------------*/
  void enable (void)
  {
    digitalWrite(GPIO_OUT_LCD_BACKLIGHT, TFT_STATE_ON);
    this->timerOffScreen_ms = millis();
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Disable TFT backlight
  /*-------------------------------------------------------------------------------------------------------------------*/
  void disable (void)
  {
    digitalWrite(GPIO_OUT_LCD_BACKLIGHT, TFT_STATE_OFF);
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Switch state of the TFT backlight
  /*-------------------------------------------------------------------------------------------------------------------*/
  void switch_state (void)
  {
    // Current TFT state : OFF
    if (digitalRead(GPIO_OUT_LCD_BACKLIGHT) == TFT_STATE_OFF)
    {
      digitalWrite(GPIO_OUT_LCD_BACKLIGHT, TFT_STATE_ON);
      Serial.println("TFT : switch state to ON");

      if (this->timerOffScreen_ms != TFT_STATE_NO_AUTO_SHUTDOWN)
        this->timerOffScreen_ms = millis();
    }

    // Current TFT state : ON
    else
    {
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
      if ((millis()-this->timerOffScreen_ms) > TIMER_OFF_SCREEN_TIMEOUT_MS)
      {
        digitalWrite(GPIO_OUT_LCD_BACKLIGHT, TFT_STATE_OFF);
      }
    }
  }
};
