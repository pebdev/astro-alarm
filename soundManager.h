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
//#define CONFIG_SOUND_ENABLED      (0)


/** S O U N D ********************************************************************************************************/
class SoundManager
{
private:
  uint8_t pin;


public:
  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Constructor
  // @param _pin : pin of the buzzer
  /*-------------------------------------------------------------------------------------------------------------------*/
  SoundManager (uint8_t _pin)
  {
    this->pin = _pin;

    #ifdef CONFIG_SOUND_ENABLED
    Serial.println("SOUND : ENABLED");
    pinMode(this->pin, OUTPUT);
    #endif
  }
  
  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Play a sound to notify that the system mode has changed
  /*-------------------------------------------------------------------------------------------------------------------*/
  void play_mode_change (void)
  {
    #ifdef CONFIG_SOUND_ENABLED
    tone(this->pin, 4000, 250);
    #endif

    delay(500);
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Play a sound alarm
  /*-------------------------------------------------------------------------------------------------------------------*/
  void play_alarm (void)
  {
    #ifdef CONFIG_SOUND_ENABLED
    const byte countNotes = 8;
    int frequences[countNotes] = {
      2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000
    };
    int durations[countNotes] = {
      300, 300, 300, 300, 300, 300, 300, 300
    };

    for (int i=0; i<countNotes; i++)
    {
      tone(this->pin, frequences[i], durations[i] * 2);
      delay(durations[i] * 2);
      noTone(this->pin);
    }
    #endif
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Stop the sound alarm
  /*-------------------------------------------------------------------------------------------------------------------*/
  void stop_alarm (void)
  {
    #ifdef CONFIG_SOUND_ENABLED
    noTone(this->pin);
    #endif
  }
};
