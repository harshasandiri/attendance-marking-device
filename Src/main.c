
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "fatfs.h"
#include "libjpeg.h"
#include "usb_host.h"
#include "fingerprint.h"

/* Jpeg includes component */
#include <stdint.h>
#include <string.h>
#include "jpeglib.h"

#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_sdram.h"

/* Private variables ---------------------------------------------------------*/

I2C_HandleTypeDef hi2c3;
DMA_HandleTypeDef hdma_i2c3_rx;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi5;

UART_HandleTypeDef huart5;

UART_HandleTypeDef huart1;


/* Private variables ---------------------------------------------------------*/
uint8_t pin[8]; //Student Registration number
uint8_t user_selection; //store user input in GUI
uint8_t receive_data[7],send_data[7];//external RTC module buffers
uint8_t second,minute,hour,day,date,month,year;//external RTC module
uint32_t byteswritten; //bytes written to USB file
uint32_t bytesread;//bytes read from file
uint8_t read_data[10];//store user data from matched file to display on the LCD
uint8_t id_match_status = 0; //sets to 1 if match found
extern char ID[4];//template ID retrieved when match found....
uint32_t offset = 0xD0000000;//for jpeg
RGB_typedef *RGB_matrix;//for jpeg
uint8_t template_no=0x00;
uint8_t prev_template_no;
char template_no_string[4];//used to store hex as string
__IO uint8_t UserButtonStatus = 0; //__IO used for volatile../ value stated for the volatile variable will be unpredictable/changing without the knowledge of the compiler or the user...
extern uint8_t finger_found_state;
unsigned char buffer_Rx[60];//TEST UART BUFFER...
uint8_t buffer_Rx_USB[2] = {'0'};//	UART BUFFER FOR SOFTWARE CONNECTION...

/* Buffers used for displaying Time and Date - Internal RTC module */
extern uint8_t aShowTime[20];
extern uint8_t aShowDate[10];

uint8_t admin_pin[4];//admin pin to gain admin access...


/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C3_Init(void);
static void MX_UART5_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_RTC_Init(void);
void MX_USB_HOST_Process(void);

/* Private function prototypes -----------------------------------------------*/
void LCD_screen_init(void);
static void LCD_Config(void);
void Main_menu(void);
void admin_mode(void);
void reg_screen(void);
void Enter_ID(void);
void search(void);
void reg(void);
void GUI_PROCESS(void);
void GUI_PROCESS_WITH_ADMIN_LOCK(void);
void admin_enter_pin(void);
uint8_t keypad_admin_enter_pin(void);
uint8_t BCD2DEC(uint8_t data);
uint8_t DEC2BCD(uint8_t data);
uint8_t keypad_user_selection(void);
void keypad_scan(void);

/*------------------MAIN FUNCTION---------------------------------------------------------------*/

int main(void)
{

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C3_Init();
  MX_UART5_Init();
  MX_USART1_UART_Init();
  MX_RTC_Init();

  /* Configure USER Button */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);

  /* Configure LED */
  BSP_LED_Init(LED3);
  BSP_LED_Init(LED4);

  LCD_screen_init();


  while (1)
  {
	  GUI_PROCESS_WITH_ADMIN_LOCK();
  }
}

/*------------------END OF MAIN FUNCTION-----------------------------------------------------------*/


void GUI_PROCESS(void)
{
	LCD_screen_init();

	user_selection = keypad_user_selection();

	if(user_selection == '#')
	{
		Main_menu();
		user_selection = keypad_user_selection();

		if(user_selection == '1')
		{
			search();
		}
		else if (user_selection == '2')
		{
			admin_mode();
			user_selection = keypad_user_selection();

			if(user_selection == '1')
			{

				reg_screen();
				user_selection = keypad_user_selection();
				if(user_selection == '1')
				{
					reg_from_computer();
				}
				else if (user_selection == '2')
				{
					reg();
				}

			}
			else if (user_selection == '2')
			{

				BSP_LCD_Clear(LCD_COLOR_BLACK);
				BSP_LCD_SetFont(&Font16);
				BSP_LCD_DrawHLine(0,40,240);
				BSP_LCD_DisplayStringAt(0, 80, (uint8_t*)"Press 1 :", CENTER_MODE);
				BSP_LCD_DisplayStringAt(0, 100, (uint8_t*)"To confirm!", CENTER_MODE);
				BSP_LCD_DrawHLine(0,150,240);
				BSP_LCD_DisplayStringAt(0, 180, (uint8_t*)"Press 2 :", CENTER_MODE);
				BSP_LCD_DisplayStringAt(0, 200, (uint8_t*)"Exit!", CENTER_MODE);
				BSP_LCD_DrawHLine(0,250,240);

				user_selection = keypad_user_selection();

				if(user_selection == '1')
				{
					delete_sensor_data_from_pc();

				}else if(user_selection == '2'){
					BSP_LCD_Clear(LCD_COLOR_BLACK);
					BSP_LCD_DisplayStringAt(0, 100, (uint8_t*)"Returning!", CENTER_MODE);
					HAL_Delay(250);
				}
			}
		}
	}
}

