// Pimp Kelvin Standard Edition
// Author: Alexander Göbel
// For more information visit https://www.tindie.com/stores/alegro/
// For latest Software visit https://github.com/Voltage-Ranger-Electronics
// Created 15 Feb 2016
// For Arduino Nano w/ ATmega328
// Function     PIN
// SDA          A4
// SCL          A5
// LED          A13

/*
 * Libary Problem LCD Durcheinander bei Zeilen -> Behoben durch vertauschen
 */

// ### Libarys ---------------------------------------------------------------------------------------###
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


// ### Constants -------------------------------------------------------------------------------------###
//#define ADRESS  0x6E // ADC Adress on Demoboard
#define ADRESS  0x68 // ADC Adress on Pimp Kelvin the 1st
//(Excludes R/W bit)

#define COMAND 0x9C
#define COUNT 5
#define DATA 5
#define WAIT 250
// Minimum wait 125 for 4sps with 2 delays
#define BAUD_RATE 9600
#define LED 13

// ### HexToVoltage ###
#define Gain_1_Res_12_                (int64_t)1000000000//pV
#define Gain_2_Res_12_                (int64_t) 500000000
#define Gain_4_Res_12_X_Gain_1_Res_14 (int64_t) 250000000
#define Gain_8_Res_12_X_Gain_2_Res_14 (int64_t) 125000000
#define Gain_4_Res_14_X_Gain_1_Res_16 (int64_t)  62500000
#define Gain_8_Res_14_X_Gain_2_Res_16 (int64_t)  31250000
#define Gain_4_Res_16_X_Gain_1_Res_18 (int64_t)  15625000
#define Gain_8_Res_16_X_Gain_2_Res_18 (int64_t)   7812500
#define Gain_4_Res_18_                (int64_t)   3906250
#define Gain_8_Res_18_                (int64_t)   1953125

// ### CaseSelectMultiply ###
// TODO Verstehen wieso theoretisch berechneter case nicht mit terminal stimmt (bitschieben nicht richtig verstanden deshalb auch problem bei 3 oder 2 datenbbyte von I2C)
#define Case_Gain_1_Res_12_ (byte)0xC1
#define Case_Gain_1_Res_14_ (byte)0xE1

#define Case_Gain_1_Res_16_ (byte)0x01
#define Case_Gain_1_Res_18_ (byte)33//0x81

#define Case_Gain_2_Res_12_ (byte)0xC2
#define Case_Gain_2_Res_14_ (byte)0xE2

#define Case_Gain_2_Res_16_ (byte)0x02
#define Case_Gain_2_Res_18_ (byte)34//0x82

#define Case_Gain_4_Res_12_ (byte)0xC4
#define Case_Gain_4_Res_14_ (byte)0xE4

#define Case_Gain_4_Res_16_ (byte)0x04
#define Case_Gain_4_Res_18_ (byte)36//0x84

#define Case_Gain_8_Res_12_ (byte)0xC8
#define Case_Gain_8_Res_14_ (byte)0xE8

#define Case_Gain_8_Res_16_ (byte)0x08
#define Case_Gain_8_Res_18_ (uint8_t)40//(byte)40//(uint8_t)0x88

// ### Variables -------------------------------------------------------------------------------------###
// ### ADC ###
byte adress = ADRESS;// Write 0x14, Read 0x15 (Includes R/W bit)
byte comand = COMAND;// Standard command
byte count = COUNT; // Data length read from sensor

byte i = 0; // counter
byte z = 0; // counter
byte in[COUNT];// Data Input

unsigned int ptat = 0; // reference temperature 10 times value of degC
unsigned int p[DATA];// Measure Value
byte pec = 0;//packet error check code based on the "sm bus" specification

