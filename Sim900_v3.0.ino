 
#include <SoftwareSerial.h>
#include <EEPROM.h> // build-in IDE libary
#include <HX711.h> // libary used: BOGDE HX711 from Github

#define APIKEY         "AN3gJzGEn1cRtsmrygdi5uvrIz18i5prmXI8g5sAmZJEA1y4" // replace your Xively api key here
#define FEEDID         1865252593   // replace your Xively feed ID
#define SCALEID        "Scales1"    //change this to match xively scale feed ID

SoftwareSerial GPRS(7,8);  //SoftwareSerial mySerial(7, 8); //set pins to communicate with GSM sheild
HX711 scale(A3, A2);   // parameter "gain" is ommited; the default value 128 is used by the library

int LoadA;               // to calculate the loadcell ramp low value
int LoadB;               // to calculate the loadcell ramp high value
int LoadBRX = 3;      // change this figure to the predefined calibration weight
long ValueA;             // stores the low value of the ramp
long ValueB;             // stores the high value of the ramp
long ValueATemp;         // stores the temp low value of the ramp
int ValueOut;          // the output value in kilograms        (changed from long, could change to float for decimel - ws)
long ForceValueRaw;      // the raw value from the HX711
long ForceTemp;          // Value without zero-ing value
int CounterPulse;        // for calibration counter
int IncomingByte;        // serial.read
long Address = 0;        // Starting at the first byte on the eeprom
//int weight;

const int greenPin =  13;    // green LED pin 13 or onboard
const int redPin = 12;       // red LED pin 12

const int buttonPin = 2;     // the number of the pushbutton pin
int buttonState = 0;         // variable for reading the pushbutton status

//------EEprom-----function-----------------------------------
//This write function will write a 4 byte (32bit) long to the eeprom at the specified Address to adress + 3.
void EEPROMWritelong(int Address, long value)
{
  //Decomposition from a long to 4 bytes by using bitshift. One = Most significant -> Four = Least significant byte
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);

  //Write the 4 bytes into the eeprom memory.
  EEPROM.write(Address, four);
  EEPROM.write(Address + 1, three);
  EEPROM.write(Address + 2, two);
  EEPROM.write(Address + 3, one);
}

//This read function will return a 4 byte (32bit) long from the eeprom at the specified Address to adress + 3.
long EEPROMReadlong(long Address)
{
  //Read the 4 bytes from the eeprom memory.
  long four = EEPROM.read(Address);
  long three = EEPROM.read(Address + 1);
  long two = EEPROM.read(Address + 2);
  long one = EEPROM.read(Address + 3);
  //Return the recomposed long by using bitshift.
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}
///--------end of EEprom Function---------///


 
void setup()
{
  
  
  delay(500);
  pinMode(buttonPin, INPUT); // sets buttonPin as input for switch control
  pinMode(greenPin, OUTPUT); // initialize led pin 13 as output
  pinMode(redPin, OUTPUT); // set led pin 12 as output
  
  GPRS.begin(19200); //setting GPS baud
  Serial.begin(19200);    // the serial baud rate 
  delay(500);

  ValueOut = 0;
  ValueA = (EEPROMReadlong(0)); // read the EEprom value
  ValueB = (EEPROMReadlong(4)); // read the EEprom value
  LoadB  = (EEPROMReadlong(8)); // read the EEprom value
  ValueATemp = 0;
  CounterPulse = 0;
 
}

////End of Setup////


///connect to gsm///
void ConnectNetwork()
{
  GPRS.println("AT+CGATT=1"); // Attach from GPRS serivce
  Response();
  delay(1000);
  GPRS.println("AT+CSTT=\"internet\""); // Start task and set APN
  delay(3000);
  Response();
  GPRS.println("AT+CIICR"); // Bring up wireless connection with GPRS
  delay(2000);
  Response();
  GPRS.println("AT+CIFSR"); // Get local IP address
  Response();
  GPRS.println("AT+CIPSPRT=1"); // Set its prompt echo '>'and shows "send ok" when sending is successful
  Response();
  GPRS.println("AT+CIPQSEND=0"); // Set normal data transmitting mode
  Response();
}
/////enf of function////


