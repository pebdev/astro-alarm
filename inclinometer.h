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
  struct strAcceleration incAcceleration;
  struct strAngularVelocity inclAngularVelocity;
  struct strAngular incAngular;


public:
  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Constructor
  /*-------------------------------------------------------------------------------------------------------------------*/
  Inclinometer (void)
  {
    //
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
        case 0x51:	memcpy(&incAccelerationRaw,&ucRxBuffer[2],9);break;
        case 0x52:	memcpy(&inclAngularVelocityRaw,&ucRxBuffer[2],9);break;
        case 0x53:	memcpy(&incAngularRaw,&ucRxBuffer[2],9);break;
      }
      ucRxCnt = 0;
    }
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Process inclinometer data
  /*-------------------------------------------------------------------------------------------------------------------*/
  void process_data (void)
  {
    if (incAccelerationRaw.checksum == checksum(&incAccelerationRaw, 0x51))
    {
      incAcceleration.acceleration[0] = (double)incAccelerationRaw.acceleration[0]/32768.0*16.0;
      incAcceleration.acceleration[1] = (double)incAccelerationRaw.acceleration[1]/32768.0*16.0;
      incAcceleration.acceleration[2] = (double)incAccelerationRaw.acceleration[2]/32768.0*16.0;
      incAcceleration.temperature     = (double)incAccelerationRaw.temperature/100.0;
    }
    else
    {
      Serial.println("acceleration : CHECKSUM ERROR");
    }

    if (inclAngularVelocityRaw.checksum == checksum(&inclAngularVelocityRaw, 0x52))
    {
      inclAngularVelocity.velocity[0] = (double)inclAngularVelocityRaw.velocity[0]/32768.0*2000.0;
      inclAngularVelocity.velocity[1] = (double)inclAngularVelocityRaw.velocity[1]/32768.0*2000.0;
      inclAngularVelocity.velocity[2] = (double)inclAngularVelocityRaw.velocity[2]/32768.0*2000.0;
    }
    else
    {
      Serial.println("velocity : CHECKSUM ERROR");
    }

    if (incAngularRaw.checksum == checksum(&incAngularRaw, 0x53))
    {
      incAngular.angle[0] = angle_converter((double)incAngularRaw.angle[0]/32768.0*180.0);
      incAngular.angle[1] = angle_converter((double)incAngularRaw.angle[1]/32768.0*180.0);
      incAngular.angle[2] = angle_converter((double)incAngularRaw.angle[2]/32768.0*180.0);
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
    return incAcceleration;
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Provide angular velocity data
  // @return strAngularVelocity data
  /*-------------------------------------------------------------------------------------------------------------------*/
  struct strAngularVelocity get_angular_velocity_data (void)
  {
    return inclAngularVelocity;
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Provide angular data
  // @return strAngular data
  /*-------------------------------------------------------------------------------------------------------------------*/
  struct strAngular get_angular_data (void)
  {
    return incAngular;
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Show inclinometer data in the console
  /*-------------------------------------------------------------------------------------------------------------------*/
  void show_data (void)
  {
    Serial.println("-----------------------------------------------");
    Serial.print("acceleration : X=");
    Serial.print(incAcceleration.acceleration[0]);
    Serial.print(" | Y=");
    Serial.print(incAcceleration.acceleration[1]);
    Serial.print(" | Z=");
    Serial.println(incAcceleration.acceleration[2]);

    Serial.print("temperature  : T=");
    Serial.println(incAcceleration.temperature);

    Serial.print("velocity     : X=");
    Serial.print(inclAngularVelocity.velocity[0]);
    Serial.print(" | Y=");
    Serial.print(inclAngularVelocity.velocity[1]);
    Serial.print(" | Z=");
    Serial.println(inclAngularVelocity.velocity[2]);

    Serial.print("angular      : X=");
    Serial.print(incAngular.angle[0]);
    Serial.print(" | Y=");
    Serial.print(incAngular.angle[1]);
    Serial.print(" | Z=");
    Serial.print(incAngular.angle[2]);
    Serial.println(" (X=Roll | Y=Pitch | Z=Yaw)");
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
  // @brief [PRIVATE] Convert angle from [0:360] to [-180:180]
  // @param _angle : angle to convert
  // @return angle from [0:360]
  /*-------------------------------------------------------------------------------------------------------------------*/
  double angle_converter (double _angle)
  {
    // Convert angle in the range [-180°, 180°]
    if (_angle > 180)
      _angle -= 360;

    return _angle;
  }
};
