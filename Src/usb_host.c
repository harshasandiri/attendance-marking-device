/**
 ******************************************************************************
  * @file            : USB_HOST
  * @version         : v1.0_Cube
  * @brief           :  This file implements the USB Host 
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_host.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include "fatfs.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"

/* USB Host Core handle declaration */
USBH_HandleTypeDef hUsbHostHS;
ApplicationTypeDef Appli_state = APPLICATION_IDLE;


FATFS USBDISKFatFs;           /* File system object for USB disk logical drive */
FIL MyFile;                   /* File object */
extern uint8_t pin[8]; //Student ID
extern uint32_t byteswritten; //bytes written to USB file
extern uint32_t bytesread;//bytes read from file
extern RTC_HandleTypeDef hrtc;
extern I2C_HandleTypeDef hi2c3;
extern DMA_HandleTypeDef hdma_i2c3_rx;
extern char ID[4];//template ID retrieved when match found....
extern uint8_t id_match_status; //sets to 1 if match found....
extern uint8_t read_data[10];//store user data from matched file to display
extern uint8_t receive_data[7],send_data[7];
extern uint8_t second,minute,hour,day,date,month,year;
extern uint32_t offset;
extern RGB_typedef *RGB_matrix;
extern uint8_t template_no;
extern char template_no_string[4];



/* Buffers used for displaying Time and Date */
extern uint8_t aShowTime[20];
extern uint8_t aShowDate[10];

/* Private function prototypes -----------------------------------------------*/
void ID_seek(void);
void find_student_data_USB(void);
void get_image(void);
void mark_attendance_on_usb(void);
void find_template_number(void);
void write_new_user_to_usb(void);
void write_current_template_number(void);
static uint8_t Jpeg_CallbackFunction(uint8_t* Row, uint32_t DataLength);
static void USBH_UserProcess  (USBH_HandleTypeDef *phost, uint8_t id);
static void RTC_CalendarShow(uint8_t *showtime, uint8_t *showdate);


/* init function */				        
void MX_USB_HOST_Init(void)
{
  /* Init Host Library,Add Supported Class and Start the library*/
  USBH_Init(&hUsbHostHS, USBH_UserProcess, HOST_HS);

  USBH_RegisterClass(&hUsbHostHS, USBH_MSC_CLASS);

  USBH_Start(&hUsbHostHS);
}

/*
 * Background task
*/ 
void MX_USB_HOST_Process(void) 
{
  /* USB Host Background task */
    USBH_Process(&hUsbHostHS); 						
}
/*
 * user callback definition
*/ 
static void USBH_UserProcess  (USBH_HandleTypeDef *phost, uint8_t id)
{

  /* USER CODE BEGIN CALL_BACK_1 */
  switch(id)
  { 
  case HOST_USER_SELECT_CONFIGURATION:
  break;
    
  case HOST_USER_DISCONNECTION:
  Appli_state = APPLICATION_DISCONNECT;
  break;
    
  case HOST_USER_CLASS_ACTIVE:
  Appli_state = APPLICATION_READY;
  break;

  case HOST_USER_CONNECTION:
  Appli_state = APPLICATION_START;
  break;

  default:
  break; 
  }
  /* USER CODE END CALL_BACK_1 */
}

void ID_seek(void)
{

	uint8_t state = 0;
	char file_name[80];
	strcpy(file_name,ID);//copy input to file_name
	strcat(file_name,".TXT");//concatenate .TXT
		while(state==0){
		MX_USB_HOST_Process();

		/* Mass Storage Application State Machine */
		       switch(Appli_state)
		       {
		       case APPLICATION_READY:

		    	   f_mount(&USBDISKFatFs, (TCHAR const*)USBH_Path, 0);
		    	   f_chdir("/FP_DB");
		    	   if(f_open(&MyFile,file_name, FA_OPEN_EXISTING | FA_READ ) != FR_OK)
		    	   {
		    		   BSP_LED_On(LED4);
		    		   BSP_LCD_Clear(LCD_COLOR_BLACK);
		    		   BSP_LCD_SetFont(&Font16);
		    		   BSP_LCD_DisplayStringAt(0, 50, (uint8_t*)"Database error!", CENTER_MODE);

		    	   }else{
		    	   f_read(&MyFile,read_data,sizeof(read_data),(void*)&bytesread);
		    	   f_close(&MyFile);
		    	   id_match_status=1;
		    	   BSP_LCD_Clear(LCD_COLOR_BLACK);
		    	   BSP_LCD_SetFont(&Font16);
		    	   BSP_LCD_DisplayStringAt(0, 50, (uint8_t*)"ID info found!", CENTER_MODE);
		    	   BSP_LCD_SetFont(&Font20);
		    	   BSP_LCD_DisplayStringAt(0, 90, (uint8_t*)"Student ID:", CENTER_MODE);
		    	   BSP_LCD_DisplayStringAt(0, 110, (uint8_t*)read_data, CENTER_MODE);
		    	   }
		    	   FATFS_UnLinkDriver(USBH_Path);

		    	   Appli_state = APPLICATION_DISCONNECT;
		    	   state = 1;
		           break;

		       case APPLICATION_DISCONNECT:
		       default:
		         break;
		       }
		}
}

