#define USE_WIFI_NINA         false
#define USE_WIFI101           true
#include <WiFiWebServer.h>
#include <SPI.h>

//pin setup
  //Analog
    //A0          Radio signal
    //A1          Radio Carrier
    //A2          ultrasonic
    //A5          Infrared
    //A4          Magnetic
  //Digital   //ignore following
    //0 LMotor    motor direction left
    //1 RMotor    motor direction right

//Each motor requires 2 digital outputs and 1 pwm output. 1 0 for digital would give forwards, 0 1 would give backwards

const int LMotorA = 2; // IN2
const int LMotorB = 3; // IN1
const int LMotorPower  = 4; //PWM pin

const int RMotorA = 8; // IN4
const int RMotorB = 9; // IN3
const int RMotorPower = 11; //PWM pin

const char ssid[] = "osa";
const char pass[] = "password";
const int groupNumber = 0; // Set your group number to make the IP address constant - only do this on the EEERover network

//touchstart=\"fL()\" touchend = \"halt()\" 

//Webpage to return when root is requested
const char webpage1[] = \
"<html><head><style>\
.body{\
background-color: #333343;}\
#GridFather {\
display: grid;\
grid-template-columns: auto auto auto;\
background-color: #333343;\
padding: 13px;\
position:fixed;\
top:0;\
bottom:0;\
left:0;\
right:0;}\
.sideGrid {\
display: grid;\
grid-template-columns: auto auto;\
grid-template-rows: 200px 200;\
background-color: #333343;\
padding: 13px;}\
.dataTable{\
border-collapse: collapse;\
width: auto;\
height: auto;\
font-size: 15px;}\
color:#FFFF00;\
.grid-item{ /* Used within subgrids and centre grid */\
/* background-color: rgba(255, 255, 255, 0.8); */\
padding: 0px;\
font-size: 60px;\
width: 100%;\
height: 100%;\
text-align: center;}\
.btn {\
border: 1px solid rgba(0, 0, 0, 0.8);\
font-size: 40px;\
width: 150px;\
height: 180px;}\
.btn:active {\
background-color: #F5F5F5;\
box-shadow: 0 5px #666;\
transform: translateY(4px);\
</style>";

const char webpage2[] = \
"<title>Control 21</title>\
</head>\
<body>\
<!--Look at touchstart and touchend for touch interaction, use touchend to halt the robot also implement mousedoswn and mouseup for mouse events-->\
<div id=\"GridFather\">\
<div class=\"sideGrid\"> <!--Left controls-->\
<div class=\"grid-item\"><button class=\"btn\" ontouchstart=\"fL()\" ontouchend=\"halt()\" onmousedown=\"fL()\" onmouseup=\"halt()\" >Left</button></div>\
<div class=\"grid-item\"></div>\
<div class=\"grid-item\"></div>\
<div class=\"grid-item\"><button class=\"btn\" ontouchstart=\"sense()\" onmousedown=\"sense()\">Sense</button></div>\
<div class=\"grid-item\"></div>\
<div class=\"grid-item\"><button class=\"btn\" ontouchstart=\"halt()\" onmousedown=\"halt()\" >Halt</button></div>\
<div class=\"grid-item\"><button class=\"btn\" ontouchstart=\"acT()\" ontouchend=\"halt()\" onmousedown=\"acT()\" onmouseup=\"halt()\">ACW</button></div>\
<div class=\"grid-item\"><button class=\"btn\" ontouchstart=\"calibrate_magnetic()\" ontouchend=\"halt()\" onmousedown=\"calibrate_magnetic()\" onmouseup=\"halt()\">Magetic calibrate</button></div>\
</div>";

const char webpage3[] = \
"<div id= \"dataItem\" class=\"grid-item\">\
<span id=\"rock\">OFF</span>\
</div>\
<div class=\"sideGrid\"> <!-- Right controls -->\
<div class=\"grid-item\"><button class=\"btn\" ontouchstart=\"fF()\" ontouchend=\"halt()\" onmousedown=\"fF()\" onmouseup=\"halt()\">Fast</button></div>\
<div class=\"grid-item\"><button class=\"btn\" ontouchstart=\"fR()\" ontouchend=\"halt()\" onmousedown=\"fR()\" onmouseup=\"halt()\">Right</button></div>\
<div class=\"grid-item\"><button class=\"btn\" ontouchstart=\"sF()\" ontouchend=\"halt()\" onmousedown=\"sF()\" onmouseup=\"halt()\">Slow</button></div>\
<div class=\"grid-item\"></div>";

