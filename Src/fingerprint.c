/*
 * fingerprint.c
 *
 *DY50 finger print sensor functions for stm32f429
 *
 *  Created on: Jul 30, 2017
 *      Author: Harsha Sandirigama
 */

#include "fingerprint.h"


/* Private variables ---------------------------------------------------------*/
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart1;

uint8_t finger_found_state;
__IO ITStatus UartReady = RESET;// __IO macro expands to whatever the particular compiler requires to ensure correct I/O access and addressing
extern uint8_t pin[8]; //Student ID
extern uint8_t template_no;
extern char template_no_string[4];
extern uint8_t buffer_Rx_USB[2];
extern __IO uint8_t UserButtonStatus;
extern unsigned char buffer_Rx[60];//data buffer...
char ID[4];//template ID retrieved when match found....


/*---Finger print sensor instruction codes---------------------------------------------------------*/
unsigned char VPWD[16]={0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x07,0x13,0x00,0x00,0x00,0x00,0x00,0x1b};

unsigned char LAMP[13]={0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x03,0x50,0x00,0x54};

unsigned char HANDSHAKE[13]={0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x03,0x53,0x00,0x57};

unsigned char SEAT[17]={0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x08,0x04,0x01,0x00,0x00,0x00,0xa3,0x00,0xb1};

unsigned char AUTOSEARCH[17]={0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x08,0x55,0x36,0x00,0x00,0x00,0x02,0x96};//?checksum?done!

unsigned char GIMG[12]={0xef,0x01,0xff,0xff,0xff,0xff, 0x01,0x00,0x03,0x01,0x00,0x05};

unsigned char  GENT1[13]={0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x04,0x02,0x01,0x00,0x08};

unsigned char  GENT2[13]={0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x04,0x02,0x02,0x00,0x09};

unsigned char  MERG[12]={0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x03,0x05,0x00,0x09};//merge feature in charbuffer1 and charbuffer2 to generate template...

unsigned char  STORE[15]={0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x06,0x06,0x00,0x00,0x06,0x00,0x13};

unsigned char DelTemplate[16]={0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x07,0x0c,0x00,0x01,0x00,0x01,0x00,0x0f};

unsigned char  DelAll[12]={0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x03,0x0d,0x00,0x11};



/*------------------functions--------------------*/
void reg_from_computer(void)
{
	while(1)
	{

		BSP_LCD_Clear(LCD_COLOR_BLACK);
		BSP_LCD_SetFont(&Font16);
		BSP_LCD_DisplayStringAt(0, 100, (uint8_t*)"USB Register Mode", CENTER_MODE);
		BSP_LCD_SetFont(&Font12);
		BSP_LCD_DisplayStringAt(0, 180, (uint8_t*)"Press Reset to Exit", CENTER_MODE);
		HAL_Delay(500);

		/* Reset transmission flag */
		UartReady = RESET;

		/*## Put UART peripheral in reception process ###########################*/
		if(HAL_UART_Receive_IT(&huart1,buffer_Rx_USB,1)!=HAL_OK )
		{
			Error_Handler();
		}

		/*## Wait for the end of the transfer ###################################*/
		while(UartReady != SET){BSP_LED_On(LED4);}

		/* Reset transmission flag */
		UartReady = RESET;

		BSP_LED_Off(LED4);

		if(buffer_Rx_USB[0]=='1')
		{
			MX_USB_HOST_Init();
			MX_FATFS_Init();
			find_template_number();//gets the last template number form the memory...
			finger_register_gui(template_no);
		}

		BSP_LCD_Clear(LCD_COLOR_BLACK);
		BSP_LCD_SetFont(&Font16);
		BSP_LCD_DisplayStringAt(0, 100, (uint8_t*)"Waiting for ID...", CENTER_MODE);
		BSP_LCD_SetFont(&Font12);
		BSP_LCD_DisplayStringAt(0, 160, (uint8_t*)"Press \"Register Student\" on PC!", CENTER_MODE);
		HAL_Delay(500);

		/* Reset transmission flag */
		UartReady = RESET;

		/*## Put UART peripheral in reception process ###########################*/
		if(HAL_UART_Receive_IT(&huart1,pin,8)!=HAL_OK)
		{
			Error_Handler();
		}

		/*## Wait for the end of the transfer ###################################*/
		while(UartReady != SET){BSP_LED_On(LED4);}

		/* Reset transmission flag */
		UartReady = RESET;

		BSP_LED_Off(LED4);


		BSP_LCD_Clear(LCD_COLOR_BLACK);
		BSP_LCD_SetFont(&Font16);
		BSP_LCD_DisplayStringAt(0, 30,(uint8_t*)"EN:",CENTER_MODE);
		BSP_LCD_DisplayStringAt(0, 60,(uint8_t*)pin,CENTER_MODE);
		BSP_LCD_DisplayStringAt(0, 80,(uint8_t*)"Registered!",CENTER_MODE);

		sprintf(template_no_string,"%02X",template_no);//STORE THE HEX VALUE AS STRING...

		MX_USB_HOST_Init();
		MX_FATFS_Init();
		write_new_user_to_usb();

		template_no=template_no+0x01;//put at the end...

		HAL_Delay(500);

		MX_USB_HOST_Init();
		MX_FATFS_Init();
		write_current_template_number();
	}
}


