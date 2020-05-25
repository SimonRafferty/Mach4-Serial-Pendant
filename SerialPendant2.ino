// This is a Pendant for Mach4 which runs over Serial / USB
// Attach an MPG Wheel to your Arduino Nano.  I'm using A to D3
// & B to D4.  D3 is important as it maps to Interrupt 1
// There's an I2C Touch display for the Pendant buttons.
// When a button is pressed or the MPG moved, a single character 
// command is sent to Mach4 over Serial.  When it's acted upon, 
// Mach4 sends the button state or DRO value back which is updated
// on the display.  This gives a remote confirmation of the command.

// Sent Commands:
// I : MPG Increment    D : MPG Decrement
// X : Jog X Axis      Y : Jog Y Axis      Z : Jog Z Axis      F : Feed Rate Override   Q : Spindle Override  N : Move No Axis (disable MPG)         
// 1 : Jog Inc 0.001    2 : Jog Inc 0.010    3 : Jog Inc 0.100    4 : Jog Inc 1.000
// E : E-Stop
// C : Coolant
// H : Feed Hold
// S : Start Cycle
// T : Stop Cycle
// K : In resopnse to a sent 'O' - used to check comms

// Received Commands.  The Parser (https://github.com/pvizeli/CmdParser)
// Expects "Command<space>Param1<space>Param2....etc..\n"
// D ? #.####   : DRO Count [X,Y,Z,S,F]
// J ? : [X,Y,Z,S,F,N] Which Axis Mach4 thinks is Active for Jogging 
// I # : [1,2,3,4] Active Jog Increment
// B E # : 1=E-Stop 0=Not E-Stop
// B C # : 1=Coolant Active 0=Coolant Inactive
// B H # : 1=Feed Hold Active 0=Feed Hold Inactive
// B S # : 1=Start Cycle Active   0=Stopped
// O : Replies with K - used to check comms


#include <CmdBuffer.hpp>
#include <CmdCallback.hpp>
#include <CmdParser.hpp>

CmdCallback<5> cmdCallback;

CmdBuffer<32> myBuffer;
CmdParser     myParser;


float XDRO = 0;      //Store for current X Position
float YDRO = 0;      //Store for current X Position
float ZDRO = 0;      //Store for current X Position
float LastXDRO = 0;      //Store for current X Position
float LastYDRO = 0;      //Store for current X Position
float LastZDRO = 0;      //Store for current X Position
int SDRO = 0;        //Store for Spindle Speed
int FDRO = 0;        //Store for Feed Rate Override
char DROVal[9]; // Buffer big enough for 8-character float

int32_t MPGCount = 0;   //Last MPG Count Value
int step_state = 1;

#include "LiquidCrystalDisplay.h"

const uint8_t User01[]="USER";  //Text for user defined button
const uint8_t User02[]="USER";  //Text for user defined button
const uint8_t InitStep[]="0.0000";  //Initialisation of text labels 
const uint8_t InitDisp[]="0000.0000";  //Initialisation of text labels 

const uint8_t String01[]="Password ok!";
const uint8_t String02[]="Password error!";
const uint8_t String03[]="Error";

const uint8_t String04[]="Input number ok!";
const uint8_t String05[]="Input number error!";

uint16_t HighlightColour = 0xFEA0; //Yellow
uint16_t DROTextColour = 0xFFFF; //Light Yellow
uint16_t TransparentColour = 0xC618;

int EnabledState = 0;

uint8_t   identifier,cnt;
uint8_t   cmd_buffer[CMD_MAX_SIZE];
uint8_t   data_size;
uint8_t   update_en;
uint8_t   command_cmd;
uint8_t   command_statu;
uint8_t   command_length;
uint8_t   page_Id_bk = 0;
uint8_t   page_Id = 1;
uint8_t   targe_Id = 0;

long DROUpdate = 0;

LiquidCrystal TFTlcd(13);//RST pin13