void GUI_PROCESS_WITH_ADMIN_LOCK(void)
{
	LCD_screen_init();
	user_selection = keypad_user_selection();
	if(user_selection == '#')
	{
		Main_menu();
		user_selection = keypad_user_selection();
		if(user_selection == '1')
		{
			search();
		}
		else if (user_selection == '2')
		{
			admin_enter_pin();
		}
	}
}

void admin_enter_pin(void)
{

	uint8_t pin_entry_done = 0;
	uint8_t pin_retry_count = 0;
	uint8_t auth_flag;


	while(pin_entry_done == 0 && pin_retry_count != 3)
	{

		BSP_LCD_Clear(LCD_COLOR_BLACK);

		BSP_LCD_SetFont(&Font20);

		BSP_LCD_DisplayStringAt(0, 50, (uint8_t*)"Enter Password:", CENTER_MODE);

		BSP_LCD_SetFont(&Font16);

		BSP_LCD_DisplayStringAt(0, 90, (uint8_t*)"To gain access...", CENTER_MODE);

		BSP_LCD_SetFont(&Font24);

		BSP_LCD_DisplayStringAt(0, 160,(uint8_t*)"____",CENTER_MODE);

		auth_flag = keypad_admin_enter_pin();

		if(auth_flag == 1){

			pin_entry_done = 1;

		}else if(auth_flag == 0){

			pin_entry_done = 0;
			pin_retry_count++;
			BSP_LCD_Clear(LCD_COLOR_BLACK);
			BSP_LCD_SetFont(&Font16);
			BSP_LCD_DisplayStringAt(0, 90, (uint8_t*)"Incorrect PIN! ", CENTER_MODE);
			BSP_LCD_DisplayStringAt(0, 140, (uint8_t*)"Access Denied! ", CENTER_MODE);
			BSP_LCD_DisplayStringAt(0, 180, (uint8_t*)"Try Again! ", CENTER_MODE);

			if(pin_retry_count == 3)
			{
				BSP_LCD_Clear(LCD_COLOR_BLACK);
				BSP_LCD_SetFont(&Font16);
				BSP_LCD_DisplayStringAt(0, 90, (uint8_t*)"Maximum Tries", CENTER_MODE);
				BSP_LCD_DisplayStringAt(0, 110, (uint8_t*)"Exceeded! ", CENTER_MODE);
				BSP_LCD_DisplayStringAt(0, 160, (uint8_t*)"Access Denied! ", CENTER_MODE);
			}

			HAL_Delay(1000);
		}

		/** Clear the Pin*/
		uint8_t a=0;
		while(a<=4){
			admin_pin[a]=0;
			a++;}
	}

	if(pin_entry_done == 1)
	{

		admin_mode();
		user_selection = keypad_user_selection();

		if(user_selection == '1')
		{

			reg_screen();
			user_selection = keypad_user_selection();
			if(user_selection == '1')
			{
				reg_from_computer();
			}
			else if (user_selection == '2')
			{
				reg();
			}

		}
		else if (user_selection == '2')
		{

			BSP_LCD_Clear(LCD_COLOR_BLACK);
			BSP_LCD_SetFont(&Font16);
			BSP_LCD_DrawHLine(0,40,240);
			BSP_LCD_DisplayStringAt(0, 80, (uint8_t*)"Press 1 :", CENTER_MODE);
			BSP_LCD_DisplayStringAt(0, 100, (uint8_t*)"To confirm!", CENTER_MODE);
			BSP_LCD_DrawHLine(0,150,240);
			BSP_LCD_DisplayStringAt(0, 180, (uint8_t*)"Press 2 :", CENTER_MODE);
			BSP_LCD_DisplayStringAt(0, 200, (uint8_t*)"Exit!", CENTER_MODE);
			BSP_LCD_DrawHLine(0,250,240);

			user_selection = keypad_user_selection();

			if(user_selection == '1')
			{
				delete_sensor_data_from_pc();

			}else if(user_selection == '2'){
				BSP_LCD_Clear(LCD_COLOR_BLACK);
				BSP_LCD_DisplayStringAt(0, 100, (uint8_t*)"Returning!", CENTER_MODE);
				HAL_Delay(250);
			}
		}
	}
}