void delete_sensor_data_from_pc(void)
{
	uint8_t buffer_Rx_USB_del[2];

	BSP_LCD_Clear(LCD_COLOR_BLACK);
	BSP_LCD_SetFont(&Font16);
	BSP_LCD_DisplayStringAt(0, 100, (uint8_t*)"Command from PC!", CENTER_MODE);
	BSP_LCD_SetFont(&Font12);
	BSP_LCD_DisplayStringAt(0, 180, (uint8_t*)"Press Reset to Exit", CENTER_MODE);
	HAL_Delay(500);

	/* Reset transmission flag */
	UartReady = RESET;

	/*## Put UART peripheral in reception process ###########################*/
	if(HAL_UART_Receive_IT(&huart1,buffer_Rx_USB_del,1)!=HAL_OK )
	{
		Error_Handler();
	}

	/*## Wait for the end of the transfer ###################################*/
	while(UartReady != SET){BSP_LED_On(LED4);}

	/* Reset transmission flag */
	UartReady = RESET;

	if(buffer_Rx_USB_del[0]=='1')
	{
		BSP_LCD_Clear(LCD_COLOR_BLACK);
		BSP_LCD_SetFont(&Font16);
		BSP_LCD_DisplayStringAt(0, 100, (uint8_t*)"Deleting...!", CENTER_MODE);
		Delete_all();//delete templates from sensor database....
		HAL_Delay(1000);
		template_no = 0x00;
		MX_USB_HOST_Init();
		MX_FATFS_Init();
		write_current_template_number();//reset the template number on the USB database..
	}
}


void finger_search_gui(void)
{
	finger_found_state = 0;

	uint8_t count = 0;

	while(finger_found_state == 0 && count != 3)
	{

		   BSP_LCD_Clear(LCD_COLOR_BLACK);
		   BSP_LCD_SetFont(&Font20);
	       /* Display LCD messages */
	       BSP_LCD_DisplayStringAt(0, 50, (uint8_t*)"Please place", CENTER_MODE);
	       BSP_LCD_DisplayStringAt(0, 70, (uint8_t*)"your Thumb", CENTER_MODE);
	       BSP_LCD_DisplayStringAt(0, 90, (uint8_t*)"on the sensor...", CENTER_MODE);


	VERIFY_PWD();
	scan_finger();
	Img_generation_buffer1();
	search_finger_database();

	count++;//allow three tries...

	if(count<3 && finger_found_state == 0){
		BSP_LCD_DisplayStringAt(0, 100, (uint8_t*)"Try again!", CENTER_MODE);}
	else if (count == 3 && finger_found_state == 0){
		BSP_LCD_DisplayStringAt(0, 100, (uint8_t*)"Maximum tries", CENTER_MODE);
		BSP_LCD_DisplayStringAt(0, 120, (uint8_t*)"exceeded!", CENTER_MODE);}

	HAL_Delay(1000);

	}
}

