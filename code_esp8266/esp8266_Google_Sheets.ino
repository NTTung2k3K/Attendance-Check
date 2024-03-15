#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>




#define RX_PIN 4  // D2
#define TX_PIN 5  // D1




SoftwareSerial mySerial(RX_PIN, TX_PIN);








const char* host = "script.google.com";
const int httpsPort = 443;




String GAS_ID = "AKfycbxwqrEoKz6spYC2vFJJ03DF2JwKmve4ciUnCzW0JE8JXLMWz986iE2LpHgzlFvvJXmC";








const char* ssid = "FPTU_Student";
const char* password = "12345678";


















WiFiClientSecure client;
















void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  delay(500);




  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }




  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();




  client.setInsecure();
}




void loop() {
  if (mySerial.available()) {
    String studentId = mySerial.readStringUntil('\n');
    Serial.println("Chuỗi vừa nhận được là: ");
    Serial.println(studentId);
    sendata(studentId);
    delay(4000);
  }
}








void sendata(String studentId) {
  Serial.print("connecting to ");
  Serial.println(host);




  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }




  String url = "/macros/s/" + GAS_ID + "/exec?id=" + studentId;
  Serial.println(url);




  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "User-Agent: BuildFailureDetectorESP8266\r\n" + "Connection: close\r\n\r\n");
  Serial.println("OK");
}