// ### LCD ###
// Set the LCD address to 0x27=7bit Adress for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 20, 4);
double d = 0xbf800000;
signed long long32_1 = 0x00000000;
signed long long32_2 = 0x00000000;
int64_t longlong64_1 = 0x0000000000000000;
int64_t longlong64_2 = 0x0000000000000000;
String s;
char dispalayNumber12[] = "+2,048.000 V";
// ### Channel ###
// TODO Defines für ch1-4 damit 0te speicherstelle entgfält!!
#define CHANNEL1 0x90
#define CHANNEL2 0xB4
#define CHANNEL3 0xD8
#define CHANNEL4 0xFC
byte chComand[] = {0x00, CHANNEL1, CHANNEL2, CHANNEL3, CHANNEL4};//{0x00, 0x9C, 0xBD, 0xDE, 0xFF};<<<--------------------------------------KANAL EINSTELLUNG
//byte chComand[] = {0x00, 0x9C, 0xBD, 0xDE, 0xFF};
byte chCount[] = {0, 5, 5, 5, 5};// Von 5 für 18bit auf 4 für 12,14,16 bit gestellt
byte chData[] = {0, 5, 5, 5, 5};// Von 5 für 18bit auf 4 für 12,14,16 bit gestellt
///*
byte msb[] = {0, 0, 0, 0, 0};
byte b[] = {0, 0, 0, 0, 0};
byte lsb[] = {0, 0, 0, 0, 0};
byte redData[] = {0, 0, 0, 0, 0};
byte conData[] = {0, 0, 0, 0, 0};
//*/
/*
byte msb[] = {0x11, 0x12, 0x13, 0x14};
byte b[] = {0x21, 0x22, 0x23, 0x24};
byte lsb[] = {0x31, 0x32, 0x33, 0x34};
byte redData[] = {0, 0, 0, 0};
byte conData[] = {0, 0, 0, 0};
*/
// ### Setup -----------------------------------------------------------------------------------------###
void setup()
{
  Wire.begin(1);
  Serial.begin(BAUD_RATE);
  pinMode(LED, OUTPUT);
  setupLCD();
}
// ### Main ------------------------------------------------------------------------------------------###
void loop()
{
  //readChannel(adress,comand);
  for (int i = 1; i < 5; i++) {
    startADC(i);
    delay(500);
    readADC2(i);
    writeCh_20x4_LCD2();
  }
}
// ### Helper ----------------------------------------------------------------------------------------###
// ### Start Screen ###
void setupLCD(void)
{
  Serial.println("Start Screen");
  // initialize the LCD
  lcd.begin();

  // Turn on the blacklight and print a message.
  lcd.backlight();
  lcd.print("Voltage-Ranger-Electronics (Lord Kelvin)");
}