void setup()
{
  update_en =0;
  data_size =0;
  TFTlcd.queue_reset();
  pinMode(3,INPUT_PULLUP); //MPG A Signal
  pinMode(4,INPUT_PULLUP); //MPG B Signal
  
  Serial.begin(115200);
  Serial.println(F("Pendant Started"));
  TFTlcd.begin();
  TFTlcd.SetBackLight(100); //Set the backlight
  delay(100);

  /*
  TFTlcd.SetBackLight(30); //Set the backlight
  delay(500);
  TFTlcd.SetBackLight(100); //Set the backlight
  delay(500);
  TFTlcd.SetBackLight(30); //Set the backlight
  delay(500);
  TFTlcd.SetBackLight(100); //Set the backlight
  delay(500);
  */
  attachInterrupt(0,LcdIICInterrupt,FALLING);//Interrupt 0 is D2 PIN
  attachInterrupt(1,UpdateMPG,RISING);//Interrupt 1 is D3 PIN
  TFTlcd.SetPage(1);//main page id is 4
  
  
  //Setup command parser callback functions
  cmdCallback.addCmd("B", &fUpdateButton);
  cmdCallback.addCmd("D", &fUpdateDRO);  
  cmdCallback.addCmd("J", &fUpdateMove);
  cmdCallback.addCmd("I", &fUpdateJogInc);
  cmdCallback.addCmd("O", &fEcho);
  
  myBuffer.setEcho(false);
  
  //Clear Jog Indicators
  
  TFTlcd.RectangleFill(80,15, 20,35, TransparentColour);
  TFTlcd.RectangleFill(80,75, 20,35, TransparentColour);
  TFTlcd.RectangleFill(80,135, 20,35, TransparentColour);


  TFTlcd.PutString(110,10,0x02,0x30,0x01,0x00,DROTextColour, (unsigned char *)InitDisp);  //X DRO
  TFTlcd.PutString(110,70,0x02,0x30,0x01,0x00,DROTextColour, (unsigned char *)InitDisp);  //Y DRO
  TFTlcd.PutString(110,130,0x02,0x30,0x01,0x00,DROTextColour, (unsigned char *)InitDisp); //Z DRO
  TFTlcd.PutString(120,330,0x02,0x30,0x01,0x00,DROTextColour, (unsigned char *)InitStep); //Step DRO

  drawEnableButton(0);
  TFTlcd.PutString(125,270,0x02,0x20,0x00,0x00,0x00, (unsigned char *)User01);
  TFTlcd.PutString(235,270,0x02,0x20,0x00,0x00,0x00, (unsigned char *)User02);

}

void loop()
{
  data_size = TFTlcd.queue_find_cmd(cmd_buffer,CMD_MAX_SIZE);
  if(data_size>0)//receive command
  {
    //Serial.println(data_size, HEX);
    //Serial.println(F("ProcessMessage"));
    ProcessMessage((PCTRL_MSG)cmd_buffer, data_size);//command process
  } 
  
  // Check for new char on serial and call function if command was entered
  cmdCallback.updateCmdProcessing(&myParser, &myBuffer, &Serial);

  
  //Update DROs every 100ms
  if(millis()-DROUpdate>100) {
    if(LastXDRO!=XDRO) {
      dtostrf(XDRO, 8, 4, DROVal);        
      TFTlcd.DisplayCut_Image(110,10,200,40,0x01);  //Clear box
      TFTlcd.PutString(110,10,0x02,0x30,0x01,0x00,DROTextColour, (unsigned char *)DROVal);  //X DRO
      LastXDRO=XDRO;
    }
    if(LastYDRO!=YDRO) {
      dtostrf(YDRO, 8, 4, DROVal);        
      TFTlcd.DisplayCut_Image(110,70,200,40,0x01);  //Clear box
      TFTlcd.PutString(110,70,0x02,0x30,0x01,0x00,DROTextColour, (unsigned char *)DROVal);  //Y DRO
      LastYDRO=YDRO;
    }

    
    if(LastZDRO!=ZDRO) {
      dtostrf(ZDRO, 8, 4, DROVal);        
      TFTlcd.DisplayCut_Image(110,130,200,40,0x01);  //Clear box
      TFTlcd.PutString(110,130,0x02,0x30,0x01,0x00,DROTextColour, (unsigned char *)DROVal); //Z DRO
      LastZDRO=ZDRO;
    }
    DROUpdate = millis();
  }
  
}


