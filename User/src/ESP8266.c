#include "ESP8266.h"
#include "userIO.h"

/*
 * ��������ESP8266_Cmd
 * ����  ����WF-ESP8266ģ�鷢��ATָ��
 * ����  ��cmd�������͵�ָ��
 *         reply1��reply2���ڴ�����Ӧ��ΪNULL��������Ӧ������Ϊ���߼���ϵ
 *         waittime���ȴ���Ӧ��ʱ��
 * ����  : 1��ָ��ͳɹ�
 *         0��ָ���ʧ��
 * ����  �����ⲿ����
 */
u8 buf[USART_BUF_SIZE+1];
bool ESP8266_Cmd ( char * cmd, char * reply1, char * reply2, u32 waittime, bool log)
{    
	int l,cnt=waittime;
	Usart_SetData(COM_WIFI,(u8*)cmd,strlen(cmd));
	Usart_SetData(COM_WIFI,(u8*)"\r\n",2);

	if ( ( reply1 == 0 ) && ( reply2 == 0 ) )                      //����Ҫ��������
		return true;
	
	delay_ms ( DELAY_TIME );                 //��ʱ
	
	l = Usart_GetData(COM_WIFI,buf,sizeof(buf)-1);
	buf[l]=0;
  
	do{
		if ( ( reply1 != 0 ) && ( reply2 != 0 ) ){
			if( ( bool ) strstr ( (const char*)buf, reply1 ) || ( bool ) strstr ( (const char*)buf, reply2 ) ){
				if(log)UserIO_Send(buf,l);//���͵����Զ�
				return true;
			}
		}else if ( reply1 != 0 ){
			if ( ( bool ) strstr ( (const char*)buf, reply1 ) ){
				if(log)UserIO_Send(buf,l);//���͵����Զ�
				return true;
			}
		
		}else{
			if( ( bool ) strstr ( (const char*)buf, reply2 ) ){
				if(log)UserIO_Send(buf,l);//���͵����Զ�
				return true;
			}
		}
		delay_ms ( DELAY_TIME );                 //��ʱ
		l += Usart_GetData(COM_WIFI,buf+l,sizeof(buf)-l-1);
		buf[l]=0;
	}while(cnt--);
	if(log)UserIO_Send(buf,l);//���͵����Զ�
	return false;
}

/*
 * ��������ESP8266_Rst
 * ����  ������WF-ESP8266ģ��
 * ����  ����
 * ����  : ��
 * ����  ����ESP8266_AT_Test����
 */
void ESP8266_Rst ( void )
{
	ESP8266_Cmd ( "AT+RST", "OK", "ready", DELAY_CNT_1, true);   	
}

/*
 * ��������ESP8266_AT_Test
 * ����  ����WF-ESP8266ģ�����AT��������
 * ����  ����
 * ����  : ��
 * ����  �����ⲿ����
 */
void ESP8266_AT_Test ( void )
{
	delay_ms ( 1500 ); 
	while ( ! ESP8266_Cmd ( "AT", "OK", NULL, DELAY_CNT_0, true) ) ESP8266_Rst ();  	
}

/*
 * ��������ESP8266_Net_Mode_Choose
 * ����  ��ѡ��WF-ESP8266ģ��Ĺ���ģʽ
 * ����  ��enumMode������ģʽ
 * ����  : 1��ѡ��ɹ�
 *         0��ѡ��ʧ��
 * ����  �����ⲿ����
 */
bool ESP8266_Net_Mode_Choose ( ENUM_Net_ModeTypeDef enumMode )
{
	switch ( enumMode )
	{
		case STA:
			return ESP8266_Cmd ( "AT+CWMODE=1", "OK", "no change", DELAY_CNT_1, true); 
		
		case AP:
			return ESP8266_Cmd ( "AT+CWMODE=2", "OK", "no change", DELAY_CNT_1, true); 
		
		case STA_AP:
			return ESP8266_Cmd ( "AT+CWMODE=3", "OK", "no change", DELAY_CNT_1, true); 
		
		default:
		  return false;
	}
}