// ### Start ADC ###
void startADC(byte c)
{
  Serial.println("Start ADC");
  Wire.beginTransmission(byte(ADRESS));
  Wire.write(byte(chComand[c]));
  Wire.endTransmission();
  delay(1);
}
// ### Read ADC ###
// read that works with structure
// c=1,2,3,4;
void readADC (byte c)
{
  Serial.println("Read ADC");
  Wire.requestFrom(ADRESS, chCount[c]);
  msb[c] = Wire.read();
  b[c] = Wire.read();
  lsb[c] = Wire.read();
  redData[c] = Wire.read();
  conData[c] = Wire.read();
  Wire.endTransmission();
  // Data Ready?
}
// ### Read ADC2 ###
// read that works with structure
// c=1,2,3,4;
void readADC2 (byte c)
{
  Serial.println("Read ADC2");
  Wire.requestFrom(ADRESS, chCount[c]);
  // Msb byte only in 18bit mode
  //byte chComand[] = {0x00, 0x93, 0xB7, 0xDB, 0xFF};//{0x00, 0x9C, 0xBD, 0xDE, 0xFF};<<<--------------------------------------KANAL EINSTELLUNG    ### WEITER ###
  if ((byte(chComand[c]) & 0x0C ) == 0x0C) {    // Befehl lässt garnicht hinein !!
    msb[c] = Wire.read();

  }
  b[c] = Wire.read();
  lsb[c] = Wire.read();
  redData[c] = Wire.read();
  conData[c] = Wire.read();

  Wire.endTransmission();

  // Je nach sign und Resolution msb 00 oder ff setzen!!!
  //if ((byte(chComand[c]) & 0x0C ) == 0x0C) { // Befehl lässt garnicht hinein !!
  if (1) {
    // Positiv oder negativ an b ablesen?
    if ((byte(b[c] & 0x80)) == 0x80) {
      msb[c] = 0xff;
    }
    else {
      msb[c] = 0x00;
    }
  }
}
// ### Update LCD (All Channel) ###
void writeCh_20x4_LCD2()
{
  Serial.println("Update LCD");
  String s1 = channelToString3(1, 1);
  String s2 = channelToString3(2, 1);
  String s3 = channelToString3(3, 1);
  String s4 = channelToString3(4, 1);
  lcd.clear();
  lcd.print(s1);
  lcd.print(s3);//<------------------------------------------------------ VERTAUSCHT DA LIBARYPROBLEM
  lcd.print(s2);
  lcd.print(s4);
}
// ### Get Gain as char ###
char getGainChar(byte c)
{
  // Analyse Gain
  if ((redData[c] & 0x03) == 0x00)return'1';
  if ((redData[c] & 0x03) == 0x01)return'2';
  if ((redData[c] & 0x03) == 0x02)return'4';
  if ((redData[c] & 0x03) == 0x03)return'8';

}
// ### get Gain as byte ###
byte getGainByte(byte c)
{
  if ((redData[c] & 0x03) == 0x00)return 1;
  if ((redData[c] & 0x03) == 0x01)return 2;
  if ((redData[c] & 0x03) == 0x02)return 4;
  if ((redData[c] & 0x03) == 0x03)return 8;

}
// ### Get Resolution as char ###
// Return x as char to get correct Resolution as String '1',x
char getResolutionChar(byte c)
{
  // Analyse Gain
  if ((redData[c] & 0x0C) == 0x00)return'2';
  if ((redData[c] & 0x0C) == 0x04)return'4';
  if ((redData[c] & 0x0C) == 0x08)return'6';
  if ((redData[c] & 0x0C) == 0x0C)return'8';
}
// ### get Resolution as byte ###
byte getResolutionByte(byte c)
{
  if ((redData[c] & 0x0C) == 0x00)return 12;
  if ((redData[c] & 0x0C) == 0x04)return 14;
  if ((redData[c] & 0x0C) == 0x08)return 16;
  if ((redData[c] & 0x0C) == 0x0C)return 18;
}
// ### get Channel as byte ###
char getChannelChar(byte c)
{
  if ((redData[c] & 0x60) == 0x00)return '1';
  if ((redData[c] & 0x60) == 0x20)return '2';
  if ((redData[c] & 0x60) == 0x40)return '3';
  if ((redData[c] & 0x60) == 0x60)return '4';
}
// ### get Channel as byte ###
byte getChannelByte(byte c)
{
  if ((redData[c] & 0x60) == 0x00)return 1;
  if ((redData[c] & 0x60) == 0x20)return 2;
  if ((redData[c] & 0x60) == 0x40)return 3;
  if ((redData[c] & 0x60) == 0x60)return 4;
}
// ### ChannelToString2 ###
String channelToString2 (byte chNr)
{
  Serial.println("ChannelToString2");
  // TODO convert to decimal char array!!!
  char str[21] = "CH1: +    0.00 uV G1";
  str[2] = getChannelChar(chNr);
  str[19] = getGainChar(chNr);
  if ((msb[chNr] & 0xF0) == 0xF0 )
  {
    str[5] = '-';
    //d = 0xff << 12 | msb[chNr] << 8 |  b[chNr] << 4  | lsb[chNr] ;
    // TODO 2s complement
    d =  d;
    //convert to positive DEC
    str[8] = hexToChar (msb[chNr], 1);
    str[9] = hexToChar (msb[chNr], 2);
    str[10] = hexToChar (b[chNr], 1);
    str[11] = hexToChar (b[chNr], 2);
    str[12] = hexToChar (lsb[chNr], 1);
    str[13] = hexToChar (lsb[chNr], 2);

  }
  else
  {
    str[5] = '+';
    //d = 0x00 << 12 | msb[chNr] << 8 |  b[chNr] << 4  | lsb[chNr] ;
    //convert to positive DEC
    str[8] = hexToChar (msb[chNr], 1);
    str[9] = hexToChar (msb[chNr], 2);
    str[10] = hexToChar (b[chNr], 1);
    str[11] = hexToChar (b[chNr], 2);
    str[12] = hexToChar (lsb[chNr], 1);
    str[13] = hexToChar (lsb[chNr], 2);
  }
  return str;
}