const char webpage4[] = \
"<div class=\"grid-item\"><button class=\"btn\" ontouchstart=\"sRe()\" ontouchend=\"halt()\" onmousedown=\"sRe()\" onmouseup=\"halt()\">Slow Rev</button></div>\
<div class=\"grid-item\"></div>\
<div class=\"grid-item\"><button class=\"btn\" ontouchstart=\"fRe()\" ontouchend=\"halt()\" onmousedown=\"fRe()\" onmouseup=\"halt()\">Fast Rev</button></div>\
<div class=\"grid-item\"><button class=\"btn\" ontouchstart=\"cT()\" ontouchend=\"halt()\" onmousedown=\"cT()\" onmouseup=\"halt()\">CW</button></div></div>\</div>\
</body>";

//xhttp.onreadystatechange = function() {if (this.readyState == 4 && this.status == 200) {document.getElementById(\"state\").innerHTML = this.responseText;}};\
//function ledOn() {xhttp.open(\"GET\", \"/on\"); xhttp.send();}\
//function ledOff() {xhttp.open(\"GET\", \"/off\"); xhttp.send();}\

const char webpage5[] = \
"<script>\
var xhttp = new XMLHttpRequest();\
xhttp.onreadystatechange = function() {if (this.readyState == 4 && this.status == 200) {document.getElementById(\"rock\").innerHTML = this.responseText;}};\
function fF() {xhttp.open(\"GET\", \"/fF\"); xhttp.send();}\
function sF() {xhttp.open(\"GET\", \"/sF\"); xhttp.send();}\
function fR() {xhttp.open(\"GET\", \"/fR\"); xhttp.send();}\
function fL() {xhttp.open(\"GET\", \"/fL\"); xhttp.send();}";

const char webpage6[] = \
"function fRe() {xhttp.open(\"GET\", \"/fRe\"); xhttp.send();}\
function sRe() {xhttp.open(\"GET\", \"/sR\"); xhttp.send();}\
function acT() {xhttp.open(\"GET\", \"/acT\"); xhttp.send();}\
function cT() {xhttp.open(\"GET\", \"/cT\"); xhttp.send();}\
function halt() {xhttp.open(\"GET\", \"/halt\"); xhttp.send();}\
function sense() {xhttp.open(\"GET\", \"/sense\"); xhttp.send();}\
function calibrate_magnetic() {xhttp.open(\"GET\", \"/calibrate_magnetic\"); xhttp.send();}\
</script></html>";


WiFiWebServer server(80);

//Sensing functions start
  // gets the average voltage from a specified analogue pin
float analogue_average(int analogue_pin){
  
  int   samples = 500;
  float vcrnt;
  float vtot    = 0;
  float vavg;
  
  // get average
  for (int i = 0; i < samples; i++) {
    vcrnt = analogRead(analogue_pin);
    vtot += vcrnt;
//    Serial.println(vcrnt);
  }
  
  vavg = vtot / samples;
  
  return vavg;
}

// gets the standard deviation in voltage from a specified analogue pin
float analogue_standard_deviation(int analogue_pin, float vavg){

  int   samples = 300;
  float vcrnt;
  float vtot    = 0;
  float vsd;
    
  // get standard deviation
  for (int i = 0; i < samples; i++) {  
    vcrnt = analogRead(analogue_pin);
    vtot += (vcrnt - vavg) * (vcrnt - vavg);
  }
  
  vsd = sqrt(vtot / samples);

  return vsd;
}

