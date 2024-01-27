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
#include <TFT_eSPI.h>


/** D R A W E R ******************************************************************************************************/
class DrawerManager
{
private:
  uint16_t isAlarmBarDisplayed;
  TFT_eSPI tft = TFT_eSPI();

public:
  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Constructor
  /*-------------------------------------------------------------------------------------------------------------------*/
  DrawerManager (void)
  {
    isAlarmBarDisplayed = false;
    
    tft.init();
    tft.setRotation(1);
    tft.setSwapBytes(true);
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Draw background
  /*-------------------------------------------------------------------------------------------------------------------*/
  void draw_background (void)
  {
    uint32_t color  = TFT_DARKCYAN;
    uint16_t size   = 5;
    uint16_t cx     = tft.width() / 2;
    uint16_t cy     = tft.height() / 2;
    
    // Clear screen
    tft.fillScreen(TFT_BLACK);
    
    // Draw background
    for (uint16_t i=0; i<7; i++)
    {
      if (color == TFT_DARKCYAN)
        color = TFT_NAVY;
      else
        color = TFT_DARKCYAN;
      
      tft.drawCircle(cx, cy, i*size, color);
      size += 3;
    }

    // Lines
    tft.drawLine(cx, 0, cx, tft.height(), TFT_NAVY);
    tft.drawLine(0, cy, tft.width(), cy, TFT_NAVY);
    tft.drawLine(0, 0, tft.width(), tft.height(), TFT_NAVY);
    tft.drawLine(0, tft.height(), tft.width(), 0, TFT_NAVY);

    // axis name
    tft.setTextSize(2);
    tft.setTextColor(TFT_NAVY);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("x+", cx+3, 0);
    tft.drawString("x-", cx+3, tft.height()-18);
    tft.drawString("y-", 3, cy+1);
    tft.drawString("y+", tft.width()-25, cy+1);
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Draw main point of the inclinometer
  // @param _x : X angle
  // @param _y : Y angle
  /*-------------------------------------------------------------------------------------------------------------------*/
  void draw_main_point (double _x, double _y)
  {
    uint8_t circleSize = 5;
    double scale  = 10.0;
    double cx = tft.width() / 2;
    double cy = tft.height() / 2;
    double xp = _x*scale + cx;
    double yp = _y*scale + cy;
    
    if (xp < 0)
      xp = circleSize;
    
    if (xp > tft.width())
      xp = tft.width()-circleSize;
    
    if (yp < 0)
      yp = circleSize;
    
    if (isAlarmBarDisplayed == true)
    {
      if (yp > tft.height()-10)
        yp = tft.height()-10-circleSize;
    }
    else
    {
      if (yp > tft.height())
        yp = tft.height()-circleSize;
    }

    tft.fillCircle(xp, yp, circleSize, TFT_RED);
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Draw inclinometer values
  // @param _x : X angle
  // @param _y : Y angle
  /*-------------------------------------------------------------------------------------------------------------------*/
  void draw_inclinometer_values (double _x, double _y)
  {
    String Xval = "X=" + String(_x);
    String Yval = "Y=" + String(_y);

    tft.setTextSize(2);
    tft.setTextColor(TFT_DARKCYAN);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(Xval, 2, 0);
    tft.drawString(Yval, 2, 20);
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
    String AccelerationCurrent = "Acc.curr="+String(_Xacc_init)+"|"+String(_Yacc_init)+"|"+String(_Zacc_init);
    String AccelerationInit    = "Acc.init="+String(_Xacc_current)+"|"+String(_Yacc_current)+"|"+String(_Zacc_current);

    tft.setTextSize(3);
    tft.setTextColor(TFT_RED);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("ALARM TRIGGERED", 25, 50);

    tft.setTextSize(2);
    tft.setTextColor(TFT_RED);
    tft.drawString(AccelerationCurrent, 10, tft.height()-60);
    tft.drawString(AccelerationInit, 10, tft.height()-40);
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Draw wifi status
  // @param _color : color of the indicator
  /*-------------------------------------------------------------------------------------------------------------------*/
  void draw_wifi_status (uint32_t _color)
  {
    tft.fillRoundRect(tft.width()-50, 0, 50, 5, 3, _color);
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Draw alarm state
  // @param _color : color of the indicator
  /*-------------------------------------------------------------------------------------------------------------------*/
  void draw_alarm_state (uint32_t _color, String _state)
  {
    isAlarmBarDisplayed = true;
    String state = "ALARM STATE : "+_state;

    tft.fillRoundRect(0, tft.height()-10, tft.width(), tft.height(), 3, _color);
    tft.setTextSize(1);
    tft.setTextColor(TFT_NAVY);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(state, tft.width()/3, tft.height()-8);
  }
};
