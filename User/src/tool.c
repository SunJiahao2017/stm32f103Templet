#include "tool.h"

u32 Tool_GetBitFor(u32 data, u8 index) {
	return (data >> index) & 0x00000001;
}

u32 Tool_SetBitFor(u32 data, u8 index, BOOL d) {
	if (d) {
		data |= 1 << index;
	} else {
		data &= ~(1 << index);
	}
	return data;
}

/*
// C prototype : void StrToHex(BYTE *pbDest, BYTE *pbSrc, int nLen)
// parameter(s): [OUT] pbDest - ���������
//	[IN] pbSrc - �ַ���
//	[IN] nLen - 16���������ֽ���(�ַ����ĳ���/2)
// return value: 
// remarks : ���ַ���ת��Ϊ16������
*/
void Tool_StrToHex(u8 *pbDest, u8 *pbSrc, int nLen)
{
	char h1,h2;
	u8 s1,s2;
	int i;

	for (i=0; i<nLen; i++)
	{
		h1 = pbSrc[2*i];
		h2 = pbSrc[2*i+1];

		s1 = toupper(h1) - 0x30;
		if (s1 > 9) 
		s1 -= 7;

		s2 = toupper(h2) - 0x30;
		if (s2 > 9) 
		s2 -= 7;

		pbDest[i] = s1*16 + s2;
	}
}

/*
// C prototype : void HexToStr(BYTE *pbDest, BYTE *pbSrc, int nLen)
// parameter(s): [OUT] pbDest - ���Ŀ���ַ���
//	[IN] pbSrc - ����16����������ʼ��ַ
//	[IN] nLen - 16���������ֽ���
// return value: 
// remarks : ��16������ת��Ϊ�ַ���
*/
void Tool_HexToStr(u8 *pbDest, u8 *pbSrc, int nLen)
{
	char	ddl,ddh;
	int i;

	for (i=0; i<nLen; i++)
	{
		ddh = 48 + pbSrc[i] / 16;
		ddl = 48 + pbSrc[i] % 16;
		if (ddh > 57) ddh = ddh + 7;
		if (ddl > 57) ddl = ddl + 7;
		pbDest[i*2] = ddh;
		pbDest[i*2+1] = ddl;
	}

	pbDest[nLen*2] = '\0';
}


void Tool_BubbleSort(u16 L[],u16 n) { 
	int i,j;  
	u16 temp;
	BOOL ischanged;//����������� 
	for(j=n;j<0;j--){  
		ischanged =FALSE; 
		for(i=0;i<j;i++) {  
			if(L[i]>L[i+1]){//������ֽ���Ԫ�ؾ�����ƶ� 
  				temp=L[i];
 				L[i]=L[i+1]; 
				L[i+1]=temp;
 				ischanged =TRUE; 
			}
 		}
		if(!ischanged)//��û���ƶ���˵�������Ѿ�����ֱ������  
		break;  
	} 
}


u16 Tool_GetByteIndex(u8 *pBuf, u16 start, u16 len, u8 byte){
	u16 index=start;
	while(index<len){
		if(pBuf[index]==byte){
			break;
		}
		index++;
	}
	
	return index;
}

u32 Tool_LoopSub(u32 last, u32 next)
{
	if(next>=last){
		return next-last;
	}
	
	return 0xFFFFFFFF-last+next+1;
}


