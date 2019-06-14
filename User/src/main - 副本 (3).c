/* Includes ------------------------------------------------------------------*/
#define USER_EEPROM 1
#include "sys.h"
#include "led.h"
#include "coin.h"
#if USER_EEPROM==0
	#include "24cxx.h"
#else
	#include "myiic.h"
	#include "Eeprom.h"
#endif
#include "ESP8266.h"
#include "DES.h"

#include "userIO.h"

#include <stdlib.h>


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#if SYSTEM_SUPPORT_UCOS
//���������ջ��С
#define STK_SIZE      64

//�����������ȼ�
#define TASK_Prio     10

//��������ڴ��С
#define MEM_SIZE_BIG		64
#define MEM_SIZE_LIT		32
#define MEM_NUM_BIG			20
#define MEM_NUM_LIT			10
#define MEM_NUM				MEM_NUM_LIT
#define MEM_SIZE			MEM_SIZE_BIG
#endif

#define COM_END				"\r\n"

#define COM_CHECK_OFF		2
#define COM_ORDER_OFF		4
#define COM_COM_OFF			5
#define COM_DATA_OFF		6


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
#if SYSTEM_SUPPORT_UCOS
//��������
void TaskStart(void *pdata);
void TaskOne(void *pdata);
void TaskTwo(void *pdata);	
void TaskThree(void *pdata);

//_TASK���� _NUM��ջ��С _PRIO���ȼ�
#define CREATE_TASK(_TASK,_STKSIZE,_PRIO)	do{\
		static OS_STK _TASK##_Stk[_STKSIZE];			\
		OSTaskCreate(_TASK, (void *)0, &_TASK##_Stk[_STKSIZE-1],_PRIO);\
	}while(0)
#define	CREATE_SMSG(_OSQ,_NUM)	do{\
		static void* _OSQ##_MSG[_NUM];	\
		_OSQ =	OSQCreate(_OSQ##_MSG, _NUM); \
	}while(0)
#define CREATE_MEM(_OSMEM,_NUM,_SIZE)	do{\
		INT8U err;\
		static INT32U _OSMEM##_MEM[_NUM][_SIZE];	\
		_OSMEM =	OSMemCreate((void*)_OSMEM##_MEM, _NUM, _SIZE, &err); \
	}while(0)

#endif


/* Private functions ---------------------------------------------------------*/
typedef  bool (*MSG_CALLBACK)(u8 *buf);

typedef struct com_struct {
    char* com;
	u8 level;//�ȼ������Ƿ�ת��
	MSG_CALLBACK cb;//�ص�����
} COM;

#define COM_LOGIN		0x10
#define COM_LOGIN_LEN	34
#define COM_COIN		0x20
#define COM_COIN_LEN	10

//#define COM_USER		0
#define COM_WIFI		1

#define START_TIME_MIN	5
#define START_TIME_MAX	60

const int COIN_IO_TIME[5] ={50,100,150,200,250};

//OS_EVENT * UartMutex;
//OS_MEM * Mem;
//OS_EVENT * ComQWifi;
//OS_EVENT * ComQUser;
//OS_EVENT * ComSemSend;//������Ϣ
OS_EVENT * QCoin;//����Ͷ����

int Level=0;
u32 time=0;
int IOTime=0;
int StartTime=5;
u32 AllCoin=0;//��Ͷ����
u8 C0=0,C1=0,C2=0,C3=0;
u8 Id[9]="80061";
u8 Password[9]="1234";
//��ҪEEPROM�洢�����ݣ�8�ֽ�id��8�ֽ����룬256+3�ֽ��ܱ�����1�ֽ�ioʱ��,1�ֽ�����ʱ��
//24c04�ܹ�512�ֽڣ�
#define ADDRESS_ID			0x110	//272
#define ADDRESS_PASSWORD	0x118	//280
#define ADDRESS_IO			0x120	//288
#define ADDRESS_START_TIME	0x124	//292
#define ADDRESS_C0			258
#define ADDRESS_C1			257
#define ADDRESS_C2			256




//OS_EVENT * msg_page;			//��������ҳ�����¼���ָ��

