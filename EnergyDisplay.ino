#include <WiFi.h>
#include <SPI.h>
#include <GxEPD.h>
#include <GxGDEW042T2/GxGDEW042T2.cpp>
#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>
#include "bgimage.h"
#include "arrowimage.h"
#include "arrowrightimage.h"

#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

#define ONE_WIRE_BUS 15 

GxIO_Class io(SPI, SS, 17, 16);
GxEPD_Class display(io, 16, 4); 

const char* ssid     = "";
const char* password = "";

const char* host = "";

const int GRAPH_SIZE = 130;
int useGraph[GRAPH_SIZE];

void setup()
{
    Serial.begin(115200);
    delay(10);
    display.init();
    btStop();

    display.fillScreen(GxEPD_WHITE);
    bigText(90, 105, "Solar Monitor");
    smallText(90, 135, "(c)2018 Josef Jahn");
    display.update();

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

//    WiFi.begin(ssid, password);
//    while (WiFi.status() != WL_CONNECTED) {
//        delay(500);
//        Serial.print(".");
//    }

}

void wifiReconnect() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
}
int value = 0;

void loop()
{
    delay(5000);

//    if (WiFi.status() != WL_CONNECTED) {
      wifiReconnect();
//      delay (1000);
//    }
    
    Serial.print("connecting to ");
    Serial.println(host);

    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
    }

    Serial.println("Getting Data");
    // This will send the request to the server
    client.print(String("GET ") + "/status/soc.txt" + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: Keep-Alive\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
    }

    // Read all the lines of the reply from server and print them to Serial
    if (client.available()) {
        Serial.println("Data received!");
        readPastHeader(&client);
        String battery = client.readStringUntil('\n');
        String pv = client.readStringUntil('\n');
        String use = client.readStringUntil('\n');
        String grid = client.readStringUntil('\n');
        String battuse = client.readStringUntil('\n');
        String curtime = client.readStringUntil('\n');
        String curdate = client.readStringUntil('\n');
        if (pv == "-0")
          pv = "0";
        if (grid == "-0")
          grid = "0";
        if (use == "-0")
          use = "0";
        if (battuse == "-0")
          battuse = "0";
          
        display.fillScreen(GxEPD_WHITE);
        display.drawExampleBitmap(gImage_IMG_0001, 0, 0, 400, 300, GxEPD_WHITE);

        smallText(90, 115, battuse+" kW");
        smallText(245, 115, grid+" kW");
        bigText(245, 25, pv+" kW");
        bigText(245, 260, use+" kW");
        tinyText(350, 8, curtime);


        int battgraphmax = battery.toInt(); //Just so happens to be 100 px high, so batt/100*100
        
        for (int x=16; x<69; x++) {
          for (int y=0; y<battgraphmax; y++) {
            display.drawPixel(x, 204-y, GxEPD_BLACK);
          }
        }

        if (battgraphmax == 100) {
          smallTextWhite(20, 140, "99%");
        } else if (battgraphmax > 70) {
          smallTextWhite(20, 140, battery+"%");
        } else if (battgraphmax > 30) {
          smallTextWhite(20, 175, battery+"%");
        } else {
          smallText(20, 140, battery+"%");
        }

        int numBattArrows = 0;
        float fbattuse = battuse.toFloat();
        if (fbattuse < -0.1) {
          if (fbattuse >= -0.7) {
            numBattArrows = 1;
          } else if (fbattuse >= -1.4) {
            numBattArrows = 2;
          } else {
            numBattArrows = 3;
          }
          drawArrows(gImage_arrowrightimage, numBattArrows, 100, 160);
        } else if (fbattuse > 0.1) {
          if (fbattuse <= 0.7) {
            numBattArrows = 1;
          } else if (fbattuse <= 1.4) {
            numBattArrows = 2;
          } else {
            numBattArrows = 3;
          }
          drawArrows(gImage_arrowimage, numBattArrows, 100, 160);
        }

        int numGridArrows = 0;
        float fgrid = grid.toFloat();
        if (fgrid < -0.1) {
          if (fgrid >= -.7) {
            numGridArrows = 1;
          } else if (fgrid >= -2.0) {
            numGridArrows = 2;
          } else if (fgrid >= -6.0) {
            numGridArrows = 3;
          } else {
            numGridArrows = 4;
          }
          drawArrows(gImage_arrowrightimage, numGridArrows, 248, 160);
        } else if (fgrid > 0.1) {
          if (fgrid <= 0.7) {
            numGridArrows = 1;
          } else if (fgrid <= 2.0) {
            numGridArrows = 2;
          } else if (fgrid <= 6.0) {
            numGridArrows = 3;
          } else {
            numGridArrows = 4;
          }
          drawArrows(gImage_arrowimage, numGridArrows, 248, 160);
        }

    }


    Serial.println("Getting PV Histogram");
    client.print(String("GET ") + "/status/lastpv.txt" + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: Keep-Alive\r\n\r\n");
    timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
    }

    if (client.available()) {
        readPastHeader(&client);
        Serial.println("PV Histogram received!");

        for (int a=10; a<=140; a+=2) {
          display.drawPixel(a, 10, GxEPD_BLACK);
          display.drawPixel(a, 70, GxEPD_BLACK);
        }
        for (int a=10; a<=70; a+=2) {
          display.drawPixel(10, a, GxEPD_BLACK);
          display.drawPixel(140, a, GxEPD_BLACK);
        }
        int x=0;
        while (client.available()) {
          String curpv = client.readStringUntil('\n');
          float curpvf = curpv.toFloat();
          float maxy = curpvf / 7800.0 * 60.0;
          for (int y = 0; y<=((int)maxy); y++) {
            display.drawPixel(x+10, 70-y, GxEPD_BLACK);
          }
          x++;
          if (x>130)
            break;
        }
    }
    tinyText(146, 8, "7.8");
    tinyText(146, 64, "0");

    Serial.println("PV Histogram completed!");

    Serial.println("Getting Usage Histogram");
    client.print(String("GET ") + "/status/lastuse.txt" + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
    }

    int useGraphX1 = 10;
    int useGraphY1 = 231;
    int useGraphX2 = useGraphX1 + 130;
    int useGraphY2 = useGraphY1 + 60;
    int maxUse = 0;

    if (client.available()) {
        readPastHeader(&client);
        Serial.println("Usage Histogram received!");

        for (int a=useGraphX1; a<=useGraphX2; a+=2) {
          display.drawPixel(a, useGraphY1, GxEPD_BLACK);
          display.drawPixel(a, useGraphY2, GxEPD_BLACK);
        }
        for (int a=useGraphY1; a<=useGraphY2; a+=2) {
          display.drawPixel(useGraphX1, a, GxEPD_BLACK);
          display.drawPixel(useGraphX2, a, GxEPD_BLACK);
        }
        int x=0;
        while (x < 130) {
          if (client.available()) {
            float u = client.readStringUntil('\n').toFloat();
            useGraph[x] = (int)u;
          } else {
            useGraph[x] = 0;
          }
          if (maxUse < useGraph[x]) {
            maxUse = useGraph[x];
          }
          x++;
        }

        if (maxUse < 2000) {
          maxUse = 2000;
        }
        
        x=0;
        while (x < 130) {
          float curpvf = (float)useGraph[x];
          float maxy = curpvf / (float)maxUse * 60.0;
          for (int y = 0; y<=((int)maxy); y++) {
            display.drawPixel(x+useGraphX1, useGraphY2-y, GxEPD_BLACK);
          }
          x++;
        }
    }
    tinyText(useGraphX2 + 6, useGraphY1 - 2, String((float)maxUse / 1000, 1) );
    tinyText(useGraphX2 + 6, useGraphY2 - 6, "0");
    Serial.println("Usage Histogram completed!");
    display.update();    
    Serial.println();
    Serial.println("closing connection");
    client.stop();
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);

    delay(1000*60*5);


}