////close connection function///
void CloseConnection()
{
  GPRS.println("AT+CIPCLOSE"); // Close TCP connection
  Response();
  delay(3000);
}
////end of function///



/////Get Current Date & Time////
void GetDateTime()
{
  ConnectNetwork();
  delay(500);
  GPRS.println("AT+CLTS=1"); //get current date & time from network
  delay(500);
  Response();
  GPRS.println("AT+CCLK?"); //display timestamp
  Response();
  delay(500);
  
}
/////end of function////



////Get raw value reading///
void RawValue()
{
  ForceValueRaw = (scale.read() / 128); // gain 128/128 = 1// raw value from loadcell A

}
/////End of Function////



////Calibration Function/////
void CalibrationMode()
{
    RawValue();
    
    long address = 0;

    if (LoadBRX >= 1) // do something if the value is larger then 1
    {
      LoadB = LoadBRX;
      LoadBRX = 0;
      Serial.println ("Calibration weight value stored");
    }
{
    CounterPulse++;  // raise pulse counter
    IncomingByte = ' '; // clear incomingbyte
    delay (2000);
  }

  if (CounterPulse == 1)
  {
    ValueATemp = ForceValueRaw;  // store the 0 load value in a variable
    Serial.println ("Calibration started >> Place calibration load on scales");
    CounterPulse = 2;
    digitalWrite(greenPin, HIGH);// turn LED on, added
    delay (1000); // changed from 100
    digitalWrite(greenPin, LOW);// turn LED off, added
    delay (500);
    digitalWrite(greenPin, HIGH);// turn LED on, added
    delay (1000); // changed from 100
    digitalWrite(greenPin, LOW);// turn LED off, added
    delay (500);
    digitalWrite(greenPin, HIGH);// turn LED on, added
    delay (1000); // changed from 100
    digitalWrite(greenPin, LOW);//turn LED off, added
    delay (500);
    digitalWrite(greenPin, HIGH);// turn LED on, added
    delay (1000); // changed from 100
    digitalWrite(greenPin, LOW);// turn LED off, added
    delay (500);
    digitalWrite(greenPin, HIGH);// turn LED on, added
    delay (1000); // changed from 100
    digitalWrite(greenPin, LOW);// turn LED off, added
    delay (500);
    digitalWrite(greenPin, HIGH);// turn LED on, added
    delay (1000); // changed from 100
    digitalWrite(greenPin, LOW); //turn LED off, added
    delay (500);
    digitalWrite(greenPin, HIGH);// turn LED on, added
  }

  if (CounterPulse == 3)
  {
    // Storing values to EEprom
    //Writing first long.
    EEPROMWritelong(address, ValueATemp);  //ValueA
    address += 4;
    //Writing second long.
    EEPROMWritelong(address, ForceValueRaw);  //ValueB
    address += 4;
    //Writing third long.
    EEPROMWritelong(address, LoadB);  //LoadB
    address += 4;
    delay(2000);
    digitalWrite(greenPin, LOW);
    Serial.println ("Calibration finished and values stored,remove weigth, Arduino will perform auto restart");
    delay(5000);
    digitalWrite(redPin, HIGH);// turn LED on, added
    delay (10); //changed from 10
    CounterPulse = 0; // reset of Pulse counter
    delay(5000);
    digitalWrite(redPin, LOW);// turn LED off, added
    delay(500);
    digitalWrite(redPin, HIGH);// turn LED on, added
    delay(500);
    digitalWrite(redPin, LOW);// turn LED off, added
    delay(500);
    digitalWrite(redPin, HIGH);// turn LED off, added
    delay(500);
    digitalWrite(redPin, LOW);// turn LED off, added
    delay(500);
    digitalWrite(redPin, HIGH);// turn LED off, added
    delay(500);
    digitalWrite(redPin, LOW);// turn LED on, added
    delay(500);
    digitalWrite(redPin, HIGH);// turn LED off, added
    delay(500);
    digitalWrite(redPin, LOW);// turn LED off, added
    delay(500);
    digitalWrite(redPin, HIGH);// turn LED off, added
    delay(500);
    digitalWrite(redPin, LOW);// turn LED off, added
    delay(500);
    asm volatile ("  jmp 0"); // invoke restart of arduino
  }
}
/////End of Function////



