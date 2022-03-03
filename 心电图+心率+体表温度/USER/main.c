#include "sys.h"         //ϵͳ����
#include "delay.h"       //��ʱ
#include "usart.h"       //����  		
#include "led.h"       
#include "ADS1292.h"
#include "Timer.h"
#include "dma.h"
#include "adc.h"

s32 y[3],x[3];
u8 f_n,f_flag;

s32 fillter(void);//�˲�

/*
VCC---3.3V/4.2-6V
/RESET��PB10
START��PB11
/DRDY��PA8
/CS------PB12
MOSI---PB15
MISO---PB14
SCK-----PB13
GND---GND
*/

s32 get_volt(u32 num);//�Ѳɵ���3���ֽڲ���ת���з���32λ��

//main
int main(void)
{	
		u8 res,i,sum;	
	  u32 rate;
	  u32 step=0;
	  u8 flag_step=0;
	  u32 temp=0;
		u8 data_to_send[60]={0};//���ڷ��ͻ���
		u8 usbstatus=0;	
		u32 cannle[2];	//�洢����ͨ��������
		s32	p_Temp[2];	//���ݻ���
	  
		u32 tem[100]={0};
		u16 q=0,j;
		
		
		data_to_send[0]=0xAA;
		data_to_send[1]=0xAA;
		data_to_send[2]=0xF1;	
		data_to_send[3]=8;

//��ʼ��ϵͳʱ��	 72M	
		SystemInit();	
		delay_init();	
		delay_ms(100);
		uart1_init(115200);//���ڳ�ʼ��Ϊ115200		
//		uart2_init(9600);
	  DMA_Config(DMA1_Channel4,(u32)&USART1->DR,(u32)data_to_send);//����1DMA����
		USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE); //DMA	
		Adc_Init();	//ADC��ʼ��
//		LED_Init();			

	PBout(10)=1;
	PBout(11)=0;
		ADS1292_Init();	//��ʼ��ads1292		
		
		while(Set_ADS1292_Collect(0))//0 �����ɼ�  //1 1mV1Hz�ڲ������ź� //2 �ڲ��̽���������
		{	
				printf("1292�Ĵ�������ʧ��\r\n");
				delay_s(1);		
				DS3 =!DS3;	
				DS4 =!DS4;	
		}	
		printf("�Ĵ������óɹ�\r\n");
		delay_s(1);		
		DS3 =LEDOFF;		
		DS4 =LEDOFF;
		
		TIM2_Init(10000,7200);//ϵͳָʾ
		TIM4_Init(20000,7200);//��ʱ
		
		EXTI->IMR |= EXTI_Line8;//��DRDY�ж�			
		while(1)//ѭ����������		
		{				
				if(ads1292_recive_flag)
				{										
							cannle[0]=ads1292_Cache[3]<<16 | ads1292_Cache[4]<<8 | ads1292_Cache[5];//��ȡԭʼ����		
							cannle[1]=ads1292_Cache[6]<<16 | ads1292_Cache[7]<<8 | ads1292_Cache[8];
						
							p_Temp[0] = get_volt(cannle[0]);	//�Ѳɵ���3���ֽ�ת���з���32λ��
							p_Temp[1] = get_volt(cannle[1]);	//�Ѳɵ���3���ֽ�ת���з���32λ��
					
							//�з�����Ϊ��תΪ�޷��ţ��޷�����Ϊ�߼�����
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
					
							data_to_send[4]=cannle[0]>>24;		//25-32λ
							data_to_send[5]=cannle[0]>>16;  	//17-24
							data_to_send[6]=cannle[0]>>8;		//9-16
							data_to_send[7]=cannle[0]; 			//1-8

							data_to_send[8]=cannle[1]>>24;		//25-32λ
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
              
				  data_to_send[17] = sum;	//У���																		
				  DMA_Enable(DMA1_Channel4,18);//����1DMA 
				  sum = 0;
				}
				if(TIM4->CNT%500==0)
				{
					double temp,Voltage;
					u16 adcx,s1,s2;
					adcx=Get_Adc_Average(ADC_Channel_7,10);//ADCֵ
		      Voltage=(double)adcx*(3.3/4096)*1000;//����ʵ�ʵ�ѹ
		
		      temp=204.6398-(7.857923E-06*Voltage*Voltage)-(1.777501E-1*Voltage)+0.5;//�¶ȼ��� ��϶��״��ݺ��� 
					s1=(u16)(temp*10)%10;
					s2=(u16)(temp);
					data_to_send[14]= s2>>8;
					data_to_send[15]= s2;
					data_to_send[16]= s1;
				}
				delay_us(100);
				
				
				
		}		
}


/*���ܣ��Ѳɵ���3���ֽ�ת���з���32λ�� */
s32 get_volt(u32 num)
{		
			s32 temp;			
			temp = num;
			temp <<= 8;
			temp >>= 8;
			return temp;
}

/**********************************************************************
����������ļ������ݵ����壺
Code����ʾ������ռ�� FLASH �Ĵ�С��FLASH��
RO-data���� Read Only-data�� ��ʾ������ĳ������� const ���ͣ�FLASH��
RW-data���� Read Write-data�� ��ʾ�ѱ���ʼ����ȫ�ֱ�����SRAM��
ZI-data���� Zero Init-data�� ��ʾδ����ʼ����ȫ�ֱ���(SRAM)
***********************************************************************/

//��ͨ�˲�
s32 fillter()
{
	u8 x_n=f_n-1 , y_n=f_n-1;
	
	if(x_n-2<0) x_n=3+x_n;
	if(y_n-1<0||y_n-2<0) y_n+=3;
	y[y_n]=53*y[y_n-1]+1421*(y[y_n-2]-x[x_n-2]);
	return y[y_n];
}
