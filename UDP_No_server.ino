#define USE_WIFI_NINA         false
#define USE_WIFI101           true
#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>

//pin setup
  //Analog
    //A0          Radio signal
    //A1          Radio Carrier
    //A2          ultrasonic
    //A3          Infrared
    //A4          Magnetic
  //Digital   //ignore following
    //0 LMotor    motor direction left
    //1 RMotor    motor direction right

//Each motor requires 2 digital outputs and 1 pwm output. 1 0 for digital would give forwards, 0 1 would give backwards

const int LMotorA = 2;
const int LMotorB = 3;
const int LMotorPower = 4; //PWM pin

const int RMotorA = 8;
const int RMotorB = 9;
const int RMotorPower = 10; //PWM pin


//Webpage to return when root is requested //Method: \ at the end of every line, and before all quotations

const char ssid[] = "Cameron Arduino";
const char pass[] = "CCWCM34PD";
const int groupNumber = 0; // Set your group number to make the IP address constant - only do this on the EEERover network

//Begin UDP
unsigned int localPort = 2390;      // local port to listen on (python connects to this)
char packetBuffer[255]; //buffer to hold incoming packet
char  ReplyBuffer[] = "acknowledged";       // a string to send back

WiFiUDP Udp;


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
    analogue_pin = A3;
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

