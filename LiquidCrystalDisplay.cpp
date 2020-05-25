#include "LiquidCrystalDisplay.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"

// When the display powers up, it is configured as follows:

LiquidCrystal::LiquidCrystal(uint8_t rst)
{
	I2c.begin();
  TFTInit(rst);
}


void LiquidCrystal::TFTInit(uint8_t rst)
{
	_rst_pin = rst;  //Reset LCDs PIN
 
  pinMode(_rst_pin, OUTPUT);
 
  digitalWrite(_rst_pin,LO);//Reset LCDs
  delayMicroseconds(1000);
  digitalWrite(_rst_pin,HI);
  
  
}


void LiquidCrystal::begin()
{
	
}


void LiquidCrystal::I2C_Send(uint8_t *buf,uint8_t len)
{
  I2c.write(LCD_I2C_ADDRESS,buf,len);
  
}

uint8_t LiquidCrystal::I2C_Read(void)
{
	uint8_t _data;
   _data = I2c.read(LCD_I2C_ADDRESS,1);
   return _data;
}



void LiquidCrystal::SetReset(void)
{
	sendbuf[0] =CMD_RESET;
	sendbuf[1] =0x52;
	sendbuf[2] =0x65;
	sendbuf[3] =0x73;
	sendbuf[4] =0x65;
	sendbuf[5] =0x74;
	I2C_Send(sendbuf,6);
}

void LiquidCrystal::ClrScreen(uint16_t Color)
{
	sendbuf[0] = CMD_CLEAR_SCREEN;
	sendbuf[1] = (Color>>8);
	sendbuf[2] =  Color;
	I2C_Send(sendbuf,3);
}

void LiquidCrystal::Display_Image(uint16_t x,uint16_t y,uint8_t image_id)
{
	sendbuf[0] = CMD_ICON_DISPLAY;
	sendbuf[1] = (x>>8);
	sendbuf[2] =  x;
	sendbuf[3] = (y>>8);
	sendbuf[4] = y;
	sendbuf[5] = image_id;

	I2C_Send(sendbuf,6);
}

void LiquidCrystal::DisplayCut_Image(uint16_t image_x,uint16_t image_y,uint16_t image_w, uint16_t image_h,uint8_t  image_id)
{
	sendbuf[0] = CMD_CUT_ICON;
	sendbuf[1] = (image_x>>8);
	sendbuf[2] =  image_x;
	sendbuf[3] = (image_y>>8);
	sendbuf[4] = image_y;
	sendbuf[5] = (image_w>>8);
	sendbuf[6] =  image_w;
	sendbuf[7] = (image_h>>8);
	sendbuf[8] = image_h;
	sendbuf[9] = image_id;

	I2C_Send(sendbuf,10);
}


void LiquidCrystal::PutString(uint16_t x,uint16_t y,uint8_t DisplayType,uint8_t FontSize,uint8_t ImageNo,uint16_t BackColor,uint16_t ForeColor, uint8_t *strings)
{
	uint8_t i=0;
	
	sendbuf[0]= CMD_STR_DISPLAY;
	sendbuf[1]= (x>>8);
	sendbuf[2]= x;
	sendbuf[3]= (y>>8);
	sendbuf[4]= y;
	sendbuf[5]= DisplayType;
	sendbuf[6]= FontSize;
	sendbuf[7]= ImageNo;
	sendbuf[8]= (BackColor>>8);
	sendbuf[9]= BackColor;
	sendbuf[10]= (ForeColor>>8);
	sendbuf[11]= ForeColor;

	while(*strings !=0)
	{									
		sendbuf[i+12] = *(strings++);
		i++;
	}
	I2C_Send(sendbuf,12+i);
}

void LiquidCrystal::Display_Message(uint8_t FontSize,uint8_t time,uint8_t *strings )
{
	uint8_t i=0;
	sendbuf[0]= CMD_MESSAGE_DISPLAY;
	sendbuf[1]= FontSize;
	sendbuf[2]= time;
	while(*strings !=0)
	{									
		sendbuf[i+3] = *(strings++);
		i++;
	}
	I2C_Send(sendbuf,3+i);
	
}