void finger_register_gui(unsigned char ID)
{

	   BSP_LCD_Clear(LCD_COLOR_BLACK);
	   BSP_LCD_SetFont(&Font20);
	       /* Display LCD messages */
	       BSP_LCD_DisplayStringAt(0, 50, (uint8_t*)"Please place", CENTER_MODE);
	       BSP_LCD_DisplayStringAt(0, 70, (uint8_t*)"your Thumb", CENTER_MODE);
	       BSP_LCD_DisplayStringAt(0, 90, (uint8_t*)"on the sensor...", CENTER_MODE);

	VERIFY_PWD();
	scan_finger();
	Img_generation_buffer1();

	  /* Display LCD messages */
	           BSP_LCD_Clear(LCD_COLOR_BLACK);
		       BSP_LCD_DisplayStringAt(0, 50, (uint8_t*)"Please place", CENTER_MODE);
		       BSP_LCD_DisplayStringAt(0, 70, (uint8_t*)"your Thumb", CENTER_MODE);
		       BSP_LCD_DisplayStringAt(0, 90, (uint8_t*)"on the sensor", CENTER_MODE);
		       BSP_LCD_DisplayStringAt(0, 110, (uint8_t*)"again!", CENTER_MODE);
		       HAL_Delay(1000);

	scan_finger();
	Img_generation_buffer2();
	Reg_Model();
	store_template(ID);

	HAL_Delay(1000);//Delayed to display information on screen...
}

void VERIFY_PWD(void)
{
	uint8_t verified = 0;//variable used to break the loop
	unsigned char VPWD_RX[12];//buffer used for reception
	while(verified == 0){

	HAL_Delay(1000);//allow the device to start up

	/*## Start the transmission process #####################################*/
	if(HAL_UART_Transmit_IT(&huart5,VPWD,16)!=HAL_OK)
	{
		Error_Handler();
	}
	/*## Wait for the end of the transfer ###################################*/
	while(UartReady != SET){}

	/* Reset transmission flag */
	UartReady = RESET;

	/*## Put UART peripheral in reception process ###########################*/
	if(HAL_UART_Receive_IT(&huart5,VPWD_RX,12)!=HAL_OK)
	{
		Error_Handler();
	}

	/*## Wait for the end of the transfer ###################################*/
	while(UartReady != SET){}

	/* Reset transmission flag */
	UartReady = RESET;

	if(VPWD_RX[9]==0x00 && VPWD_RX[11]==0x0A){   //check the confirmation code and the checksum..
		verified = 1;
		}
	}
}

void scan_finger(void)
{
	unsigned char GIMG_RX[12];//buffer used for reception

	uint8_t verified = 0;//variable used to break the loop

	while(verified == 0){

	HAL_Delay(500);

	/*## Start the transmission process #####################################*/
	if(HAL_UART_Transmit_IT(&huart5,GIMG,12)!=HAL_OK)
	{
		Error_Handler();
	}
	/*## Wait for the end of the transfer ###################################*/
	while(UartReady != SET){}

	/* Reset transmission flag */
	UartReady = RESET;

	/*## Put UART peripheral in reception process ###########################*/
	if(HAL_UART_Receive_IT(&huart5,GIMG_RX,12)!=HAL_OK)
	{
		Error_Handler();
	}

	HAL_Delay(1000);//delayed to read the finger....

	/*## Wait for the end of the transfer ###################################*/
	while(UartReady != SET){}

	/* Reset transmission flag */
	UartReady = RESET;

	if(GIMG_RX[9]==0x00 && GIMG_RX[11]==0x0A){//check the confirmation code and the checksum..//read finger..
		//verified = 1;//FINGER FOUND!!!
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_13,GPIO_PIN_SET);//recognizes finger when covered with fingerprints, had to wipe off..
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_RESET);
		verified = 1;//FINGER FOUND!!!
		BSP_LCD_Clear(LCD_COLOR_BLACK);
		BSP_LCD_SetFont(&Font20);
		BSP_LCD_DisplayStringAt(0, 70, (uint8_t*)"Finger Scanned!", CENTER_MODE);

	}else if(GIMG_RX[9]==0x02){//No finger detected!!!
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_SET);//Turn on red LED...
		BSP_LCD_Clear(LCD_COLOR_BLACK);
		BSP_LCD_SetFont(&Font20);
		BSP_LCD_DisplayStringAt(0, 50, (uint8_t*)"Place your finger", CENTER_MODE);
		BSP_LCD_DisplayStringAt(0, 70, (uint8_t*)"on the sensor", CENTER_MODE);
		BSP_LCD_DisplayStringAt(0, 90, (uint8_t*)"properly!", CENTER_MODE);
		}
	}
}

