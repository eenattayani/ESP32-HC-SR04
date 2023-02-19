/*  WEB Server for Head Circle counter
 *  
 *  
 * 
 *  
 *  
 */

#include <WiFi.h>
#include <Preferences.h>
#include <LiquidCrystal_I2C.h>

//Wi-Fi
const char* ssid     = "wifi-APLKB";
const char* password = "aplkb";

WiFiServer server(80);
String header;

// Preferences /EEPROM
Preferences preferences;

unsigned long ms_current  = 0;
unsigned long ms_previous = 0;

const int depanTrig = 4;
const int depanEcho = 5;
const int belakangTrig = 2;
const int belakangEcho = 15;
const int kananTrig = 26;
const int kananEcho = 25;
const int kiriTrig = 19;
const int kiriEcho = 18;

// define sound speed in cm/us
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

const float konstanta = 45; // 44.5;
const float midKonstanta = konstanta * 2/3;
const float phi = 3.14;

// ukuran kotak = 44.2 cm
const float kaliF = 0; // 1.8
const float kaliB = 0; // 1.3
const float kaliR = 0; // 2
const float kaliL = 0; // 1.8


float jarakDepan;
float jarakBelakang;
float jarakKanan;
float jarakKiri;

float diaFB;
float diaRL;
float rataDia;
float lingkaran;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  // wait for Serial to be ready
  delay(50);

  pinMode(depanTrig, OUTPUT);     // Sets the depanTrig as an OUTPUT
  pinMode(depanEcho, INPUT);      // Sets the depanEcho as an INPUT
  pinMode(belakangTrig, OUTPUT);  // Sets the belakang Trig as an OUTPUT
  pinMode(belakangEcho, INPUT);   // Sets the belakang Echo as an INPUT
  pinMode(kananTrig, OUTPUT);     // Sets the kananTrig as an OUTPUT
  pinMode(kananEcho, INPUT);      // Sets the kananEcho as an INPUT
  pinMode(kiriTrig, OUTPUT);      // Sets the kiriTrig as an OUTPUT
  pinMode(kiriEcho, INPUT);       // Sets the kiriEcho as an INPUT

  lcd.begin();
  lcd.backlight();

  startup_wifi();
  
}

float sensor_baca(int trig, int echo)
{
  long duration; 
  float jarakCm;
  float jarakInch;

  // Clears the Trig
  digitalWrite(trig, LOW);  
  delayMicroseconds(2);
  
  // Sets the Trig on HIGH state for 10 micro seconds
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  
  // Reads the Echo, returns the sound wave travel time in microseconds
  duration = pulseIn(echo, HIGH);

  // Calculate the distance
  jarakCm = duration * SOUND_SPEED/2;
  
  // Convert to inches
  jarakInch = jarakCm * CM_TO_INCH;

  return jarakCm;  
}



void ukur()
{
  jarakDepan = sensor_baca(depanTrig, depanEcho);
  jarakDepan = jarakDepan + kaliF;
  delay(100);
  jarakBelakang = sensor_baca(belakangTrig, belakangEcho);
  jarakBelakang = jarakBelakang + kaliB;
  delay(100);
  jarakKiri = sensor_baca(kiriTrig, kiriEcho);
  jarakKiri = jarakKiri + kaliL;
  delay(100);
  jarakKanan = sensor_baca(kananTrig, kananEcho);
  jarakKanan = jarakKanan + kaliR;
  delay(100);

  // jika ada objek, maka data sensor diolah
  if (jarakDepan < midKonstanta && jarakBelakang < midKonstanta && jarakKanan < midKonstanta && jarakKiri < midKonstanta)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("  Scanning...");
    lcd.setCursor(0,1);
    lcd.print(" Object Ready");
    
    // jika 
    // sensor depan & sensor belakang selisih di bawah 2 cm, 
    // sensor kanan & sensor kiri selisih di bawah 2 cm,
    // maka ready ambil data
    
    float selFB = jarakDepan - jarakBelakang;
    float selRL = jarakKanan - jarakKiri;
    if ( selFB < 0 ) selFB = selFB * -1;
    if ( selRL < 0 ) selRL = selRL * -1;
    

    if ( selFB < 2 && selRL  < 2)
    {
      // olah data dari sensor 
      diaFB = konstanta - ( jarakDepan + jarakBelakang );
      diaRL = konstanta - ( jarakKanan + jarakKiri );
      rataDia = ( diaFB + diaRL ) / 2;
      lingkaran = phi * rataDia;
    
      lcd.clear();
    
      // Prints the distance in the Serial Monitor
      Serial.print("      ");
      Serial.println(jarakDepan, 1);
      Serial.print(jarakKiri, 1);
      Serial.print("            ");
      Serial.println(jarakKanan, 1);
      Serial.print("      ");
      Serial.println(jarakBelakang, 1);
    
      Serial.print("diameter FB: ");
      Serial.println(diaFB);
      Serial.print("diameter RL: ");
      Serial.println(diaRL);
      Serial.print("diameter lingkaran: ");
      Serial.println(rataDia);
      
      
      Serial.println("===");
      Serial.println(lingkaran, 1);
      Serial.println("===");
    
      lcd.setCursor(0,0);
      lcd.print("D1:");
      lcd.print(diaFB, 1);
    
      lcd.setCursor(9,0);
      lcd.print("D2:");
      lcd.print(diaRL, 1);
    
      lcd.setCursor(0,1);
      lcd.print("L.Kepala:");
      lcd.print(lingkaran, 1);
      lcd.print(" cm");
    }    
  } else {
    Serial.println("objek tidak terdeteksi...");    

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("  Scanning...");
    lcd.setCursor(0,1);
    lcd.print("   No Object  ");
  }
  
  
  
}

