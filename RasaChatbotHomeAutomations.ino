// Load Wi-Fi library
#include "WiFi.h"
#include <HTTPClient.h>

// Replace with your network credentials
const char* ssid = "ZTE_2.4G_9hTyU2";    
const char* password = "N35QywEJ"; 

char chatbot_server[] = "http://192.168.1.2:5005/webhooks/rest/webhook";

// Set web server port number to 80
WiFiServer server(80);
WiFiClient client1;

HTTPClient https;

String chatbot_Q;
String chatbot_A;
String json_String;
uint16_t dataStart = 0;
uint16_t dataEnd = 0;
String dataStr;
int httpCode = 0;

int relay1=2,relay2=4;

typedef enum 
{
  do_webserver_index,
  send_chatbot_request,
  get_chatbot_list,
}STATE_;

STATE_ currentState;

void WiFiConnect(void){
    WiFi.begin(ssid, password);
    Serial.print("Connecting to ");
    Serial.println(ssid);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    currentState = do_webserver_index;
}

const char html_page[] PROGMEM = {
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n"  // the connection will be closed after completion of the response
    //"Refresh: 1\r\n"         // refresh the page automatically every n sec
    "\r\n"
    "<!DOCTYPE HTML>\r\n"
    "<html>\r\n"
    "<head>\r\n"
      "<meta charset=\"UTF-8\">\r\n"
      "<title>Rasa Chatbot | Innovate Yourself</title>\r\n"
      "<link rel=\"icon\" href=\"https://scontent.fdel12-1.fna.fbcdn.net/v/t39.30808-6/294621658_418452853633900_3701435387171708210_n.jpg?_nc_cat=109&ccb=1-7&_nc_sid=09cbfe&_nc_ohc=Lm62Jiqmjd0AX_HHOef&_nc_ht=scontent.fdel12-1.fna&oh=00_AfBG9wGzPzMX9lSwBBDUcRasYxLPe93SjHPOlNa9wUsSQQ&oe=64890A1C\" type=\"image/x-icon\">\r\n"
    "</head>\r\n"
    "<body>\r\n"
    "<img alt=\"SEEED\" src=\"https://i0.wp.com/innovationyourself.com/wp-content/uploads/2020/07/WhatsApp-Image-2020-07-24-at-12.18.03-AM.jpeg\" height=\"100\" width=\"250\">\r\n"
    "<p style=\"text-align:center;\">\r\n"
    "<img alt=\"Chatbot\" src=\"https://info.rasa.com/hubfs/rasa_logo_horizontal_purple-3.png\" height=\"200\" width=\"400\">\r\n"
    "<h1 align=\"center\">Innovate Yourself</h1>\r\n" 
    "<h1 align=\"center\">Rasa Chatbot | Home Automation</h1>\r\n" 
    "<div style=\"text-align:center;vertical-align:middle;\">"
    "<form action=\"/\" method=\"post\">"
    "<input type=\"text\" placeholder=\"Please enter your question\" size=\"35\" name=\"chatbottext\" required=\"required\"/>\r\n"
    "<input type=\"submit\" value=\"Submit\" style=\"height:30px; width:80px;\"/>"
    "</form>"
    "</div>"
    "</p>\r\n"
    "</body>\r\n"
    "<html>\r\n"
};
 
void setup()
{
    Serial.begin(115200);

    pinMode(relay1, OUTPUT);
    pinMode(relay2, OUTPUT);
    
    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    while(!Serial);

    Serial.println("WiFi Setup done!");
    
    WiFiConnect();
    // Start the TCP server server
    server.begin();
}
 
void loop()
{
  switch(currentState){
    case do_webserver_index:
      Serial.println("Web Production Task Launch");
      client1 = server.available();              // Check if the client is connected
      if (client1){
        Serial.println("New Client.");           // print a message out the serial port
        // an http request ends with a blank line
        boolean currentLineIsBlank = true;    
        while (client1.connected()){
          if (client1.available()){
            char c = client1.read();             // Read the rest of the content, used to clear the cache
            json_String += c;
            if (c == '\n' && currentLineIsBlank) {                                 
              dataStr = json_String.substring(0, 4);
              Serial.println(dataStr);
              if(dataStr == "GET "){
                client1.print(html_page);        //Send the response body to the client
              }         
              else if(dataStr == "POST"){
                json_String = "";
                while(client1.available()){
                  json_String += (char)client1.read();
                }
                Serial.println(json_String); 
                dataStart = json_String.indexOf("chatbottext=") + strlen("chatbottext="); // parse the request for the following content
                chatbot_Q = json_String.substring(dataStart, json_String.length());                    
                client1.print(html_page);
                Serial.print("Your Question is: ");
                Serial.println(chatbot_Q);
                // close the connection:
                delay(10);
                client1.stop();       
                currentState = send_chatbot_request;
              }
              json_String = "";
              break;
            }
            if (c == '\n') {
              // you're starting a new line
              currentLineIsBlank = true;
            }
            else if (c != '\r') {
              // you've gotten a character on the current line
              currentLineIsBlank = false;
            }
          }
        }
      }
      delay(1000);
      break;
    case send_chatbot_request:
      Serial.println("Ask Chatbot a Question Task Launch");
      if (https.begin(chatbot_server)) {  // HTTPS
        https.addHeader("Content-Type", "application/json"); 
//        String token_key = String("Bearer ") + chatbot_token;
//        https.addHeader("Authorization", token_key);
//        String payload = String("{\"model\": \"text-davinci-003\", \"prompt\": \"") + chatbot_Q + String("\", \"temperature\": 0, \"max_tokens\": 100}"); //Instead of TEXT as Payload, can be JSON as Paylaod
        Serial.println(chatbot_Q);
        String payload = String("{\"sender\": \"Ashish\", \"message\": \"") + chatbot_Q + "\"}"; //Instead of TEXT as Payload, can be JSON as Paylaod
        Serial.println(payload);
        httpCode = https.POST(payload);   // start connection and send HTTP header
        payload = "";
        currentState = get_chatbot_list;
      }
      else {
        Serial.println("[HTTPS] Unable to connect");
        delay(1000);
      }
      break;
    case get_chatbot_list:
      Serial.println("Get Chatbot Answers Task Launch");
      // httpCode will be negative on error      
      // file found at server
      Serial.println(httpCode);
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        
        String payload = https.getString();
        Serial.println(payload);
        dataStart = payload.indexOf("\"text\":\"") + 8;
        dataEnd = payload.indexOf("\"}]", dataStart); 
        chatbot_A = payload.substring(dataStart, dataEnd);
        Serial.print("Chatbot Answer is: ");
        Serial.println(chatbot_A);
        if(chatbot_A=="Light ON"){
          Serial.println("Lights on");
          digitalWrite(relay1, 0);
        }
        else if(chatbot_A=="Light OFF"){
          Serial.println("Lights off");
          digitalWrite(relay1, 1);
        }
        else if(chatbot_A=="Fan ON"){
          Serial.println("fan on");
          digitalWrite(relay2, 0);
        }
        else if(chatbot_A=="Fan OFF"){
          Serial.println("fan off");
          digitalWrite(relay2, 1);
        }
        
        Serial.println("Wait 5 before next round...");
        currentState = do_webserver_index;
      }
      else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
        while(1);
      }
      https.end();
      delay(5000);
      break;
  }
}