void LiquidCrystal::Line(uint16_t s_x,uint16_t s_y,uint16_t e_x,uint16_t e_y,uint16_t ForeColor)
{
	sendbuf[0] = CMD_DRAW_LINE;
	sendbuf[1] = (s_x>>8);
	sendbuf[2] = s_x;
	sendbuf[3] = (s_y>>8);
	sendbuf[4] = s_y;
	sendbuf[5] = (e_x>>8);
	sendbuf[6] = e_x;
	sendbuf[7] = (e_y>>8);
	sendbuf[8] = e_y;
	sendbuf[9] = (ForeColor>>8);
	sendbuf[10] = ForeColor;
	I2C_Send(sendbuf,11);
}
 

void LiquidCrystal::Rectangle(uint16_t x0,uint16_t y0, uint16_t x1,uint16_t y1, uint16_t ForeColor)
{
	sendbuf[0] = CMD_DRAW_RECT;
	sendbuf[1] = (x0>>8);
	sendbuf[2] = x0;
	sendbuf[3] = (y0>>8);									
	sendbuf[4] = y0;
	sendbuf[5] = (x1>>8);									
	sendbuf[6] = x1;
	sendbuf[7] = (y1>>8);
	sendbuf[8] = y1;
	sendbuf[9] = (ForeColor>>8);									
	sendbuf[10] = ForeColor;
	I2C_Send(sendbuf,11);
}

void LiquidCrystal::RectangleFill(uint16_t x0,uint16_t y0, uint16_t x1,uint16_t y1, uint16_t ForeColor)
{
	sendbuf[0] = CMD_DRAW_FILL_RECT;
	sendbuf[1] = (x0>>8);
	sendbuf[2] = x0;
	sendbuf[3] = (y0>>8);									
	sendbuf[4] = y0;
	sendbuf[5] = (x1>>8);									
	sendbuf[6] = x1;
	sendbuf[7] = (y1>>8);
	sendbuf[8] = y1;
	sendbuf[9] = (ForeColor>>8);									
	sendbuf[10] = ForeColor;
	I2C_Send(sendbuf,11);
}


void LiquidCrystal::SetBackLight(uint8_t brightness)
{
	sendbuf[0] = CMD_SET_LCD_BRIGHTNESS;
	sendbuf[1] = brightness;
	I2C_Send(sendbuf,2);	
}

void LiquidCrystal::SetTouchPaneOption(uint8_t enable)
{
	sendbuf[0] = CMD_TP_SET;
	sendbuf[1] = enable;
	I2C_Send(sendbuf,2);
}

void LiquidCrystal::CalibrateTouchPane(void)
{
	sendbuf[0] = CMD_TP_CALIBRATION;
	sendbuf[1] = 0x50;
	I2C_Send(sendbuf,2);
}

void LiquidCrystal::TestTouchPane(uint8_t enable)
{
	sendbuf[0] = CMD_TP_TEST;
	sendbuf[1] = enable;
	I2C_Send(sendbuf,2);
}

//object 
//===============================
void LiquidCrystal::SetPage(uint8_t page_id)
{
	sendbuf[0] = PAGE;
	sendbuf[1] = page_id;
	I2C_Send(sendbuf,2);
}

void LiquidCrystal::GetPage(void)
{
	sendbuf[0] = GET_PAGE_ID;
	I2C_Send(sendbuf,1);
}

void LiquidCrystal::SetLableValue(uint8_t page_id,uint8_t control_id,uint8_t *strings)
{
	uint8_t i=0;
	sendbuf[0] = LABLE;
	sendbuf[1] = control_id;
	sendbuf[2] = page_id;
	while(*strings !=0)
	{									
		sendbuf[i+3] = *(strings++);
		i++;
	}
	I2C_Send(sendbuf,3+i);
}

void LiquidCrystal::SetNumberValue(uint8_t page_id,uint8_t control_id,uint16_t number)
{
	sendbuf[0] = NUMBER;
	sendbuf[1] = control_id;
	sendbuf[2] = page_id;
	sendbuf[3] = (number>>8);
	sendbuf[4] = number;
	I2C_Send(sendbuf,5);
}

void LiquidCrystal::SetEditValue(uint8_t page_id,uint8_t control_id,uint8_t *strings)
{
	uint8_t i=0;
	sendbuf[0] = EDIT;
	sendbuf[1] = control_id;
	sendbuf[2] = page_id;
	while(*strings !=0)
	{									
		sendbuf[i+3] = *(strings++);
		i++;
	}
	I2C_Send(sendbuf,3+i);
}

