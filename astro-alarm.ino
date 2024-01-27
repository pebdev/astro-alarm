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

// Board modes
#define BOARD_MODE_SERVER           (0)
#define BOARD_MODE_CLIENT           (1)

// Network
#define TIMER_REFRESH_WIFI_DATA_MS  (10000)


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
uint8_t boardMode;

// Devices
Inclinometer inclinometer = Inclinometer();
ButtonManager buttonMain  = ButtonManager(GPIO_IN_BUTTON);
WifiManager wifiMgr       = WifiManager();

// UI
DrawerManager drawerMgr   = DrawerManager();
SoundManager soundMgr     = SoundManager();
AlarmManager alarmMgr     = AlarmManager();

// Timer
unsigned long timerToSendWifiData_ms = millis();


/** M A I N  F U N C T I O N S ***************************************************************************************/
void setup (void)
{
  //------------------------------------------------------------------------------------
  // TO SET BEFORE TO COMPILE THE SOFWARE : BOARD_MODE_SERVER | BOARD_MODE_CLIENT
  //------------------------------------------------------------------------------------
  boardMode = BOARD_MODE_SERVER;

  // Debug connection
  Serial.begin(115200);
  Serial.println("\n\n\n----------------------------------------------------------------------");
  Serial.print("BOARD MODE (");
  Serial.print(BOARD_MODE_SERVER);
  Serial.print("=BOARD_MODE_SERVER | ");
  Serial.print(BOARD_MODE_CLIENT);
  Serial.print("=BOARD_MODE_CLIENT) : ");
  Serial.println(boardMode);
  Serial.println("----------------------------------------------------------------------");

  // Uart for the inclinometer
  if (boardMode == BOARD_MODE_SERVER)
    Serial1.begin(115200, SERIAL_8N1, 18, 17);  // RX2=GPIO18, TX2=GPIO17

  // Start wifi
  wifiMgr.start();
}

/*-------------------------------------------------------------------------------------------------------------------*/
void loop (void)
{
  uint8_t wifiStatus;
  struct strComData comData;

  if (boardMode == BOARD_MODE_SERVER)
  {
    // ------ Process incliometer data ---------
    inclinometer.process_data ();
    inclinometer.show_data ();
    comData.incAcceleration     = inclinometer.get_acceleration_data();
    comData.inclAngularVelocity = inclinometer.get_angular_velocity_data();
    comData.incAngular          = inclinometer.get_angular_data();
  

    // ------ Wifi management --------------------
    wifiStatus = wifiMgr.server_update();
    if ((wifiStatus == CONNECTION_STATUS_CONNECTED) && ((millis()-timerToSendWifiData_ms) > TIMER_REFRESH_WIFI_DATA_MS))
    {
      timerToSendWifiData_ms = millis();
      wifiMgr.send_data(network_prepare_data(comData));
    }


    // ------ Screen drawing ---------------------
    drawerMgr.draw_background();
    drawerMgr.draw_main_point(comData.incAngular.angle[0], comData.incAngular.angle[1]);
    drawerMgr.draw_inclinometer_values(comData.incAngular.angle[0], comData.incAngular.angle[1]);
    drawerMgr.draw_wifi_status(get_color_from_wifi_status(wifiStatus));

    delay(200);
  }
  

  if (boardMode == BOARD_MODE_CLIENT)
  {
    // ------ Read button state ------------------
    uint8_t buttonState = buttonMain.update();

    if (buttonState == BUTTON_LONG_PUSH)
      alarmMgr.switch_state();


    // ------ Wifi management --------------------
    wifiStatus  = wifiMgr.client_update();
    comData     = network_parse_data(wifiMgr.read_data());


    // ------ Screen drawing ---------------------
    drawerMgr.draw_background();
    drawerMgr.draw_main_point(comData.incAngular.angle[0], comData.incAngular.angle[1]);
    drawerMgr.draw_inclinometer_values(comData.incAngular.angle[0], comData.incAngular.angle[1]);
    drawerMgr.draw_wifi_status(get_color_from_wifi_status(wifiStatus));


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


    if (comData.error == 1)
      delay(200);
    else
      delay(2);
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
std::vector<String> extract_substring (String _input, char _separator)
{
  uint16_t index      = 0;
  uint16_t lastIndex  = 0;
  std::vector<String> retval;

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

  return retval;
}

/*-------------------------------------------------------------------------------------------------------------------*/
String network_prepare_data (struct strComData _data)
{
  String retval = "";

  retval += "Xacc="+String(_data.incAcceleration.acceleration[0])+";";
  retval += "Yacc="+String(_data.incAcceleration.acceleration[1])+";";
  retval += "Zacc="+String(_data.incAcceleration.acceleration[2])+";";

  retval += "Xangle="+String(_data.incAngular.angle[0])+";";
  retval += "Yangle="+String(_data.incAngular.angle[1])+";";
  retval += "Zangle="+String(_data.incAngular.angle[2])+";";

  retval += "Xvelocity="+String(_data.inclAngularVelocity.velocity[0])+";";
  retval += "Yvelocity="+String(_data.inclAngularVelocity.velocity[1])+";";
  retval += "Zvelocity="+String(_data.inclAngularVelocity.velocity[2])+"\n";

  return retval;
}

/*-------------------------------------------------------------------------------------------------------------------*/
struct strComData network_parse_data (String _data)
{
  struct strComData retval;
  retval.error = 1;

  if (_data != "")
  {
    retval.error = 0;
    std::vector<String> DataList = extract_substring(_data, ';');

    for (const auto& data : DataList)
    {
      std::vector<String> var = extract_substring(_data, '=');

      if (var.size() < 2)
      {
        Serial.println("-----------------------");
        Serial.println("ERROR : invalid data from network");
        Serial.println(_data);
        Serial.println("-----------------------");
        continue;
      }

      if (var[0] == "Xacc")
        retval.incAcceleration.acceleration[0] = String(var[1]).toDouble();

      if (var[0] == "Yacc")
        retval.incAcceleration.acceleration[1] = String(var[1]).toDouble();;

      if (var[0] == "Zacc")
        retval.incAcceleration.acceleration[2] = String(var[1]).toDouble();;

      if (var[0] == "Xangle")
        retval.incAngular.angle[0] = String(var[1]).toDouble();;

      if (var[0] == "Yangle")
        retval.incAngular.angle[1] = String(var[1]).toDouble();;

      if (var[0] == "Zangle")
        retval.incAngular.angle[2] = String(var[1]).toDouble();;

      if (var[0] == "Xvelocity")
        retval.inclAngularVelocity.velocity[0] = String(var[1]).toDouble();;

      if (var[0] == "Yvelocity")
        retval.inclAngularVelocity.velocity[1] = String(var[1]).toDouble();;

      if (var[0] == "Zvelocity")
        retval.inclAngularVelocity.velocity[2] = String(var[1]).toDouble();;
    }
  }
    
  return retval;
}

/*-------------------------------------------------------------------------------------------------------------------*/
uint32_t get_color_from_wifi_status (uint8_t _status)
{
  uint32_t color = 0;

  switch (_status)
  {
    case CONNECTION_STATUS_NOT_CONNECTED:
      color = TFT_RED;
      break;

    case CONNECTION_STATUS_WAITING_CLIENT:
    case CONNECTION_STATUS_WAITING_SERVER:
      color = TFT_ORANGE;
      break;
    
    case CONNECTION_STATUS_CONNECTED:
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