void Img_generation_buffer1(void)
{
	unsigned char GENT1_RX[12];//buffer used for reception
	/*## Start the transmission process #####################################*/
	if(HAL_UART_Transmit_IT(&huart5,GENT1,13)!=HAL_OK)
	{
		Error_Handler();
	}
	/*## Wait for the end of the transfer ###################################*/
	while(UartReady != SET){}

	/* Reset transmission flag */
	UartReady = RESET;

	/*## Put UART peripheral in reception process ###########################*/
	if(HAL_UART_Receive_IT(&huart5,GENT1_RX,12)!=HAL_OK)
	{
		Error_Handler();
	}

	HAL_Delay(600);//delayed for image generation....

	/*## Wait for the end of the transfer ###################################*/
	while(UartReady != SET){}

	/* Reset transmission flag */
	UartReady = RESET;

	if(GENT1_RX[9]==0x00 && GENT1_RX[11]==0x0A){   //check the confirmation code and the checksum..
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_13,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_RESET);
	}
	else if(GENT1_RX[9]==0x06){//messy
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_SET);}
	else if(GENT1_RX[9]==0x07){//normal
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_SET);}
	else if(GENT1_RX[9]==0x15){//not valid image
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_SET);}
	else if(GENT1_RX[9]==0x01){//packet error
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_SET);}

}


void Img_generation_buffer2(void)
{
	unsigned char GENT2_RX[12];//buffer used for reception
	/*## Start the transmission process #####################################*/
	if(HAL_UART_Transmit_IT(&huart5,GENT2,13)!=HAL_OK)
	{
		Error_Handler();
	}
	/*## Wait for the end of the transfer ###################################*/
	while(UartReady != SET){}

	/* Reset transmission flag */
	UartReady = RESET;

	/*## Put UART peripheral in reception process ###########################*/
	if(HAL_UART_Receive_IT(&huart5,GENT2_RX,12)!=HAL_OK)
	{
		Error_Handler();
	}

	HAL_Delay(600);//delayed for image generation....

	/*## Wait for the end of the transfer ###################################*/
	while(UartReady != SET){}

	/* Reset transmission flag */
	UartReady = RESET;

	if(GENT2_RX[9]==0x00 && GENT2_RX[11]==0x0A){   //check the confirmation code and the checksum..
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_13,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_RESET);
	}
	else if(GENT2_RX[9]==0x06){//messy
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_SET);}
	else if(GENT2_RX[9]==0x07){//normal
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_SET);}
	else if(GENT2_RX[9]==0x15){//not valid image
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_SET);}
	else if(GENT2_RX[9]==0x01){//packet error
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_SET);}
}

