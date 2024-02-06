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
#include <vector>
#include "inclinometer.h"
#include "buttonManager.h"
#include "wifiManager.h"
#include "soundManager.h"
#include "drawerManager.h"
#include "alarmManager.h"


/** D E F I N E S ****************************************************************************************************/
// GPIO
#define GPIO_IN_BUTTON              (14)
#define GPIO_OUT_BUZZER             (16)

// Board modes
#define BOARD_MODE_UNKNOWN          (0)
#define BOARD_MODE_SERVER           (1)
#define BOARD_MODE_CLIENT           (2)

// Timers
#define TIMER_REFRESH_WIFI_DATA_MS  (200)
#define TIMER_IDENTIFY_BOARD_MS     (2000)


/** S T R U C T S ****************************************************************************************************/
struct strComData
{
  uint8_t error;
  struct strAngular incAngular;
  struct strAngularVelocity inclAngularVelocity;
  struct strAcceleration incAcceleration;
};


/** D E C L A R A T I O N S ******************************************************************************************/
// Board
uint8_t boardMode = BOARD_MODE_UNKNOWN;

// Devices
Inclinometer inclinometer = Inclinometer();
ButtonManager buttonMain  = ButtonManager(GPIO_IN_BUTTON);
WifiManager wifiMgr       = WifiManager();

// UI
DrawerManager drawerMgr   = DrawerManager();
SoundManager soundMgr     = SoundManager(GPIO_OUT_BUZZER);
AlarmManager alarmMgr     = AlarmManager();

// Timer
unsigned long timerToIdentifyBoard_ms = millis();

// Memory
struct strAngular incAngularMemory;


/** M A I N  F U N C T I O N S ***************************************************************************************/
void setup (void)
{
  // Debug connection
  Serial.begin(115200);

  // Uart for the inclinometer
  Serial1.begin(115200, SERIAL_8N1, 18, 17);  // RX2=GPIO18, TX2=GPIO17

  // Start wifi
  wifiMgr.start();

  // Initial value
  incAngularMemory.version = 0;
}

/*-------------------------------------------------------------------------------------------------------------------*/
void loop (void)
{
  uint8_t wifiAppStatus;
  struct strComData comData;

  // First loop, identify the board
  if (boardMode == BOARD_MODE_UNKNOWN)
  {
    if (identify_board () == BOARD_MODE_UNKNOWN)
      return;
  }  

  // Server mode
  if (boardMode == BOARD_MODE_SERVER)
  {
    // ------ Process inclinometer data --------
    inclinometer.process_data ();
    //inclinometer.show_data ();
    comData.incAcceleration     = inclinometer.get_acceleration_data();
    comData.inclAngularVelocity = inclinometer.get_angular_velocity_data();
    comData.incAngular          = inclinometer.get_angular_data();


    // ------ Read button state ------------------
    uint8_t buttonState = buttonMain.update();

    if (buttonState == BUTTON_SHORT_PUSH)
    {
      incAngularMemory = comData.incAngular;
    }
    else if (buttonState == BUTTON_LONG_PUSH)
    {
      incAngularMemory.version = 0; //use the version to know if we must display or not
    }
  

    // ------ Wifi management --------------------
    wifiAppStatus = wifiMgr.server_update();
    wifiMgr.send_data(network_prepare_data(comData), TIMER_REFRESH_WIFI_DATA_MS);


    // ------ Screen drawing ---------------------
    drawerMgr.draw_background();
    drawerMgr.draw_ping_status(wifiMgr.is_ping_received());
    drawerMgr.draw_wifi_status(get_color_from_wifi_status(wifiAppStatus));
    drawerMgr.draw_main_point(comData.incAngular.angle[0], comData.incAngular.angle[1]);
    drawerMgr.draw_inclinometer_values(comData.incAngular.angle[0], comData.incAngular.angle[1]);
    drawerMgr.draw_temperature_value(comData.incAcceleration.temperature);

    if (incAngularMemory.version > 0)
      drawerMgr.draw_memory_values(incAngularMemory.angle[0], incAngularMemory.angle[1]);

    // Delay according wifi activity
    if (wifiAppStatus == CONNECTION_STATUS_APP_DISCONNECTED)
      delay(100);
    else
      delay(50);
  }
  
  // Client mode
  if (boardMode == BOARD_MODE_CLIENT)
  {
    // ------ Read button state ------------------
    uint8_t buttonState = buttonMain.update();

    if (buttonState == BUTTON_LONG_PUSH)
    {
      soundMgr.play_mode_change();
      alarmMgr.switch_state();
    }


    // ------ Wifi management --------------------
    wifiAppStatus  = wifiMgr.client_update();
    comData = network_parse_data(wifiMgr.read_data(true));


    // ------ Screen drawing ---------------------
    drawerMgr.draw_background();
    drawerMgr.draw_ping_status(wifiMgr.is_ping_received());
    drawerMgr.draw_wifi_status(get_color_from_wifi_status(wifiAppStatus));
    drawerMgr.draw_main_point(comData.incAngular.angle[0], comData.incAngular.angle[1]);
    drawerMgr.draw_inclinometer_values(comData.incAngular.angle[0], comData.incAngular.angle[1]);
    drawerMgr.draw_temperature_value(comData.incAcceleration.temperature);


    // ------ Alarm update -----------------------
    struct strAlarmData alarmData = alarmMgr.update(comData.incAcceleration.acceleration[0], comData.incAcceleration.acceleration[1], comData.incAcceleration.acceleration[2]);
    if (alarmData.alarmStatus == ALARM_STATUS_TRIGGERED)
    {
      drawerMgr.draw_alarm_data(alarmData.XaccInit, alarmData.YaccInit, alarmData.ZaccInit,
                                alarmData.XaccCurrent, alarmData.YaccCurrent, alarmData.ZaccCurrent);
      soundMgr.play_alarm();
    }
    else
    {
      soundMgr.stop_alarm();
    }    
    drawerMgr.draw_alarm_state(get_color_from_alarm_state(alarmData.alarmState), get_text_from_alarm_state(alarmData.alarmState));

    // Delay according wifi activity
    if (wifiAppStatus == CONNECTION_STATUS_APP_DISCONNECTED)
      delay(100);
    else
      delay(50);
  }
}