// ### ChannelToString3 ###
// chNr 1,2,3,4
// mode 0=HEX,1=Voltage,default = error massage;
String channelToString3 (byte chNr, char mode)
{
  Serial.println("ChannelToString3");
  // TODO convert to decimal char array!!!
  char str[21] = "CHX: +    Y.YY uV GZ";
  str[2] = getChannelChar(chNr);

  switch (mode) {
    case 0:
      str[19] = getGainChar(chNr);
      if ((msb[chNr] & 0xF0) == 0xF0 )
      {
        str[5] = '-';
        str[8] = hexToChar (msb[chNr], 1);
        str[9] = hexToChar (msb[chNr], 2);
        str[10] = hexToChar (b[chNr], 1);
        str[11] = hexToChar (b[chNr], 2);
        str[12] = hexToChar (lsb[chNr], 1);
        str[13] = hexToChar (lsb[chNr], 2);

      }
      else
      {
        str[5] = '+';
        str[8] = hexToChar (msb[chNr], 1);
        str[9] = hexToChar (msb[chNr], 2);
        str[10] = hexToChar (b[chNr], 1);
        str[11] = hexToChar (b[chNr], 2);
        str[12] = hexToChar (lsb[chNr], 1);
        str[13] = hexToChar (lsb[chNr], 2);
      }
      break;

    // Case 1 Voltage output
    case 1:
      hexToVoltageCharArray(chNr);
      multiplyHexToVoltage(chNr);
      displayVoltageCharArray(1);
      str[5] = ' ';
      str[8] = dispalayNumber12[0];
      str[9] = dispalayNumber12[1];
      str[10] = dispalayNumber12[2];
      str[11] = dispalayNumber12[3];
      str[12] = dispalayNumber12[4];
      str[13] = dispalayNumber12[5];
      str[14] = dispalayNumber12[6];
      str[15] = dispalayNumber12[7];
      str[16] = dispalayNumber12[8];
      str[17] = dispalayNumber12[9];
      str[18] = dispalayNumber12[10];
      str[19] = dispalayNumber12[11];
      break;

    default:
      // Error 100: No equivalent channel mode selected
      str[5] = ' ';
      str[8] = 'E';
      str[9] = 'R';
      str[10] = ':';
      str[11] = '1';
      str[12] = '0';
      str[13] = '0';
      break;
  }// End Switch
  return str;
}//channelToString3

