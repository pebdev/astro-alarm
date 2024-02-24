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
#include <SPI.h>
#include <TFT_eSPI.h>


/** D R A W E R ******************************************************************************************************/
class DrawerManager
{
private:
  uint16_t isAlarmBarDisplayed;
  unsigned long timerRrefreshPing_ms = millis();
  TFT_eSPI tft = TFT_eSPI();
  TFT_eSprite spriteScreen = TFT_eSprite(&tft);
  unsigned long timerAlarmDraw_ms = 0;


public:
  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Constructor
  /*-------------------------------------------------------------------------------------------------------------------*/
  DrawerManager (void)
  {
    this->isAlarmBarDisplayed = false;
    
    // Initialize the TFT
    this->tft.init();
    this->tft.setRotation(1);
    this->tft.setSwapBytes(true);

    // Use sprite to avoid screen flickering
    this->spriteScreen.createSprite(tft.width(), tft.height());
    this->spriteScreen.setSwapBytes(true);
    this->spriteScreen.setTextColor(TFT_WHITE, TFT_BLACK);
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Update drawing
  /*-------------------------------------------------------------------------------------------------------------------*/
  void draw_update (void)
  {
    // Send data to the TFT
    this->spriteScreen.pushSprite(0, 0);
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Draw background
  /*-------------------------------------------------------------------------------------------------------------------*/
  void draw_background (void)
  {
    uint32_t color  = TFT_DARKCYAN;
    uint16_t size   = 5;
    uint16_t cx     = this->tft.width() / 2;
    uint16_t cy     = this->tft.height() / 2;
    
    // Clear screen
    this->spriteScreen.fillSprite(TFT_BLACK);
    
    // Draw background
    for (uint16_t i=0; i<7; i++)
    {
      if (color == TFT_DARKCYAN)
        color = TFT_NAVY;
      else
        color = TFT_DARKCYAN;
      
      this->spriteScreen.drawCircle(cx, cy, i*size, color);
      size += 3;
    }

    // Lines
    this->spriteScreen.drawLine(cx, 0, cx, this->tft.height(), TFT_NAVY);
    this->spriteScreen.drawLine(0, cy, this->tft.width(), cy, TFT_NAVY);
    this->spriteScreen.drawLine(0, 0, this->tft.width(), this->tft.height(), TFT_NAVY);
    this->spriteScreen.drawLine(0, this->tft.height(), this->tft.width(), 0, TFT_NAVY);

    // axis name
    this->spriteScreen.setTextColor(TFT_NAVY);
    this->spriteScreen.setTextDatum(TL_DATUM);
    this->spriteScreen.drawString("x+", cx+3, -5, 4);
    this->spriteScreen.drawString("x-", cx+3, this->tft.height()-18, 4);
    this->spriteScreen.drawString("y-", 3, cy+1, 4);
    this->spriteScreen.drawString("y+", this->tft.width()-25, cy+1, 4);
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Draw main point of the inclinometer
  // @param _x : X angle
  // @param _y : Y angle
  /*-------------------------------------------------------------------------------------------------------------------*/
  void draw_main_point (double _x, double _y)
  {
    uint8_t circleSize = 7;
    double scale  = 10.0;
    double cx = this->tft.width() / 2;
    double cy = this->tft.height() / 2;
    double xp = _y*scale + cx;    // volontary invert x and y to match on GUI
    double yp = _x*scale + cy;
    
    if (xp < 0)
      xp = circleSize;
    
    if (xp > (this->tft.width()-circleSize))
      xp = this->tft.width()-circleSize;
    
    if (yp < 0)
      yp = circleSize;
    
    if (this->isAlarmBarDisplayed == true)
    {
      if (yp > (this->tft.height()-20-circleSize))
        yp = this->tft.height()-20-circleSize;
    }
    else
    {
      if (yp > (this->tft.height()-circleSize))
        yp = this->tft.height()-circleSize;
    }

    this->spriteScreen.fillCircle(xp, yp, circleSize-2, TFT_RED);
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Draw inclinometer values
  // @param _x : X angle
  // @param _y : Y angle
  /*-------------------------------------------------------------------------------------------------------------------*/
  void draw_inclinometer_values (double _x, double _y)
  {
    uint32_t color = TFT_RED;
    String Xval = "X=" + String(_x);
    String Yval = "Y=" + String(_y);

    if ((abs(_x) < 0.1) && (abs(_y) < 0.1))
      color = TFT_GREEN;
    else if ((abs(_x) < 1.0) && (abs(_y) < 1.0))
      color = TFT_ORANGE;

    this->spriteScreen.setTextColor(color);
    this->spriteScreen.setTextDatum(TL_DATUM);
    this->spriteScreen.drawString(Xval, 2, 0, 4);
    this->spriteScreen.drawString(Yval, 2, 25, 4);
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Draw memory values
  // @param _x : X angle
  // @param _y : Y angle
  /*-------------------------------------------------------------------------------------------------------------------*/
  void draw_memory_values (double _x, double _y)
  {
    String Xval = "Xm=" + String(_x);
    String Yval = "Ym=" + String(_y);

    this->spriteScreen.setTextColor(TFT_DARKCYAN);
    this->spriteScreen.setTextDatum(TL_DATUM);
    this->spriteScreen.drawString(Xval, this->tft.width()-80, this->tft.height()-35, 2);
    this->spriteScreen.drawString(Yval, this->tft.width()-80, this->tft.height()-20, 2);
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Draw temperature value
  // @param _temperature : temperature
  /*-------------------------------------------------------------------------------------------------------------------*/
  void draw_temperature_value (double _temperature)
  {
    uint32_t heightObject = this->tft.height()-35;
    String Stemperature = "T=" + String(int(_temperature))+"C";

    if (this->isAlarmBarDisplayed == true)
      heightObject -= 20;

    this->spriteScreen.setTextColor(TFT_DARKCYAN);
    this->spriteScreen.setTextDatum(TL_DATUM);
    this->spriteScreen.drawString(Stemperature, 2, heightObject, 2);
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Draw battery data.
  // @param _vbat_percentage : percentage of the battery voltage.
  // @param _vbat_voltage    : voltage of the battery.
  /*-------------------------------------------------------------------------------------------------------------------*/
  void draw_battery_data (double _vbat_percentage, double _vbat_voltage)
  {
    uint32_t heightObject = this->tft.height()-20;
    String VbatData = "";

    if (this->isAlarmBarDisplayed == true)
      heightObject -= 20;

    if (_vbat_voltage > 4.5)
      VbatData = "VBat=charging...";
    else
      VbatData = "VBat=" + String(int(_vbat_percentage))+"%"; //-"+String(_vbat_voltage)+"V";

    this->spriteScreen.setTextColor(TFT_DARKCYAN);
    this->spriteScreen.setTextDatum(TL_DATUM);
    this->spriteScreen.drawString(VbatData, 2, heightObject, 2);
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Draw alarm data if it is triggered
  // @param _Xacc_init    : X acceleration initiale value
  // @param _Yacc_init    : Y acceleration initiale value
  // @param _Zacc_init    : Z acceleration initiale value
  // @param _Xacc_current : X acceleration current value
  // @param _Yacc_current : Y acceleration current value
  // @param _Zacc_current : Z acceleration current value
  /*-------------------------------------------------------------------------------------------------------------------*/
  void draw_alarm_data (double _Xacc_init, double _Yacc_init, double _Zacc_init, double _Xacc_current, double _Yacc_current, double _Zacc_current)
  {
    String AccelerationCurrent = "Acc.curr="+String(_Xacc_init)+" | "+String(_Yacc_init)+" | "+String(_Zacc_init);
    String AccelerationInit    = "Acc.init="+String(_Xacc_current)+" | "+String(_Yacc_current)+" | "+String(_Zacc_current);
    
    // if alarm display is not yet enabled
    if (timerAlarmDraw_ms == 0)
      timerAlarmDraw_ms = millis();

    if ((millis()-timerAlarmDraw_ms) < 500)
    {
      this->spriteScreen.setTextColor(TFT_RED);
      this->spriteScreen.setTextDatum(TL_DATUM);
      this->spriteScreen.drawString("ALARM TRIGGERED", 45, 55, 4);
    }

    if ((millis()-timerAlarmDraw_ms) > 250)
    {
      timerAlarmDraw_ms = 0;
    }

    this->spriteScreen.setTextColor(TFT_RED);
    this->spriteScreen.drawString(AccelerationCurrent, 10, this->tft.height()-60, 2);
    this->spriteScreen.drawString(AccelerationInit, 10, this->tft.height()-40, 2);
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Draw wifi status
  // @param _color            : color of the indicator
  // @param _signal_strength  : wifi signal strength
  /*-------------------------------------------------------------------------------------------------------------------*/
  void draw_wifi_status (uint32_t _color, int16_t _signal_strength)
  {
    String wifiQuality = String(_signal_strength)+"%";

    this->spriteScreen.fillRoundRect(this->tft.width()-50, 0, 50, 5, 3, _color);
    this->spriteScreen.setTextColor(_color);
    this->spriteScreen.drawString(wifiQuality, this->tft.width()-37, 10, 2);
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Draw ping status
  // @param _status : status of the ping
  /*-------------------------------------------------------------------------------------------------------------------*/
  void draw_ping_status (bool _status)
  {
    unsigned long timeout_ms = 250;

    if ((_status == true) && ((millis()-this->timerRrefreshPing_ms) > timeout_ms))
    {
      if ((millis()-this->timerRrefreshPing_ms) > (timeout_ms+500))
        this->timerRrefreshPing_ms = millis();
    }
    
    if ((millis()-this->timerRrefreshPing_ms) < timeout_ms)
      this->spriteScreen.fillRoundRect(this->tft.width()-70, 0, 8, 5, 3, TFT_GREEN);
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Draw alarm state
  // @param _color : color of the indicator
  // @param _state : state of the alarm
  /*-------------------------------------------------------------------------------------------------------------------*/
  void draw_alarm_state (uint32_t _color, String _state)
  {
    isAlarmBarDisplayed = true;
    String state = "ALARM "+_state;

    this->spriteScreen.fillRoundRect(0, this->tft.height()-20, this->tft.width(), this->tft.height(), 3, _color);
    this->spriteScreen.setTextColor(TFT_NAVY);
    this->spriteScreen.setTextDatum(TL_DATUM);
    this->spriteScreen.drawString(state, 55, this->tft.height()-20, 4);
  }
};
