#include "esp_camera.h"
#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

AsyncWebServer server(80);
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    input1: <input type="text" name="input1">
    <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";
void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
} 


//
// WARNING!!! Make sure that you have either selected ESP32 Wrover Module,
//            or another board which has PSRAM enabled
//

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
#define CAMERA_MODEL_AI_THINKER

#include "camera_pins.h"
char in[20];
const char* PARAM_INPUT_1 = "input1";
const char* soft_ap_ssid = "ESP32AP";
const char* soft_ap_password = "12345678";
const char* ssid = "TP-Link_2.4G";
const char* password = "B512BGCE";
String websocket_server_host = "192.168.1.171";
const uint16_t websocket_server_port = 8888;
void OnWiFiEvent(WiFiEvent_t event)
{
  switch (event) {
 
    case SYSTEM_EVENT_STA_CONNECTED:
      Serial.println("ESP32 Connected to WiFi Network");
      break;
    case SYSTEM_EVENT_AP_START:
      Serial.println("ESP32 soft AP started");
      
      break;
    case SYSTEM_EVENT_AP_STACONNECTED:
      Serial.println("Station connected to ESP32 soft AP");
      break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      Serial.println("Station disconnected from ESP32 soft AP");
      break;
  
    default: break;
  }
}
using namespace websockets;
WebsocketsClient client;

void setup() {
  

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  WiFi.onEvent(OnWiFiEvent);
 
  WiFi.mode(WIFI_MODE_AP);
 
  WiFi.softAP(soft_ap_ssid, soft_ap_password);

  delay(100);
  
  IPAddress Ip(192, 168, 1, 1);
  IPAddress NMask(255, 255, 255, 0);
  delay(100);
  WiFi.softAPConfig(Ip,Ip,NMask);
  Serial.print(WiFi.softAPIP());

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 40;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_96X96;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }


  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

 

  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      
      inputParam = PARAM_INPUT_1;
      
    }

    else {
      inputMessage = "No message sent";
      inputParam = "192.168.1.1";
    }
  
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                    + inputParam + ") with value: " + inputMessage +
                                    "<br><a href=\"/\">Return to Home Page</a>");
    delay(100);
    websocket_server_host = inputMessage;
    delay(100);
    connectToWebSocket();
  });
  server.onNotFound(notFound);
  server.begin();
  
  
 
}
void connectToWebSocket() {
  WiFi.softAPdisconnect(true);
 
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
  bool connected = client.connect(websocket_server_host,websocket_server_port,"/");
  delay(100);
  while (!connected) {
    delay(500);
    Serial.println(connected);
    delay(500);
    Serial.print('.');
    
  }
  Serial.println('connected');

  
}


void loop() {
  if (client.available()){
    camera_fb_t *fb = esp_camera_fb_get();
    if(!fb){
      Serial.println("Camera capture failed");
      esp_camera_fb_return(fb);
      return;
    }

    if(fb->format != PIXFORMAT_JPEG){
      Serial.println("Non-JPEG data not implemented");
      return;
    }

    client.sendBinary((const char*) fb->buf, fb->len);
    esp_camera_fb_return(fb);
  }
  else{
    
  }
  

}