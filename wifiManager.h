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
#include <WiFi.h>
#include "wifi_info.h"


/** D E F I N E S ****************************************************************************************************/
// Wifi connection state
#define CONNECTION_STATUS_WIFI_DISCONNECTED       (0)
#define CONNECTION_STATUS_WIFI_CONNECTING         (1)
#define CONNECTION_STATUS_WIFI_CONNECTED          (2)

// Client and Server states
#define CONNECTION_STATUS_APP_DISCONNECTED        (0)
#define CONNECTION_STATUS_APP_CONNECTING          (1)
#define CONNECTION_STATUS_APP_CONNECTED           (2)

// Timer
#define CONNECTION_RETRY_INTERVAL_MS              (5000)
#define CONNECTION_ALIVE_TIMEOUT_MS               (5000)
#define CONNECTION_ALIVE_SEND_INTERVAL_MS         (1000)


/** W I F I **********************************************************************************************************/
class WifiManager
{
private:
  bool isPingReceived;
  uint8_t wifiConnectionState;
  uint8_t appConnectionState;
  WiFiServer server;
  WiFiClient client;
  unsigned long timerCheckConnectionAlive_ms  = millis();
  unsigned long timerToSendWifiData_ms        = millis();


public:
  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Constructor
  /*-------------------------------------------------------------------------------------------------------------------*/
  WifiManager (void)
  {
    this->isPingReceived      = false;
    this->wifiConnectionState = CONNECTION_STATUS_WIFI_DISCONNECTED;
    this->appConnectionState  = CONNECTION_STATUS_APP_DISCONNECTED;
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Start the wifi
  /*-------------------------------------------------------------------------------------------------------------------*/
  void start (void)
  {
    WiFi.begin(wifi_ssid, wifi_key);
    this->server = WiFiServer(wifi_port);
    this->wifiConnectionState = CONNECTION_STATUS_WIFI_CONNECTING;
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Send data to a client
  // @param _data      : string to send 
  // @param _period_ms : elapsed time in ms between two send frames
  /*-------------------------------------------------------------------------------------------------------------------*/
  void send_data (String _data, unsigned long _period_ms)
  {
    if (this->appConnectionState == CONNECTION_STATUS_APP_CONNECTED)
    {
      if ((millis()-this->timerToSendWifiData_ms) > _period_ms)
      {
        this->client.println(_data);
        this->timerToSendWifiData_ms = millis();
      }
    }
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Read data from the server
  // @param _last_msg : return the latest received message
  // @param _force    : force the read even if status is not connected
  // @return empty string if there is no data, otherwise received data
  /*-------------------------------------------------------------------------------------------------------------------*/
  String read_data (bool _last_msg=false, bool _force=false)
  {
    String previousRetval = "";
    String retval = "";

    if ((_force == true) || (this->appConnectionState == CONNECTION_STATUS_APP_CONNECTED))
    {
      do
      {
        previousRetval = retval;
        retval = "";

        // Read data in the buffer
        if (this->client.available())
        {
          retval = this->client.readStringUntil('\n');
          
          // Reset the watchdog
          if (retval != "")
            this->timerCheckConnectionAlive_ms = millis();
          
          // isAlive fram is only used by tye watchdog, filter this
          if (retval.indexOf("isAlive") != -1)
            this->isPingReceived = true;
          
          //Serial.print("received==["); Serial.print(retval);Serial.println("]");
        }
      } while ((_last_msg == true) && (retval != ""));

      if (_last_msg == true)
      {
        retval = previousRetval;
      }
    }

    return retval;
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Allow user to see if a ping was received
  // @return true | false
  /*-------------------------------------------------------------------------------------------------------------------*/
  bool is_ping_received (void)
  {
    bool retval = this->isPingReceived;
    this->isPingReceived = false;
    
    return retval;
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Update server state
  // @return CONNECTION_STATUS_APP_DISCONNECTED | CONNECTION_STATUS_APP_CONNECTING | CONNECTION_STATUS_APP_CONNECTED
  /*-------------------------------------------------------------------------------------------------------------------*/
  uint8_t server_update (void)
  {
    // Update Wifi connection state
    this->wifi_manage();

    // If we are connected to the router
    if (this->wifiConnectionState == CONNECTION_STATUS_WIFI_CONNECTED)
    { 
      // Not yet connected
      if (this->appConnectionState == CONNECTION_STATUS_APP_DISCONNECTED)
      {
        this->server.begin();
        this->appConnectionState = CONNECTION_STATUS_APP_CONNECTING;
        Serial.println("WIFI : server started !");
      }

      // Check client status
      else
      {
        if ((!this->client.connected()) || ((this->appConnectionState == CONNECTION_STATUS_APP_CONNECTED) && (!this->is_connection_alive())))
        {
          if (this->appConnectionState == CONNECTION_STATUS_APP_CONNECTED)
            Serial.println("WIFI : connection lost with client !");
          
          this->appConnectionState = CONNECTION_STATUS_APP_CONNECTING;
          this->client = this->server.available();
        }
        else
        {
          if (this->appConnectionState == CONNECTION_STATUS_APP_CONNECTING)
          {
            this->timerCheckConnectionAlive_ms = millis();
            this->appConnectionState = CONNECTION_STATUS_APP_CONNECTED;
            Serial.println("WIFI : client connected !");
          }
          else
          {
            // Refresh the ping
            this->read_data(true, false);
          }
        }
      }
    }

    // Wifi disconnected
    else
    {
      if (this->appConnectionState != CONNECTION_STATUS_APP_DISCONNECTED)
      {
        Serial.println("WIFI : server closed !");
        this->server.end();
      }

      this->appConnectionState = CONNECTION_STATUS_APP_DISCONNECTED;
    }

    return this->appConnectionState;
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Update client state
  // @return CONNECTION_STATUS_APP_DISCONNECTED | CONNECTION_STATUS_APP_CONNECTING | CONNECTION_STATUS_APP_CONNECTED
  /*-------------------------------------------------------------------------------------------------------------------*/
  uint8_t client_update (void)
  {
    // Update Wifi connection state
    this->wifi_manage();

    // If we are connected to the router
    if (this->wifiConnectionState == CONNECTION_STATUS_WIFI_CONNECTED)
    { 
      // Not yet connected
      if (this->appConnectionState == CONNECTION_STATUS_APP_DISCONNECTED)
      {
        this->client.connect(wifi_ip_server, wifi_port);
        this->timerCheckConnectionAlive_ms = millis();
        this->appConnectionState = CONNECTION_STATUS_APP_CONNECTING;
        Serial.println("WIFI : client connection...");
      }

      // Wait for server connection
      if (this->appConnectionState == CONNECTION_STATUS_APP_CONNECTING)
      {
        if (this->client.connected())
        {
          this->timerCheckConnectionAlive_ms = millis();
          this->appConnectionState = CONNECTION_STATUS_APP_CONNECTED;
          Serial.println("WIFI : connected to the server !");
        }
        else
        {
          // Retry to connect
          if ((millis()-this->timerCheckConnectionAlive_ms) > CONNECTION_RETRY_INTERVAL_MS)
          {
            this->client.stop();
            this->flush();
            this->client.connect(wifi_ip_server, wifi_port);
            this->timerCheckConnectionAlive_ms = millis();
          }
        }
      }

      // Check for connection status
      if (this->appConnectionState == CONNECTION_STATUS_APP_CONNECTED)
      {
        this->send_data("isAlive", CONNECTION_ALIVE_SEND_INTERVAL_MS);

        // Check connection status
        if (!this->is_connection_alive())
        {
          this->appConnectionState = CONNECTION_STATUS_APP_DISCONNECTED;
          this->client.stop();
          this->flush();
          Serial.println("WIFI : disconnected from the server !");
        }
      }
    }

    // Wifi disconnected
    else
    {
      if (this->appConnectionState != CONNECTION_STATUS_APP_DISCONNECTED)
      {
        Serial.println("WIFI : connection lost !");
        client.stop();
      }

      this->appConnectionState = CONNECTION_STATUS_APP_DISCONNECTED;
    }

    return this->appConnectionState;
  }


private:
  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PRIVATE] Manage connection state with the Wifi router
  /*-------------------------------------------------------------------------------------------------------------------*/
  void wifi_manage (void)
  {
    // Connected to the wifi router
    if (WiFi.status() == WL_CONNECTED)
    {
      if (this->wifiConnectionState == CONNECTION_STATUS_WIFI_CONNECTING)
        Serial.println("WIFI : connected to the router !");
      
      this->wifiConnectionState = CONNECTION_STATUS_WIFI_CONNECTED;
    }
    // Disconnected or not yet connected
    else
    {
      // Lost Wifi connection
      if (this->wifiConnectionState != CONNECTION_STATUS_WIFI_CONNECTING)
      {
        this->wifiConnectionState = CONNECTION_STATUS_WIFI_CONNECTING;
        WiFi.reconnect();
        Serial.println("WIFI : connecting to the router...");
      }
    }
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PRIVATE] Get connection status with the other device
  // @return true | false
  /*-------------------------------------------------------------------------------------------------------------------*/
  bool is_connection_alive (void)
  {
    bool retval = false;

    // timer was reseted when a 'isAlive' data is received
    if ((millis()-this->timerCheckConnectionAlive_ms) < CONNECTION_ALIVE_TIMEOUT_MS)
      retval = true;

    return retval;
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PRIVATE] Flush Rx buffer
  /*-------------------------------------------------------------------------------------------------------------------*/
  void flush (void)
  {
    while (this->read_data(false, true) != "");
    this->isPingReceived = false;
  }
};
