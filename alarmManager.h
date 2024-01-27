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
  struct strAlarmData alarmData;
  
  
public:
  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Constructor
  /*-------------------------------------------------------------------------------------------------------------------*/
  AlarmManager (void)
  {
    alarmData.alarmStatus   = ALARM_STATUS_NOT_TRIGGERED;
    alarmData.alarmState    = ALARM_STATE_OFF;
    alarmData.XaccInit      = 0.0;
    alarmData.YaccInit      = 0.0;
    alarmData.ZaccInit      = 0.0;
    alarmData.XaccCurrent   = 0.0;
    alarmData.YaccCurrent   = 0.0;
    alarmData.ZaccCurrent   = 0.0;
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Switch the alarm state : ON -> OFF -> ON -> ...
  /*-------------------------------------------------------------------------------------------------------------------*/
  void switch_state (void)
  {
    if ((alarmData.alarmState == ALARM_STATE_ON) || (alarmData.alarmState == ALARM_STATE_LOCKED))
    {
      alarmData.alarmState  = ALARM_STATE_OFF;
      alarmData.alarmStatus = ALARM_STATUS_NOT_TRIGGERED;
    }
    else
    {
      alarmData.alarmState  = ALARM_STATE_ENABLING;
    }
  }
  
  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Update the alarm state
  // @param _Xacc : acceleration on X
  // @param _Yacc : acceleration on Y
  // @param _Zacc : acceleration on Z
  // @return strAlarmData data
  /*-------------------------------------------------------------------------------------------------------------------*/
  struct strAlarmData update (double _Xacc, double _Yacc, double _Zacc)
  {
    double margin = 1.0;

    // When we enable alarm, we record current inclinometer data to display these when alarm is triggered
    if (alarmData.alarmState == ALARM_STATE_ENABLING)
    {
      alarmData.XaccInit      = _Xacc;
      alarmData.YaccInit      = _Yacc;
      alarmData.ZaccInit      = _Zacc;
      alarmData.alarmState    = ALARM_STATE_ON;
    }

    alarmData.XaccCurrent   = _Xacc;
    alarmData.YaccCurrent   = _Yacc;
    alarmData.ZaccCurrent   = _Zacc;

    // If alarm is enabled
    if (alarmData.alarmState == ALARM_STATE_ON)
    {
      // Check trigger
      if ((alarmData.alarmState == ALARM_STATE_LOCKED) 
       || ((!is_in_range(alarmData.XaccInit, alarmData.XaccCurrent, margin) 
         || !is_in_range(alarmData.YaccInit, alarmData.YaccCurrent, margin) 
         || !is_in_range(alarmData.ZaccInit, alarmData.ZaccCurrent, margin))))
      {
        alarmData.alarmState  = ALARM_STATE_LOCKED;
        alarmData.alarmStatus = ALARM_STATUS_TRIGGERED;
      }
      else
      {
        alarmData.alarmStatus = ALARM_STATUS_NOT_TRIGGERED;
      }
    }
    
    return alarmData;
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