void UpdateMPG() {
//Send Serial command to Mach4 to Jog  Triggered on rising edge of MPG A line
//This is a very simple, single interrupt, quadrature decoder
  if(digitalRead(4)) {
    Serial.println("D");
  } else {
    Serial.println("I");
  }
}



void LcdIICInterrupt()
{
    //Serial.println("IRQ 0");
    command_cmd = TFTlcd.I2C_Read();
    TFTlcd.queue_push(command_cmd);
    for(cnt =0;cnt <2;cnt++)
    {
     identifier = TFTlcd.I2C_Read();
     TFTlcd.queue_push(identifier);
     //Serial.println(identifier, HEX);
    }
    command_statu = TFTlcd.I2C_Read();
    TFTlcd.queue_push(command_statu);
    identifier = TFTlcd.I2C_Read();
    TFTlcd.queue_push(identifier);
    command_length = TFTlcd.I2C_Read();
    TFTlcd.queue_push(command_length);
    if((command_cmd == GET_EDIT_VALUE && command_statu == 0x6f)||(command_cmd == GET_TOUCH_EDIT_VALUE && command_statu == 0x6f))
    {
       for(cnt =0;cnt <command_length;cnt++)
      {
       identifier = TFTlcd.I2C_Read();
       TFTlcd.queue_push(identifier);
       //Serial.println(identifier, HEX);
      }
    }
}

  
void fEcho(CmdParser *myParser) {
//Comms check.  An 'O' has been received, send 'K' 
  Serial.println("K");
}
  
  

void fUpdateDRO(CmdParser *myParser) {
//Command will have two parameters - The Axis Name [X,Y,Z,S,F] and a Value

if (myParser->equalCmdParam(1, "X")) {
        XDRO = atof(myParser->getCmdParam(2));
        //Set X DRO value on screen
        //Serial.print("Set X DRO To: "); Serial.println(XDRO);
    }
    if (myParser->equalCmdParam(1, "Y")) {
        YDRO = atof(myParser->getCmdParam(2));
    }
    if (myParser->equalCmdParam(1, "Z")) {
        ZDRO = atof(myParser->getCmdParam(2));       
    }
    if (myParser->equalCmdParam(1, "S")) {
        SDRO = atoi(myParser->getCmdParam(2));
        dtostrf(SDRO, 8, 4, DROVal);
        //Not implemented
    }
    if (myParser->equalCmdParam(1, "F")) {
        FDRO = atoi(myParser->getCmdParam(2));
        dtostrf(FDRO, 8, 4, DROVal);
        //Not implemented
    }
  
}
void fUpdateMove(CmdParser *myParser) 
{
//Command will have one parameter - The Axis Name [X,Y,Z,S,F,N] for which Jog is active  N=None
    //Clear 
    TFTlcd.RectangleFill(80,15, 20,35, TransparentColour);
    TFTlcd.RectangleFill(80,75, 20,35, TransparentColour);
    TFTlcd.RectangleFill(80,135, 20,35, TransparentColour);

    if (myParser->equalCmdParam(1, "X")) {
        //Set indicator active (and others inactive)
        //Draw filled rectangle next to X
        TFTlcd.RectangleFill(80,15, 20,35, HighlightColour);
        
        //Serial.println("Jog X");
    }
    if (myParser->equalCmdParam(1, "Y")) {
        //Set indicator active (and others inactive)
        //Draw filled rectangle next to Y
        TFTlcd.RectangleFill(80,75, 20,35, HighlightColour);
        //Serial.println("Jog Y");
    }
    if (myParser->equalCmdParam(1, "Z")) {
        //Set indicator active (and others inactive)
        //Draw filled rectangle next to Z
        TFTlcd.RectangleFill(80,135, 20,35, HighlightColour);
        //Serial.println("Jog Z");
    }
    if (myParser->equalCmdParam(1, "S")) {
        //Set indicator active (and others inactive)
        //Serial.println("Jog S");
    }
    if (myParser->equalCmdParam(1, "F")) {
        //Set indicator active (and others inactive)
        Serial.println("Jog FRO");
    }
    if (myParser->equalCmdParam(1, "N")) {
        //Set all indicators Inactive
        //Clear 
        TFTlcd.RectangleFill(80,15, 20,35, TransparentColour);
        TFTlcd.RectangleFill(80,75, 20,35, TransparentColour);
        TFTlcd.RectangleFill(80,135, 20,35, TransparentColour);
        //Serial.println("Jog NONE");
    }
  
}
void fUpdateJogInc(CmdParser *myParser) 
{
//Command will have one parameter - 1,2,3,4 indicating the joc increment
  const uint8_t sInc1[]="0.0010";    
  const uint8_t sInc2[]="0.0100";    
  const uint8_t sInc3[]="0.1000";    
  const uint8_t sInc4[]="1.0000";    
  
    TFTlcd.DisplayCut_Image(120,330,200,40,0x01);  //Clear box
    if (myParser->equalCmdParam(1, "1")) {
        //Set Jog Increment to 0.001
        //Serial.println("Update Jog DRO 0.0010");
        TFTlcd.PutString(120,330,0x02,0x30,0x01,0x00,DROTextColour, (unsigned char *)sInc1); //Step DRO
    }
    if (myParser->equalCmdParam(1, "2")) {
        //Set Jog Increment to 0.010
        TFTlcd.PutString(120,330,0x02,0x30,0x01,0x00,DROTextColour, (unsigned char *)sInc2); //Step DRO
    }
    if (myParser->equalCmdParam(1, "3")) {
        //Set Jog Increment to 0.100
        TFTlcd.PutString(120,330,0x02,0x30,0x01,0x00,DROTextColour, (unsigned char *)sInc3); //Step DRO
    }
    if (myParser->equalCmdParam(1, "4")) {
        //Set Jog Increment to 1.000
        TFTlcd.PutString(120,330,0x02,0x30,0x01,0x00,DROTextColour, (unsigned char *)sInc4); //Step DRO
    }
  
}
void drawEnableButton(int Active)
{
  if(Active==1) {
    //Draw rectangle on ENABLE Button
    TFTlcd.RectangleFill(5,405,310,70, 0xF800); //Colour button RED
    TFTlcd.PutString(90,415,0x01,0x30,0x01,0xF800,DROTextColour, (unsigned char *)"E-STOP"); //E-Stop
  } else {
    //Clear rectangle
    //TFTlcd.DisplayCut_Image(5,405,310,70,0x01);  //Clear box
    
    TFTlcd.RectangleFill(5,405,310,70, 0xC618); //Colour button GREY
    TFTlcd.PutString(80,415,0x01,0x30,0x01,0xC618,0x00, (unsigned char *)"MPG OFF"); //E-Stop
  }

}