void find_student_data_USB(void)
{

	uint8_t state = 0;
	id_match_status=0;
	uint8_t info[30];
	char file_name[80];
	strcpy(file_name,read_data);//copy input to file_name
	strcat(file_name,".TXT");//concatenate .TXT
		while(state==0){
		MX_USB_HOST_Process();

		/* Mass Storage Application State Machine */
		       switch(Appli_state)
		       {
		       case APPLICATION_READY:

		    	   f_mount(&USBDISKFatFs, (TCHAR const*)USBH_Path, 0);
		    	   f_chdir("/STU_INFO");
		    	   if(f_open(&MyFile,file_name, FA_OPEN_EXISTING | FA_READ ) != FR_OK)
		    	   {
		    		   BSP_LED_On(LED4);
		    		   BSP_LCD_Clear(LCD_COLOR_BLACK);
		    		   BSP_LCD_SetFont(&Font16);
		    		   BSP_LCD_DisplayStringAt(0, 50, (uint8_t*)"NO INFO FOUND!", CENTER_MODE);

		    	   }else{
		    	   f_read(&MyFile,info,sizeof(info),(void*)&bytesread);
		    	   f_close(&MyFile);
		    	   id_match_status=1;
		    	   BSP_LCD_Clear(LCD_COLOR_BLACK);
		    	   BSP_LCD_SetFont(&Font16);//change to size 12 if the name gets chopped off...
		    	   BSP_LCD_DisplayStringAt(0, 100, (uint8_t*)info, CENTER_MODE);
		    	   }
		    	   FATFS_UnLinkDriver(USBH_Path);

		    	   Appli_state = APPLICATION_DISCONNECT;
		    	   state = 1;
		           break;

		       case APPLICATION_DISCONNECT:
		       default:
		         break;
		       }
		}
}

void get_image(void)
{

	uint8_t _aucLine[2048];
	offset = 0xD0000000;
	FIL MyPhoto;//photo object
	uint8_t state = 0;
	char file_name[20];
	strcpy(file_name,read_data);//copy input to file_name
	strcat(file_name,".jpg");//concatenate .jpg
	while(state==0){
	MX_USB_HOST_Process();

	/* Mass Storage Application State Machine */
	       switch(Appli_state)
	       {
	       case APPLICATION_READY:

	    	   f_mount(&USBDISKFatFs, (TCHAR const*)USBH_Path, 0);
	    	   f_chdir("/PHOTOS");
	    	   if(f_open(&MyPhoto,file_name, FA_OPEN_EXISTING | FA_READ) != FR_OK)
	    	   {

	    		   BSP_LED_On(LED4);
	    		   BSP_LCD_Clear(LCD_COLOR_BLACK);
	    		   BSP_LCD_SetFont(&Font16);
	    		   BSP_LCD_DisplayStringAt(0, 50, (uint8_t*)"No Photo Found!", CENTER_MODE);
	    		   FATFS_UnLinkDriver(USBH_Path);
		    	   Appli_state = APPLICATION_DISCONNECT;
		    	   state = 1;
	    		   break;

	    	   }
	    	   //f_lseek(&MyPhoto,0);
	    	   jpeg_decode(&MyPhoto, IMAGE_WIDTH, _aucLine, Jpeg_CallbackFunction);
	    	   f_sync(&MyPhoto);
	    	   f_close(&MyPhoto);
	    	   FATFS_UnLinkDriver(USBH_Path);

	    	   //Appli_state = APPLICATION_DISCONNECT;
	    	   state = 1;
	           break;

	       case APPLICATION_DISCONNECT:
	       default:
	         break;
	       }
	}
}

