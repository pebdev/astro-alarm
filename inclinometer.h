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


/** S T R U C T S ****************************************************************************************************/
struct strAcceleration
{
  double acceleration[3];   // Acceleration X|Y|Z=((AxH<<8)|AxL)/32768*16g
  double temperature;       // Temperature=((TH<<8)|TL) /100 °C
};

struct strAngularVelocity
{
  double velocity[3];   // X, Y, Z => X|Y|Z=((WxH<<8)|WxL)/32768*2000°/s
	double voltage;       // Vbluetooth=((VolH<<8)|VolL) /100°C ====> BLUETOOTH DEVICE ONLY
};

struct strAngular
{
	double angle[3];        // Roll  angleX=((RollH<<8)|RollL)/32768*180(°)
                          // Pitch angleY=((PitchH<<8)|PitchL)/32768*180(°)
                          // Yaw   angleZ=((YawH<<8)|YawL)/32768*180(°)
	uint16_t version;       // Version Formula number=(VH<<8)|VL
};


/** I N C L I N O M E T E R ******************************************************************************************/
class Inclinometer
{
private:
  struct strAccelerationRaw
  {
    uint16_t acceleration[3];
    uint16_t temperature;
    uint8_t checksum;
  };

  struct strAngularVelocityRaw
  {
    uint16_t velocity[3];
    uint16_t voltage;
    uint8_t checksum;
  };

  struct strAngularRaw
  {
    uint16_t angle[3];
    uint16_t version;
    uint8_t checksum;
  };

  // Raw data
  struct strAccelerationRaw incAccelerationRaw;
  struct strAngularVelocityRaw inclAngularVelocityRaw;
  struct strAngularRaw incAngularRaw;

