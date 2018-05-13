#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <time.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "constants.h"

// ID's of rooms can be found here: 
// hue_bridge_address/debug/clip.html
// http://192.168.0.XX/debug/clip.html?commandurl=%2Fapi%2Fnewdeveloper&messagebody=&response=
// https://www.developers.meethue.com/documentation/getting-started
// READ THIS CAREFULLY!
String LIVING_ROOM = "1";
String KITCHEN = "2";
String BEDROOM = "3";

// URL for me: 
String hue_url = hue_bridge_address + "/api/" + hue_developer_key + "/groups/" + KITCHEN + "/action";
String on_argument = "{\"on\":true}";
String off_argument = "{\"on\":false}";

const byte interruptPin = 12; // D6
volatile bool detected_motion = false;
bool last_detected_motion = false;


// By default 'time.nist.gov' is used with 60 seconds update interval and
// no offset
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 60*60*2, 600000);


// You can specify the time server pool and the offset, (in seconds)
// additionaly you can specify the update interval (in milliseconds).
// NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

void setup() {
  pinMode(interruptPin, INPUT);
  
    // Start connecting to your local router
  WiFi.begin(ssid.c_str(), password.c_str());

  // begin print function
  Serial.begin(115200);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Print that we are connected and our IP.
  // Note that writing down the IP helps us later when setting the lights
  // and the IP is a variable in the app we will make in part 3.
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();

}

void send_signal_to_ifttt(String event_name, String key){
   HTTPClient http;
  http.begin("http://maker.ifttt.com/trigger/" + event_name + "/with/key/" + key);
  int httpCode = http.GET();
  if(httpCode > 0){
    String payload = http.getString();
    Serial.println(payload);
  }
}

void turn_hue_light_off(){
  HTTPClient http;
  http.begin(hue_url);
  int httpCode = http.PUT(off_argument);
}

void turn_hue_light_on(){
  HTTPClient http;
  http.begin(hue_url);
  int httpCode = http.PUT(on_argument);
}

void toggle_hue_light(bool newstate){
  if(newstate){
    turn_hue_light_on();
  }
  else{
    turn_hue_light_off();
  }
}

void toggle_ifttt(bool detected_motion){
  if(detected_motion){
    send_signal_to_ifttt(detected_motion_key, ifttt_key);
  }
  else{
    send_signal_to_ifttt(no_motion_key, ifttt_key);
  }
}
int total_time = 0;
void loop() {

  timeClient.update();
  if(timeClient.getHours() > 7 && timeClient.getHours() < 20){
    delay(1000*60*5); // Sleep for a long amount of minutes as we have nothing to do during the day
  }
  else{
    detected_motion = digitalRead(interruptPin);
  
    if(detected_motion != last_detected_motion){
      last_detected_motion = detected_motion;
    
      // Print stuff to console
      Serial.print("A state in motion occurred");
      toggle_hue_light(detected_motion);
      toggle_ifttt(detected_motion);
    }
  }
  delay(250);
  
}