void mark_attendance_on_usb(void)
{
	uint8_t state = 0;
	char time_array[25];
	char date_array[40];
	char temp_hour[4];
	char temp_minute[4];
	char temp_second[4];
	char temp_date[5];
	char temp_month[5];
	char temp_year[5];
	HAL_I2C_Mem_Read_DMA(&hi2c3,DS3231_ADD<<1,0,I2C_MEMADD_SIZE_8BIT,receive_data,7);
	HAL_Delay(200);//Get the Time and date from the External RTC module...

	sprintf(temp_hour,"%02d",hour);
	sprintf(temp_minute,"%02d",minute);
	sprintf(temp_second,"%02d",second);
	sprintf(temp_date,"%02d",date);
	sprintf(temp_month,"%02d",month);
	sprintf(temp_year,"%02d",year);
	strcpy(time_array,temp_hour);
	strcat(time_array,":");
	strcat(time_array,temp_minute);
	strcat(time_array,":");
	strcat(time_array,temp_second);
	strcpy(date_array,temp_date);
	strcat(date_array,"/");
	strcat(date_array,temp_month);
	strcat(date_array,"/");
	strcat(date_array,"20");
	strcat(date_array,temp_year);

	while(state==0){
	MX_USB_HOST_Process();

	/* Mass Storage Application State Machine */
	       switch(Appli_state)
	       {
	       case APPLICATION_READY:

	    	   f_mount(&USBDISKFatFs, (TCHAR const*)USBH_Path, 0);
	    	   f_chdir("/ATD");
	    	   f_open(&MyFile,"SHEET.TXT", FA_OPEN_ALWAYS | FA_WRITE | FA_READ );//can create a specific directory for a class
	    	   f_lseek(&MyFile,f_size(&MyFile));
	    	   f_printf(&MyFile,"%s","EN");//EN added for the engineering faculty/removed \nEn
	    	   f_write(&MyFile,read_data, sizeof(read_data), (void *)&byteswritten);
	    	   f_printf(&MyFile, "%c", '\n'); // go to next line
	    	   f_printf(&MyFile, "%s",time_array);
	    	   f_printf(&MyFile, "%c", '\n');// go to next line
	    	   f_printf(&MyFile, "%s",date_array);
	    	   f_printf(&MyFile, "%c", '\n');// go to next line
	    	   f_close(&MyFile);

	    	   FATFS_UnLinkDriver(USBH_Path);

	    	   Appli_state = APPLICATION_DISCONNECT;
	    	   state = 1;
	           break;

	       case APPLICATION_DISCONNECT:
	       default:
	         break;
	       }
	}
}


void find_template_number(void)
{
	uint8_t state = 0;
	uint8_t temp_read[2];
		while(state==0){
		MX_USB_HOST_Process();

		/* Mass Storage Application State Machine */
		       switch(Appli_state)
		       {
		       case APPLICATION_READY:

		    	   f_mount(&USBDISKFatFs, (TCHAR const*)USBH_Path, 0);
		    	   if(f_open(&MyFile,"TEMP.TXT", FA_OPEN_EXISTING | FA_READ) != FR_OK)
		    	   {
		    		   BSP_LED_On(LED4);
		    	   }
		    	   f_read(&MyFile,temp_read,sizeof(temp_read),(void*)&bytesread);

		    	   sscanf(temp_read,"%02X",&template_no);//read the previous template number form disk...

		    	   f_close(&MyFile);
		    	   FATFS_UnLinkDriver(USBH_Path);

		    	   Appli_state = APPLICATION_DISCONNECT;
		    	   state = 1;
		           break;

		       case APPLICATION_DISCONNECT:
		       default:
		         break;
		       }
		}

}

void write_current_template_number(void)
{

	uint8_t state = 0;
		while(state==0){
		MX_USB_HOST_Process();

		/* Mass Storage Application State Machine */
		       switch(Appli_state)
		       {
		       case APPLICATION_READY:

		    	   f_mount(&USBDISKFatFs, (TCHAR const*)USBH_Path, 0);
		    	   if(f_open(&MyFile,"TEMP.TXT", FA_OPEN_ALWAYS | FA_READ |FA_WRITE) != FR_OK)
		    	   {
		    		   BSP_LED_On(LED4);
		    	   }
		    	   f_printf(&MyFile,"%02X",template_no);

		    	   f_close(&MyFile);
		    	   FATFS_UnLinkDriver(USBH_Path);

		    	   Appli_state = APPLICATION_DISCONNECT;
		    	   state = 1;
		           break;

		       case APPLICATION_DISCONNECT:
		       default:
		         break;
		       }
		}

}