/*-------------------------------------------------------------------------------------------------------------------*/
void serialEvent1 (void) 
{
  while (Serial1.available())
  {
    inclinometer.read(Serial1.read());
  }
}

/*-------------------------------------------------------------------------------------------------------------------*/
uint8_t identify_board (void)
{
  uint8_t retval = BOARD_MODE_UNKNOWN;

  // Indenty the board
  if (inclinometer.is_new_data_ready())
  {
    boardMode = BOARD_MODE_SERVER;
  }
  else
  {
    if ((millis()-timerToIdentifyBoard_ms)>TIMER_IDENTIFY_BOARD_MS)
      boardMode = BOARD_MODE_CLIENT;
  }

  if (boardMode != BOARD_MODE_UNKNOWN)
  {
    Serial.println("\n\n\n----------------------------------------------------------------------");
    Serial.print("BOARD MODE : ");

    if (boardMode == BOARD_MODE_SERVER)
      Serial.println("SERVER");
    else
      Serial.println("CLIENT");
    Serial.println("----------------------------------------------------------------------");
  }

  return retval;
}

/*-------------------------------------------------------------------------------------------------------------------*/
std::vector<String> extract_substring (String _input, char _separator)
{
  int16_t index      = 0;
  int16_t lastIndex  = 0;
  std::vector<String> retval;

  if (_input != "")
  {
    while (index != -1)
    {
      // Extract part of the string
      index = _input.indexOf(_separator, lastIndex);

      if (index != -1)
      {
        retval.push_back(_input.substring(lastIndex, index));
        lastIndex = index + 1;
      }
      else
      {
        retval.push_back(_input.substring(lastIndex));
      }
    }
  }

  return retval;
}

/*-------------------------------------------------------------------------------------------------------------------*/
String network_prepare_data (struct strComData _data)
{
  String retval = "isAlive=1;";

  retval += "Xac="+String(_data.incAcceleration.acceleration[0])+";";
  retval += "Yac="+String(_data.incAcceleration.acceleration[1])+";";
  retval += "Zac="+String(_data.incAcceleration.acceleration[2])+";";

  retval += "Xan="+String(_data.incAngular.angle[0])+";";
  retval += "Yan="+String(_data.incAngular.angle[1])+";";
  retval += "Zan="+String(_data.incAngular.angle[2])+";";

  retval += "Xve="+String(_data.inclAngularVelocity.velocity[0])+";";
  retval += "Yve="+String(_data.inclAngularVelocity.velocity[1])+";";
  retval += "Zve="+String(_data.inclAngularVelocity.velocity[2])+";";
  
  retval += "Tmp="+String(_data.incAcceleration.temperature);

  return retval;
}