void search(void)
{
	uint8_t scan_loop_status = 0;

	while(scan_loop_status == 0)
	{
		LCD_screen_init();
		BSP_LCD_Clear(LCD_COLOR_BLACK);
		BSP_LCD_SetFont(&Font16);
		BSP_LCD_DrawHLine(0,40,240);
		BSP_LCD_DisplayStringAt(0, 80, (uint8_t*)"Press 1 :", CENTER_MODE);
		BSP_LCD_DisplayStringAt(0, 100, (uint8_t*)"to Scan finger", CENTER_MODE);
		BSP_LCD_DrawHLine(0,150,240);
		BSP_LCD_DisplayStringAt(0, 180, (uint8_t*)"Press 2 :", CENTER_MODE);
		BSP_LCD_DisplayStringAt(0, 200, (uint8_t*)"to Exit", CENTER_MODE);
		BSP_LCD_DrawHLine(0,250,240);

		user_selection = keypad_user_selection();

		if(user_selection == '1')
		{

			finger_search_gui();

			if(finger_found_state ==1){


				MX_USB_HOST_Init();
				MX_FATFS_Init();
				ID_seek();

				HAL_Delay(1000);

				MX_USB_HOST_Init();
				MX_FATFS_Init();
				find_student_data_USB();

				HAL_Delay(1000);

				LCD_Config();
				MX_FATFS_Init();
				MX_USB_HOST_Init();
				get_image();
				HAL_Delay(2000);

				MX_USB_HOST_Init();
				MX_FATFS_Init();
				mark_attendance_on_usb(); //mark attendance sheet....
			}
		}
		else if(user_selection == '2')
		{
			scan_loop_status = 1;
			BSP_LCD_Clear(LCD_COLOR_BLACK);
			BSP_LCD_DisplayStringAt(0, 100, (uint8_t*)"Returning!", CENTER_MODE);
			HAL_Delay(250);
		}
	}
}


void reg(void)
{

	MX_USB_HOST_Init();
	MX_FATFS_Init();
	find_template_number();//gets the last template number from the memory...
	finger_register_gui(template_no);

	Enter_ID();
	BSP_LCD_Clear(LCD_COLOR_BLACK);
	BSP_LCD_SetFont(&Font20);
	BSP_LCD_DisplayStringAt(0, 50, (uint8_t*)"Added to", CENTER_MODE);
	BSP_LCD_DisplayStringAt(0, 80, (uint8_t*)"Memory!", CENTER_MODE);

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


void LCD_screen_init(void)
{
   /* Initialize the LCD */
   BSP_LCD_Init();

  /* Initialize the LCD Layers */
   BSP_LCD_LayerDefaultInit(1, LCD_FRAME_BUFFER);

  /* Set LCD Foreground Layer  */
   BSP_LCD_SelectLayer(1);

   BSP_LCD_SetFont(&Font20);

   /* Clear the LCD */
   BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
   BSP_LCD_Clear(LCD_COLOR_BLACK);

   /* Set the LCD Text Color */
   BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGREEN);


   /* Display LCD messages */
   BSP_LCD_DisplayStringAt(0, 10, (uint8_t*)"Student", CENTER_MODE);
   BSP_LCD_DisplayStringAt(0, 30, (uint8_t*)"Attendance", CENTER_MODE);
   BSP_LCD_DisplayStringAt(0, 50, (uint8_t*)"Marking Device", CENTER_MODE);

   BSP_LCD_SetFont(&Font16);

   BSP_LCD_DisplayStringAt(0, 100, (uint8_t*)"By", CENTER_MODE);
   BSP_LCD_DisplayStringAt(0, 120, (uint8_t*)"Harsha Sandirigama", CENTER_MODE);
   BSP_LCD_DisplayStringAt(10, 240, (uint8_t*)"Press # to continue..", CENTER_MODE);
}