// ### hexToVoltageCharArray ###
void hexToVoltageCharArray(byte chNr)
{
  // #Vorzeichen ermitteln und eintragen in vorlage
  // *Umwandlung von msb b lsb auf longlong
  // *Einstellung Smplerate und Gain ermitteln
  // *Multiplikation mit lookuptable
  // *Teilen auf anfangswert / %
  //
  // *Chars nach lookuptable eintragen in vorlage
  // *Einheit und m u Eintragen in vorlage
  //msb[chNr] = 0x00;
  //b[chNr] = 0xF9;
  //lsb[chNr] = 0x92;

  if ((msb[chNr] & 0x80) == 0x80 )
  {
    dispalayNumber12[0] = '-';
    longlong64_1 = 0xFFFFFFFFFF000000 | ( (int64_t)msb[chNr] << 16 | (int64_t) b[chNr] << 8  | (int64_t)lsb[chNr]) ;
  }
  else {
    dispalayNumber12[0] = '+';
    longlong64_1 = (int64_t)0x00 | (int64_t)msb[chNr] << 16 | (int64_t)b[chNr] << 8 | (int64_t)lsb[chNr] ;
  }
  dispalayNumber12[10] = ' ';
}
// ### multiplyHexToVoltage ###
// Get Channel Gain and Res
void multiplyHexToVoltage(byte chNr)
{
  uint8_t multiGain;
  uint8_t multiRes;
  uint8_t selectCase;
  multiGain = getGainByte(chNr); // 1,2,4,8
  multiRes = getResolutionByte(chNr); //12,14,16,18
  selectCase = ((uint8_t)multiRes << 4) | (uint8_t)multiGain;

  // DEBUG WIEDER ENTFERNEN
  Serial.print("Channel:");
  Serial.println(chNr, DEC);

  Serial.print("MultiGain:");
  Serial.println(multiGain, DEC);

  Serial.print("MultiResolution:");
  Serial.println(multiRes, DEC);


  Serial.print("SelectCase:");
  Serial.println(selectCase, DEC);



  switch (selectCase) {
    case Case_Gain_1_Res_12_ :
    Serial.println("#G1 Res12");
      longlong64_1 = longlong64_1 * Gain_1_Res_12_;
      break;
    case Case_Gain_1_Res_14_ :
    Serial.println("#G1 Res14");
      longlong64_1 = longlong64_1 * Gain_4_Res_12_X_Gain_1_Res_14;
      break;
    case Case_Gain_1_Res_16_ :
    Serial.println("#G1 Res16");
      longlong64_1 = longlong64_1 * Gain_4_Res_14_X_Gain_1_Res_16;
      break;
    case Case_Gain_1_Res_18_ :
    Serial.println("#G1 Res18");
      longlong64_1 = longlong64_1 * Gain_4_Res_16_X_Gain_1_Res_18;
      break;

    case Case_Gain_2_Res_12_ :
    Serial.println("#G2 Res12");
      longlong64_1 = longlong64_1 * Gain_2_Res_12_;
      break;
    case Case_Gain_2_Res_14_ :
    Serial.println("#G2 Res14");
      longlong64_1 = longlong64_1 * Gain_8_Res_12_X_Gain_2_Res_14;
      break;
    case Case_Gain_2_Res_16_ :
    Serial.println("#G2 Res16");
      longlong64_1 = longlong64_1 * Gain_8_Res_14_X_Gain_2_Res_16;
      break;
    case Case_Gain_2_Res_18_ :
    Serial.println("#G2 Res18");
      longlong64_1 = longlong64_1 * Gain_8_Res_16_X_Gain_2_Res_18;
      break;

    case Case_Gain_4_Res_12_ :
    Serial.println("#G4 Res12");
      longlong64_1 = longlong64_1 * Gain_4_Res_12_X_Gain_1_Res_14;
      break;
    case Case_Gain_4_Res_14_ :
    Serial.println("#G4 Res14");
      longlong64_1 = longlong64_1 * Gain_4_Res_14_X_Gain_1_Res_16;
      break;
    case Case_Gain_4_Res_16_ :
    Serial.println("#G4 Res16");
      longlong64_1 = longlong64_1 * Gain_4_Res_16_X_Gain_1_Res_18;
      break;
    case Case_Gain_4_Res_18_ :
    Serial.println("#G4 Res18");
      longlong64_1 = longlong64_1 * Gain_4_Res_18_;
      break;

    case Case_Gain_8_Res_12_ :
      Serial.println("#G8 Res12");
      longlong64_1 = longlong64_1 * Gain_8_Res_12_X_Gain_2_Res_14;//Multiplayer kontrullieren
      break;
    case Case_Gain_8_Res_14_ :
      Serial.println("#G8 Res14");
      longlong64_1 = longlong64_1 * Gain_8_Res_14_X_Gain_2_Res_16;
      break;
    case Case_Gain_8_Res_16_ :
      Serial.println("#G8 Res16");
      longlong64_1 = longlong64_1 * Gain_8_Res_16_X_Gain_2_Res_18;
      break;
    case Case_Gain_8_Res_18_ :
      Serial.println("#G8 Res18");
      longlong64_1 = longlong64_1 * Gain_8_Res_18_;// Scheint ok
      break;

    default:
    Serial.println("#GX ResYY DEFAULT! ERROR:101");
      break;
  }
  //*/
}
// ### displayVoltageCharArray ###
// Option 1 = Decimal separator Komma
void displayVoltageCharArray(byte option)
{
  // DEBUG WIEDER ENTFERNEN
  //longlong64_1 = -2345678000000;
  //longlong64_1 = 2345678000000;
  //longlong64_1 = (int64_t)131071*(int64_t)15625000;
  //longlong64_1 = (int64_t)131071 * Gain_4_Res_16_X_Gain_1_Res_18;

  // Subtract (pV basis to uV basis)
  longlong64_1 = longlong64_1 / 1000000;
  dispalayNumber12[9] = lookUpTableRestToChar(longlong64_1 % 10);
  longlong64_1 = longlong64_1 / 10;
  dispalayNumber12[8] = lookUpTableRestToChar(longlong64_1 % 10);
  longlong64_1 = longlong64_1 / 10;
  dispalayNumber12[7] = lookUpTableRestToChar(longlong64_1 % 10);
  longlong64_1 = longlong64_1 / 10;

  dispalayNumber12[5] = lookUpTableRestToChar(longlong64_1 % 10);
  longlong64_1 = longlong64_1 / 10;
  dispalayNumber12[4] = lookUpTableRestToChar(longlong64_1 % 10);
  longlong64_1 = longlong64_1 / 10;
  dispalayNumber12[3] = lookUpTableRestToChar(longlong64_1 % 10);
  longlong64_1 = longlong64_1 / 10;

  dispalayNumber12[1] = lookUpTableRestToChar(longlong64_1 % 10);

}
char lookUpTableRestToChar( signed int rest)
{
  switch (rest) {
    case 0:
      return '0';
      break;
    case 1:
    case -1:
      return '1';
      break;
    case 2:
    case -2:
      return '2';
      break;
    case 3:
    case -3:
      return '3';
      break;
    case 4:
    case -4:
      return '4';
      break;
    case 5:
    case -5:
      return '5';
      break;
    case 6:
    case -6:
      return '6';
      break;
    case 7:
    case -7:
      return '7';
      break;
    case 8:
    case -8:
      return '8';
      break;
    case 9:
    case -9:
      return '9';
      break;

    default:
      return 'E';
      break;
  }

}
// ### hexToChar ###
// pos = return 1st or 2nd nibbel
char hexToChar (byte b, byte pos)
{
  if (pos == 1) b = b >> 4 ;
  else b = b & 0x0F;
  switch (b) {
    case 0x00: return '0';
    case 0x01: return '1';
    case 0x02: return '2';
    case 0x03: return '3';
    case 0x04: return '4';
    case 0x05: return '5';
    case 0x06: return '6';
    case 0x07: return '7';
    case 0x08: return '8';
    case 0x09: return '9';
    case 0x0A: return 'A';
    case 0x0B: return 'B';
    case 0x0C: return 'C';
    case 0x0D: return 'D';
    case 0x0E: return 'E';
    case 0x0F: return 'F';
    default: return 'X';
  }
}