void fUpdateButton(CmdParser *myParser) 
{
int Active = 0;
//Serial.println("Button Pressed");
//Command will have two parameters - A character indicating the Button then 1 to indicate Active, 0 for Inactive
    if (myParser->equalCmdParam(1, "E")) {
        Active = atoi(myParser->getCmdParam(2));
        drawEnableButton(Active);
        EnabledState = Active;
    }
    if (myParser->equalCmdParam(1, "C")) {
        Active = atoi(myParser->getCmdParam(2));
        //Serial.print("Coolant "); Serial.println(Active);
        if(Active==1) {
          //Draw rectangle on Coolant Button
          TFTlcd.RectangleFill(15,260,75,10, HighlightColour);
        } else {
          //Clear rectangle
          TFTlcd.DisplayCut_Image(15,260,75,10,0x00);  //Clear box
        }
    }
    if (myParser->equalCmdParam(1, "H")) {
        Active = atoi(myParser->getCmdParam(2));
        if(Active==1) {
          //Draw rectangle on Hold Button
          TFTlcd.RectangleFill(120,200,75,10, HighlightColour);
        } else {
          //Clear rectangle
          TFTlcd.DisplayCut_Image(120,200,75,10,0x00);  //Clear box
        }
    }
    if (myParser->equalCmdParam(1, "S")) {
        Active = atoi(myParser->getCmdParam(2));
        if(Active==1) {
          //Draw rectangle on START Button
          TFTlcd.RectangleFill(15,200,75,10, HighlightColour);
          // Clear Rectangle on STOP Button
          TFTlcd.DisplayCut_Image(230,200,75,10,0x00);  //Clear box
          
        } else {
          //Draw rectangle on STOP Button
          TFTlcd.RectangleFill(230,200,75,10, HighlightColour);
          //Clear rectangle on START
          TFTlcd.DisplayCut_Image(15,200,75,10,0x00);  //Clear box
        }
    }
 
}
void ProcessMessage( PCTRL_MSG msg, uint16_t dataSize )
{
    uint8_t cmd_type    = msg->cmd_type;
    uint8_t control_id  = msg->control_id;
    uint8_t page_id     = msg->page_id;
    uint8_t _status     = msg->status;
    uint8_t key_type    = msg->key_type;
    uint8_t key_value   = msg->key_value;
    /*
    Serial.println("Process Message");
    Serial.print("cmd_type"); Serial.println(cmd_type);
    Serial.print("control_id"); Serial.println(control_id);
    Serial.print("page_id"); Serial.println(page_id);
    Serial.print("_status"); Serial.println(_status);
    Serial.print("key_type"); Serial.println(key_type);
    Serial.print("key_value"); Serial.println(key_value);
    */
    if(cmd_type==NOTIFY_TOUCH_BUTTON){
      NotifyTouchButton(page_id,control_id,_status,key_type,key_value);
    }
  
}


