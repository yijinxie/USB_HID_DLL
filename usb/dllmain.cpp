// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "usb.h"
#include "GHid.h"
#include "string.h"
GHid m_Hid;
bool load=false;
void CBYTE(BYTE* buf,char* a,int len){
	for (int i=0;i<=len;i++){
		buf[i]=BYTE(a[i]);
	}
}
void CSCHAR(char* a,BYTE* buf,int len){
	for (int i=0;i<=len;i++){
		a[i]=char(buf[i]);
	}
}
// 这是导出函数的一个示例。
USB_API BOOL write(char *data)
{
	//m_Hid.Write(data);
	BYTE* b=new BYTE[64];
	CBYTE(b,data,64);
	return m_Hid.Write(b);
}

USB_API void read(char *data)
{
	BYTE* b=new BYTE[64];
	//CBYTE(b,"CONF:VOLT:DC D0,L\n",64);
	m_Hid.Read(b);
	CSCHAR(data,b,64);
}
USB_API void open(UCHAR index)
{
	BYTE* b = new BYTE[64];
	//CBYTE(b,"CONF:VOLT:DC D0,L\n",64);
	if (!m_Hid.Open(index)) printf("Open device number->%d fail", index);
}
USB_API UCHAR get_device_number(void)
{
	return m_Hid.ExistDevice;
}
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		printf("Loading USB Device.\n");
		
		if (m_Hid.Scan(0x003f, 0x04d8)) printf("Find %d devices.\n", m_Hid.ExistDevice);
		else printf("Cannot find any device!\n");
		
		if (load) {
			return FALSE;
		}
		if (m_Hid.Open(0)){
			//HANDLE hThread1 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)USBRead, NULL, 0, &dwGenericThread);
			/*BYTE* b=new BYTE[64];
			CBYTE(b,"CONF:VOLT:DC D0,L\n",64);
			m_Hid.Write(b);
			if(m_Hid.Read(b)){
				printf("write::%d\n",b[0]);
			}*/
			load=true;
		}
		else
		{
			printf("Loading fail.\n");
			return FALSE;
		}
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		printf("Close USB Device.\n");
		m_Hid.Close();
		load=false;
		break;
	}
	return TRUE;
}