// ### hexToTempToString ###
// needs uv in mikro Volt
void hexToTempToString(int64_t uv) {
  // ### Float ###
  // Konstanten
  float c0 =  0;
  float c1 =  2.508355E-2;
  float c2 =  7.860106E-8;
  float c3 = -2.503131E-10;
  float c4 =  8.315270E-14;
  float c5 = -1.228034E-17;
  float c6 =  9.804036E-22;
  float c7 = -4.413030E-26;
  float c8 =  1.057734E-30;
  float c9 = -1.052755E-35;
  float c10 = 0;

  long l = 1;
  float e = 1;
  float t = 1;
  
  l = long(uv);
  e = (float)l;
  t=0;
  t = c0 + c1 * e + c2 * e * e + c3 * e * e * e + c4 * e * e * e * e + c5 * e * e * e * e * e + c6 * e * e * e * e * e * e + c7 * e * e * e * e * e * e * e + c8 * e * e * e * e * e * e * e * e + c9 * e * e * e * e * e * e * e * e * e + c10 * e * e * e * e * e * e * e * e * e * e;
  l = (long)t;
  l = l * 10;
  Serial.println(l);
  // lookUpTableRestToChar
  Serial.println(int(l % 10));
  l = l / 10;
  Serial.println(",");
  Serial.println(int(l % 10));
  l = l / 10;
  Serial.println(int(l % 10));
  l = l / 10;
  Serial.println(int(l % 10));
  l = l / 10;
  Serial.println(int(l % 10));
  l = l / 10;
}