void LiquidCrystal::GetEditValue(uint8_t page_id,uint8_t control_id)
{
	sendbuf[0] = GET_EDIT_VALUE;
	sendbuf[1] = control_id;
	sendbuf[2] = page_id;
	I2C_Send(sendbuf,3);
}

void LiquidCrystal::GetTouchEditValue(uint8_t page_id,uint8_t control_id)
{
	sendbuf[0] = GET_TOUCH_EDIT_VALUE;
	sendbuf[1] = control_id;
	sendbuf[2] = page_id;
	I2C_Send(sendbuf,3);
}


void LiquidCrystal::SetProgressbarValue(uint8_t page_id,uint8_t control_id,uint8_t value)
{
	sendbuf[0] = PROGRESS;
	sendbuf[1] = control_id;
	sendbuf[2] = page_id;
	sendbuf[3] = value;
	I2C_Send(sendbuf,4);
}

void LiquidCrystal::SetCheckboxValue(uint8_t page_id,uint8_t control_id,uint8_t value)
{
	sendbuf[0] = CHECKBOX;
	sendbuf[1] = control_id;
	sendbuf[2] = page_id;
	sendbuf[3] = value;
	I2C_Send(sendbuf,4);
}

void LiquidCrystal::GetCheckboxValue(uint8_t page_id,uint8_t control_id)
{
	sendbuf[0] = GET_CHECKBOX_VALUE;
	sendbuf[1] = control_id;
	sendbuf[2] = page_id;
	I2C_Send(sendbuf,3);
}

void LiquidCrystal::SetCircleGaugeValue(uint8_t page_id,uint8_t control_id,uint16_t value)
{
	sendbuf[0] = CIRCLEGAUGE;
	sendbuf[1] = control_id;
	sendbuf[2] = page_id;
	sendbuf[3] = (value>>8);
	sendbuf[4] = value;
	I2C_Send(sendbuf,5);
}

void LiquidCrystal::SetBarGaugeValue(uint8_t page_id,uint8_t control_id,uint16_t value)
{
	sendbuf[0] = BARGAUGE;
	sendbuf[1] = control_id;
	sendbuf[2] = page_id;
	sendbuf[3] = (value>>8);
	sendbuf[4] = value;
	I2C_Send(sendbuf,5);
}

void LiquidCrystal::SetWaterGaugeValue(uint8_t page_id,uint8_t control_id,uint16_t value)
{
	sendbuf[0] = WATERGAUGE;
	sendbuf[1] = control_id;
	sendbuf[2] = page_id;
	sendbuf[3] = (value>>8);
	sendbuf[4] = value;
	I2C_Send(sendbuf,5);
}

void LiquidCrystal::SetThermometerValue(uint8_t page_id,uint8_t control_id,uint16_t value)
{
	sendbuf[0] = THERMOMETER;
	sendbuf[1] = control_id;
	sendbuf[2] = page_id;
	sendbuf[3] = (value>>8);
	sendbuf[4] = value;
	I2C_Send(sendbuf,5);
}

void LiquidCrystal::SetBatteryValue(uint8_t page_id,uint8_t control_id,uint16_t value)
{
	sendbuf[0] = BATTERY;
	sendbuf[1] = control_id;
	sendbuf[2] = page_id;
	sendbuf[3] = (value>>8);
	sendbuf[4] = value;
	I2C_Send(sendbuf,5);
}

void LiquidCrystal::SetWaveformValue(uint8_t page_id,uint8_t control_id,uint8_t channelNo,uint8_t value)
{
	sendbuf[0] = WAVEFORM;
	sendbuf[1] = control_id;
	sendbuf[2] = page_id;
	sendbuf[3] = channelNo;
	sendbuf[4] = value;
	I2C_Send(sendbuf,5);
}

void LiquidCrystal::WaveformDataClear(uint8_t page_id,uint8_t control_id)
{
	sendbuf[0] = CLEAR_WAVEFORM;
	sendbuf[1] = control_id;
	sendbuf[2] = page_id;
	I2C_Send(sendbuf,3);
}