void NotifyTouchButton(uint8_t page_id, uint8_t control_id, uint8_t  state,uint8_t type,uint8_t value)
{
  //Serial.println("Touch Button:");
  if(state== KEY_DOWN && control_id==100 )
  { //X Button pressed.
    Serial.println("X"); //Send command to Mach4
  }
  if(state== KEY_DOWN && control_id==101 )
  { //Y Button pressed.
    Serial.println("Y"); //Send command to Mach4
  }
  if(state== KEY_DOWN && control_id==102 )
  { //Z Button pressed.
    Serial.println("Z"); //Send command to Mach4
  }
  if(state== KEY_DOWN && control_id==103 )
  { //START Button pressed.
    Serial.println("S"); //Send command to Mach4
  }
  if(state== KEY_DOWN && control_id==104 )
  { //HOLD Button pressed.
    update_en=true;
    Serial.println("H"); //Send command to Mach4
  }
  if(state== KEY_DOWN && control_id==105 )
  { //STOP Button pressed.
    Serial.println("T"); //Send command to Mach4
  }

  if(state== KEY_DOWN && control_id==106 )
  { //COOLANT Button pressed.
    Serial.println("C"); //Send command to Mach4
  }
  if(state== KEY_DOWN && control_id==107 )
  { //User 1 Button pressed.
    Serial.println("U"); //Send command to Mach4
    TFTlcd.PutString(125,270,0x02,0x20,0x00,0x00,0x00, (unsigned char *)User01);
    TFTlcd.PutString(235,270,0x02,0x20,0x00,0x00,0x00, (unsigned char *)User02);

  }
  if(state== KEY_DOWN && control_id==108 )
  { //User 2 Button pressed.
    Serial.println("V"); //Send command to Mach4
    TFTlcd.PutString(125,270,0x02,0x20,0x00,0x00,0x00, (unsigned char *)User01);
    TFTlcd.PutString(235,270,0x02,0x20,0x00,0x00,0x00, (unsigned char *)User02);
  }
  if(state== KEY_RELEASE && (control_id==108 || control_id==107))
  { //User Button released.
    TFTlcd.PutString(125,270,0x02,0x20,0x00,0x00,0x00, (unsigned char *)User01);
    TFTlcd.PutString(235,270,0x02,0x20,0x00,0x00,0x00, (unsigned char *)User02);
  }
  
  if(state== KEY_DOWN && control_id==109 )
  { //STEP Increment Button pressed.
    update_en=true;
    step_state++;
    if(step_state>4) step_state=1;
    
    //Send serial command to Mach4
    Serial.println(step_state); //Send command to Mach4
  }
  
  if(state== KEY_DOWN && control_id==110 )
  { //ENABLE Increment Button pressed.
    Serial.println("E"); //Send command to Mach4
    drawEnableButton(EnabledState);
  }
  if(state== KEY_RELEASE && control_id==110 )
  { //ENABLE Increment Button pressed.
    //Serial.println("E"); //Send command to Mach4
    drawEnableButton(EnabledState);
  }
}