/*
 * ��������ESP8266_JoinAP
 * ����  ��WF-ESP8266ģ�������ⲿWiFi
 * ����  ��pSSID��WiFi�����ַ���
 *       ��pPassWord��WiFi�����ַ���
 * ����  : 1�����ӳɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
bool ESP8266_JoinAP ( char * pSSID, char * pPassWord )
{
	char cCmd [120];

	sprintf ( cCmd, "AT+CWJAP=\"%s\",\"%s\"", pSSID, pPassWord );
	
	return ESP8266_Cmd ( cCmd, "OK", NULL, DELAY_CNT_1, true );
	
}

/*
 * ��������ESP8266_BuildAP
 * ����  ��WF-ESP8266ģ�鴴��WiFi�ȵ�
 * ����  ��pSSID��WiFi�����ַ���
 *       ��pPassWord��WiFi�����ַ���
 *       ��enunPsdMode��WiFi���ܷ�ʽ�����ַ���
 * ����  : 1�������ɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
bool ESP8266_BuildAP ( char * pSSID, char * pPassWord, char * enunPsdMode )
{
	char cCmd [120];

	sprintf ( cCmd, "AT+CWSAP=\"%s\",\"%s\",1,%s", pSSID, pPassWord, enunPsdMode );
	
	return ESP8266_Cmd ( cCmd, "OK", 0, DELAY_CNT_1, true );
	
}


/*
 * ��������ESP8266_Enable_MultipleId
 * ����  ��WF-ESP8266ģ������������
 * ����  ��enumEnUnvarnishTx�������Ƿ������
 * ����  : 1�����óɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
bool ESP8266_Enable_MultipleId ( FunctionalState enumEnUnvarnishTx )
{
	char cStr [20];
	
	sprintf ( cStr, "AT+CIPMUX=%d", ( enumEnUnvarnishTx ? 1 : 0 ) );
	
	return ESP8266_Cmd ( cStr, "OK", 0, DELAY_CNT_1, true );
	
}


/*
 * ��������ESP8266_Link_Server
 * ����  ��WF-ESP8266ģ�������ⲿ������
 * ����  ��enumE������Э��
 *       ��ip��������IP�ַ���
 *       ��ComNum���������˿��ַ���
 *       ��id��ģ�����ӷ�������ID
 * ����  : 1�����ӳɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
bool ESP8266_Link_Server ( ENUM_NetPro_TypeDef enumE, char * ip, char * ComNum, ENUM_ID_NO_TypeDef id)
{
	char cStr [100] = { 0 }, cCmd [120];

  switch (  enumE )
  {
		case enumTCP:
		  sprintf ( cStr, "\"%s\",\"%s\",%s", "TCP", ip, ComNum );
		  break;
		
		case enumUDP:
		  sprintf ( cStr, "\"%s\",\"%s\",%s", "UDP", ip, ComNum );
		  break;
		
		default:
			break;
  }

  if ( id < 5 )//������
    sprintf ( cCmd, "AT+CIPSTART=%d,%s", id, cStr);    ///AT+CIPSTART="TCP","ip","80"

  else
	  sprintf ( cCmd, "AT+CIPSTART=%s", cStr );//������

	return ESP8266_Cmd ( cCmd, "OK", "ALREAY CONNECT", DELAY_CNT_1, true );
	
}

/*
 * ��������ESP8266_StartOrShutServer
 * ����  ��WF-ESP8266ģ�鿪����رշ�����ģʽ
 * ����  ��enumMode������/�ر�
 *       ��pPortNum���������˿ں��ַ���
 *       ��pTimeOver����������ʱʱ���ַ�������λ����
 * ����  : 1�������ɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
bool ESP8266_StartOrShutServer ( FunctionalState enumMode, char * pPortNum, char * pTimeOver )
{
	char cCmd1 [120], cCmd2 [120];

	if ( enumMode )
	{
		sprintf ( cCmd1, "AT+CIPSERVER=%d,%s", 1, pPortNum );
		
		sprintf ( cCmd2, "AT+CIPSTO=%s", pTimeOver );

		return ( ESP8266_Cmd ( cCmd1, "OK", 0, DELAY_CNT_1, true ) &&
						 ESP8266_Cmd ( cCmd2, "OK", 0, DELAY_CNT_1, true ) );
	}
	
	else
	{
		sprintf ( cCmd1, "AT+CIPSERVER=%d,%s", 0, pPortNum );

		return ESP8266_Cmd ( cCmd1, "OK", 0, DELAY_CNT_1, true );
	}
	
}


/*
 * ��������ESP8266_UnvarnishSend
 * ����  ������WF-ESP8266ģ�����͸������
 * ����  ����
 * ����  : 1�����óɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
bool ESP8266_UnvarnishSend ( void )
{
	return (
	  ESP8266_Cmd ( "AT+CIPMODE=1", "OK", 0, DELAY_CNT_1, true ) &&
	  ESP8266_Cmd ( "AT+CIPSEND", "\r\n", ">", DELAY_CNT_1, true ) );	
}

/*
 * ��������ESP8266_SendString
 * ����  ��WF-ESP8266ģ�鷢���ַ���
 * ����  ��enumEnUnvarnishTx�������Ƿ���ʹ����͸��ģʽ
 *       ��pStr��Ҫ���͵��ַ���
 *       ��ulStrLength��Ҫ���͵��ַ������ֽ���
 *       ��ucId���ĸ�ID���͵��ַ���
 * ����  : 1�����ͳɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
bool ESP8266_SendString ( FunctionalState enumEnUnvarnishTx, char * pStr, u32 ulStrLength, ENUM_ID_NO_TypeDef ucId )
{
	char cStr [20];
	bool bRet = false;
		
	if ( enumEnUnvarnishTx ){
		Usart_SetData(COM_WIFI,(u8*)pStr,ulStrLength);
	}else{
		if ( ucId < 5 )
			sprintf ( cStr, "AT+CIPSEND=%d,%d", ucId, ulStrLength + 2 );
		else
			sprintf ( cStr, "AT+CIPSEND=%d", ulStrLength + 2 );
		
		ESP8266_Cmd ( cStr, "> ", 0, DELAY_CNT_1, true );

		bRet = ESP8266_Cmd ( pStr, "SEND OK", 0, DELAY_CNT_1, true );
	}
	
	return bRet;

}

/*
 * ��������ESP8266_UnvarnishSend
 * ����  ������WF-ESP8266ģ���˳�͸������
 * ����  ����
 * ����  : 1�����óɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
bool ESP8266_ExitUnvarnishSend ( void )
{
	return ESP8266_SendString(ENABLE,"++",2,Single_ID);
	
}


/*
 * ��������ESP8266_ReceiveString
 * ����  ��WF-ESP8266ģ������ַ���
 * ����  ��enumEnUnvarnishTx�������Ƿ���ʹ����͸��ģʽ
 * ����  : ���յ����ַ����׵�ַ
 * ����  �����ⲿ����
 */