void search_finger_database(void)
{
	unsigned char SEAT_RX[16];//buffer used for reception
	/*## Start the transmission process #####################################*/
	if(HAL_UART_Transmit_IT(&huart5,SEAT,17)!=HAL_OK)
	{
		Error_Handler();
	}
	/*## Wait for the end of the transfer ###################################*/
	while(UartReady != SET){}

	/* Reset transmission flag */
	UartReady = RESET;

	/*## Put UART peripheral in reception process ###########################*/
	if(HAL_UART_Receive_IT(&huart5,SEAT_RX,16)!=HAL_OK)
	{
		Error_Handler();
	}

	HAL_Delay(500);//delayed for database search....

	/*## Wait for the end of the transfer ###################################*/
	while(UartReady != SET){}

	/* Reset transmission flag */
	UartReady = RESET;

	if(SEAT_RX[9]==0x00){   //check the confirmation code
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_13,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_RESET);

		finger_found_state = 1;//used to break the loop...

		BSP_LCD_Clear(LCD_COLOR_BLACK);
		BSP_LCD_SetFont(&Font20);
		BSP_LCD_DisplayStringAt(0, 100, (uint8_t*)"Match Found!", CENTER_MODE);

		/*## copy buffer data to data space buffer ###################################*/
		uint8_t j=0;
		while(j<=15){
		buffer_Rx[j] =SEAT_RX[j];
		j++;}

		buffer_Rx[20] = SEAT_RX[11]; //extract the fingerprint template ID...

		sprintf(ID,"%02X",SEAT_RX[11]);//STORE THE HEX VALUE AS STRING...

	}
	else if(SEAT_RX[9]==0x01){//packet error
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_SET);}
	else if(SEAT_RX[9]==0x09){//not found
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_SET);
		BSP_LCD_Clear(LCD_COLOR_BLACK);
		BSP_LCD_SetFont(&Font16);
		BSP_LCD_DisplayStringAt(0, 50, (uint8_t*)"Finger Not Found!", CENTER_MODE);
	}

}

void Reg_Model(void)
{
	unsigned char MERG_RX[12];//buffer used for reception
	/*## Start the transmission process #####################################*/
	if(HAL_UART_Transmit_IT(&huart5,MERG,12)!=HAL_OK)
	{
		Error_Handler();
	}
	/*## Wait for the end of the transfer ###################################*/
	while(UartReady != SET){}

	/* Reset transmission flag */
	UartReady = RESET;

	/*## Put UART peripheral in reception process ###########################*/
	if(HAL_UART_Receive_IT(&huart5,MERG_RX,12)!=HAL_OK)
	{
		Error_Handler();
	}

	HAL_Delay(600);//delayed....

	/*## Wait for the end of the transfer ###################################*/
	while(UartReady != SET){}

	/* Reset transmission flag */
	UartReady = RESET;

	if(MERG_RX[9]==0x00 && MERG_RX[11]==0x0A){   //check the confirmation code and the checksum..
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_13,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_RESET);
	}
	else if(MERG_RX[9]==0x01){//Packet error
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_SET);}
	else if(MERG_RX[9]==0x0a){//Merge failed
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_SET);}


}


unsigned char store_template(unsigned char ID)

{
	unsigned char STORE_RX[12];//buffer used for reception

	unsigned char checksum = ID + 0x0d;//calculate the checksum

	STORE[12] = ID;

	STORE[14] = checksum;

	/*## Start the transmission process #####################################*/
	if(HAL_UART_Transmit_IT(&huart5,STORE,15)!=HAL_OK)
	{
		Error_Handler();
	}
	/*## Wait for the end of the transfer ###################################*/
	while(UartReady != SET){}

	/* Reset transmission flag */
	UartReady = RESET;

	/*## Put UART peripheral in reception process ###########################*/
	if(HAL_UART_Receive_IT(&huart5,STORE_RX,12)!=HAL_OK)
	{
		Error_Handler();
	}

	HAL_Delay(600);//delayed..

	/*## Wait for the end of the transfer ###################################*/
	while(UartReady != SET){}

	/* Reset transmission flag */
	UartReady = RESET;

	if(STORE_RX[9]==0x00 && STORE_RX[11]==0x0A){   //check the confirmation code and the checksum..
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_13,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_RESET);

		BSP_LCD_Clear(LCD_COLOR_BLACK);
		BSP_LCD_SetFont(&Font20);
		BSP_LCD_DisplayStringAt(0, 50, (uint8_t*)"New User", CENTER_MODE);
		BSP_LCD_DisplayStringAt(0, 80, (uint8_t*)"Registered!", CENTER_MODE);

	}
	else if(STORE_RX[9]==0x01){//Packet error
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_SET);}
	else if(STORE_RX[9]==0x0b){//Incorrect page ID
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_SET);}
	else if(STORE_RX[9]==0x18){//FLASH write error
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_SET);}

}