///take weight reading///
void GetWeight()
{
  RawValue();
  
  Serial.println("Weighing Mode Active"); // added
  ForceTemp = ((LoadB - 0.0) / (ValueB - ValueA)) * ((ForceValueRaw) - ValueA) + 0.0;
  ValueOut = ForceTemp;

  
  Serial.print("ForceValueRaw> ");
  Serial.print(ForceValueRaw);
  Serial.print(" kilograms> ");
  Serial.print(ValueOut);
  Serial.print(" LoadB> ");
  Serial.print(LoadB);
  Serial.print(" ValueA> ");
  Serial.print(ValueA);
  Serial.print(" ValueB> ");
  Serial.print(ValueB);
  Serial.print(" PC>");
  Serial.println(CounterPulse);
  delay(500);

  ConnectNetwork();
  delay(500);
  
  Send2Xively();
}

/////End of Function////



///Send To Xively Function///
void Send2Xively()
{
  /*
  GPRS.println("AT+CGATT=1"); // Attach from GPRS serivce
  Response();
  delay(1000);
  GPRS.println("AT+CSTT=\"internet\""); // Start task and set APN
  delay(3000);
  Response();
  GPRS.println("AT+CIICR"); // Bring up wireless connection with GPRS
  delay(2000);
  Response();
  GPRS.println("AT+CIFSR"); // Get local IP address
  Response();
  GPRS.println("AT+CIPSPRT=1"); // Set its prompt echo '>'and shows "send ok" when sending is successful
  Response();
  GPRS.println("AT+CIPQSEND=0"); // Set normal data transmitting mode
  Response();
*/
  
  GPRS.println("AT+CIPSTART=\"tcp\",\"api.xively.com\",\"8081\""); //Start up TCP connection
  delay(2000);
  Response();
  GPRS.println("AT+CIPSEND"); // Begin send data to Xively through TCP connection
  Response();
  // Request Syntax for TCP Socket Connection Type
  GPRS.print("{");
  GPRS.print("\"method\" : \"put\",");
  GPRS.print("\"resource\" : \"/feeds/");
  GPRS.print(FEEDID);
  GPRS.print("\",");
  GPRS.print("\"params\" : {},");
  GPRS.print("\"headers\" : {\"X-ApiKey\":\"");
  GPRS.print(APIKEY);
  GPRS.print("\"},");
  GPRS.print("\"body\" : ");
  GPRS.print("{\"version\" : \"1.0.0\",");
  GPRS.print("\"datastreams\" : ");
//  GPRS.print("[{\"id\" : \"weight\",");
  GPRS.print("[{\"id\" : \"");
  GPRS.print(SCALEID);
  GPRS.print("\",");
  GPRS.print("\"current_value\" : \"");
  GPRS.print(ValueOut,1);
  GPRS.println("\"}]},"); 
  GPRS.print("\"token\" : \"lee\"}");
  delay(500);
  // End of Request Syntax
  GPRS.println((char)26); // After sending AT+CIPSEND ,tap CTRL+Z to send data
  delay(500);
  Response();
//  GPRS.println("AT+CIPCLOSE"); // Close TCP connection
//  Response();
//  delay(3000);
CloseConnection();
}

void Response()
{
  delay(100);
  while(!GPRS.available());
  if(GPRS.available()){
    while(GPRS.available()){
      Serial.write(GPRS.read());
    }
  }
}

/////End of Function////

 
void loop()

{
 
  buttonState = digitalRead(buttonPin); // read the state of the pushbutton value

 
  if (buttonState == HIGH)  // check if the pushbutton is pressed, if it is, the buttonState is HIGH:
  {
  CalibrationMode();
  }


  if (buttonState == LOW)
  {
    GetWeight();
  }
}