/**
  * @brief  LCD Configuration.
  * @param  None
  * @retval None
  */
static void LCD_Config(void)
{
  /* Initialize the LCD */
  BSP_LCD_Init();

  /* Background Layer Initialization */
  BSP_LCD_LayerDefaultInit(0, LCD_BUFFER);

  /* Set Foreground Layer */
  BSP_LCD_SelectLayer(0);

  /* Enable the LCD */
  BSP_LCD_DisplayOn();

  /* Set the layer window */
  BSP_LCD_SetLayerWindow(0, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT);

  /* Clear the LCD Background layer */
  BSP_LCD_Clear(LCD_COLOR_BLACK);
}

void Main_menu(void)
{
	BSP_LCD_Clear(LCD_COLOR_BLACK);
	BSP_LCD_SetFont(&Font16);
	BSP_LCD_DrawHLine(0,40,240);
	BSP_LCD_DisplayStringAt(0, 80, (uint8_t*)"Press 1 :", CENTER_MODE);
	BSP_LCD_DisplayStringAt(0, 100, (uint8_t*)"to Mark attendance", CENTER_MODE);
	BSP_LCD_DrawHLine(0,150,240);
	BSP_LCD_DisplayStringAt(0, 180, (uint8_t*)"Press 2 :", CENTER_MODE);
	BSP_LCD_DisplayStringAt(0, 200, (uint8_t*)"to Enter Admin mode", CENTER_MODE);
	BSP_LCD_DrawHLine(0,250,240);
}

void admin_mode(void)
{
	BSP_LCD_Clear(LCD_COLOR_BLACK);
	BSP_LCD_SetFont(&Font16);
	BSP_LCD_DrawHLine(0,40,240);
	BSP_LCD_DisplayStringAt(0, 80, (uint8_t*)"Press 1 :", CENTER_MODE);
	BSP_LCD_DisplayStringAt(0, 100, (uint8_t*)"Add student", CENTER_MODE);
	BSP_LCD_DrawHLine(0,150,240);
	BSP_LCD_DisplayStringAt(0, 180, (uint8_t*)"Press 2 :", CENTER_MODE);
	BSP_LCD_DisplayStringAt(0, 200, (uint8_t*)"to Remove students!", CENTER_MODE);
	BSP_LCD_DrawHLine(0,250,240);
	BSP_LCD_SetFont(&Font12);
	BSP_LCD_DisplayStringAt(0, 280, (uint8_t*)"Press # to Exit!", CENTER_MODE);
}

void reg_screen(void)
{
	BSP_LCD_Clear(LCD_COLOR_BLACK);
	BSP_LCD_SetFont(&Font16);
	BSP_LCD_DrawHLine(0,40,240);
	BSP_LCD_DisplayStringAt(0, 80, (uint8_t*)"Press 1 :", CENTER_MODE);
	BSP_LCD_DisplayStringAt(0, 100, (uint8_t*)"to Register via PC", CENTER_MODE);
	BSP_LCD_DrawHLine(0,150,240);
	BSP_LCD_DisplayStringAt(0, 180, (uint8_t*)"Press 2 :", CENTER_MODE);
	BSP_LCD_DisplayStringAt(0, 200, (uint8_t*)"Register on Device", CENTER_MODE);
	BSP_LCD_DrawHLine(0,250,240);
	BSP_LCD_SetFont(&Font12);
	BSP_LCD_DisplayStringAt(0, 280, (uint8_t*)"Press # to Exit!", CENTER_MODE);
}

