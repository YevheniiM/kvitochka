#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <SoftwareSerial.h> 
#include <WString.h>

// SoftwareSerial ESPserial(1, 3);

const char* ssid = "Redmi123";
const char* password = "1234567890";
unsigned long previousMillis = 0;
unsigned int lighting = 0;
unsigned int humidity = 0;

int greenPin = 5; // 5
int redPin = 4;
int bluePin = 14; // 14
int greenPinOn = 0;
int redPinOn = 0;
int bluePinOn = 0; 
int ip_first = -1, ip_second = -1, ip_third = -1, ip_fourth = -1;

String webStringForLighting =  "";
String webStringForHumidity =  "";
String webPage = "", javaScript, XML;
int current_time = 0;

ESP8266WebServer server(80);

void web_build(){
  //get_data();
  //handleXML();
  webPage = "";
  
  webPage += "<!DOCTYPE html>";
  buildJavascript();
  webPage += javaScript;  
  webPage += "<head>";
  webPage += "<style>";

  webPage += "table {";
  webPage += "font-family: arial, sans-serif;";
  webPage += "border-collapse: collapse;";
  webPage += "width: 100%;";
  webPage += "}";
  webPage += "td, th {";
  webPage += "border: 1px solid #dddddd;";
  webPage += "text-align: left;";
  webPage += "padding: 8px;";
  webPage += "}";
  webPage += "tr:nth-child(even) {";
  webPage += "background-color: #89b6ff;";
  webPage += "}";
  
  webPage += ".btn-group button {";
  webPage += "width: 100%;";
  webPage += "background-color: #4CAF50; /* Green background */";
  webPage += "border: 2px solid; /* Green border */";
  webPage += "color: white; /* White text */";
  webPage += "padding: 10px 24px; /* Some padding */";
  webPage += "cursor: pointer; /* Pointer/hand icon */";
  webPage += "float: left; /* Float the buttons side by side */";
  webPage += "}";
  
  webPage += ".btn-group button:not(:last-child) {";
  webPage += "border-right: none; /* Prevent double borders */";
  webPage += "}";

  webPage += ".btn-group:after {";
  webPage += "content: "";";
  webPage += "clear: both;";
  webPage += "display: table;";
  webPage += "}";
  
  webPage += "</style>";
  webPage += "</head>";
  
  webPage += "<body onload='process();'>";
  
  webPage += "<div><h2 style='margin: 0px 0px 10px 0px; padding: 0px'>Lighting color</h2></div>";
  
  webPage += "<div class=\"btn-group\">";
  if(!greenPinOn){
    webPage += "<a href=\"green\"><button style='background-color: darkgray; border: 3px solid green; width: 33%'>Green</button></a>";
  }
  else{
    webPage += "<a href=\"green\"><button style='background-color: green; border: 3px solid green; width: 33%'>Green</button></a>";
  }

  if(!redPinOn){
    webPage += "<a href=\"red\"><button style='background-color: darkgray; border: 3px solid red; width: 33%'>Red</button></a>";
  }
  else{
    webPage += "<a href=\"red\"><button style='background-color: red; border: 3px solid red; width: 33%'>Red</button></a>";
  }

  if(!bluePinOn){
    webPage += "<a href=\"blue\"><button style='background-color: darkgray; border: 3px solid blue; width: 33%'>Blue</button></a>";
  }
  else{
    webPage += "<a href=\"blue\"><button style='background-color: blue; border: 3px solid blue; width: 33%'>Blue</button></a>";
  }
  webPage += "</div>";
  
  webPage += "<div><h2 style='margin: 10px 0px 10px 0px; padding: 0px'>Information</h2></div>";
  
  webPage += "<table>";
  webPage += "<tr>";
  webPage += "<td>Lighting level</td>";
  webPage += "<td id=\"lighting\"></td>";
  webPage += "</tr>";
  webPage += "<tr>";
  webPage += "<td>Level of water</td>";
  webPage += "<td id=\"humidity\"></td>";
  webPage += "</tr>";
  webPage += "</table>";
  
  webPage += "</body></html>";
}

void buildXML() {
//     get_data();
     XML = "<?xml version=\"1.0\"?>";
     XML += "<data>";
     XML += "<lighting>";
     XML += (String)lighting;
     XML += "</lighting>";
     XML += "<humidity>";
     XML += (String)humidity;
     XML += "</humidity>";
     XML += "</data>";
}