  // Final data, shared with users
  bool firstFrameDetected;
  bool newDataReady;
  struct strAcceleration incAcceleration;
  struct strAngularVelocity inclAngularVelocity;
  struct strAngular incAngular;


public:
  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Constructor
  /*-------------------------------------------------------------------------------------------------------------------*/
  Inclinometer (void)
  {
    this->newDataReady        = false;
    this->firstFrameDetected  = false;
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Allow user to know if there is new data ready to be proccessed
  // @return true | false
  /*-------------------------------------------------------------------------------------------------------------------*/
  bool is_new_data_ready (void)
  {
    return this->newDataReady;
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Read inclinometer data
  // @param _ucData : data received from the UART link
  /*-------------------------------------------------------------------------------------------------------------------*/
  void read (unsigned char _ucData)
  {
    static unsigned char ucRxBuffer[250];
    static unsigned char ucRxCnt = 0;	

    // Save data
    ucRxBuffer[ucRxCnt++] = _ucData;

    // Check first Byte
    if (ucRxBuffer[0] != 0x55) 
    {
      ucRxCnt = 0;
      return;
    }

    // If we haven't yet received all Bytes, we wait next Bytes
    if (ucRxCnt < 11)
    {
      return;
    }
    // Full frame, process data
    else
    {
      switch(ucRxBuffer[1])
      {
        case 0x51:	memcpy(&this->incAccelerationRaw,&ucRxBuffer[2],9);     this->newDataReady = true; break;
        case 0x52:	memcpy(&this->inclAngularVelocityRaw,&ucRxBuffer[2],9); this->newDataReady = true; break;
        case 0x53:	memcpy(&this->incAngularRaw,&ucRxBuffer[2],9);          this->newDataReady = true; break;
      }
      ucRxCnt = 0;
    }
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Process inclinometer data
  /*-------------------------------------------------------------------------------------------------------------------*/
  void process_data (void)
  {
    // Proccess data only if we have new data 
    if (this->newDataReady == false)
      return;
    this->newDataReady = false;

    // Because the first frame can't be good, skip it
    if (this->firstFrameDetected == false)
    {
      this->firstFrameDetected = true;
      return;
    }

    if (this->incAccelerationRaw.checksum == this->checksum(&this->incAccelerationRaw, 0x51))
    {
      this->incAcceleration.acceleration[0] = this->value_saturation((double)this->incAccelerationRaw.acceleration[0]/32768.0*16.0, 16.0);
      this->incAcceleration.acceleration[1] = this->value_saturation((double)this->incAccelerationRaw.acceleration[1]/32768.0*16.0, 16.0);
      this->incAcceleration.acceleration[2] = this->value_saturation((double)this->incAccelerationRaw.acceleration[2]/32768.0*16.0, 16.0);
      this->incAcceleration.temperature     = (double)this->incAccelerationRaw.temperature/100.0;
    }
    else
    {
      Serial.println("acceleration : CHECKSUM ERROR");
    }

    if (this->inclAngularVelocityRaw.checksum == this->checksum(&this->inclAngularVelocityRaw, 0x52))
    {
      this->inclAngularVelocity.velocity[0] = this->value_saturation((double)this->inclAngularVelocityRaw.velocity[0]/32768.0*2000.0, 2000.0);
      this->inclAngularVelocity.velocity[1] = this->value_saturation((double)this->inclAngularVelocityRaw.velocity[1]/32768.0*2000.0, 2000.0);
      this->inclAngularVelocity.velocity[2] = this->value_saturation((double)this->inclAngularVelocityRaw.velocity[2]/32768.0*2000.0, 2000.0);
    }
    else
    {
      Serial.println("velocity : CHECKSUM ERROR");
    }

    if (this->incAngularRaw.checksum == this->checksum(&this->incAngularRaw, 0x53))
    {
      this->incAngular.angle[0] = this->value_saturation((double)this->incAngularRaw.angle[0]/32768.0*180.0, 180.0);
      this->incAngular.angle[1] = this->value_saturation((double)this->incAngularRaw.angle[1]/32768.0*180.0, 180.0);
      this->incAngular.angle[2] = this->value_saturation((double)this->incAngularRaw.angle[2]/32768.0*180.0, 180.0);
      this->incAngular.version  = this->incAngularRaw.version;
    }
    else
    {
      Serial.println("angular : CHECKSUM ERROR");
    }
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Provide acceleration data
  // @return strAcceleration data
  /*-------------------------------------------------------------------------------------------------------------------*/
  struct strAcceleration get_acceleration_data (void)
  {
    return this->incAcceleration;
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Provide angular velocity data
  // @return strAngularVelocity data
  /*-------------------------------------------------------------------------------------------------------------------*/
  struct strAngularVelocity get_angular_velocity_data (void)
  {
    return this->inclAngularVelocity;
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Provide angular data
  // @return strAngular data
  /*-------------------------------------------------------------------------------------------------------------------*/
  struct strAngular get_angular_data (void)
  {
    return this->incAngular;
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Show inclinometer data in the console
  /*-------------------------------------------------------------------------------------------------------------------*/
  void show_data (void)
  {
    Serial.println("-----------------------------------------------");
    Serial.print("angular      : X=");
    Serial.print(this->incAngular.angle[0]);
    Serial.print(" | Y=");
    Serial.print(this->incAngular.angle[1]);
    Serial.print(" | Z=");
    Serial.print(this->incAngular.angle[2]);
    Serial.println(" (X=Roll | Y=Pitch | Z=Yaw)");

    Serial.print("acceleration : X=");
    Serial.print(this->incAcceleration.acceleration[0]);
    Serial.print(" | Y=");
    Serial.print(this->incAcceleration.acceleration[1]);
    Serial.print(" | Z=");
    Serial.println(this->incAcceleration.acceleration[2]);

    Serial.print("velocity     : X=");
    Serial.print(this->inclAngularVelocity.velocity[0]);
    Serial.print(" | Y=");
    Serial.print(this->inclAngularVelocity.velocity[1]);
    Serial.print(" | Z=");
    Serial.println(this->inclAngularVelocity.velocity[2]);

    Serial.print("temperature  : T=");
    Serial.println(this->incAcceleration.temperature);
  }


private:
  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PRIVATE] Compute checksum of the inclinometer frame
  // @param _memory_addr : address of the memory with received data
  // @param _id : data id
  // @return checksum value
  /*-------------------------------------------------------------------------------------------------------------------*/
  uint8_t checksum (const void* _memory_addr, uint8_t _id)
  {
    const uint8_t* bytes = (const uint8_t*)_memory_addr;
    uint8_t sum = 0x55 + _id;

    for (size_t i=0; i<8; ++i)
      sum += bytes[i];

    return sum;
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PRIVATE] Apply a 180° on an axis : -180°=+180° ==> 0°
  // @param _angle : angle to invert
  // @return inverted angle
  /*-------------------------------------------------------------------------------------------------------------------*/
  double angle_inverter (double _angle)
  {
    _angle += 180;

    if (_angle > 180.0)
      _angle -= 360.0;
      
    return _angle;
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PRIVATE] Saturation of a value
  // @param _value      : value to check
  // @param _value_max  : max value to check
  // @return value or saturated value
  /*-------------------------------------------------------------------------------------------------------------------*/
  double value_saturation (double _value, double _value_max)
  {
    if (_value >= _value_max)
      _value -= 2.0 * _value_max;

    return _value;
  }
};
