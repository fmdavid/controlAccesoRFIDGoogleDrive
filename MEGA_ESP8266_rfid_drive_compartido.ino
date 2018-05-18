#include "WiFiEsp.h" 
#include "DHT.h"
#include <SPI.h>
#include <MFRC522.h>
char ssid[] = "WIFISSID";            
char pass[] = "WIFIPASS";        
int status = WL_IDLE_STATUS;     

#define RST_PIN  8    
#define SS_PIN  9   
MFRC522 mfrc522(SS_PIN, RST_PIN); 

char host[] = "maker.ifttt.com";
char eventName[]   = "eventName";
char key[] = "key";

WiFiEspClient client;

#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const int pinBuzzer = 4 ; 

void setup()
{
  for (int i =10 ; i<13 ; i++)
            pinMode(i, OUTPUT);

  Color(255,0,0);
  
  Serial.begin(115200); 
  Serial1.begin(115200); 
  SPI.begin();        
  mfrc522.PCD_Init();
  Serial.println("Lector RFID iniciado");

  dht.begin();
  
  WiFi.init(&Serial1);

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("Modulo no presente. Reinicie el Arduino y el ESP01 (Quite el cable que va de CH_PD a 3.3V y vuelvalo a colocar)");
    while (true);
  }

  while ( status != WL_CONNECTED) {
    Serial.print("Intentando conectar a la red WiFi: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
  }

  Serial.println();
  printWifiStatus();
  Serial.println();
  Serial.println("Esperando leer RFID...");

  pinMode (pinBuzzer , OUTPUT) ; 

  for (int i = 0; i <50; i++) 
  {
  digitalWrite (pinBuzzer, LOW);
  delay(2);
  digitalWrite (pinBuzzer, HIGH);
  delay(2);
  }  

  for (int i = 0; i <50; i++) 
  {
  digitalWrite (pinBuzzer, LOW);
  delay(1);
  digitalWrite (pinBuzzer, HIGH);
  delay(1);
  }  
  
  Color(0,0,255);
}

void loop()
{
  if ( mfrc522.PICC_IsNewCardPresent()) 
        {  
            if ( mfrc522.PICC_ReadCardSerial()) 
            {
                  Color(0,255,0);
                  for (int i = 0; i <100; i++) 
                  {
                  digitalWrite (pinBuzzer, LOW);
                  delay(1);
                  digitalWrite (pinBuzzer, HIGH);
                  delay(1);
                  }  
              
                  Serial.print("Card UID:");
                  String UID_HEX="";
                  Serial.print("Printing HEX UID : ");
                  for (byte i = 0; i < mfrc522.uid.size; i++) {
                    UID_HEX += String(mfrc522.uid.uidByte[i], HEX);
                  }
                  Serial.println(UID_HEX);
                  Serial.println();
				  
                  mfrc522.PICC_HaltA();
                  
                  float h = dht.readHumidity();
                  float t = dht.readTemperature();

                if (isnan(h) || isnan(t)) {
                  Serial.println("Error DHT sensor!");
                  return;
                }
                 
                
                  Serial.print("TEMP. ");
                  Serial.print(t);
                  Serial.println("*C");
                  Serial.print("HUMED. ");
                  Serial.print(h);
                  Serial.println("%");

                  int n = map (analogRead(A0), 0,1024, 0, 100) ;
                  Serial.print("LUZ ");
                  Serial.print(n);
                  Serial.println("%");
                  
                  Serial.println("Iniciando conexion...");
                  if (client.connect(host, 80)) {
                    Serial.println("Conectado al servidor");
                
                    String url = "/trigger/";
                    url += eventName;
                    url += "/with/key/";
                    url += key;
                    url += "?value1=";
                    url += UID_HEX;
                    url += "&value2=";
                    url += "[TEMP:" + String(t) + "][HUM:" + String(h) + "%][LUZ:" + String(n) + "%]";
                
                    Serial.print("Solicitando URL: ");
                    Serial.println(url);
                
                    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                                 "Host: " + host + "\r\n" +
                                 "Connection: close\r\n\r\n");
                    
                  }
                  while (client.available()) {
                    char c = client.read();
                    Serial.write(c);
                  
                  }
                
                  if (client.connected()) {
                    Serial.println();
                    Serial.println("Desconectando del servidor...");
                    client.flush();
                    client.stop();
                  }

                  Color(0,0,255);
            }      
      } 
  }

void printWifiStatus()
{
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void Color(int R, int G, int B)
{     
    analogWrite(10 , G) ; 
    analogWrite(11, R) ; 
    analogWrite(12, B) ; 
}