void buildJavascript() {
     javaScript = "<SCRIPT>\n";
     javaScript += "var xmlHttp=createXmlHttpObject();\n";
     javaScript += "function createXmlHttpObject(){\n";
     javaScript += " if(window.XMLHttpRequest){\n";
     javaScript += "    xmlHttp=new XMLHttpRequest();\n";
     javaScript += " }else{\n";
     javaScript += "    xmlHttp=new ActiveXObject('Microsoft.XMLHTTP');\n";
     javaScript += " }\n";
     javaScript += " return xmlHttp;\n";
     javaScript += "}\n";
     javaScript += "function process(){\n";
     javaScript += " if(xmlHttp.readyState==0 || xmlHttp.readyState==4){\n";
     javaScript += "   xmlHttp.open('PUT','xml',true);\n";
     javaScript += "   xmlHttp.onreadystatechange=handleServerResponse;\n"; // no brackets?????
     javaScript += "   xmlHttp.send(null);\n";
     javaScript += " }\n";
     javaScript += " setTimeout('process()',1000);\n";
     javaScript += "}\n";
     javaScript += "function handleServerResponse(){\n";
     javaScript += " if(xmlHttp.readyState==4 && xmlHttp.status==200){\n";
     javaScript += "   xmlResponse=xmlHttp.responseXML;\n";
     
     javaScript += "   xmldoc = xmlResponse.getElementsByTagName('lighting');\n";
     javaScript += "   message = xmldoc[0].firstChild.nodeValue;\n";
     javaScript += "   document.getElementById('lighting').textContent=message;\n";
     
     javaScript += "   xmldoc = xmlResponse.getElementsByTagName('humidity');\n";
     javaScript += "   message = xmldoc[0].firstChild.nodeValue;\n";
     javaScript += "   document.getElementById('humidity').textContent=message;\n";
     javaScript += " }\n";
     javaScript += "}\n";
     javaScript += "</SCRIPT>\n";
}

void handleXML() {
     buildXML();
     server.send(200, "text/xml", XML);
}

void handleRoot() {
  server.send(200, "text/html", webPage);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void) {
  pinMode(greenPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  Serial.begin(115200);
  Serial.swap();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

 

  previousMillis = millis();
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }

  
String ipadress = WiFi.localIP().toString();
  ipadress += ".";
  String sended = "";
  int int_sended;
  int i = 0;
  int id_ip = 0;
  while (ipadress[i]) {
    if (ipadress[i] != '.'){
        sended += ipadress[i];
    }else{
      int_sended = sended.toInt();
      switch(id_ip){
        case 0: ip_first = int_sended; break;
        case 1: ip_second = int_sended; break;
        case 2: ip_third = int_sended; break;
        case 3: ip_fourth = int_sended; break;
        } 
      sended = ""; 
      ++id_ip;
    }
    ++i;
}

  server.on("/", []() {
    web_build();
    server.send(200, "text/html", webPage);
  });

  server.on("/xml", handleXML);

  server.on("/green", []() {
    if(greenPinOn){
      digitalWrite(greenPin, LOW);
      greenPinOn = 0;
      //Serial.println((int)(greenPinOn*100 + redPinOn*10 + bluePinOn));
    }
    else{
      digitalWrite(greenPin, HIGH);
      greenPinOn = 1;
      //Serial.println((int)(greenPinOn*100 + redPinOn*10 + bluePinOn));
    }
    web_build();
    server.send(200, "text/html", webPage);
  });

  server.on("/red", []() {
    if(redPinOn){
      digitalWrite(redPin, LOW);
      redPinOn = 0;
      //Serial.println((int)(greenPinOn*100 + redPinOn*10 + bluePinOn));
    }
    else{
      digitalWrite(redPin, HIGH);
      redPinOn = 1;
      //Serial.println((int)(greenPinOn*100 + redPinOn*10 + bluePinOn));
    }
    web_build();
    server.send(200, "text/html", webPage);
  });

  server.on("/blue", []() {
    if(bluePinOn){
      digitalWrite(bluePin, LOW);
      bluePinOn = 0;
      //Serial.println((int)(greenPinOn*100 + redPinOn*10 + bluePinOn));
    }
    else{
      digitalWrite(bluePin, HIGH);
      bluePinOn = 1;
      //Serial.println((int)(greenPinOn*100 + redPinOn*10 + bluePinOn));
    }
    web_build();
    server.send(200, "text/html", webPage);
  });

  server.onNotFound(handleNotFound);  

  server.begin();
}

void send_ip_data(int ip_con_id, int value){
      uint8_t buff[4] = {(uint8_t)ip_con_id, (uint8_t)ip_con_id, (uint8_t)value, (uint8_t)value};
       Serial.write(buff, sizeof(buff));
}

void loop(void) {
    if(current_time < 15000){
      String str = WiFi.localIP().toString();
      int just_because_i_can = 20 - WiFi.localIP().toString().length();
    
      String minuses = "";
      for(int i = 0; i < just_because_i_can; i++){
          minuses += "-";
      }
      str += minuses;
      
      Serial.print(str);
      delay(50);
}
  

  server.handleClient();

  if(Serial.available() > 0) {
    unsigned int data = Serial.parseInt();
    if(data){
          lighting = (int)(data / 5000);
          humidity = data % 5000;
    }
  }
 
current_time++;
}