void Enter_ID(void)
{
	/** Clear the Pin*/
	uint8_t x=0;
	while(x<=10){
		pin[x]=' ';
		x++;}

	uint8_t pin_entry_done = 0;

	while(pin_entry_done == 0)
	{

	BSP_LCD_Clear(LCD_COLOR_BLACK);

	BSP_LCD_SetFont(&Font20);

	BSP_LCD_DisplayStringAt(0, 50, (uint8_t*)"Enter ID (EN):", CENTER_MODE);

	BSP_LCD_SetFont(&Font24);

	//BSP_LCD_DisplayStringAt(0, 160,(uint8_t*)"________",CENTER_MODE);

	keypad_scan();

	BSP_LCD_Clear(LCD_COLOR_BLACK);
	BSP_LCD_SetFont(&Font16);
	BSP_LCD_DisplayStringAt(0, 30,(uint8_t*)"EN:",CENTER_MODE);
	BSP_LCD_DisplayStringAt(0, 60,(uint8_t*)pin,CENTER_MODE);
	BSP_LCD_DrawHLine(0,125,240);
	BSP_LCD_DisplayStringAt(0, 150, (uint8_t*)"Press 1 :", CENTER_MODE);
	BSP_LCD_DisplayStringAt(0, 170, (uint8_t*)"to Confirm", CENTER_MODE);
	BSP_LCD_DrawHLine(0,200,240);
	BSP_LCD_DisplayStringAt(0, 220, (uint8_t*)"Press 2 :", CENTER_MODE);
	BSP_LCD_DisplayStringAt(0, 240, (uint8_t*)"to re-enter", CENTER_MODE);
	BSP_LCD_DrawHLine(0,280,240);

	user_selection = keypad_user_selection();

		if(user_selection == '1'){
			pin_entry_done = 1;
		}else if(user_selection == '2'){
			pin_entry_done = 0;
			/** Clear the Pin*/
			uint8_t x=0;
			while(x<=10){
				pin[x]=' ';
				x++;}
		}
	}
}

uint8_t keypad_user_selection(void)
{
	uint8_t user_sel = 0;
	while(user_sel == 0)
	{
	 	 /* CHECK FIRST ROW */
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET);//TURN OFF FIRST ROW
	 if (HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)!= 1)
	 {
		 HAL_Delay(20);
		 while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)!= 1);//runs until the condition is false
		 user_sel = '1';
		//1
	 }
	 else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)!= 1)
	 {
		 HAL_Delay(20);
		 while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)!= 1);//runs until the condition is false
		 user_sel = '2';
		   //2
	 }
	 else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)!= 1)
	 {
		HAL_Delay(20);
		while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)!= 1);//runs until the condition is false
		//user_sel ='3';
		  //3
	 }
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);//TURN ON FIRST ROW


				/* CHECK 2ND ROW */
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_RESET);//TURN OFF 2ND ROW
		if (HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)!= 1);//runs until the condition is false
			//user_sel ='4';
		//4
		}
		else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)!= 1);//runs until the condition is false
			//user_sel = '5';
		//5
		}
		else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)!= 1);//runs until the condition is false
			//user_sel = '6';
		//6
		}
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_SET);//TURN ON 2ND ROW


				/* CHECK 3RD ROW */
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_RESET);//TURN OFF 3RD ROW
		if (HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)!= 1);//runs until the condition is false
			//user_sel = '7';
		//7
		}
		else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)!= 1);//runs until the condition is false
			//user_sel = '8';
		//8
		}
		else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)!= 1);//runs until the condition is false
			//user_sel = '9';
		//9
		}
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_SET);//TURN ON 3RD ROW



				/* CHECK 4TH ROW */
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, GPIO_PIN_RESET);//TURN OFF 4TH ROW
		if (HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)!= 1);//runs until the condition is false
			//user_sel = '*';
		//*
		}
		else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)!= 1);//runs until the condition is false
			//user_sel = '0';
		//0
		}
		else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)!= 1);//runs until the condition is false
			user_sel = '#';
		//#
		}
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, GPIO_PIN_SET);//TURN ON 4TH ROW

	}return(user_sel);
}

