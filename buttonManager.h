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
#include <Button.h>   // Hardware Buttons library


/** D E F I N E S ****************************************************************************************************/
// Button states
#define BUTTON_NOT_PUSH                     (0)
#define BUTTON_SHORT_PUSH                   (1)
#define BUTTON_LONG_PUSH                    (2)
#define BUTTON_LONG_PUSH_NOT_YET_RELEASED   (3)


/** D E C L A R A T I O N S ******************************************************************************************/
uint8_t buttonState;
Hardware::Button button = Hardware::Button(255);  //Because there is no default constructor, create dummy object


/** C A L L B A C K **************************************************************************************************/
/*-------------------------------------------------------------------------------------------------------------------*/
// @brief [PRIVATE] Callback on press action
// @param _sender : sender info
/*-------------------------------------------------------------------------------------------------------------------*/
void button_onPress (Hardware::Button& _sender)
{
  buttonState = BUTTON_SHORT_PUSH;
}

/*-------------------------------------------------------------------------------------------------------------------*/
// @brief [PRIVATE] Callback on long pressing action (long press not finished)
// @param _sender : sender info
/*-------------------------------------------------------------------------------------------------------------------*/
void button_onLongPressing (Hardware::Button& _sender)
{
  if ((buttonState == BUTTON_LONG_PUSH) || (buttonState == BUTTON_LONG_PUSH_NOT_YET_RELEASED))
    buttonState = BUTTON_LONG_PUSH_NOT_YET_RELEASED;
  else
    buttonState = BUTTON_LONG_PUSH;
}

/*-------------------------------------------------------------------------------------------------------------------*/
// @brief [PRIVATE] Callback on long press action (long press finished)
// @param _sender : sender info
/*-------------------------------------------------------------------------------------------------------------------*/
void button_onLongPress (Hardware::Button& _sender)
{
  buttonState = BUTTON_NOT_PUSH;
}


/** B U T T O N ******************************************************************************************************/
class ButtonManager
{
public:
  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Constructor
  // @param _gpio : gpio of the button
  /*-------------------------------------------------------------------------------------------------------------------*/
  ButtonManager (int _gpio)
  {
    uint8_t buttonState = BUTTON_NOT_PUSH;
    button = Hardware::Button(_gpio, true);
    
    // Callbacks
    button.onPress        = &button_onPress;
    button.onLongPress    = &button_onLongPress;
    button.onLongPressing = &button_onLongPressing;

    // Button settings
    button.setPressTimeout(600);
    button.setLongPressTimeout(1000);
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Function to update button state.
  // @return BUTTON_NOT_PUSH | BUTTON_SHORT_PUSH | BUTTON_LONG_PUSH |BUTTON_LONG_PUSH_NOT_YET_RELEASED
  /*-------------------------------------------------------------------------------------------------------------------*/
  uint8_t update (void)
  {
    uint8_t retval = buttonState;

    // Reset button state if it's a short push, because not updated by a callback
    if (buttonState == BUTTON_SHORT_PUSH)
      buttonState = BUTTON_NOT_PUSH;

    // Update button callback
    button.update();

    return retval;
  }
};