u8 BufRec[USART_BUF_SIZE+1];
char * ESP8266_ReceiveString ( FunctionalState enumEnUnvarnishTx )
{
	char * pRecStr = 0;
	int l=0;
	l = Usart_GetData(COM_WIFI,BufRec,sizeof(BufRec)-1);
	if(l>0){
		delay_ms(10);
		l+= Usart_GetData(COM_WIFI,BufRec+l,sizeof(BufRec)-1-l);
		BufRec[l]='\0';
		if ( enumEnUnvarnishTx )
		{
	//		if ( strstr ( (const char*)BufRec, ">" ) )
				pRecStr = (char*)BufRec;

		}
		
		else 
		{
			if ( strstr ( (const char*)BufRec, "+IPD" ) )
				pRecStr = (char*)BufRec;

		}
	}
	
	
	

	return pRecStr;
}


/*
 * ��������ESP8266_WebFetch_Test
 * ����  ��WF-ESP8266ģ�����ץȡ��ҳ����
 * ����  ����
 * ����  : ��
 * ����  �����ⲿ����
 */
void ESP8266_init( void )
{
//	char cStrInput [100] = { 0 }, * pStrDelimiter [2], * pBuf, * pStr;
//	u8 uc = 0;

	//ESP8266_Choose ( ENABLE );	


	ESP8266_AT_Test ();
	

	ESP8266_Net_Mode_Choose ( STA );
	
	
	//ESP8266_Cmd ( "AT+CWLAP", "OK", 0, 10000, true );


//  do
//	{
//		PC_Usart ( "\r\n�����������ӵ�Internet��WiFi���ƺ���Կ�������ʽΪ�������ַ�+Ӣ�Ķ���+��Կ�ַ�+�ո񣬵������\r\n" );

//		scanf ( "%s", cStrInput );

//		PC_Usart ( "\r\n�Ե�Ƭ�� ����\r\n" );

//		pBuf = cStrInput;
//		uc = 0;
//		while ( ( pStr = strtok ( pBuf, "," ) ) != NULL )
//		{
//			pStrDelimiter [ uc ++ ] = pStr;
//			pBuf = NULL;
//		} 
//		
//  } while ( ! ESP8266_JoinAP ( pStrDelimiter [0], pStrDelimiter [1] ) );
//	ESP8266_Cmd ( "AT+CIPMUX=0", "OK", 0, 500 );            //������ 
//  do
//	{
//		PC_Usart ( "\r\n���ӵ�WiFi\r\n" );

//  } while ( ! ESP8266_JoinAP ( "LieBao", "12345678") );

}
/*****************************************************************************************************/
/*�ӷ�������������*/
//void Web_To_ESP(void)
//{ 	
//	u16 i;
//	bool state;
////	ESP8266_Cmd ( "AT", "OK", NULL, 200 );
//	ESP8266_Cmd ( "AT+CIPMUX=0", "OK", 0, 500 );            //������ 
//	Delay_ms(1000);
//	do
//	{
//		strEsp8266_Fram_Record .InfBit .FramLength = 0;               //���¿�ʼ�����µ����ݰ�
//		for(i=0;i<2048;i++)
//			strEsp8266_Fram_Record .Data_RX_BUF[i]=0;                //�������	
//		ESP8266_Usart ( "%s\r\n", "AT+CIPSTART=\"TCP\",\"192.168.191.2\",8080" );	
//		Delay_ms ( 1000);                 //��ʱ	
//		strEsp8266_Fram_Record .Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength ]  = '\0';
//		PC_Usart ( "%s", strEsp8266_Fram_Record .Data_RX_BUF );
//      state= (( bool ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "OK" )|( bool ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "ALREAY CONNECT" ));
//		if(( bool ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "ERROR" ))
//			state=false; 		
//	}
//	while(state==false);
//    ESP8266_Cmd ( "AT+CIPMODE=1", "OK", 0, 500 );            //0,��͸����1��͸��
//	ESP8266_Cmd ( "AT+CIPSEND", "\r\n", ">", 500 );
//	ESP8266_SendString ( ENABLE, "GET /LibrarySeatServer/seatInfo?room=1 HTTP/1.1\r\n",     NULL, Single_ID );
//	ESP8266_SendString ( ENABLE, "Host: 192.168.191.2:8080\r\n",                            NULL, Single_ID );
//	ESP8266_SendString ( ENABLE, "User-Agent: abc\r\n",                                     NULL, Single_ID );
//	ESP8266_SendString ( ENABLE, "Connection: close\r\n",                                   NULL, Single_ID );
//	ESP8266_SendString ( ENABLE, "\r\n",                                                    NULL, Single_ID );
//	strEsp8266_Fram_Record .InfBit .FramLength = 0;
//	Delay_ms(1200);
//	strEsp8266_Fram_Record .Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength ] = '\0';
//	PC_Usart ("%s",strEsp8266_Fram_Record .Data_RX_BUF);
//}	
///***************************************************************************************/
////��½�����ߺ�������һ��ˢ��½���ڶ���ˢ����
////������usr_Id             ��1,2,3,4,5,.......
//void login(u8 usr_Id)
//{
//	char cStr[100]={0};
//	u16 i;
//	bool state;
//	switch(usr_Id)
//	{
//		case 1:
//			sprintf ( cStr, "GET /LibrarySeatServer/usrInfo?%s HTTP/1.1\r\n", "usr=june&password=19920211&req_type=SWIPE" );//�û�june
//			break;
//		case 2:
//			sprintf ( cStr, "GET /LibrarySeatServer/usrInfo?%s HTTP/1.1\r\n", "usr=solar&password=19920211&req_type=SWIPE" );//�û�solar
//			break;
//		default:
//			break;
//	}
//	
//	ESP8266_Cmd ( "AT+CIPMUX=0", "OK", 0, 500 );//������
//	Delay_ms(500);
//	do
//	{
//		strEsp8266_Fram_Record .InfBit .FramLength = 0;               //���¿�ʼ�����µ����ݰ�
//		for(i=0;i<1024;i++)
//			strEsp8266_Fram_Record .Data_RX_BUF[i]=0;                //�������
//		ESP8266_Usart ( "%s\r\n", "AT+CIPSTART=\"TCP\",\"192.168.191.3\",8080");		
//		Delay_ms ( 1000);                 //��ʱ	
//		strEsp8266_Fram_Record .Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength ]  = '\0';
//		PC_Usart ( "%s", strEsp8266_Fram_Record .Data_RX_BUF );
//      state= ( bool ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "OK" )||( bool ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "ALREAY CONNECT" );
//		if(( bool ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "ERROR" ))
//			state=false; 
//	
//	}
//	while(state==false);
//    ESP8266_Cmd ( "AT+CIPMODE=1", "OK", 0, 500 );            //0,��͸����1��͸��
//	ESP8266_Cmd ( "AT+CIPSEND", "\r\n", ">", 500 );
//	ESP8266_SendString ( ENABLE, cStr,                                NULL, Single_ID );//������
//	ESP8266_SendString ( ENABLE, "Host: 192.168.191.3:8080\r\n",      NULL, Single_ID );//����Ϊ����ͷ
//	ESP8266_SendString ( ENABLE, "User-Agent: abc\r\n",               NULL, Single_ID );
//	ESP8266_SendString ( ENABLE, "Content-Length: 38\r\n",            NULL, Single_ID );
//	ESP8266_SendString ( ENABLE, "Connection: close\r\n",             NULL, Single_ID );
//	ESP8266_SendString ( ENABLE, "\r\n",                              NULL, Single_ID );
//	strEsp8266_Fram_Record .InfBit .FramLength = 0;
//	Delay_ms(3000);
//	strEsp8266_Fram_Record .Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength ] = '\0';
//	PC_Usart ("%s",strEsp8266_Fram_Record .Data_RX_BUF);
//	PC_Usart ("\r\n");
//	if((strstr(strEsp8266_Fram_Record .Data_RX_BUF,"OK"))!=NULL)
//		PC_Usart ("log in successfully\r\n");
//}	