void keypad_scan(void)
{

	uint8_t pin_complete = 0;
	uint8_t pin_pos = 0;
	while(pin_complete == 0 && pin_pos!=8)
	{
		BSP_LCD_DisplayStringAt(0, 160,(uint8_t*)pin,CENTER_MODE);
	 	 /* CHECK FIRST ROW */
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET);//TURN OFF FIRST ROW
	 if (HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)!= 1)
	 {
		 HAL_Delay(20);
		 while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)!= 1);//runs until the condition is false
		 pin[pin_pos++] = '1';
		//1
	 }
	 else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)!= 1)
	 {
		 HAL_Delay(20);
		 while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)!= 1);//runs until the condition is false
		 pin[pin_pos++] = '2';
		   //2
	 }
	 else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)!= 1)
	 {
		HAL_Delay(20);
		while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)!= 1);//runs until the condition is false
		pin[pin_pos++] ='3';
					   //3
	 }
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);//TURN ON FIRST ROW


				/* CHECK 2ND ROW */
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_RESET);//TURN OFF 2ND ROW
		if (HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)!= 1);//runs until the condition is false
			pin[pin_pos++] ='4';
		//4
		}
		else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)!= 1);//runs until the condition is false
			pin[pin_pos++] = '5';
		//5
		}
		else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)!= 1);//runs until the condition is false
			pin[pin_pos++] = '6';
		//6
		}
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_SET);//TURN ON 2ND ROW


				/* CHECK 3RD ROW */
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_RESET);//TURN OFF 3RD ROW
		if (HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)!= 1);//runs until the condition is false
			pin[pin_pos++] = '7';
		//7
		}
		else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)!= 1);//runs until the condition is false
			pin[pin_pos++] = '8';
		//8
		}
		else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)!= 1);//runs until the condition is false
			pin[pin_pos++] = '9';
		//9
		}
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_SET);//TURN ON 3RD ROW



				/* CHECK 4TH ROW */
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, GPIO_PIN_RESET);//TURN OFF 4TH ROW
		if (HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)!= 1);//runs until the condition is false
		//*
		}
		else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)!= 1);//runs until the condition is false
			pin[pin_pos++] = '0';
		//0
		}
		else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)!= 1);//runs until the condition is false
			pin_complete = 1;
		//#
		}
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, GPIO_PIN_SET);//TURN ON 4TH ROW

	}pin_complete = 0;pin_pos = 0;//BSP_LCD_DisplayStringAt(0, 160,(uint8_t*)pin,CENTER_MODE);

}

uint8_t keypad_admin_enter_pin(void)
{
	uint8_t password_match;
	uint8_t pin_complete = 0;
	uint8_t pin_pos = 0;
	uint8_t pin_mask[3]={'_','_','_','_'};
	uint8_t pin_mask_pos = 0;

	while(pin_complete == 0 && pin_pos!=4)
	{
		//BSP_LCD_DisplayStringAt(0, 160,(uint8_t*)admin_pin,CENTER_MODE);
		BSP_LCD_DisplayStringAt(0, 160,(uint8_t*)pin_mask,CENTER_MODE);
	 	 /* CHECK FIRST ROW */
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET);//TURN OFF FIRST ROW
	 if (HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)!= 1)
	 {
		 HAL_Delay(20);
		 while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)!= 1);//runs until the condition is false
		 admin_pin[pin_pos++]  = '1';
		 pin_mask[pin_mask_pos++] = '*';
		//1
	 }
	 else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)!= 1)
	 {
		 HAL_Delay(20);
		 while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)!= 1);//runs until the condition is false
		 admin_pin[pin_pos++]  = '2';
		 pin_mask[pin_mask_pos++] = '*';
		   //2
	 }
	 else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)!= 1)
	 {
		HAL_Delay(20);
		while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)!= 1);//runs until the condition is false
		admin_pin[pin_pos++]  ='3';
		pin_mask[pin_mask_pos++] = '*';
		//3
	 }
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);//TURN ON FIRST ROW


				/* CHECK 2ND ROW */
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_RESET);//TURN OFF 2ND ROW
		if (HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)!= 1);//runs until the condition is false
			admin_pin[pin_pos++]  ='4';
			pin_mask[pin_mask_pos++] = '*';
		//4
		}
		else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)!= 1);//runs until the condition is false
			admin_pin[pin_pos++]  = '5';
			pin_mask[pin_mask_pos++] = '*';
		//5
		}
		else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)!= 1);//runs until the condition is false
			admin_pin[pin_pos++]  = '6';
			pin_mask[pin_mask_pos++] = '*';
		//6
		}
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_SET);//TURN ON 2ND ROW


				/* CHECK 3RD ROW */
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_RESET);//TURN OFF 3RD ROW
		if (HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)!= 1);//runs until the condition is false
			admin_pin[pin_pos++]  = '7';
			pin_mask[pin_mask_pos++] = '*';
		//7
		}
		else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)!= 1);//runs until the condition is false
			admin_pin[pin_pos++]  = '8';
			pin_mask[pin_mask_pos++] = '*';
		//8
		}
		else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)!= 1);//runs until the condition is false
			admin_pin[pin_pos++]  = '9';
			pin_mask[pin_mask_pos++] = '*';
		//9
		}
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_SET);//TURN ON 3RD ROW



				/* CHECK 4TH ROW */
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, GPIO_PIN_RESET);//TURN OFF 4TH ROW
		if (HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)!= 1);//runs until the condition is false
		//*
		}
		else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)!= 1);//runs until the condition is false
			admin_pin[pin_pos++]  = '0';
			pin_mask[pin_mask_pos++] = '*';
		//0
		}
		else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)!= 1)
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)!= 1);//runs until the condition is false
			pin_complete = 1;
		//#
		}
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, GPIO_PIN_SET);//TURN ON 4TH ROW

	}pin_complete = 0;pin_pos = 0;pin_mask_pos = 0;

	if(admin_pin[0] == '1' && admin_pin[1] == '2' && admin_pin[2] == '3' && admin_pin[3] == '4'){ //admin passcode is set to 1234//can be altered...
		password_match = 1;
		BSP_LED_On(LED3);
	}else{
		password_match = 0;
		BSP_LED_On(LED4);
	}
	return(password_match);
}