// gets the analogue signal frequency for either radio or infrared signals
float analogue_frequency(char type){
  
  int   analogue_pin;     // which pin is used to get the frequency from
  float noise_threshold;  // standard deviation cutoff for noise
  float vprev;            // voltage read from the current sample
  float vcrnt;            // voltage read from the previous sample
  float vavg = 0;         // average voltage
  float vsd;              // standard deviation in voltage
  float frequency;
  int   samples = 500;    // how many samples we take in order to calculate the frequency
  int   total_cycles = 0; // a counter for the number of waveforms we detect
  float start_time;       // start time of the frequency counting
  float end_time;         // end time of the frequency counting
  float total_time;

  if (type == 'r'){
    analogue_pin = A0;
    noise_threshold = 2;
  }
  else if (type == 'i'){
    analogue_pin = A5;
    noise_threshold = 6;
  }
  else{
    return 0;
  }

  vavg  = analogue_average(analogue_pin);
  vsd   = analogue_standard_deviation(analogue_pin, vavg);
     
  // reject noise
  if (vsd < noise_threshold){
    return 0;
  }
   
 
   start_time = millis();
   vprev = analogRead(analogue_pin);
   // get signal frequency seperately as ir signal is inverting amplified
  if (type == 'r'){
 
    for (int i = 0; i < samples; i++) {
    vcrnt = analogRead(analogue_pin);
    if((vcrnt > vavg) && (vprev < vavg)) {
      total_cycles += 1;
  }
    vprev = vcrnt;
   }

    }
  
  else if (type=='i')
  {
    for (int i = 0; i < samples; i++) {
    vcrnt = analogRead(analogue_pin);
    //note for ir is vcrnt smaller than vavg 
    //a constant is multiplied with vavg to get rid of unwanted signal
    if((vcrnt < 0.8*vavg) && (vprev >0.8* vavg)) {
      total_cycles += 1;
    }
    vprev = vcrnt;
     }
      }
  // calculate frequency
 end_time   = millis();
  total_time = (end_time - start_time) / 1000;
  frequency  = total_cycles/total_time;
  return frequency;
  
}

// this function detects the radio signal and carrier frequency and returns an integer:
// 0 -> no radio waves detected
// 1 -> 61kHz carrier, 151Hz signal (Gaborium)
// 2 -> 61kHz carrier, 239Hz signal (Lathwaite)
// 3 -> 89kHz carrier, 151Hz signal (Adamantine)
// 4 -> 89kHz carrier, 239Hz signal (Xirang)
int radio_detect(){
  
  float signal_frequency = analogue_frequency('r');
  float carrier_frequency;

  // rounds signal frequency
  if ((signal_frequency > 195) && (signal_frequency < 300)){
    signal_frequency = 239;
  }
  else if ((signal_frequency > 80) && (signal_frequency <= 195)){
    signal_frequency = 151;
  }
  else{
    return 0;
  }
  
  // determines carrier frequency
  
 // Serial.println(a);
  if (analogue_average(A1) >= 3){
    carrier_frequency = 61000;
  }
  else{
    carrier_frequency = 89000;
  }

  // determines which rock it is
  if ((signal_frequency == 151) && (carrier_frequency == 61000)){
    return 1;
  }
  else if ((signal_frequency == 239) && (carrier_frequency == 61000)){
    return 2;
  }
  else if ((signal_frequency == 151) && (carrier_frequency == 89000)){
    return 3;
  }
  else{
    return 4;
  }
}

// this function detects the infrared frequency and returns and returns an integer:
// 0 -> no infrared detected
// 5 -> pulses at 353Hz (Thiotimoline)
// 6 -> pulses at 571Hz (Netherite)
int infrared_detect(){
  
  float frequency = analogue_frequency('i');

  // determines which rock it is
  if ((frequency >500 ) && (frequency <650 )){
    return 6;
  }
  else if ((frequency >= 300) && (frequency < 400)){
    return 5;
  }
  else{
    return 0;
  }
}

float initial_vavg;

void calibrate_magnetic(){
  initial_vavg = analogue_average(A4);
}


// this function detects a magnetic field and returns an integer:
// 0 -> no magnetic field
// 3 -> magnetic field up   (Adamantine)
// 4 -> magnetic field down (Xirang)
int magnetic_detect(){


  float vavg = analogue_average(A4);
  float up_threshold = initial_vavg + 1;
  float down_threshold = initial_vavg - 0.5;
Serial.print (initial_vavg);
Serial.print(" ");
Serial.println(vavg);

   if (vavg > up_threshold){
      return 3;
    }
    else if (vavg < down_threshold){
      return 4;
    }
    else{
      return 0;
    }
}

// this function detects the presence of 40kHz ultrasound and returns a boolean:
// false -> no ultrasound
// true  -> ultrasound
bool ultrasound_detect(){
  
  float noise_threshold = 10;
  
  float vavg = analogue_average(A2);
  float vsd  = analogue_standard_deviation(A2, vavg);

  //Serial.println(vsd);
  // reject noise
  if (vsd < noise_threshold){
    return false;
  }
  else{
    return true;
  }
}

