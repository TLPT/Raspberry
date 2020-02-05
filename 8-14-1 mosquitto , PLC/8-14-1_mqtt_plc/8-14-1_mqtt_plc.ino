// 2-4-2 프로그램을 사용 유튜브 참조 https://youtu.be/3ZRlAd884MI
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "i2r"; // 와이파이 AP 이름
const char* password = "00000000";  //와이파이 비밀번호
const char* mqtt_server = "192.168.0.19"; //브로커 주소
const char* outTopic = "outTopic"; // 사용자가 결정해서 기록
const char* inTopic = "inTopic"; // 사용자가 결정해서 기록
const char* clientName = "/kdi/603333Client";  // 다음 이름이 중복되지 않게 꼭 수정 바람 - 생년월일 추천

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[200];

String inputString = "";         // a String to hold incoming data
boolean stringComplete = false;  // whether the string is complete

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    Serial1.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientName)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(outTopic, "Reconnected");
      // ... and resubscribe
      client.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  serialEvent();
}

void serialEvent() {
  if (stringComplete) {
    //Serial.println(inputString);
    for(int i=0;i<inputString.length();i++)  
      msg[i]=inputString.charAt(i);
    msg[inputString.length()]=0;
    CheckMsg();
    client.publish(outTopic,msg);  // RS232통신 RX 단자로 들어온 값 mqtt로 송신
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
  if(Serial.available() == false)
    return;
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:`
    inputString += inChar;

    Serial.write(inChar);
    //LS산전 PLC 응답의 마지막은 0x03dlek.
    if (inChar == char(3)) {
      stringComplete = true;
    }
  }
}

//아스키번호 0x20 아래는 mqtt 통신에서 깨져 이를 0x20으로 바꾼다.
void CheckMsg() {
  int i;
  for(i=0; i<200; i++)
    if(msg[i] >0 && msg[i] < 0x20)
      msg[i] = 0x20;
}
