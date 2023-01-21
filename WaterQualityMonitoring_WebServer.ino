// libraries
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
// Libraries for DS18B20 temperature sensor
#include <OneWire.h>
#include <DallasTemperature.h>

const char* ssid = "HaziqiPhone";   // Enter SSID here
const char* password = "123cacaca"; // Enter Password here

ESP8266WebServer server(80);        // create server object

OneWire oneWire(5);                 // DS18B20 connect to pin 5
DallasTemperature temp_sensor(&oneWire);

// Declaration
float Temperature = 0;
int read_ADC;
int NTU;
float pHvalue;
unsigned long int avgValue;  // Store the average value of the sensor feedback
int buf[10],temp;

void setup() {
  Serial.begin(9600);
  delay(100);
  
  temp_sensor.begin();
  pinMode(A0,INPUT);   // turbidity sensor
  pinMode(A1,INPUT);   // pH sensor

  Serial.println("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("IPv4 Address: "); Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

void handle_OnConnect() {

  // reading temperature
  temp_sensor.requestTemperatures(); // to send a command to get the temperature values from the sensor
  Temperature = temp_sensor.getTempCByIndex(0); // read temperature value

  // reading turbidity(NTU)
  read_ADC = analogRead(A0);
  
  if(read_ADC>208){
    read_ADC=208;
  }
  NTU = map(read_ADC, 0, 208, 300, 0);
  
    // reading acidity(pH)
    for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
  { 
    buf[i]=analogRead(A1);
    delay(10);
  }
  for(int i=0;i<9;i++)          //sort the analog from small to large
  {
    for(int j=i+1;j<10;j++)
    {
      if(buf[i]>buf[j])
      {
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }
  avgValue=0;
  for(int i=2;i<8;i++)                      //take the average value of 6 center sample
    avgValue+=buf[i];
    
  float phValue=(float)avgValue*5.0/1024/6; //convert the analog into millivolt
  phValue = 3.5*phValue;                      //convert the millivolt into pH value

  // combine all data into one format (JSONf format)

  server.send(200, "text/html", SendHTML(Temperature)); // send HTML page
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float Temperaturestat){
  String page = "<!DOCTYPE html> <html>\n";
  page +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  page +="<title>ESP8266 Weather Report</title>\n";
  page +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  page +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  page +="p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  page +="</style>\n";
  page +="</head>\n";
  page +="<body>\n";
  page +="<div id=\"webpage\">\n";
  page +="<h1>Temperature</h1>\n";

  page +="<p>Temperature: ";
  page +=(int)Temperaturestat;
  page +="°C</p>";
  page +="</div>\n";
  page +="</body>\n";
  page +="</html>\n";
  
  return page;
}