// this function uses all 4 sensors to determine the rock type and returns an integer:
// -1 -> inconsistant information
//  0 -> no rock detected
//  1 -> Gaborium
//  2 -> Lathwaite
//  3 -> Adamantine
//  4 -> Xirang
//  5 -> Thiotimoline
//  6 -> Netherite
int identify(){

  int  attempts     = 0;    // number of attempts to identify the rock
  int  attempts_max = 5;   // max number of attempts to identify the rock
  bool not_finished = true; // false when we have identified the rock
  int  rock         = 0;    // returns the rock's number

  
  // identifies the rock based on sensor data
  while ((attempts < attempts_max) && not_finished){
      
    int  radio      = radio_detect();
    int  infrared   = infrared_detect();
    int  magnetic   = magnetic_detect();
    bool ultrasound = ultrasound_detect();

    Serial.print(radio);
    Serial.print(", ");
    Serial.print(infrared);
    Serial.print(", ");
    Serial.print(magnetic);
    Serial.print(", ");
    Serial.println(ultrasound);
    
    // Gaborium
    if (radio == 1){
      if (ultrasound == true){
        not_finished = false;
        rock = 1;
      }
      else{
        rock = -1;
      }
    }

    // Lathwaite
    else if (radio == 2){
      not_finished = false;
      rock = 2;
    }

    // Adamantine
    else if ((radio == 3)){
      if ((radio == 3) && (magnetic == 3)){
        not_finished = false;
        rock = 3;
      }
      else{
        rock = -1;
      }
    }

    // Xirang
    else if ((radio == 4) ){
      if ((radio == 4) && (magnetic == 4)){
        not_finished = false;
        rock = 4;
      }
      else{
        rock = -1;
      }
    }
    
    //Thiotimoline
    else if (infrared == 5){
      not_finished = false;
      rock = 5;
    }

    // Netherite
    else if (infrared == 6){
      if (ultrasound == true){
        not_finished = false;
        rock = 6;
      }
      else{
        rock = -1;
      }
    }
    attempts += 1;
  }

  return rock;
}

//Return the web page
void handleRoot()
{
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, F("text/html"), "");
  server.sendContent(webpage1);
  server.sendContent(webpage2);
  server.sendContent(webpage3);
  server.sendContent(webpage4);
  server.sendContent(webpage5);
  server.sendContent(webpage6);
  server.sendContent("");
}

//Switch LED on and acknowledge
void ledON()
{
  digitalWrite(LED_BUILTIN,1);
  server.send(200, F("text/plain"), F("ON"));
}

//Switch LED on and acknowledge
void ledOFF()
{
  digitalWrite(LED_BUILTIN,0);
  server.send(200, F("text/plain"), F("OFF"));
}

// Begin motor control functions
/* Motor control logic
const int LMotorA = 2;
const int LMotorB = 3;
const int LMotorPower = 4; //PWM pin

const int RMotorA = 8;
const int RMotorB = 9;
const int RMotorPower = 10; //PWM pin

Forwards digitalWrite LMotorA, RMotorA: 1
        digitalWrite LMotorB, RMotorB: 0
        digitalWrite LMotorPower, RMotorPower: Power

RightT   digitalWrite LMotorA, RMotorA: 1
        digitalWrite LMotorB, RMotorB: 0
        digitalWrite LMotorPower Power, RMotorPower: Power/2
LeftT    digitalWrite LMotorA, RMotorA: 1
        digitalWrite LMotorB, RMotorB: 0
        digitalWrite LMotorPower Power/2, RMotorPower: Power

cT       digitalWrite LMotorA, RMotorB: 1
        digitalWrite LMotorB, RMotorA: 0
        digitalWrite LMotorPower Power, RMotorPower: Power
acT      digitalWrite LMotorB, RMotorA: 1
        digitalWrite LMotorA, RMotorB: 0
        digitalWrite LMotorPower Power, RMotorPower: Power
*/

void fF(){ // works
  int power = 255;
  Serial.println("start");
  digitalWrite(LMotorA, HIGH);
  digitalWrite(RMotorA, HIGH);
  digitalWrite(LMotorB, LOW);
  digitalWrite(RMotorB, LOW);
  analogWrite(LMotorPower, 255);
  analogWrite(RMotorPower, 255);
  Serial.println("1");
  Serial.println("2");
  Serial.println("3");
  server.send(200, F("text/plain"), F("fF")); //stub
  Serial.println("fForwards");  
}

void sF(){ // works
  int power = 110;
  Serial.println("start");
  digitalWrite(LMotorA, HIGH);
  digitalWrite(RMotorA, HIGH);
  digitalWrite(LMotorB, LOW);
  digitalWrite(RMotorB, LOW);
  analogWrite(LMotorPower, power);
  analogWrite(RMotorPower, power);
  server.send(200, F("text/plain"), F("sF")); //stub
  Serial.println("slowForwards");
}

