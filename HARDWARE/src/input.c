#include "input.h"
#include "sys.h"

void input_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//��ʼ��KEY0-->GPIOA.13,KEY1-->GPIOA.15  ��������
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOE,ENABLE);//ʹ��PORTA,PORTEʱ��

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;//PE2~4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOE, &GPIO_InitStructure);//��ʼ��GPIOE2,3,4

	//��ʼ�� WK_UP-->GPIOA.0	  ��������
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0���ó����룬Ĭ������	  
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.0
}
//32-25 null 24-17 pb5:pc0-pc8 16-9 pb13:pc0-pc8 8-1 pb15:pc0-pc8

GPIO_TypeDef* INPUT_GPIO[]={GPIOA,GPIOE,GPIOE,GPIOE};
const uint16_t INPUT_PIN[]={GPIO_Pin_0,GPIO_Pin_2,GPIO_Pin_3,GPIO_Pin_4};

u32 input_Data()
{
	u16 i=0;
	u32 in=0;
	//unsigned int temp=0;
	
	for(i=0;i<countof(INPUT_PIN);i++){
		in <<= 1;
		if(GPIO_ReadInputDataBit(INPUT_GPIO[i],INPUT_PIN[i])){
			in |= 0x01;
		}
	}
	if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)){
		in |= 0x01;
	}
	
	return in;
}

