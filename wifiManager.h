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
// Client and Server states
#define CONNECTION_STATUS_NOT_CONNECTED         (0)
#define CONNECTION_STATUS_WAITING_CLIENT        (1)
#define CONNECTION_STATUS_WAITING_SERVER        (2)
#define CONNECTION_STATUS_CONNECTED             (3)


/** W I F I **********************************************************************************************************/
class WifiManager
{
private:
  uint8_t connectionState;
  WiFiServer server;
  WiFiClient client;


public:
  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Constructor
  /*-------------------------------------------------------------------------------------------------------------------*/
  WifiManager (void)
  {
    connectionState = CONNECTION_STATUS_NOT_CONNECTED;
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Start the wifi
  /*-------------------------------------------------------------------------------------------------------------------*/
  void start (void)
  {
    WiFi.begin(wifi_ssid, wifi_key);
    server = WiFiServer(wifi_port);
  }  

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Send data to a client
  // @param _data : string to send 
  /*-------------------------------------------------------------------------------------------------------------------*/
  void send_data (String _data)
  {
    if (connectionState == CONNECTION_STATUS_CONNECTED)
      client.println(_data);
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Read data from the server
  // @return empty string if there is no data, otherwise received data
  /*-------------------------------------------------------------------------------------------------------------------*/
  String read_data (void)
  {
    String retval = "";

    if (client)
      retval = client.readStringUntil('\n');
      
    return retval;
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Update server processing data
  // @return CONNECTION_STATUS_NOT_CONNECTED | CONNECTION_STATUS_WAITING_CLIENT | CONNECTION_STATUS_CONNECTED
  /*-------------------------------------------------------------------------------------------------------------------*/
  uint8_t server_update (void)
  {
    // If we are not connected, stop the process waiting for a connection
    if (WiFi.status() != WL_CONNECTED)
    {
      if (connectionState != CONNECTION_STATUS_NOT_CONNECTED)
      {
        server.end();
        connectionState = CONNECTION_STATUS_NOT_CONNECTED;
      }
    }
    else
    {
      // If we are connected but not yet started, we do it
      if (connectionState == CONNECTION_STATUS_NOT_CONNECTED)
      {
        server.begin();
        connectionState = CONNECTION_STATUS_WAITING_CLIENT;
      }

      // Now wait for a client
      if (!client.connected())
      {
        client = server.available();
        if (client)
          Serial.println("Client connected");
      }

      // If we have a client, we send the AstroTool data
      if (client.connected())
      {
        connectionState = CONNECTION_STATUS_CONNECTED;
      }
    }

    return connectionState;
  }

  /*-------------------------------------------------------------------------------------------------------------------*/
  // @brief [PUBLIC] Update client processing data
  // @return CONNECTION_STATUS_NOT_CONNECTED | CONNECTION_STATUS_WAITING_SERVER | CONNECTION_STATUS_CONNECTED
  /*-------------------------------------------------------------------------------------------------------------------*/
  uint8_t client_update (void)
  {
    // If we are not connected, stop the process waiting for a connection
    if (WiFi.status() != WL_CONNECTED)
    {
      connectionState = CONNECTION_STATUS_NOT_CONNECTED;
    }
    else
    {      
      if (connectionState == CONNECTION_STATUS_NOT_CONNECTED)
      {
        connectionState = CONNECTION_STATUS_WAITING_SERVER;
        client.connect(wifi_ip_server, wifi_port);
      }

      if (client.connected())
      {
        connectionState = CONNECTION_STATUS_CONNECTED;
        Serial.println("Connected to the server");
      }
    }

    return connectionState;
  }
};