void fRe(){ // goes forwards
  int power = 255;
  Serial.println("start");
  digitalWrite(LMotorA, LOW);
  digitalWrite(RMotorA, LOW);
  digitalWrite(LMotorB, HIGH);
  digitalWrite(RMotorB, HIGH);
  analogWrite(LMotorPower, power);
  analogWrite(RMotorPower, power);
  server.send(200, F("text/plain"), F("fRe")); //stub
  Serial.println("fastReverse");
  
}

void sRe(){ // doesn't run at all
  int power = 110;
  Serial.println("start");
  digitalWrite(LMotorA, LOW);
  digitalWrite(RMotorA, LOW);
  digitalWrite(LMotorB, HIGH);
  digitalWrite(RMotorB, HIGH);
  analogWrite(LMotorPower, power);
  analogWrite(RMotorPower, power);
  server.send(200, F("text/plain"), F("sRe")); //stub
  Serial.println("slowReverse");
  
}

void fR(){ // forwards
  int power = 255;
  Serial.println("start");
  digitalWrite(LMotorA, HIGH);
  digitalWrite(RMotorA, HIGH);
  digitalWrite(LMotorB, LOW);
  digitalWrite(RMotorB, LOW);
  analogWrite(LMotorPower, power);
  analogWrite(RMotorPower, power/2);
  server.send(200, F("text/plain"), F("fR")); //stub
  Serial.println("fastRight");
  
}

void fL(){ // right wheel forwards only
  int power = 255;
  Serial.println("start");
  digitalWrite(LMotorA, HIGH);
  digitalWrite(RMotorA, HIGH);
  digitalWrite(LMotorB, LOW);
  digitalWrite(RMotorB, LOW);
  analogWrite(LMotorPower, power/2);
  analogWrite(RMotorPower, power);
  server.send(200, F("text/plain"), F("fL")); //stub
  Serial.println("fastLeft");
  
}


//cT       digitalWrite LMotorA, RMotorB: 1
//         digitalWrite LMotorB, RMotorA: 0
//         digitalWrite LMotorPower Power, RMotorPower: Power
//acT      digitalWrite LMotorB, RMotorA: 1
//         digitalWrite LMotorA, RMotorB: 0
//         digitalWrite LMotorPower Power, RMotorPower: Power

void acT(){ // doesn't work at all
  int power = 110;
  Serial.println("start");
  digitalWrite(LMotorA, LOW);
  digitalWrite(RMotorA, HIGH);
  digitalWrite(LMotorB, HIGH);
  digitalWrite(RMotorB, LOW);
  analogWrite(LMotorPower, power);
  analogWrite(RMotorPower, power);
  server.send(200, F("text/plain"), F("acT")); //stub
  Serial.println("acTurn");
  
}

void cT(){ // left forwards, right backwards
  int power = 110;
  Serial.println("start");
  //same as acturn, (reverse motors though)
  digitalWrite(LMotorA, HIGH);
  digitalWrite(RMotorA, LOW);
  digitalWrite(LMotorB, LOW);
  digitalWrite(RMotorB, HIGH);
  analogWrite(LMotorPower, power);
  analogWrite(RMotorPower, power);
  server.send(200, F("text/plain"), F("cT")); //stub
  Serial.println("cTurn");
  
}


void halt(){ // works as it should
  Serial.println("halt start");
  digitalWrite(LMotorA, HIGH);
  digitalWrite(RMotorA, LOW);
  digitalWrite(LMotorB, LOW);
  digitalWrite(RMotorB, HIGH);
  analogWrite(LMotorPower, 0);
  analogWrite(RMotorPower, 0);
//  server.send(200, F("text/plain"), F("halt")); //stub
  Serial.println("halt end");
  
}


// void motor(){
//   String argument = server.arg("plain");
//   if (argument == "halt"){
//     halt();
//   }else if (argument == ""){
//     acTurn();
//   }else if (argument == ""){
//     cTurn();
//   }else if (argument == ""){
//     fastForwards();
//   }else if (argument == ""){
//     fastLeft();
//   }else if (argument == ""){
//     fastRight();
//   }else if (argument == ""){
//     fastReverse();
//   }else if (argument == ""){
//     slowForwards();
//   }else if (argument == ""){
//     slowReverse();
//   }
// }