void startup_wifi()
{
  
  Serial.print("Setting AP (Access Point)...");
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.begin();

  lcd.setCursor(0,0);
  lcd.print("scanning...");  
}

void loop() {   
  byte counterWIFI = 0; 
  
  WiFiClient client = server.available();
  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      
      ms_current = millis();

      if ( ms_current - ms_previous >= 1000)
      {
        ms_previous = ms_current;
        Serial.println(ms_current);

//        ukur();
        
        Serial.println("in while");
        counterWIFI++;
        Serial.println(counterWIFI);
        if (counterWIFI > 5)
        {
          counterWIFI = 0;
          break;
        }
      } 

      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;

        if (c == '\n') {                    // if the byte is a newline character          
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();                                                      

            ukur();
                      
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println("h5 {text-align: center;}");
            client.println(".button { border-radius: 8px; background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 18px; margin: 2px; cursor: pointer;}");
            client.println(".button1 {background-color: #FF1111;}");
            client.println(".button2 {background-color: #555555;}");
            client.println("input { border-radius: 8px; background-color: #eee; width: 30%; padding: 10px;}");
            client.println("</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>PENGUKURAN LINGKAR KEPALA</h1>");
            client.println("<hr>");                                            
                       
            client.println("<p><h4>Data Sensor Ultra Sonik</h4></p>");
            client.println("<form method=\"GET\" action=\"\">");
            client.println("  <p><h5>Sensor Depan</h5></p>");
            client.println("  <input type=\"text\" name=\"sensorF\" value=\"" + String(jarakDepan) + "\" >");  
            client.println("  <p><h5>Sensor Belakang</h5></p>");
            client.println("  <input type=\"text\" name=\"sensorB\" value=\"" + String(jarakBelakang) + "\" >");  
            client.println("  <p><h5>Sensor Kanan</h5></p>");
            client.println("  <input type=\"text\" name=\"sensorR\" value=\"" + String(jarakKanan) + "\" >");  
            client.println("  <p><h5>Sensor Kiri</h5></p>");
            client.println("  <input type=\"text\" name=\"sensorL\" value=\"" + String(jarakKiri) + "\" >");  
            
            client.println("  <br>");                                 
            client.println("  <hr>");  

            client.println("  <p><h5>Diamater Depan Belakang</h5></p>");
            client.println("  <input type=\"text\" name=\"diaFB\" value=\"" + String(diaFB) + "\" >");
            client.println("  <p><h5>Diamater Kanan Kiri</h5></p>");
            client.println("  <input type=\"text\" name=\"diaRL\" value=\"" + String(diaRL) + "\" >");
            client.println("  <p><h5>Diameter Rata-Rata</h5></p>");
            client.println("  <input type=\"text\" name=\"rataDia\" value=\"" + String(rataDia) + "\" >");
            client.println("  <p><h5>Ukuran Lingkaran Kepala</h5></p>");
            client.println("  <input type=\"text\" name=\"lingkaran\" value=\"" + String(lingkaran) + "\" >");
                        
            client.println("  <p><button class=\"button\">Ambil Data</button></p>");
            client.println("</form>");
                                    
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } 
          else 
          { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }                
  }

    // Serial.println(header);
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");    
  }

  ms_current = millis();

  if ( ms_current - ms_previous >= 1000)
  {
    ms_previous = ms_current;
//    Serial.println(ms_current);
    ukur();
  }  
}