/*-------------------------------------------------------------------------------------------------------------------*/
struct strComData network_parse_data (String _data)
{
  static struct strComData retval;
  
  if ((_data == "") || (_data.length() == 1))
  {
    retval.error = 1;
    return retval;
  }

  // Data initialization
  retval.error = 0;
  retval.incAcceleration.acceleration[0] = 0.0;
  retval.incAcceleration.acceleration[1] = 0.0;
  retval.incAcceleration.acceleration[2] = 0.0;
  retval.incAcceleration.temperature     = 0.0;
  retval.incAngular.angle[0]             = 0.0;
  retval.incAngular.angle[1]             = 0.0;
  retval.incAngular.angle[2]             = 0.0;
  retval.incAngular.version              = 0;
  retval.inclAngularVelocity.velocity[0] = 0.0;
  retval.inclAngularVelocity.velocity[1] = 0.0;
  retval.inclAngularVelocity.velocity[2] = 0.0;
  retval.inclAngularVelocity.voltage     = 0.0;

  std::vector<String> DataList = extract_substring(_data, ';');
  for (const auto& data : DataList)
  {
    std::vector<String> var = extract_substring(data, '=');

    if (var.size() < 2)
    {
      Serial.print("ERROR : invalid data from networkc => ");
      Serial.print("[");
      Serial.print(_data);
      Serial.print("] len=");
      Serial.println(_data.length());
      continue;
    }

    if (var[0] == "Xac")
      retval.incAcceleration.acceleration[0] = String(var[1]).toDouble();

    if (var[0] == "Yac")
      retval.incAcceleration.acceleration[1] = String(var[1]).toDouble();

    if (var[0] == "Zac")
      retval.incAcceleration.acceleration[2] = String(var[1]).toDouble();

    if (var[0] == "Xan")
      retval.incAngular.angle[0] = String(var[1]).toDouble();

    if (var[0] == "Yan")
      retval.incAngular.angle[1] = String(var[1]).toDouble();

    if (var[0] == "Zan")
      retval.incAngular.angle[2] = String(var[1]).toDouble();

    if (var[0] == "Xve")
      retval.inclAngularVelocity.velocity[0] = String(var[1]).toDouble();

    if (var[0] == "Yve")
      retval.inclAngularVelocity.velocity[1] = String(var[1]).toDouble();

    if (var[0] == "Zve")
      retval.inclAngularVelocity.velocity[2] = String(var[1]).toDouble();
    
    if (var[0] == "Tmp")
      retval.incAcceleration.temperature = String(var[1]).toDouble();
  }
    
  return retval;
}

/*-------------------------------------------------------------------------------------------------------------------*/
uint32_t get_color_from_wifi_status (uint8_t _status)
{
  uint32_t color = 0;

  switch (_status)
  {
    case CONNECTION_STATUS_APP_DISCONNECTED:
      color = TFT_RED;
      break;

    case CONNECTION_STATUS_APP_CONNECTING:
      color = TFT_ORANGE;
      break;
    
    case CONNECTION_STATUS_APP_CONNECTED:
      color = TFT_GREEN;
      break;
    
    default:
      color = TFT_DARKGREY;
      break;
  }

  return color;
}

/*-------------------------------------------------------------------------------------------------------------------*/
uint32_t get_color_from_alarm_state (uint8_t _state)
{
  uint32_t color = 0;

  switch (_state)
  {
    case ALARM_STATE_LOCKED:
      color = TFT_RED;
      break;

    case ALARM_STATE_OFF:
      color = TFT_ORANGE;
      break;
    
    case ALARM_STATE_ON:
      color = TFT_GREEN;
      break;
    
    default:
      color = TFT_DARKGREY;
      break;
  }

  return color;
}

/*-------------------------------------------------------------------------------------------------------------------*/
String get_text_from_alarm_state (uint8_t _state)
{
  String text = "";

  switch (_state)
  {
    case ALARM_STATE_LOCKED:
      text = "LOCKED";
      break;

    case ALARM_STATE_OFF:
      text = "DISABLED";
      break;
    
    case ALARM_STATE_ON:
      text = "ENABLED";
      break;
    
    default:
      text = "UNKNOWN";
      break;
  }

  return text;
}
