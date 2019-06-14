#include "AT_drive.h"

#define DELAY_TIME		20

extern u16 DebugLevel;

u16 AT_SendData(u8 *pData, u16 size,BOOL debug)
{
	if(debug && DebugLevel>=5){
		Usart_SetData(0,pData,size);
	}
	return Usart_SetData(1,pData,size);
}

u16 AT_ReceiveData(u8 *pData, u16 size,BOOL debug)
{
	u16 r=0;
	r = Usart_GetData(1,pData,size);
	if(debug && DebugLevel>=5){
		Usart_SetData(0,pData,r);
	}
	return r;
}

u16 AT_Log(u8 *pData, u16 size)
{
	return 0;//Usart_SetData(0,pData,size);
}

u8 my_strstr(const char *s0,const char *s1,u16 len)
{
	int i,j;
	j=strlen(s1);
	if(len<j)return 0;
	for(i=0;i<=len-j;i++){
		if(memcmp(s0+i,s1,j)==0)
			return 1;
	}
	return 0;
}

/*
 * ��������AT_Cmd
 * ����  ������ATָ��
 * ����  ��cmd�������͵�ָ��
 *         reply1��reply2���ڴ�����Ӧ��ΪNULL������Ӧ������Ϊ���߼���ϵ
 *         waittime���ȴ���Ӧ��ʱ��
 * ����  : 1��ָ��ͳɹ�
 *         0��ָ���ʧ��
 * ����  �����ⲿ����
 */

u8 AT_Cmd ( u8 *pBuf, u16 size, char * cmd, char * reply1, char * reply2, u16 waittime)
{    
	u16 l0=0,l1=0,cnt=waittime/DELAY_TIME+1;
	u8 r=0;
	//u8 buf[128+1];

	if(cmd){
		AT_SendData((u8*)cmd,strlen(cmd),FALSE);
		AT_SendData((u8*)"\r\n",2,FALSE);
	}

	if ( ( reply1 == 0 ) && ( reply2 == 0 ) )                      //����Ҫ��������
		return r;
	
	//delay_ms ( DELAY_TIME );                 //��ʱ
	
	while(cnt ||  l0>l1){
		cnt--;
		l1=l0;
		l0 += AT_ReceiveData(pBuf+l0,size-l0-1,TRUE);
		if(l0>l1){
			pBuf[l0]=0;
			if ( reply1 != 0 ){
				if (my_strstr ( (const char*)pBuf, reply1 ,l0) ){
					r = 1;
					break;
				}
			
			}
			
			if(reply2 != 0){
				if(my_strstr ( (const char*)pBuf, reply2 ,l0) ){
					r = 2;
					break;
				}
			}
		}
		
		delay_ms ( DELAY_TIME );                 //��ʱ
	}
	
	return r;
}

/*
 * ��������AT_Test
 * ����  ����AT��������
 * ����  ����
 * ����  : ��
 * ����  �����ⲿ����
 */
u8 AT_Test ( u16 time )
{
	//delay_ms ( 1500 ); 
	u8 buf[64];
	return AT_Cmd ( buf,64,"AT", "OK", NULL, time);  	
}

/*
 * ��������AT_Enable_MultipleId
 * ����  ��ģ������������
 * ����  ��enumEnUnvarnishTx�������Ƿ������
 * ����  : 1�����óɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
//u8 AT_Enable_MultipleId (u16 time, FunctionalState enumEnUnvarnishTx )
//{
//	char cStr [20];
//	sprintf ( cStr, "AT+CIPMUX=%d", ( enumEnUnvarnishTx ? 1 : 0 ) );
//	return AT_Cmd ( cStr, "OK", 0, time, TRUE );
//}

/*
 * ��������AT_Link_Server
 * ����  ��ģ�������ⲿ������
 * ����  ��enumE������Э��
 *       ��ip��������IP�ַ���
 *       ��ComNum���������˿��ַ���
 *       ��id��ģ�����ӷ�������ID
 * ����  : 1�����ӳɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
//u8 AT_Link_Server (u16 time, ENUM_NetPro_TypeDef enumE, char * ip, char * ComNum, ENUM_ID_NO_TypeDef id)
//{
//	char cStr [100] = { 0 }, cCmd [120];
//	switch (  enumE )
//	{
//		case enumTCP:
//		  sprintf ( cStr, "\"%s\",\"%s\",%s", "TCP", ip, ComNum );
//		  break;
//		case enumUDP:
//		  sprintf ( cStr, "\"%s\",\"%s\",%s", "UDP", ip, ComNum );
//		  break;
//		default:
//			break;
//	}

//	if ( id < 5 )//������
//		sprintf ( cCmd, "AT+CIPSTART=%d,%s", id, cStr);    ///AT+CIPSTART="TCP","ip","80"
//	else
//		sprintf ( cCmd, "AT+CIPSTART=%s", cStr );//������
//	return AT_Cmd ( cCmd, "OK", "ALREAY CONNECT", time, TRUE );
//}

/*
 * ��������AT_SendString
 * ����  ��ģ�鷢���ַ���
 * ����  ��enumEnUnvarnishTx�������Ƿ���ʹ����͸��ģʽ
 *       ��pStr��Ҫ���͵��ַ���
 *       ��ulStrLength��Ҫ���͵��ַ������ֽ���
 *       ��ucId���ĸ�ID���͵��ַ���
 * ����  : 1�����ͳɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
//u8 AT_SendString (u16 time, FunctionalState enumEnUnvarnishTx, char * pStr, u32 ulStrLength, ENUM_ID_NO_TypeDef ucId )
//{
//	char cStr [20];
//	u8 bRet = 0;
//		
//	if ( enumEnUnvarnishTx ){
//		AT_SendData((u8*)pStr,ulStrLength);
//	}else{
//		if ( ucId < 5 )
//			sprintf ( cStr, "AT+CIPSEND=%d,%d", ucId, ulStrLength + 2 );
//		else
//			sprintf ( cStr, "AT+CIPSEND=%d", ulStrLength + 2 );
//		
//		AT_Cmd ( cStr, "> ", 0, 250, TRUE );

//		bRet = AT_Cmd ( pStr, "OK", 0, time, TRUE );
//	}
//	return bRet;
//}


/*
 * ��������AT_ReceiveString
 * ����  ��WF-ESP8266ģ������ַ���
 * ����  ��enumEnUnvarnishTx�������Ƿ���ʹ����͸��ģʽ
 * ����  : ���յ����ַ����׵�ַ
 * ����  �����ⲿ����
 */
//u8 BufRec[USART_BUF_SIZE+1];
//char * AT_ReceiveString ( FunctionalState enumEnUnvarnishTx )
//{
//	char * pRecStr = 0;
//	u8 BufRec[USART_BUF_SIZE+1];
//	u16 l=0;
//	l = AT_ReceiveData(BufRec,sizeof(BufRec)-1);
//	if(l>0){
//		delay_ms(10);
//		l+= AT_ReceiveData(BufRec+l,sizeof(BufRec)-1-l);
//		BufRec[l]='\0';
//		if ( enumEnUnvarnishTx ){
//	//		if ( strstr ( (const char*)BufRec, ">" ) )
//				pRecStr = (char*)BufRec;
//		}else {
//			if ( strstr ( (const char*)BufRec, "+IPD" ) )
//				pRecStr = (char*)BufRec;
//		}
//	}
//	return pRecStr;
//}