///*************************************************************************************************************/
////reqType���������ͣ�1ֱ������ĳ��������λ��Ϣ��2����ռ������
////desk������ID
////room����ID 
////seat����λID
////actionType���������ͣ�ֱ�ӵ���occupy��
////usr���û�ID
//void occupy(char desk_ID,char room_ID,char seat_ID,char usr_ID)
//{
//	char cStr[100]={0};
//	u16 i;
//	bool state;
//	sprintf ( cStr, "GET /LibraryDeskServer/deskInfo?reqType=2&desk=%d&room=%d&seat=%d&actionType=occupy&usr=%d HTTP/1.1\r\n",desk_ID,room_ID,seat_ID,usr_ID);			
//	ESP8266_Cmd ( "AT+CIPMUX=0", "OK", 0, 500 );//������
//	Delay_ms(500);
//	do
//	{
//		strEsp8266_Fram_Record .InfBit .FramLength = 0;               //���¿�ʼ�����µ����ݰ�
//		for(i=0;i<1024;i++)
//			strEsp8266_Fram_Record .Data_RX_BUF[i]=0;                //�������
//		sprintf(array,"AT+CIPSTART=\"TCP\",\"%s\",8080",IP);
//		ESP8266_Usart ( "%s\r\n", array );		
//		Delay_ms ( 1000);                 //��ʱ	
//		strEsp8266_Fram_Record .Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength ]  = '\0';
//		PC_Usart ( "%s", strEsp8266_Fram_Record .Data_RX_BUF );
//      state= ( bool ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "OK" )||( bool ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "ALREAY CONNECT" );
//		if(( bool ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "ERROR" ))
//			state=false; 
//	
//	}
//	while(state==false);
//	sprintf(array,"Host: %s:8080\r\n",IP);
//    ESP8266_Cmd ( "AT+CIPMODE=1", "OK", 0, 500 );            //0,��͸����1��͸��
//	ESP8266_Cmd ( "AT+CIPSEND", "\r\n", ">", 500 );
//	ESP8266_SendString ( ENABLE, cStr,                                NULL, Single_ID );//������
//  ESP8266_SendString ( ENABLE, array,                               NULL, Single_ID );//����Ϊ����ͷ
//	ESP8266_SendString ( ENABLE, "User-Agent: abc\r\n",               NULL, Single_ID );
//	ESP8266_SendString ( ENABLE, "Content-Length: 38\r\n",            NULL, Single_ID );
//	ESP8266_SendString ( ENABLE, "Connection: close\r\n",             NULL, Single_ID );
//	ESP8266_SendString ( ENABLE, "\r\n",                              NULL, Single_ID );
//	strEsp8266_Fram_Record .InfBit .FramLength = 0;
//	Delay_ms(1500);
//	strEsp8266_Fram_Record .Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength ] = '\0';
//	if((strstr(strEsp8266_Fram_Record .Data_RX_BUF,"[{"))!=NULL)
//		PC_Usart ("occupy successfully\r\n");
//	else 
//		PC_Usart ("failed\r\n");


//}