// this function detects a magnetic field and returns an integer:
// 0 -> no magnetic field
// 3 -> magnetic field up   (Adamantine)
// 4 -> magnetic field down (Xirang)
int magnetic_detect(){

  float vavg = analogue_average(A4);
  float up_threshold = 739.5;
  float down_threshold = 737.5;
  
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
    else if ((radio == 3) || (magnetic == 3)){
      if ((radio == 3) && (magnetic == 3)){
        not_finished = false;
        rock = 3;
      }
      else{
        rock = -1;
      }
    }

    // Xirang
    else if ((radio == 4) || (magnetic == 4)){
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
}//Sensing functions end


//Switch LED on and acknowledge
void ledON()
{
  digitalWrite(LED_BUILTIN,1);
  Serial.println("ledON");//why is this line running for everything???
  }

//Switch LED on and acknowledge
void ledOFF()
{
  digitalWrite(LED_BUILTIN,0);
  Serial.println("ledOFF");
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
        analogWrite LMotorPower, RMotorPower: Power

RightT   digitalWrite LMotorA, RMotorA: 1
        digitalWrite LMotorB, RMotorB: 0
        analogWrite LMotorPower Power, RMotorPower: Power/2
LeftT    digitalWrite LMotorA, RMotorA: 1
        digitalWrite LMotorB, RMotorB: 0
        analogWrite LMotorPower Power/2, RMotorPower: Power

cT       digitalWrite LMotorA, RMotorB: 1
        digitalWrite LMotorB, RMotorA: 0
        analogWrite LMotorPower Power, RMotorPower: Power
acT      digitalWrite LMotorB, RMotorA: 1
        digitalWrite LMotorA, RMotorB: 0
        analogWrite LMotorPower Power, RMotorPower: Power
*/

void halt(){
  Serial.println("halt start");
  int power = 0;
  digitalWrite(LMotorA, 1);
  digitalWrite(RMotorA, 0);
  digitalWrite(LMotorB, 1);
  digitalWrite(RMotorB, 0);
  analogWrite(LMotorPower, power);
  analogWrite(RMotorPower, power);
  Serial.println("halt end"); 
}

void fF(){
  Serial.println("start");
  int power = 220;
  digitalWrite(LMotorA, 1);
  digitalWrite(RMotorA, 1);
  digitalWrite(LMotorB, 0);
  digitalWrite(RMotorB, 0);
  analogWrite(LMotorPower, power);
  analogWrite(RMotorPower, power);
  Serial.println("fForwards");  
}

void sF(){
  Serial.println("start");
  int power = 110;
  digitalWrite(LMotorA, 1);
  digitalWrite(RMotorA, 1);
  digitalWrite(LMotorB, 0);
  digitalWrite(RMotorB, 0);
  analogWrite(LMotorPower, power);
  analogWrite(RMotorPower, power);
  Serial.println("slowForwards");
}

void fRe(){
  Serial.println("start");
  int power = 220;
  digitalWrite(LMotorA, 0);
  digitalWrite(RMotorA, 0);
  digitalWrite(LMotorB, 1);
  digitalWrite(RMotorB, 1);
  analogWrite(LMotorPower, power);
  analogWrite(RMotorPower, power);
  Serial.println("fastReverse");
  
}

void sRe(){
  Serial.println("start");
  int power = 110;
  digitalWrite(LMotorA, 0);
  digitalWrite(RMotorA, 0);
  digitalWrite(LMotorB, 1);
  digitalWrite(RMotorB, 1);
  analogWrite(LMotorPower, power);
  analogWrite(RMotorPower, power);
  Serial.println("slowReverse");
  
}

void fR(){
  Serial.println("start");
  int power = 220;
  digitalWrite(LMotorA, 1);
  digitalWrite(RMotorA, 1);
  digitalWrite(LMotorB, 0);
  digitalWrite(RMotorB, 0);
  analogWrite(LMotorPower, power);
  analogWrite(RMotorPower, power/2);
  Serial.println("fastRight");
 }

void fL(){
  Serial.println("start");
  int power = 220;
  digitalWrite(LMotorA, 1);
  digitalWrite(RMotorA, 1);
  digitalWrite(LMotorB, 0);
  digitalWrite(RMotorB, 0);
  analogWrite(LMotorPower, power/2);
  analogWrite(RMotorPower, power);
  Serial.println("fastLeft");
}


//cT       digitalWrite LMotorA, RMotorB: 1
//         digitalWrite LMotorB, RMotorA: 0
//         analogWrite LMotorPower Power, RMotorPower: Power
//acT      digitalWrite LMotorB, RMotorA: 1
//         digitalWrite LMotorA, RMotorB: 0
//         analogWrite LMotorPower Power, RMotorPower: Power


void acT(){
  Serial.println("start");
  int power = 110;
  digitalWrite(LMotorA, 0);
  digitalWrite(RMotorA, 1);
  digitalWrite(LMotorB, 1);
  digitalWrite(RMotorB, 0);
  analogWrite(LMotorPower, power);
  analogWrite(RMotorPower, power);
  Serial.println("acTurn");
}

void cT(){
  Serial.println("start");
  //same as acturn, (reverse motors though)
  int power = 110;
  digitalWrite(LMotorA, 1);
  digitalWrite(RMotorA, 0);
  digitalWrite(LMotorB, 0);
  digitalWrite(RMotorB, 1);
  analogWrite(LMotorPower, power);
  analogWrite(RMotorPower, power);
  Serial.println("cTurn");
}




void fCalls(String fName){
  if (fName == "on"){
    ledON();
  }else if(fName == "off" ){
    ledOFF();
  }else if(fName == "sense" ){
    sense();
  }else if(fName == "FF" ){
    fF();
  }else if(fName == "SF" ){
    sF();
  }else if(fName == "FR" ){
    fR();
  }else if(fName == "FL" ){
    fL();
  }else if(fName == "FRe" ){
    fRe();
  }else if(fName == "SRe" ){
    sRe();
  }else if(fName == "ACT" ){
    acT();
  }else if(fName == "CT" ){
    cT();
  }else if(fName == "H" ){
    halt();
  }
}


void sense(){
  //server.send(200, F("text/plain"), F("ON"));
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
  String rockName = "";
  int rock = identify();
  if (rock == 0){
    rockName = "No rock detected";
  }else if (rock == 1){
    rockName = "Gaborium";
  }else if (rock == 2){
    rockName = "Lathwaite";
  }else if (rock == 3){
    rockName = "Adamantine";
  }else if (rock == 4){
    rockName = "Xirang";
  }else if (rock == 5){
    rockName = "Thiotimoline";
  }else if (rock == 6){
    rockName = "Netherite";
  }
  Serial.println("Rock:\t" + rockName);
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  char ReplyBuffer[20];
  rockName.toCharArray(ReplyBuffer,20);
  Udp.write(ReplyBuffer);
  Udp.endPacket();
}



void setup(){
  pinMode(A0,INPUT);  //radio frequency   inductor system
  pinMode(A1,INPUT);  //IR frequency      IR sensor
  pinMode(A2,INPUT);  //Magnetic field    hall effect
  pinMode(A3,INPUT);
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
  digitalWrite(LED_BUILTIN, 0);

  Serial.begin(9600);

  //Wait 10s for the serial connection before proceeding
  //This ensures you can see messages from startup() on the monitor
  //Remove this for faster startup when the USB host isn't attached
  while (!Serial && millis() < 10000);

  Serial.println(F("\nStarting Web server"));

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


  Serial.print(F("HTTP server started @ "));
  Serial.println(static_cast<IPAddress>(WiFi.localIP()));
  
  Udp.begin(localPort);

  ledON();
  ledOFF();
  fF();
  sF();
  fRe();
  sRe();
  fR();
  fL();
  acT();
  cT();
  halt();
  sense();

}

//Call the server polling function in the main loop
void loop()
{
  int packetSize = Udp.parsePacket();

  if (packetSize)

  {
    Serial.print("Received packelt of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remoteIp = Udp.remoteIP();
    Serial.print(remoteIp);
    Serial.print(", port ");
    Serial.println(Udp.remotePort());
    // read the packet into packetBufffer
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) packetBuffer[len] = 0;
    Serial.println("Contents:");
    //Serial.println(packetBuffer);
    fCalls(String(packetBuffer));
    // send a reply, to the IP address and port that sent us the packet we received
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(ReplyBuffer);
    Udp.endPacket();
  

  }
}