// ### ChannelToString4 ###
// chNr 1,2,3,4
// mode 0=HEX,1=Voltage,2=K-TypeThermocouple;default = error massage;
String channelToString4 (byte chNr, char mode)
{
  Serial.println("ChannelToString4");
  // TODO convert to decimal char array!!!
  char str[21] = "CHX: +    Y.YY uV GZ";
  str[2] = getChannelChar(chNr);

  switch (mode) {
    case 0:
      str[19] = getGainChar(chNr);
      if ((msb[chNr] & 0xF0) == 0xF0 )
      {
        str[5] = '-';
        str[8] = hexToChar (msb[chNr], 1);
        str[9] = hexToChar (msb[chNr], 2);
        str[10] = hexToChar (b[chNr], 1);
        str[11] = hexToChar (b[chNr], 2);
        str[12] = hexToChar (lsb[chNr], 1);
        str[13] = hexToChar (lsb[chNr], 2);

      }
      else
      {
        str[5] = '+';
        str[8] = hexToChar (msb[chNr], 1);
        str[9] = hexToChar (msb[chNr], 2);
        str[10] = hexToChar (b[chNr], 1);
        str[11] = hexToChar (b[chNr], 2);
        str[12] = hexToChar (lsb[chNr], 1);
        str[13] = hexToChar (lsb[chNr], 2);
      }
      break;

    // Case 1 Voltage output
    case 1:
      hexToVoltageCharArray(chNr);
      multiplyHexToVoltage(chNr);
      displayVoltageCharArray(1);
      str[5] = ' ';
      str[8] = dispalayNumber12[0];
      str[9] = dispalayNumber12[1];
      str[10] = dispalayNumber12[2];
      str[11] = dispalayNumber12[3];
      str[12] = dispalayNumber12[4];
      str[13] = dispalayNumber12[5];
      str[14] = dispalayNumber12[6];
      str[15] = dispalayNumber12[7];
      str[16] = dispalayNumber12[8];
      str[17] = dispalayNumber12[9];
      str[18] = dispalayNumber12[10];
      str[19] = dispalayNumber12[11];
      break;

    default:
      // Error 100: No equivalent channel mode selected
      str[5] = ' ';
      str[8] = 'E';
      str[9] = 'R';
      str[10] = ':';
      str[11] = '1';
      str[12] = '0';
      str[13] = '0';
      break;
  }// End Switch
  return str;
}//channelToString4