float round_to_dp( float in_value, int decimal_place )
{
  float multiplier = powf( 10.0f, decimal_place );
  in_value = roundf( in_value * multiplier ) / multiplier;
  return in_value;
}
void drawArrows(const uint8_t* image, int num, int x, int y) {
  for (int i=0; i<num; i++) {
    display.drawExampleBitmap(image, x+(i*18), y, 18, 24, GxEPD_WHITE);
  }
}
  
bool readPastHeader(WiFiClient *pClient)
{
 bool bIsBlank = true;
 while(true)
 {
   if (pClient->available()) 
   {
     char c = pClient->read();
     if(c=='\r' && bIsBlank)
     {
       // throw away the /n
       c = pClient->read();
       return true;
     }
     if(c=='\n')
       bIsBlank = true;
     else if(c!='\r')
       bIsBlank = false;
     }
  }
}

void tinyText(uint16_t x, uint16_t y, String text)
{
  const char* name = "FreeSans9pt7b";
  const GFXfont* f = &FreeSans9pt7b;
  
  display.setRotation(0);
  display.setFont(f);
  display.setTextColor(GxEPD_BLACK);

  display.setCursor(x, y+9);
  display.print(text); 
} 

void smallText(uint16_t x, uint16_t y, String text)
{
  const char* name = "FreeSans12pt7b";
  const GFXfont* f = &FreeSans12pt7b;
  
  display.setRotation(0);
  display.setFont(f);
  display.setTextColor(GxEPD_BLACK);

  display.setCursor(x, y+20);
  display.print(text); 
} 

void smallTextWhite(uint16_t x, uint16_t y, String text)
{
  const char* name = "FreeSans12pt7b";
  const GFXfont* f = &FreeSans12pt7b;
  
  display.setRotation(0);
  display.setFont(f);
  display.setTextColor(GxEPD_WHITE);

  display.setCursor(x, y+20);
  display.print(text); 
} 

void bigText(uint16_t x, uint16_t y, String text)
{
  const char* name = "FreeSans18pt7b";
  const GFXfont* f = &FreeSans18pt7b;
  
  display.setRotation(0);
  display.setFont(f);
  display.setTextColor(GxEPD_BLACK);

  display.setCursor(x, y+25);
  display.print(text); 
} 