unsigned char del_template(unsigned char ID)
{
	unsigned char DelTemplate_RX[12];//buffer used for reception

	unsigned char checksum = ID + 0x15;

	DelTemplate[13] = ID;

	DelTemplate[15] = checksum;

	/*## Start the transmission process #####################################*/
	if(HAL_UART_Transmit_IT(&huart5,DelTemplate,15)!=HAL_OK)
	{
		Error_Handler();
	}
	/*## Wait for the end of the transfer ###################################*/
	while(UartReady != SET){}

	/* Reset transmission flag */
	UartReady = RESET;

	/*## Put UART peripheral in reception process ###########################*/
	if(HAL_UART_Receive_IT(&huart5,DelTemplate_RX,12)!=HAL_OK)
	{
		Error_Handler();
	}

	HAL_Delay(600);//delayed....

	/*## Wait for the end of the transfer ###################################*/
	while(UartReady != SET){}

	/* Reset transmission flag */
	UartReady = RESET;

	if(DelTemplate_RX[9]==0x00 && DelTemplate_RX[11]==0x0A){//check the confirmation code and the checksum..
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_13,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_RESET);
	}
	else if(DelTemplate_RX[9]==0x01){//Packet error
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_SET);}
	else if(DelTemplate_RX[9]==0x10){//Delete failed...
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_SET);}

}



void Delete_all(void)
{
	unsigned char DelAll_RX[12];//buffer used for reception

	/*## Start the transmission process #####################################*/
	if(HAL_UART_Transmit_IT(&huart5,DelAll,12)!=HAL_OK)
	{
		Error_Handler();
	}
	/*## Wait for the end of the transfer ###################################*/
	while(UartReady != SET){}

	/* Reset transmission flag */
	UartReady = RESET;

	/*## Put UART peripheral in reception process ###########################*/
	if(HAL_UART_Receive_IT(&huart5,DelAll_RX,12)!=HAL_OK)
	{
		Error_Handler();
	}

	HAL_Delay(600);//delayed....

	/*## Wait for the end of the transfer ###################################*/
	while(UartReady != SET){}

	/* Reset transmission flag */
	UartReady = RESET;

	if(DelAll_RX[9]==0x00 && DelAll_RX[11]==0x0A){   //check the confirmation code and the checksum..
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_13,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_RESET);
		BSP_LCD_Clear(LCD_COLOR_BLACK);
		BSP_LCD_SetFont(&Font20);
		BSP_LCD_DisplayStringAt(0, 50, (uint8_t*)"Sensor Database", CENTER_MODE);
		BSP_LCD_DisplayStringAt(0, 80, (uint8_t*)"Deleted!", CENTER_MODE);
	}
	else if(DelAll_RX[9]==0x01){//Packet error
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_SET);}
	else if(DelAll_RX[9]==0x11){//Delete failed...
		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_SET);}

}


/**
  * @brief  Tx Transfer completed callback
  * @param  UartHandle: UART handle.
  * @note   This example shows a simple way to report end of DMA Tx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart5)
{
  /* Set transmission flag: transfer complete*/
  UartReady = SET;
}

/**
  * @brief  Rx Transfer completed callback
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report end of DMA Rx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart5)
{
  /* Set transmission flag: transfer complete*/
  UartReady = SET;
}