uint8_t BCD2DEC(uint8_t data)
{
	return (data>>4)*10 + (data&0x0f);
}

uint8_t DEC2BCD(uint8_t data)
{
	return (data/10)<<4|(data%10);
}

/*--------------------------------Interface configurations---------------------*/

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if(hi2c->Instance==hi2c3.Instance)
	{
		HAL_GPIO_TogglePin(GPIOG,GPIO_PIN_14);
		second=BCD2DEC(receive_data[0]);
		minute=BCD2DEC(receive_data[1]);
		hour=BCD2DEC(receive_data[2]);

		day=BCD2DEC(receive_data[3]);
		date=BCD2DEC(receive_data[4]);
		month=BCD2DEC(receive_data[5]);
		year= BCD2DEC(receive_data[6]);
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin==GPIO_PIN_0)
	{
		HAL_GPIO_TogglePin(GPIOG,GPIO_PIN_13);
		// use to set date and time
		/*send_data[0]=DEC2BCD(0);//SECONDS
		send_data[1]=DEC2BCD(35);//MINUTES
		send_data[2]=DEC2BCD(9);//HOURS

		send_data[3]=DEC2BCD(6);//DAY(1-7)
		send_data[4]=DEC2BCD(6);//DATE
		send_data[5]=DEC2BCD(10);//MONTH
		send_data[6]=DEC2BCD(17);//YEAR
		HAL_I2C_Mem_Write_IT(&hi2c3,DS3231_ADD<<1,0,I2C_MEMADD_SIZE_8BIT,send_data,7);
		*/
	}
}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}




/* I2C3 init function */
static void MX_I2C3_Init(void)
{

  hi2c3.Instance = I2C3;
  hi2c3.Init.ClockSpeed = 400000;
  hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* RTC init function */
static void MX_RTC_Init(void)
{

  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;

    /**Initialize RTC Only 
    */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initialize RTC and set the Time and Date 
    */
  if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != 0x32F2){
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR0,0x32F2);
  }

}

/* SPI5 init function */
static void MX_SPI5_Init(void)
{

  hspi5.Instance = SPI5;
  hspi5.Init.Mode = SPI_MODE_MASTER;
  hspi5.Init.Direction = SPI_DIRECTION_2LINES;
  hspi5.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi5.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi5.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi5.Init.NSS = SPI_NSS_SOFT;
  hspi5.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi5.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi5.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi5.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi5.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi5) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* UART5 init function */
static void MX_UART5_Init(void)
{

  huart5.Instance = UART5;
  huart5.Init.BaudRate = 57600;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}


/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  __HAL_UART_ENABLE_IT(&huart1,UART_IT_RXNE);//Activate reception flag
  __HAL_UART_ENABLE_IT(&huart1,UART_IT_TC);//Activate transmission flag

}


/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13|GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PG13 PG14 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : PE2 PE6 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pin : PG2 PG3 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

	/*Configure GPIO pins : PE3 PE4 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

}


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
	  BSP_LED_Toggle(LED4);
	  HAL_Delay(300);
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