void LiquidCrystal::WaveformDataInsert(uint8_t page_id,uint8_t control_id,uint8_t channelNo,uint8_t value)
{
	sendbuf[0] = INSER_WAVEFORM_DATA;
	sendbuf[1] = control_id;
	sendbuf[2] = page_id;
	sendbuf[3] = channelNo;
	sendbuf[4] = value;
	I2C_Send(sendbuf,5);
}

void LiquidCrystal::WaveformDataRefress(uint8_t page_id,uint8_t control_id,uint8_t channelNo)
{
	sendbuf[0] = REFRESS_WAVEFORM;
	sendbuf[1] = control_id;
	sendbuf[2] = page_id;
	sendbuf[3] = channelNo;
	I2C_Send(sendbuf,4);
}

void LiquidCrystal::GetSliderValue(uint8_t page_id,uint8_t control_id)
{
	sendbuf[0] = GET_SLIDER_VALUE;
	sendbuf[1] = control_id;
	sendbuf[2] = page_id;
	I2C_Send(sendbuf,3);
}

void LiquidCrystal::SetSliderValue(uint8_t page_id,uint8_t control_id,uint8_t value)
{
	sendbuf[0] = SET_SLIDER_VALUE;
	sendbuf[1] = control_id;
	sendbuf[2] = page_id;
	sendbuf[3] = value;
	I2C_Send(sendbuf,4);
}




//=====================================
//for queue data
void LiquidCrystal::queue_reset()
{
	que._head = que._tail = 0;
	cmd_pos  = 0;
}

void LiquidCrystal::queue_push(qdata _data)
{
	qsize pos = (que._head+1)%QUEUE_MAX_SIZE;
	if(pos!=que._tail)
	{
		que._data[que._head] = _data;
		que._head = pos;
	}
}

void LiquidCrystal::queue_pop(qdata* _data)
{
	if(que._tail!=que._head)
	{
		*_data = que._data[que._tail];
		que._tail = (que._tail+1)%QUEUE_MAX_SIZE;
	}
}

uint8_t LiquidCrystal::queue_size()
{
	return ((que._head+QUEUE_MAX_SIZE-que._tail)%QUEUE_MAX_SIZE);
}

uint8_t LiquidCrystal::queue_find_cmd(qdata *buffer,qsize buf_len)
{
	qsize cmd_size = 0;
	qdata _data = 0;
	
	cmd_length = CMD_DATA_LENGTH-1;
	
	while(queue_size()>0)
	{
		//get 1 data
		queue_pop(&_data);

		if(cmd_pos==0&&_data != NOTIFY_TOUCH_BUTTON &&_data !=  NOTIFY_TOUCH_CHECKBOX&&_data !=NOTIFY_TOUCH_SLIDER
		    &&_data !=  NOTIFY_GET_EDIT&&_data !=NOTIFY_GET_PAGE&&_data !=  NOTIFY_GET_CHECKBOX&&_data !=NOTIFY_GET_SLIDER
		    &&_data !=NOTIFY_TOUCH_EDIT&&_data !=NOTIFY_GET_TOUCH_EDIT)
		    continue;
		
		if(cmd_pos<buf_len)
		{
				if(cmd_pos ==0)
				{
					cmd_backup = _data;
				}
				
				if(cmd_pos ==(CMD_DATA_LENGTH-1))
				{
					 if(cmd_backup == NOTIFY_GET_EDIT ||cmd_backup == NOTIFY_GET_TOUCH_EDIT)
				 			cmd_length = CMD_DATA_LENGTH+ _data-1;
				 			
				}
	
				if(cmd_pos > (CMD_MAX_SIZE-1))
				{
					cmd_pos =0;
					cmd_backup = 0;
				}
			
				buffer[cmd_pos++] = _data;
		}

		//get whole cmd data
		if(cmd_pos>cmd_length)
		{
			cmd_size = cmd_pos;
			cmd_pos = 0;
			return cmd_size;
		}
	}

	return 0;//û���γ�������һ֡
}

/*********** mid level commands, for sending data/cmds */

inline void LiquidCrystal::command(uint8_t value) {
}

inline size_t LiquidCrystal::write(uint8_t value) {
  return 1; // assume sucess
}

/************ low level data pushing commands **********/
