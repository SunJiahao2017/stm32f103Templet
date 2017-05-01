

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SYS_H
#define __SYS_H	

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#include <math.h>
#include <stddef.h>
#include "config.h"

#if SYSTEM_SUPPORT_UCOS
	#include "includes.h"
	#define CREATE_TASK(_TASK,_STKSIZE,_PRIO,_PARA)	do{\
		static OS_STK _TASK##_Stk[_STKSIZE];			\
		OSTaskCreate(_TASK, (void *)_PARA, &_TASK##_Stk[_STKSIZE-1],_PRIO);\
	}while(0)
	#define	CREATE_SMSG(_OSQ,_NUM)	do{\
		static void* _OSQ##_MSG[_NUM];	\
		_OSQ =	OSQCreate(_OSQ##_MSG, _NUM); \
	}while(0)
	#define CREATE_MEM(_OSMEM,_NUM,_SIZE)	do{\
		INT8U err;\
		static INT8U _OSMEM##_MEM[_NUM][_SIZE];	\
		_OSMEM =	OSMemCreate((void*)_OSMEM##_MEM, _NUM, _SIZE, &err); \
	}while(0)
#else
	#include <stdio.h>
	#include <string.h>
	#include <ctype.h>
	#include <stdlib.h>
	#include <stdarg.h>
	#define OSIntEnter	__nop
	#define OSIntExit	__nop
	#define OS_ENTER_CRITICAL	__nop
	#define OS_EXIT_CRITICAL	__nop
	#define OS_CPU_SR	unsigned int
		
	typedef unsigned char  BOOLEAN;
	typedef unsigned char  INT8U;			/* Unsigned  8 bit quantity       */
	typedef signed   char  INT8S;			/* Signed    8 bit quantity       */
	typedef unsigned short INT16U;			/* Unsigned 16 bit quantity       */
	typedef signed   short INT16S;			/* Signed   16 bit quantity       */
	typedef unsigned int   INT32U;			/* Unsigned 32 bit quantity       */
	typedef signed   int   INT32S;			/* Signed   32 bit quantity       */
	typedef float          FP32;			/* Single precision floating point*/
	typedef double         FP64;			/* Double precision floating point*/
#endif

#if SYSTEM_SUPPORT_UCGUI
	#include "GUI.H"
#endif
	
typedef struct _MessageHead {
	unsigned short int message;
	unsigned short int para0;
	unsigned short int para1;
	unsigned short int len;
} MessageHead_sut;
	
typedef struct _Message {
	MessageHead_sut head;
	union {
        unsigned char data8[8];
		unsigned short int data16[4];
		unsigned int data32[2];
		void * pData;
    } data;
} Message_sut;



/* Exported types ------------------------------------------------------------*/
typedef enum {FALSE = 0, TRUE = !FALSE} BOOL;


/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define countof(Array) (sizeof(Array)/sizeof(Array[0]))																	    
	 
//λ������,ʵ��51���Ƶ�GPIO���ƹ���
//����ʵ��˼��,�ο�<<CM3Ȩ��ָ��>>������(87ҳ~92ҳ).
//IO�ڲ����궨��
#define BITBAND(addr, bitnum)    ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)           *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 
//IO�ڵ�ַӳ��
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 
 
//IO�ڲ���,ֻ�Ե�һ��IO��!
//ȷ��n��ֵС��16!
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //��� 
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //���� 

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //��� 
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //���� 

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //��� 
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //���� 

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  //��� 
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  //���� 

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  //��� 
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  //����

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  //��� 
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  //����

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  //��� 
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  //����



/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void     NVIC_SETPRIMASK   (void);
void     NVIC_RESETPRIMASK (void);
void     NVIC_SETBASEMASK  (uint32_t basePri);
uint32_t NVIC_GETBASEMASK  (void);
void     NVIC_Configuration(void);

void delay_init(void);
void delay_ms(u16 nms);
void delay_us(u32 nus);

void Usart_Init(int usart, u32 bound);
void Usart_BufInit(int usart);
u16 Usart_SetData(int usart, u8 *pData, u16 size);
u16 Usart_GetData(int usart, u8 *pData, u16 size);

void Debug_Printf(u16 level, const char *s,...);
u32 Sys_Init(u32 baudRate0,u32 baudRate1,u32 baudRate2);
BOOL Sys_Discryption(u8 *key);

#if SYSTEM_SUPPORT_UCOS==1
void Sys_Start(void);
void * MemGet(u16 size, u8 * perr);
u8 MemPut(void * pblk);
u8 send_Message(u8 task, u16 message, u16 para0, u16 para1, u8 * pData, u16 len);
Message_sut * receive_Message(u8 task);
u8 free_Message(Message_sut* pMessage);
#endif

/* External variables --------------------------------------------------------*/



#endif
