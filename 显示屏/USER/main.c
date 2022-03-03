#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"
#include "math.h"

u8 max(u8 a,u8 b)
{
	return a>b?a:b;
}

u8 min(u8 a,u8 b)
{
	return a<b?a:b;
}

 int main(void)
 {	 
 	u8 x=0;
	u8 lcd_id[12];			//存放LCD ID字符串
	 
	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(115200);	 	//串口初始化为115200
	u2_init(115200);
	u3_init(9600);
	LCD_Init();
	POINT_COLOR=RED;
	LCD_Clear(BLACK);
	 
  	while(1) 
	{		 
		u8 i;
		
		
    LCD_Fill(0,0,220,50,BLACK);
    LCD_ShowString(1,1,108,12,12,tp);
		LCD_ShowString(1,14,120,12,12,heart_rate);
		LCD_ShowString(1,27,108,12,12,step);
 		LCD_ShowString(1,40,200,12,12,dis);
		
		for(i=1;i<238;i++)
		{
			LCD_Fill(i,min(ecg[(i-1)],ecg[i])+52,i+1,max(ecg[(i-1)],ecg[i])+54,WHITE);
			LCD_Fill(i,max(ecg[(i-1)],ecg[i])+54,i+1,320,BLACK);
			LCD_Fill(i,52,i+1,min(ecg[(i-1)],ecg[i])+52,BLACK);
		}
		
		
    //delay_ms(100);		
	} 
}
 