void write_new_user_to_usb(void)
{
	uint8_t state = 0;
	char file_name[80];
	strcpy(file_name,template_no_string);//copy input to file_name
	strcat(file_name,".TXT");//concatenate .TXT
	while(state==0){
	MX_USB_HOST_Process();

	/* Mass Storage Application State Machine */
	       switch(Appli_state)
	       {
	       case APPLICATION_READY:

	    	   f_mount(&USBDISKFatFs, (TCHAR const*)USBH_Path, 0);
	    	   f_chdir("/FP_DB");
	    	   f_open(&MyFile,file_name, FA_OPEN_ALWAYS | FA_WRITE | FA_READ );//can create a specific directory for a class
	    	   f_write(&MyFile,pin, sizeof(pin), (void *)&byteswritten);
	    	   f_close(&MyFile);

	    	   FATFS_UnLinkDriver(USBH_Path);

	    	   Appli_state = APPLICATION_DISCONNECT;
	    	   state = 1;
	           break;

	       case APPLICATION_DISCONNECT:
	       default:
	         break;
	       }
	}
}


/**
  * @brief  Copy decompressed data to display buffer.
  * @param  Row: Output row buffer
  * @param  DataLength: Row width in output buffer
  * @retval None
  */
static uint8_t Jpeg_CallbackFunction(uint8_t* Row, uint32_t DataLength)
{

  DMA2D_HandleTypeDef hdma2d;

  hdma2d.Instance = DMA2D;

  /* Configure the DMA2D Mode, Color Mode and output offset */
  hdma2d.Init.Mode = DMA2D_M2M_PFC;
  hdma2d.Init.ColorMode = DMA2D_ARGB8888;
  hdma2d.Init.OutputOffset = 0;


  /* Foreground Configuration */
  hdma2d.LayerCfg[1].InputOffset = 0;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB888;
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0xFF;

  /* DMA2D Initialization */
  if(HAL_DMA2D_Init(&hdma2d) == HAL_OK)
  {
    if(HAL_DMA2D_ConfigLayer(&hdma2d, 1) == HAL_OK)
    {
      if (HAL_DMA2D_Start(&hdma2d, (uint32_t)Row, (uint32_t)offset, IMAGE_WIDTH, 1) == HAL_OK)
      {
        /* Polling For DMA transfer */
        HAL_DMA2D_PollForTransfer(&hdma2d, 10);//was 10
      }
    }
   }

  uint32_t pixel = 0, width_counter, result = 0, result1 = 0;

  for(width_counter = 0; width_counter < IMAGE_WIDTH; width_counter++)
  {
    pixel = *(__IO uint32_t *)(LCD_BUFFER + (width_counter*4) + (offset - LCD_BUFFER));
    result1 = (((pixel & 0x00FF0000) >> 16) | ((pixel & 0x000000FF) << 16));
    pixel = pixel & 0xFF00FF00;
    result = (result1 | pixel);
    *(__IO uint32_t *)(LCD_BUFFER + (width_counter*4) + (offset - LCD_BUFFER)) = result;

  }

  offset += (DataLength + IMAGE_WIDTH);
  return 0;

}

/** From Internal RTC if needed!!!/ using the external module for now!
  * @brief  Display the current time and date.
  * @param  showtime : pointer to buffer
  * @param  showdate : pointer to buffer
  * @retval None
  */
static void RTC_CalendarShow(uint8_t *showtime, uint8_t *showdate)
{
  RTC_DateTypeDef sdatestructureget;
  RTC_TimeTypeDef stimestructureget;

  /* Get the RTC current Time */
  HAL_RTC_GetTime(&hrtc, &stimestructureget, RTC_FORMAT_BIN);
  /* Get the RTC current Date */
  HAL_RTC_GetDate(&hrtc, &sdatestructureget, RTC_FORMAT_BIN);
  /* Display time Format : hh:mm:ss */
  sprintf((char *)showtime, "%2d:%2d:%2d", stimestructureget.Hours, stimestructureget.Minutes, stimestructureget.Seconds);
  /* Display date Format : mm-dd-yy */
  sprintf((char *)showdate, "%2d-%2d-%2d", sdatestructureget.Month, sdatestructureget.Date, 2000 + sdatestructureget.Year);
  //BSP_LCD_DisplayStringAt(0, 100,(uint8_t*)showtime, CENTER_MODE);
  //BSP_LCD_DisplayStringAt(0, 140,(uint8_t*)showdate, CENTER_MODE);
}



/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
