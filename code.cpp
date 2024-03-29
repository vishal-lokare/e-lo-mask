//HM 10 bluetooth module connected to TX and RX for Serial I/O
//LM35 at A0 for analog input
//TS118 at

#include<SoftwareSerial.h>
#include<EEPROM.h>
#include <RTCZero.h>

// constants for the thermistor calculations
const float a = -412.6;
const float b = 140.41;
const float c = 0.00764;
const float d = -0.0000000000000000625;
const float e = -0.00000000000000000000000115;

// constants for the thermopile calculation
const float k = 0.004313;
const float delta = 2.468;

const float reftemp = 25; // reference temperature is 25C
const float shiftv = 0.6; // amount to shift thermopile voltage for negative V values in range
const float verr = 0.6;  // voltage error introduced to thermopile by circuit


//strip sensors
const int LM351 = A0;    //adjustment temperature sensor
const int TS1181 = A1;    //body temp sensor
const int TS1181R = A2;

//mask sensors
const int LM352 = A3;   //adjustment temperature sensor
const int TS1182 = A4;    //body temp sensor
const int TS1182R = A5;
const int TS1183 = A6;    //body temp sensor
const int TS1183R = A7;

const int HM10Key = 8;

short Mm;
short HH;
short DD;
short MM;
short YY;

SoftwareSerial bt(2,3);
RTCZero rtc;

String WASTE, ADDR, Time;

void Register()
{
  while(1)
  {
    if(HM10Key == 1)
    {
      bt.print("AT+DISI?");
      if(bt.find("OK+DISIS"))
      {
        mySerial.readBytes(WASTE, 66); //useless bytes
        mySerial.readBytes(ADDR,12); //take the address of the connected device

        EEPROM.write(0, '1');    //to write "registered" at 0th index

        for(int i=0; i<12; i++)   // to write the MAC Address of the connected device for authentication
          EEPROM.write(i+1, ADDR[i]);


        delay(1000);

        bt.read(Time);

        DD = (int)(Time[0]-'0') * 10;
        DD += (int)(Time[1]-'0');

        MM = (int)(Time[2]-'0') * 10;
        MM += (int)(Time[3]-'0');

        YY = (int)(Time[4]-'0') * 10;
        YY += (int)(Time[5]-'0');

        HH = (int)(Time[6]-'0') *10;
        HH += (int)(Time[7]-'0');

        Mm = (int)(Time[8]-'0') * 10;
        Mm += (int)(Time[9]-'0');

        rtc.setTime(HH, Mm, SS);
        rtc.setDate(DD, MM, YY);

        break;
      }
    }
    delay(1000);
  }
}

void readTempBack()
{
  float TPback = analogRead(TS1181);
  float TRback = analogRead(TS1181R);

  // work out thermistor temp from reading
  float v1 = ( TRback/ 1024) * 5; // source voltage is 5v so reading is fraction of that
  float r = -(v1*1000)/(v1-5); // to get the resistance
  float ambtemp = a + b * sqrt(1+c*r) + d*pow(r,5) + e*pow(r,7); // ambient temp

  float comp = k * (pow(ambtemp,4-delta)-pow(reftemp,4-delta));  // equivalent thermopile V for amb temp

  // calculate the thermopile temp
  float v2 = ( TPback / 1024) * 5 + comp - verr - shiftv; // thermopile voltage
  float objtemp = pow((v2+k*pow(ambtemp,4-delta))/k, 1/(4-delta)); // object temp

  writeTemp(objtemp); 
}

void writeTemp(float a)
{
  int b = a;
  int i=0;
  String c;

  while(a)
  {
    c[i] = (a/10) + '0';
    i++;
  }
  EEPROM.write(c);

}

//with addon
void withAddOn()
{

}

//without addon
void woAddOn()
{
  if(rtc.getMinutes() == "02" || rtc.getMinutes() == "32")
  {
    readTempBack();
  }
  delay(1000);
}

int AddOn, Reg;

void setup() {

  rtc.begin();
  Serial.begin(9600);
  bt.begin(9600);

  pinMode(8, INPUT);

  int MaskCheck = analogRead(LM352);
  MaskCheck = (MaskCheck*500)/1023;

  //check mask is connected or not
  if(MaskCheck > 80 || MaskCheck < 10)
    AddOn = 0;
  else
    AddOn = 1;

  //check new device or registered
  if(!EEPROM.read(0))
    Register();

}

void loop()
{
  if(AddOn)
    withAddOn();    //function to be called when addon is connected
  else
    woAddOn();    //function to be called when addon is not connected

  delay(1000);
}