//��Ϣ��Ϊ���֣�wifi��Ϣ������wifi���͵�оƬ�˵���Ϣ���û���Ϣ�����û�ͨ�����ڷ��͸�wifiģ�����Ϣ���豸��Ϣ�������豸��������Ϣ
const COM COM_MSG_DEVEICE[] = {
	{"LEVEL=",0,0},//���õȼ�
	{"SETIOTIME=",0,0},//����ioʱ��
	{"GETIOTIME",0,0},
	{"WIFI=",0,0},//����wifi�˺�����
	{"SETID=",0,0},//�����豸id
	{"CHANGEID=",0,0},//�ı�id
	{"SETPASSWORD=",0,0},//�ı�����
	{"GETID",0,0},
	{"GETPASSWORD",0,0},
	{"GETCOIN",0,0},//��ȡ�ܱ���
	{"CWLAP",0,0},
	{"SETSTARTTIME=",0,0},
	{"GETSTARTTIME",0,0},
	{"SETCOIN=",0,0},//д���ܱ�����������
};

const char * COUNT_DOWN[]={"9","8","7","6","5","4","3","2","1","0",};
const char DES_KEY[8] = { 0xbe, 0x0a, 0x61, 0x37, 0x1a, 0x63, 0xc5, 0x1d };

/*******************************************************************************
* Function Name  : main.
* Descriptioan   : Main routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
int main(void)
{	
	OSInit();
//	OSTaskCreate( TaskStart,	                            //task pointer
//					(void *)0,	                                  //parameter
//					(OS_STK *)&TASK_START_STK[START_STK_SIZE-1],	//task stack top pointer
//					START_TASK_Prio );	                          //task priority
	CREATE_TASK(TaskStart,STK_SIZE*8,TASK_Prio);
	OSStart();
	return 0;	
}


#if USER_EEPROM==1
	#define EE_TYPE	511
#endif
void ROM_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len)
{
	OS_CPU_SR cpu_sr=0;
	OS_ENTER_CRITICAL(); 
#if USER_EEPROM==1
	I2CWriteByte(WriteAddr,(u8*)&DataToWrite,Len);
#else
	AT24CXX_WriteLenByte(WriteAddr,DataToWrite,Len);
#endif
	OS_EXIT_CRITICAL(); 
}

u32 ROM_ReadLenByte(u16 ReadAddr,u8 Len)
{
	OS_CPU_SR cpu_sr=0;
	u32 r=0;
	OS_ENTER_CRITICAL();
#if USER_EEPROM==1
	I2CReadByte(ReadAddr,(u8*)&r,Len);
#else
	r=AT24CXX_ReadLenByte(ReadAddr,Len);
#endif
	OS_EXIT_CRITICAL(); 
	return r;
}

void ROM_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite)
{
	OS_CPU_SR cpu_sr=0;
	OS_ENTER_CRITICAL();
#if USER_EEPROM==1
	I2CWriteByte(WriteAddr,pBuffer,NumToWrite);
#else
	AT24CXX_Write(WriteAddr,pBuffer, NumToWrite);
#endif
	OS_EXIT_CRITICAL(); 
}

void ROM_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead)
{
	OS_CPU_SR cpu_sr=0;
	OS_ENTER_CRITICAL();
#if USER_EEPROM==1
	I2CReadByte(ReadAddr,pBuffer,NumToRead);
#else
	AT24CXX_Read(ReadAddr,pBuffer,NumToRead);
#endif
	OS_EXIT_CRITICAL(); 
}

u8 ROM_ReadOneByte(u16 ReadAddr)
{
	u8 r;
	OS_CPU_SR cpu_sr=0;
	OS_ENTER_CRITICAL();
#if USER_EEPROM==1
	I2CReadByte(ReadAddr,&r,1);
#else
	r = AT24CXX_ReadOneByte(ReadAddr);
#endif
	OS_EXIT_CRITICAL(); 
	return r;
}

void ROM_WriteOneByte(u16 WriteAddr,u8 DataToWrite)
{
	OS_CPU_SR cpu_sr=0;
	OS_ENTER_CRITICAL();
#if USER_EEPROM==1
	I2CWriteByte(WriteAddr,&DataToWrite,1);
#else
	AT24CXX_WriteOneByte(WriteAddr,DataToWrite);
#endif
	OS_EXIT_CRITICAL();
}

u32 ROM_ReadCoin(void)
{
	u32 coin=0;
	u8 c0,c1,c2,c3;
	OS_CPU_SR cpu_sr=0;
	OS_ENTER_CRITICAL();
#if USER_EEPROM==1
	I2CReadByte(ADDRESS_C0,&c0,1);
	I2CReadByte(ADDRESS_C1,&c1,1);
	I2CReadByte(ADDRESS_C2,&c2,1);
	I2CReadByte(c2,&c3,1);
#else
	c0 = AT24CXX_ReadOneByte(ADDRESS_C0);//���
	c1 = AT24CXX_ReadOneByte(ADDRESS_C1);
	c2 = AT24CXX_ReadOneByte(ADDRESS_C2);
	c3 = AT24CXX_ReadOneByte(c2);
#endif
	OS_EXIT_CRITICAL();
	coin = (u32)c0<<24 & 0xff000000;
	coin |= (u32)c1<<16 & 0x00ff0000;
	coin |= (u32)c2<<8 & 0x0000ff00;
	coin |= c3&0x000000ff;
	return coin;
}


void ROM_WriteCoin(u32 coin)
{
	u8 c0,c1,c2,c3;
	OS_CPU_SR cpu_sr=0;
	c0 = coin>>24;
	c1 = coin>>16;
	c2 = coin>>8;
	c3 = coin;
	OS_ENTER_CRITICAL();
#if USER_EEPROM==1
	I2CWriteByte(ADDRESS_C0,&c0,1);
	I2CWriteByte(ADDRESS_C1,&c1,1);
	I2CWriteByte(ADDRESS_C2,&c2,1);
	I2CWriteByte(c2,&c3,1);
#else
	AT24CXX_WriteOneByte(ADDRESS_C0,c0);
	AT24CXX_WriteOneByte(ADDRESS_C1,c1);
	AT24CXX_WriteOneByte(ADDRESS_C2,c2);
	AT24CXX_WriteOneByte(c2,c3);
#endif
	OS_EXIT_CRITICAL();
}



bool ComExec(char *des,char *src)
{
	int k;
	char buf0[64],buf1[64];
	for(k=0;k<sizeof(COM_MSG_DEVEICE)/sizeof(COM_MSG_DEVEICE[0]);k++){//�Ƿ�Ϊ��Ч������
		if(strncmp(COM_MSG_DEVEICE[k].com,(const char*)src,strlen(COM_MSG_DEVEICE[k].com))==0){
//			if(COM_MSG_DEVEICE[k].cb){
//				COM_MSG_DEVEICE[k].cb(buf);
//			}
			switch(k){
				case 0://level
					sscanf(src,"level%d/r/n",&Level);
					break;
				case 1://SETIOTIME
					sscanf(src,"SETIOTIME=%d/r/n",&IOTime);
					if(IOTime<0)IOTime=0;
					if(IOTime>=sizeof(COIN_IO_TIME)/sizeof(COIN_IO_TIME[0]))IOTime=sizeof(COIN_IO_TIME)/sizeof(COIN_IO_TIME[0])-1;
					sprintf(des,"%d\r\nOK\r\n",IOTime);
					ROM_WriteLenByte(ADDRESS_IO,IOTime,1);//д��rom
					break;
				case 2://GETIOTIME
					sprintf(des,"%d\r\nOK\r\n",IOTime);
					break;
				case 3://WIFI
					sscanf(src,"WIFI=%s,%s/r/n",buf0,buf1);
					ESP8266_JoinAP (buf0, buf1);
					break;
				case 4://SETID
					sscanf(src,"SETID=%08s/r/n",Id);
					sprintf(des,"%s\r\nOK\r\n",Id);
					ROM_Write(ADDRESS_ID,Id,8);//д��rom
					break;
				case 5://CHANGEID
					sscanf(src,"CHANGEID=%08s/r/n",Id);
					sprintf(des,"%s\r\nOK\r\n",Id);
					ROM_Write(ADDRESS_ID,Id,8);//д��rom
					break;
				case 6://SETPASSWORD
					sscanf(src,"SETPASSWORD=%08s/r/n",Password);
					sprintf(des,"%s\r\nOK\r\n",Password);
					ROM_Write(ADDRESS_PASSWORD,Password,8);//д��rom
					break;
				case 7://GETID
					sprintf(des,"%s\r\nOK\r\n",Id);
					break;
				case 8://GETPASSWORD
					sprintf(des,"%s\r\nOK\r\n",Password);
					break;
				case 9://GETCOIN
					AllCoin = ROM_ReadCoin();
					sprintf(des,"%d\r\nOK\r\n",AllCoin);
					break;
				case 10://CWLAP
					ESP8266_Cmd ( "AT+CWLAP", "OK", 0, DELAY_CNT_1, true );
					des[0]='\0';
					break;
				case 11://SETSTARTTIME
					sscanf(src,"SETSTARTTIME=%d/r/n",&StartTime);
					if(StartTime<START_TIME_MIN)StartTime=START_TIME_MIN;
					if(StartTime>START_TIME_MAX)StartTime=START_TIME_MAX;
					sprintf(des,"%d\r\nOK\r\n",StartTime);
					ROM_WriteLenByte(ADDRESS_START_TIME,StartTime,1);//д��rom
					break;
				case 12://GETSTARTTIME
					//StartTime = ROM_ReadLenByte(ADDRESS_START_TIME, 1);
					sprintf(des,"%d\r\nOK\r\n",StartTime);
					break;
				case 13://SETCOIN,������
					sscanf(src,"SETCOIN=%d/r/n",&AllCoin);
					ROM_WriteCoin(AllCoin);
					C0 = AllCoin>>24;
					C1 = AllCoin>>16;
					C2 = AllCoin>>8;
					C3 = AllCoin;
					sprintf(des,"%d\r\nOK\r\n",AllCoin);
					break;
			}
			return true;
		}
	}
	strcpy((char*)des,"ERROR\r\n");
	return false;
}

/*******************************************************************************
* Function Name  : TaskStart.
* Descriptioan   : ��ʼ����.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
#define ROM_F	0xaa
void TaskStart(void * pdata)
{
	int i,j,l;
	u8 temp,linkState=0;
	//u8 SendBuf[10];
	u8 buf[64];
	
	OS_CPU_SR cpu_sr=0;
	
	pdata = pdata; 
	
	NVIC_Configuration();
	delay_init();	
	
	Usart_Init(COM_WIFI,115200); 
	
	LED_Init();
	COIN_Init();
#if USER_EEPROM==0
	AT24CXX_Init();
#else
	IIC_Init();
#endif
	UserIO_Init();

	temp = ROM_ReadOneByte(EE_TYPE);
	if(temp!=ROM_F){//��һ�λ���rom��
		ROM_WriteOneByte(EE_TYPE, ROM_F);
		temp = ROM_ReadOneByte(EE_TYPE);
		if(temp==ROM_F){//��һ��
			ROM_Write(ADDRESS_ID, Id, 8);
			ROM_Write(ADDRESS_PASSWORD, Password, 8);
			ROM_WriteLenByte(ADDRESS_IO,IOTime, 1);
			ROM_WriteLenByte(ADDRESS_START_TIME,StartTime, 1);
			ROM_WriteCoin(AllCoin);
		}else{//��
			#if MY_DEBUG==1
			Usart_SetData(COM_USER,(u8*)"EEPROM ERROR!\r\n",strlen("EEPROM ERROR!\r\n"));
			#endif
		}
	}else{
		ROM_Read(ADDRESS_ID, Id, 8);
		ROM_Read(ADDRESS_PASSWORD, Password, 8);
		IOTime = ROM_ReadLenByte(ADDRESS_IO, 1);
		if(IOTime<0)IOTime=0;
		if(IOTime>=sizeof(COIN_IO_TIME)/sizeof(COIN_IO_TIME[0]))IOTime=sizeof(COIN_IO_TIME)/sizeof(COIN_IO_TIME[0])-1;
		StartTime = ROM_ReadLenByte(ADDRESS_START_TIME, 1);
		if(StartTime<START_TIME_MIN)StartTime=START_TIME_MIN;
		if(StartTime>START_TIME_MAX)StartTime=START_TIME_MAX;
		AllCoin = ROM_ReadCoin();
	}
	time=COIN_IO_TIME[IOTime];//ioʱ��
	C0 = AllCoin>>24;
	C1 = AllCoin>>16;
	C2 = AllCoin>>8;
	C3 = AllCoin;
	
	des_setkey(DES_KEY);
	
	OSStatInit();
	OS_ENTER_CRITICAL(); 
	CREATE_SMSG(QCoin,10);
//	msg_page          = OSMboxCreate((void*)0                );	 //������Ϣ����
//	OSTaskCreate( TaskOne, (void * )0, (OS_STK *)&TASK_ONE_STK[ONE_STK_SIZE-1],  ONE_TASK_Prio);
//	OSTaskCreate( TaskTwo, (void * )0, (OS_STK *)&TASK_TWO_STK[TWO_STK_SIZE-1],  TWO_TASK_Prio);
	CREATE_TASK(TaskOne,STK_SIZE,TASK_Prio+1);
	CREATE_TASK(TaskTwo,STK_SIZE*2,TASK_Prio+2);
//	CREATE_TASK(TaskThree,STK_SIZE*5,TASK_Prio+3);
 	//OSTaskSuspend(START_TASK_Prio);	//suspend but not delete
	OS_EXIT_CRITICAL();
	
//	ESP8266_init();
//	ESP8266_AT_Test ();
	delay_ms ( 2000 ); 
	while ( !ESP8266_Cmd ( "AT", "OK", NULL, DELAY_CNT_0, true) ) {
		
		ESP8266_Rst ();
	}
	if(!ESP8266_Net_Mode_Choose ( STA )){
		UserIO_Send((u8*)"ESP8266 ERROR",strlen("ESP8266 ERROR"));
		UserIO_Send((u8*)"\r\n",2);
	}
	
	//����ǰ10s�����û�����
	
	while(1){
		UserIO_Send((u8*)"WAIT INPUT\r\n",strlen("WAIT INPUT\r\n"));
		for(j=0;j<StartTime;j++){
			for(i=0;i<50;i++){
				l = UserIO_Receive(buf,sizeof(buf)-1);
				buf[l]='\0';
				if(l>0){
					j=0;
					ComExec((char*)buf,(char*)buf);
					UserIO_Send(buf,strlen((const char*)buf));
				}
				OSTimeDly(1);
			}
			//�������ʱ
//			Usart_SetData(COM_USER,(u8*)"count down ",strlen("count down "));
//			Usart_SetData(COM_USER,(u8*)COUNT_DOWN[j],1);
//			Usart_SetData(COM_USER,(u8*)"\r\n",2);
		}
		UserIO_Send((u8*)"EXIT INPUT\r\n",strlen("EXIT INPUT\r\n"));
		if(Level>0)break;//�ȼ�����0������wifi���û���ͨģʽ
		if(ESP8266_Cmd ( "AT+CWJAP?", "OK", 0, DELAY_CNT_1, true )){
			linkState=1;//������wifi
			if(ESP8266_Link_Server(enumTCP,"192.168.1.103","9130",Single_ID)){
				linkState=2;//�����Ϸ�����
				ESP8266_UnvarnishSend();//͸��ģʽ
				break;//��ʼ�����
			}
		}
	}
	
	
	while(1)
	{	
		if(Level>0){
			//����2���յ�������ͨ������1���ͣ�����1���յ�������ͨ������2����
			l = Usart_GetData(COM_WIFI,buf,128);
			if(l>0){
				OSTimeDly(1);
				l += Usart_GetData(COM_WIFI,buf+l,128-l);
			}
			Usart_SetData(COM_USER,buf,l);
			OSTimeDly(1);
			
			
			l = Usart_GetData(COM_USER,buf,128);
			if(l>0){
				OSTimeDly(1);
				l += Usart_GetData(COM_USER,buf+l,128-l);
			}
			Usart_SetData(COM_WIFI,buf,l);
		}else{
			u8 * p = (u8*)ESP8266_ReceiveString(ENABLE);
			if(p){//������
				#if MY_DEBUG==1
				//Usart_SetData(COM_USER,p,p[1]);
				//Usart_SetData(COM_USER,(u8*)"\r\n",2);
				#endif
				if(p[0]==0xD5){//�յ�����ͷ
					u16 check0=0,check1=0;
					des_run((char*)p+COM_CHECK_OFF,(char*)p+COM_CHECK_OFF,decrypt);//����
					#if MY_DEBUG==1
					//Usart_SetData(COM_USER,(u8*)"decrypt\r\n",strlen("decrypt\r\n"));
					//Usart_SetData(COM_USER,p,COM_LOGIN_LEN);
					//Usart_SetData(COM_USER,(u8*)"\r\n",2);
					#endif
					for(i=COM_ORDER_OFF;i<p[1];i++){//У����ȷ
						check0 += p[i];
					}
					//��λ��ǰ
					check1=p[COM_CHECK_OFF] | p[COM_CHECK_OFF+1]<<8;
					if(check0==check1){//Ч����ȷ
						#if MY_DEBUG==1
						//Usart_SetData(COM_USER,(u8*)"check0=check1",strlen("check0=check1"));
						//Usart_SetData(COM_USER,(u8*)"\r\n",2);
						#endif
						if(p[COM_COM_OFF]==COM_LOGIN){//����
							#if MY_DEBUG==1
							//Usart_SetData(COM_USER,(u8*)"login",strlen("login"));
							//Usart_SetData(COM_USER,(u8*)"\r\n",2);
							#endif
							//���͵�����Ϣ
							p[COM_COM_OFF]++;//����ż�1
							p[1]=COM_LOGIN_LEN;//����
							memcpy(&p[10],Id,8);//���id
							memcpy(&p[18],Password,8);//�������
							memcpy(&p[26],&AllCoin,4);//��ʷ������
							memset(p+30,0,4);//���ĩβ
							check0 = 0;
							for(i=COM_ORDER_OFF;i<COM_LOGIN_LEN;i++){//����У���
								check0+=p[i];
							}
							p[COM_CHECK_OFF] = check0;
							p[COM_CHECK_OFF+1] = check0>>8;
							
							#if MY_DEBUG==1
							//Usart_SetData(COM_USER,(u8*)p,COM_LOGIN_LEN);
							//Usart_SetData(COM_USER,(u8*)"\r\n",2);
							#endif
							des_run((char*)p+COM_CHECK_OFF,(char*)p+COM_CHECK_OFF,encrypt);//����
							des_run((char*)p+COM_CHECK_OFF+8,(char*)p+COM_CHECK_OFF+8,encrypt);//����
							des_run((char*)p+COM_CHECK_OFF+16,(char*)p+COM_CHECK_OFF+16,encrypt);//����
							des_run((char*)p+COM_CHECK_OFF+24,(char*)p+COM_CHECK_OFF+24,encrypt);//����
							Usart_SetData(COM_WIFI,p,COM_LOGIN_LEN);//��������
						}else if(p[COM_COM_OFF]==COM_COIN){//����
							u32 coin;
							#if MY_DEBUG==1
							//Usart_SetData(COM_USER,(u8*)"beat",strlen("beat"));
							//Usart_SetData(COM_USER,(u8*)"\r\n",2);
							#endif
							//��ȡͶ����
							coin=p[COM_DATA_OFF] | p[COM_DATA_OFF+1]<<8;//��λ��ǰ
							//���ͷ�������
							p[COM_COM_OFF]++;//����ż�1
							p[1]=COM_COIN_LEN;//����
							memcpy(p+COM_DATA_OFF,&AllCoin,4);//����ܱ���
							check0 = 0;
							for(i=COM_ORDER_OFF;i<COM_COIN_LEN;i++){//����У���
								check0+=p[i];
							}
							p[COM_CHECK_OFF] = check0;
							p[COM_CHECK_OFF+1] = check0>>8;
							
							#if MY_DEBUG==1
							//Usart_SetData(COM_USER,(u8*)p,COM_COIN_LEN);
							//Usart_SetData(COM_USER,(u8*)"\r\n",2);
							#endif
							des_run((char*)p+COM_CHECK_OFF,(char*)p+COM_CHECK_OFF,encrypt);//����
							Usart_SetData(COM_WIFI,p,COM_COIN_LEN);//��������
							
							if(coin>0 && coin<1000)OSQPost(QCoin,(void*)coin);
						}
					}
				}
			}
		}
		OSTimeDly(2);
	}
}

/*******************************************************************************
* Function Name  : TaskOne.
* Descriptioan   : ����0.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void TaskOne(void *pdata)
{	 
  pdata = pdata; 
	
	while(1)
	{	
		LED0 = !LED0;
		OSTimeDlyHMSM(0,0,0,800);	
	}
}

/*******************************************************************************
* Function Name  : TaskTwo.
* Descriptioan   : ����1.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void TaskTwo(void *pdata)
{ 
	OS_CPU_SR cpu_sr=0;
	u8 err;
	pdata = pdata; 
	while(1)
	{ 
		u32 coin = (u32)OSQPend(QCoin,0,&err);
		while(coin--){
			COIN0 = !COIN0;
			COIN1 = !COIN1;
			
			//OS_ENTER_CRITICAL(); 
			AllCoin++;//�ܱ�����һ
			if(++C3==0){
				if(++C2==0){
					if(++C1==0){
						++C0;
						
						#if USER_EEPROM==1
							I2CWriteByte(ADDRESS_C0,&C0,1);
						#else
							AT24CXX_WriteOneByte(ADDRESS_C0,C0);
						#endif
					}
					#if USER_EEPROM==1
						I2CWriteByte(ADDRESS_C1,&C1,1);
					#else
						AT24CXX_WriteOneByte(ADDRESS_C1,C1);
					#endif
				}
				#if USER_EEPROM==1
					I2CWriteByte(ADDRESS_C2,&C2,1);
				#else
					AT24CXX_WriteOneByte(ADDRESS_C2,C2);
				#endif
			}
			#if USER_EEPROM==1
				I2CWriteByte(C2,&C3,1);
			#else
				AT24CXX_WriteOneByte(C2,C3);
			#endif
			//�����ܱ���
			//�����Ϣ
//			UserIO_Send((u8*)"C\r\n",strlen("C\r\n"));
			//OS_EXIT_CRITICAL();
			
			OSTimeDlyHMSM(0,0,0,time);
			COIN0 = !COIN0;
			COIN1 = !COIN1;
			OSTimeDlyHMSM(0,0,0,time);
		}
	}
}

/*******************************************************************************
* Function Name  : TaskTwo.
* Descriptioan   : �����յ������ݽ���Ϊ��\r\nΪ��β������.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void TaskThree(void *pdata)
{
//	int i,j,lUser=0;
//	u8 bufUser[64];
//	u8 err;
	pdata = pdata;
	while(1)
	{
		
		//�û���Ϣ
//		lUser+=Usart_GetData(COM_USER,bufUser,sizeof(bufUser));
//		if(lUser>=strlen(COM_END)){
//			j=0;
//			for(i=0;i<lUser-strlen(COM_END)+1;i++){//������Ϣ
//				if(strncmp((char*)bufUser+i,COM_END,strlen(COM_END))==0){//�յ�һ����������Ϣ
//					u8* p = OSMemGet(Mem,&err);
//					memcpy(p,bufUser+j,i+strlen(COM_END)-j);
//					p[i+strlen(COM_END)-j]='\0';
//					OSQPost(ComQUser,p);//�����յ�����Ϣ
//					j=i+strlen(COM_END);//��¼��һ����Ϣ��ʼ��λ��
//				}
//			}
//			//ǰ������
//			if(j>0){
//				lUser-=j;
//				memcpy(bufUser,bufUser+j,lUser);
//			}
//		}
		
		OSTimeDly(3);	
	}
}

