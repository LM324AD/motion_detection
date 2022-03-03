#include "sys.h"         //系统配置
#include "delay.h"       //延时
#include "usart.h"       //串口  		
#include "led.h"       
#include "ADS1292.h"
#include "Timer.h"
#include "dma.h"
#include "adc.h"

s32 y[3],x[3];
u8 f_n,f_flag;

s32 fillter(void);//滤波

/*
VCC---3.3V/4.2-6V
/RESET—PB10
START—PB11
/DRDY—PA8
/CS------PB12
MOSI---PB15
MISO---PB14
SCK-----PB13
GND---GND
*/

s32 get_volt(u32 num);//把采到的3个字节补码转成有符号32位数

//main
int main(void)
{	
		u8 res,i,sum;	
	  u32 rate;
	  u32 step=0;
	  u8 flag_step=0;
	  u32 temp=0;
		u8 data_to_send[60]={0};//串口发送缓存
		u8 usbstatus=0;	
		u32 cannle[2];	//存储两个通道的数据
		s32	p_Temp[2];	//数据缓存
	  
		u32 tem[100]={0};
		u16 q=0,j;
		
		
		data_to_send[0]=0xAA;
		data_to_send[1]=0xAA;
		data_to_send[2]=0xF1;	
		data_to_send[3]=8;

//初始化系统时钟	 72M	
		SystemInit();	
		delay_init();	
		delay_ms(100);
		uart1_init(115200);//串口初始化为115200		
//		uart2_init(9600);
	  DMA_Config(DMA1_Channel4,(u32)&USART1->DR,(u32)data_to_send);//串口1DMA设置
		USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE); //DMA	
		Adc_Init();	//ADC初始化
//		LED_Init();			

	PBout(10)=1;
	PBout(11)=0;
		ADS1292_Init();	//初始化ads1292		
		
		while(Set_ADS1292_Collect(0))//0 正常采集  //1 1mV1Hz内部侧试信号 //2 内部短接噪声测试
		{	
				printf("1292寄存器设置失败\r\n");
				delay_s(1);		
				DS3 =!DS3;	
				DS4 =!DS4;	
		}	
		printf("寄存器设置成功\r\n");
		delay_s(1);		
		DS3 =LEDOFF;		
		DS4 =LEDOFF;
		
		TIM2_Init(10000,7200);//系统指示
		TIM4_Init(20000,7200);//计时
		
		EXTI->IMR |= EXTI_Line8;//开DRDY中断			
		while(1)//循环发送数据		
		{				
				if(ads1292_recive_flag)
				{										
							cannle[0]=ads1292_Cache[3]<<16 | ads1292_Cache[4]<<8 | ads1292_Cache[5];//获取原始数据		
							cannle[1]=ads1292_Cache[6]<<16 | ads1292_Cache[7]<<8 | ads1292_Cache[8];
						
							p_Temp[0] = get_volt(cannle[0]);	//把采到的3个字节转成有符号32位数
							p_Temp[1] = get_volt(cannle[1]);	//把采到的3个字节转成有符号32位数
					
							//有符号数为再转为无符号，无符号数为逻辑右移
							cannle[0] = p_Temp[0];
							cannle[1]	= p_Temp[1];
					    
					    x[f_n]=p_Temp[1];
					    f_n++;
					    if(f_n>2) 
							{
								f_flag++;
								f_n=0;
							}
					    if(f_n>2&&f_flag>2)
					     cannle[1]=fillter();
//							if((s32)cannle[1]<0)
//								cannle[1]=0;
					
//					    tem[q]=cannle[1]/100;
//					    q++;
//					    if(q>99) q=0;
//					    for(j=0;j<100;j++) cannle[1]+=tem[j];
					
							data_to_send[4]=cannle[0]>>24;		//25-32位
							data_to_send[5]=cannle[0]>>16;  	//17-24
							data_to_send[6]=cannle[0]>>8;		//9-16
							data_to_send[7]=cannle[0]; 			//1-8

							data_to_send[8]=cannle[1]>>24;		//25-32位
							data_to_send[9]=cannle[1]>>16;  	//17-24
							data_to_send[10]=cannle[1]>>8;		//9-16
							data_to_send[11]=cannle[1];			 //1-8
							
								
              if(data_to_send[10]>temp+0x0f)
							{
								temp=data_to_send[10];
								if(!flag_step)
								{
									
									step++;
									flag_step=1;
									if(step==3){
									 rate=1*60*100000/TIM4->CNT;
								   step=0;
									
									
									 TIM4->CNT=0;
									if(step==1)
										step=1;
									}
								}
							}
							if(data_to_send[10]<temp-0x40)
							{
								flag_step=0;
								temp=data_to_send[10];
							}
							//rate/=2;
//							tem[q]=rate/5;
//							q++;
//							if(q==5) q=0;
//							for(j=0;j<5;j++) rate+=tem[j];
							
//							if(rate>500&&rate<1200)
//							{
							data_to_send[12] = rate/10;
							data_to_send[13] = rate%10;
//							}
              
							
																							
							ads1292_recive_flag=0;
							
							for(i=0;i<17;i++)
					     sum += data_to_send[i];		
              
				  data_to_send[17] = sum;	//校验和																		
				  DMA_Enable(DMA1_Channel4,18);//串口1DMA 
				  sum = 0;
				}
				if(TIM4->CNT%500==0)
				{
					double temp,Voltage;
					u16 adcx,s1,s2;
					adcx=Get_Adc_Average(ADC_Channel_7,10);//ADC值
		      Voltage=(double)adcx*(3.3/4096)*1000;//换算实际电压
		
		      temp=204.6398-(7.857923E-06*Voltage*Voltage)-(1.777501E-1*Voltage)+0.5;//温度计算 拟合二阶传递函数 
					s1=(u16)(temp*10)%10;
					s2=(u16)(temp);
					data_to_send[14]= s2>>8;
					data_to_send[15]= s2;
					data_to_send[16]= s1;
				}
				delay_us(100);
				
				
				
		}		
}


/*功能：把采到的3个字节转成有符号32位数 */
s32 get_volt(u32 num)
{		
			s32 temp;			
			temp = num;
			temp <<= 8;
			temp >>= 8;
			return temp;
}

/**********************************************************************
编译结果里面的几个数据的意义：
Code：表示程序所占用 FLASH 的大小（FLASH）
RO-data：即 Read Only-data， 表示程序定义的常量，如 const 类型（FLASH）
RW-data：即 Read Write-data， 表示已被初始化的全局变量（SRAM）
ZI-data：即 Zero Init-data， 表示未被初始化的全局变量(SRAM)
***********************************************************************/

//低通滤波
s32 fillter()
{
	u8 x_n=f_n-1 , y_n=f_n-1;
	
	if(x_n-2<0) x_n=3+x_n;
	if(y_n-1<0||y_n-2<0) y_n+=3;
	y[y_n]=53*y[y_n-1]+1421*(y[y_n-2]-x[x_n-2]);
	return y[y_n];
}