void sense(){
//  server.send(200, F("text/plain"), F("ON"));
//for debugging:
/* this function uses all 4 sensors to determine the rock type and returns an integer:
  0 -> no rock detected
  1 -> Gaborium
  2 -> Lathwaite
  3 -> Adamantine
  4 -> Xirang
  5 -> Thiotimoline
  6 -> Netherite */

  //detect();
  String rockName;
  int rock = identify();
  if (rock == 0){
    server.send(200, F("text/plain"), F("No rock detected"));
    rockName = "No rock detected";
  }
  else if (rock == -1){
    rockName = "Inconsistant Data";
    server.send(200, F("text/plain"), F("Inconsistant Data"));
  }
  else if (rock == 1){
    rockName = "Gaborium";
    server.send(200, F("text/plain"), F("Gaborium"));
  }else if (rock == 2){
    rockName = "Lathwaite";
    server.send(200, F("text/plain"), F("Lathwaite"));
  }else if (rock == 3){
    rockName = "Adamantine";
    server.send(200, F("text/plain"), F("Adamantine"));
  }else if (rock == 4){
    rockName = "Xirang";
    server.send(200, F("text/plain"), F("Xirang"));
  }else if (rock == 5){
    rockName = "Thiotimoline";
    server.send(200, F("text/plain"), F("Thiotimoline"));
  }else if (rock == 6){
    rockName = "Netherite";
    server.send(200, F("text/plain"), F("Netherite"));
  }
  Serial.println("Rock:\t" + rockName);
//  server.send(200, F("text/plain"), rockName);
  
}

//Generate a 404 response with details of the failed request
void handleNotFound()
{
  String message = F("File Not Found\n\n"); 
  message += F("URI: ");
  message += server.uri();
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? F("GET") : F("POST");
  message += F("\nArguments: ");
  message += server.args();
  message += F("\n");
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, F("text/plain"), message);
}

void setup()
{
  pinMode(A0,INPUT);  //radio frequency   inductor system
  pinMode(A1,INPUT);  //IR frequency      IR sensor
  pinMode(A2,INPUT);  //Magnetic field    hall effect
  pinMode(A5,INPUT);
  pinMode(A4,INPUT);  
    //Left Motor
  pinMode(LMotorA,OUTPUT);
  pinMode(LMotorB,OUTPUT);
  pinMode(LMotorPower,OUTPUT);
    //Right Motor
  pinMode(RMotorA,OUTPUT);
  pinMode(RMotorB,OUTPUT);
  pinMode(RMotorPower,OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LMotorA, LOW);
  digitalWrite(LMotorB, LOW);
  digitalWrite(LMotorPower, LOW);
  digitalWrite(RMotorA, LOW);
  digitalWrite(RMotorB, LOW);
  digitalWrite(RMotorPower, LOW);
  
  digitalWrite(LED_BUILTIN, LOW);
  calibrate_magnetic();
  Serial.begin(9600);

  //Wait 10s for the serial connection before proceeding
  //This ensures you can see messages from startup() on the monitor
  //Remove this for faster startup when the USB host isn't attached
  while (!Serial && millis() < 10000);  

  Serial.println(F("\nStarting Web Server"));

  //Check WiFi shield is present
  if (WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println(F("WiFi shield not present"));
    while (true);
  }

  //Configure the static IP address if group number is set
  if (groupNumber)
    WiFi.config(IPAddress(192,168,0,groupNumber+1));

  // attempt to connect to WiFi network
  Serial.print(F("Connecting to WPA SSID: "));
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED)
  {
    delay(500);
    Serial.print('.');
  }

  //Register the callbacks to respond to HTTP requests
  server.on(F("/"), handleRoot);
  server.on(F("/on"), ledON);
  server.on(F("/off"), ledOFF);
  server.on(F("/sense"), sense);
  //server.on(F("motor"), motor);

  server.on(F("/fF"), fF);
  server.on(F("/sF"), sF);
  server.on(F("/fR"), fR);
  server.on(F("/fL"), fL);
  server.on(F("/fRe"), fRe);
  server.on(F("/sRe"), sRe);
  server.on(F("/acT"), acT);
  server.on(F("/cT"), cT);
  server.on(F("/halt"), halt);
  server.on(F("/calibrate_magnetic"), calibrate_magnetic);

  
  server.onNotFound(handleNotFound);
  
  server.begin();
  
  Serial.print(F("HTTP server started @ "));
  Serial.println(static_cast<IPAddress>(WiFi.localIP()));
}

//Call the server polling function in the main loop
void loop()
{
  server.handleClient();
//  Serial.print("hello");
}
