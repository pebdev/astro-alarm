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
// Alarm state
#define ALARM_STATE_ON                  (0)
#define ALARM_STATE_OFF                 (1)
#define ALARM_STATE_ENABLING            (2)   // Internal state, do not use it
#define ALARM_STATE_LOCKED              (3)   // Internal state, do not use it

// Alarm status
#define ALARM_STATUS_NOT_TRIGGERED      (0)
#define ALARM_STATUS_TRIGGERED          (1)
#define ALARM_STATUS_WARNING            (2)

// TImeout
#define REFRESH_WARNING_TIMEOUT_MS      (10000)


/** S T R U C T S ****************************************************************************************************/
struct strAlarmData
{
  uint8_t alarmState;
  uint8_t alarmStatus;

  // Inititial acceleration values
  double XaccInit;
  double YaccInit;
  double ZaccInit;

  // Current acceleration values
  double XaccCurrent;
  double YaccCurrent;
  double ZaccCurrent;
};


/** A L A R M ********************************************************************************************************/
class AlarmManager
{
private:
  uint32_t refresh_timestamp_ms;
  struct strAlarmData alarmData;
  
  
public:
  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Constructor
  /*-------------------------------------------------------------------------------------------------------------------*/
  AlarmManager (void)
  {
    this->refresh_timestamp_ms    = 0;
    this->alarmData.alarmStatus   = ALARM_STATUS_NOT_TRIGGERED;
    this->alarmData.alarmState    = ALARM_STATE_OFF;
    this->alarmData.XaccInit      = 0.0;
    this->alarmData.YaccInit      = 0.0;
    this->alarmData.ZaccInit      = 0.0;
    this->alarmData.XaccCurrent   = 0.0;
    this->alarmData.YaccCurrent   = 0.0;
    this->alarmData.ZaccCurrent   = 0.0;
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Switch the alarm state : ON -> OFF -> ON -> ...
  // @return ALARM_STATE_OFF | ALARM_STATE_ENABLING
  /*-------------------------------------------------------------------------------------------------------------------*/
  uint8_t switch_state (void)
  {
    if ((this->alarmData.alarmState == ALARM_STATE_ON) || (this->alarmData.alarmState == ALARM_STATE_LOCKED))
    {
      this->alarmData.alarmState  = ALARM_STATE_OFF;
      this->alarmData.alarmStatus = ALARM_STATUS_NOT_TRIGGERED;
      Serial.println("ALARM : OFF");
    }
    else
    {
      this->alarmData.alarmState  = ALARM_STATE_ENABLING;
      Serial.println("ALARM : ON");
    }

    return this->alarmData.alarmState;
  }
  
  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Update the alarm state
  // @param _signal_lost : true if the root signal was lost, otherwise false
  // @param _Xacc : acceleration on X
  // @param _Yacc : acceleration on Y
  // @param _Zacc : acceleration on Z
  // @return strAlarmData data
  /*-------------------------------------------------------------------------------------------------------------------*/
  struct strAlarmData update (bool _signal_lost, double _Xacc, double _Yacc, double _Zacc)
  {
    double margin = 0.06;

    // When we enable alarm, we record current inclinometer data to display these when alarm is triggered
    if (this->alarmData.alarmState == ALARM_STATE_ENABLING)
    {
      this->alarmData.XaccInit      = _Xacc;
      this->alarmData.YaccInit      = _Yacc;
      this->alarmData.ZaccInit      = _Zacc;
      this->alarmData.alarmState    = ALARM_STATE_ON;
    }

    this->alarmData.XaccCurrent   = _Xacc;
    this->alarmData.YaccCurrent   = _Yacc;
    this->alarmData.ZaccCurrent   = _Zacc;

    // If alarm is enabled
    if (this->alarmData.alarmState == ALARM_STATE_ON)
    {
      // Signal is present, check data
      if (_signal_lost == false)
      {
        // Check trigger
        if ((this->alarmData.alarmState == ALARM_STATE_LOCKED) 
          || ((!this->is_in_range(this->alarmData.XaccInit, this->alarmData.XaccCurrent, margin) 
          ||   !this->is_in_range(this->alarmData.YaccInit, this->alarmData.YaccCurrent, margin) 
          ||   !this->is_in_range(this->alarmData.ZaccInit, this->alarmData.ZaccCurrent, margin))))
        {
          this->alarmData.alarmState  = ALARM_STATE_LOCKED;
          this->alarmData.alarmStatus = ALARM_STATUS_TRIGGERED;
          Serial.println("ALARM : TRIGGERED");
        }
        else
        {
          this->alarmData.alarmStatus = ALARM_STATUS_NOT_TRIGGERED;
        }

        this->refresh_timestamp_ms = millis();
      }
      // Signal was lost, notify user
      else
      {
        if ((millis()-this->refresh_timestamp_ms) > REFRESH_WARNING_TIMEOUT_MS)
        {
          this->refresh_timestamp_ms = millis();
          this->alarmData.alarmStatus = ALARM_STATUS_WARNING;
          Serial.println("ALARM : WARNING");
        }
        else
        {
          this->alarmData.alarmStatus = ALARM_STATUS_NOT_TRIGGERED;
        }
      }
    }
    
    return this->alarmData;
  }


private:
  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Check if two values are equals, with a margin
  // @param _value1 : value 1 to compare
  // @param _value2 : value 2 to compare
  // @param _margin : margin on the difference between both values
  // @return true | false
  /*-------------------------------------------------------------------------------------------------------------------*/
  bool is_in_range (double _value1, double _value2, double _margin)
  {
    bool retval = true;

    if (abs(_value1-_value2) >= _margin)
      retval = false;

    return retval;
  }
};

