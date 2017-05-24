#include "stdafx.h"
#include "GHid.h"
#include "stdio.h"
#include "malloc.h"
GHid::GHid(void)
{
	ReadLength = 64;//�˴���Ҫ����
	hRead = INVALID_HANDLE_VALUE;
	hWrite = INVALID_HANDLE_VALUE;
	hDevHandle = INVALID_HANDLE_VALUE;
	PSP_DEVICE_INTERFACE_DETAIL_DATA*	pDevDetailData;
	pDevDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA*)malloc(10 * sizeof(PSP_DEVICE_INTERFACE_DETAIL_DATA));
	ExistDevice = 0;
}


GHid::~GHid(void)
{
	
}
BOOL GHid::Open(UCHAR index)
{
	if (index >= ExistDevice) return FALSE;
	Close();
	hWrite = CreateFile(pDevDetailData[index]->DevicePath,
		GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	hRead = CreateFile(pDevDetailData[index]->DevicePath,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,//|FILE_FLAG_OVERLAPPED,
		NULL);
	HidD_SetNumInputBuffers(hRead, 512);
	if ((hWrite != INVALID_HANDLE_VALUE) && (hRead != INVALID_HANDLE_VALUE)) return TRUE;
	return FALSE;
}
BOOL GHid::Scan(WORD Pid_Open, WORD Vid_Open)
{
	Close();
	GUID HidGuid;														//����һ��DEVINFO�ľ��hDevInfoSet�������ȡ�����豸��Ϣ���Ͼ����
	HDEVINFO hDevInfoSet;												//����MemberIndex����ʾ��ǰ�������ڼ����豸��0��ʾ��һ���豸��
	DWORD MemberIndex;													//DevInterfaceData�����������豸�������ӿ���Ϣ
	SP_DEVICE_INTERFACE_DATA DevInterfaceData;							//����һ��BOOL���������溯�������Ƿ񷵻سɹ�
	BOOL Result;														//����һ��RequiredSize�ı���������������Ҫ������ϸ��Ϣ�Ļ��峤�ȡ�
	DWORD RequiredSize;													//����һ��ָ���豸��ϸ��Ϣ�Ľṹ��ָ�롣
	PSP_DEVICE_INTERFACE_DETAIL_DATA	pDevDetailData_local;					//����һ������������豸�ľ����//����һ��HIDD_ATTRIBUTES�Ľṹ������������豸�����ԡ�
	HIDD_ATTRIBUTES DevAttributes;
	DevInterfaceData.cbSize = sizeof(DevInterfaceData);					//��DevInterfaceData�ṹ���cbSize��ʼ��Ϊ�ṹ���С													
	DevAttributes.Size = sizeof(DevAttributes);							//��DevAttributes�ṹ���Size��ʼ��Ϊ�ṹ���С	
	HidD_GetHidGuid(&HidGuid);											//����HidD_GetHidGuid������ȡHID�豸��GUID����������HidGuid�С�
	hDevInfoSet = SetupDiGetClassDevs(&HidGuid,							//����HidGuid����ȡ�豸��Ϣ���ϡ�����Flags��������ΪDIGCF_DEVICEINTERFACE|DIGCF_PRESENT��ǰ�߱�ʾʹ�õ�GUIDΪ�ӿ���GUID�����߱�ʾֻ�о�����ʹ�õ��豸����Ϊ��������ֻ�����Ѿ������ϵ��豸�����صľ��������hDevinfo�С�ע���豸��Ϣ������ʹ����Ϻ�Ҫʹ�ú���SetupDiDestroyDeviceInfoList���٣���Ȼ������ڴ�й©��
		NULL,
		NULL,
		DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);


	//Ȼ����豸������ÿ���豸�����о٣�����Ƿ�������Ҫ�ҵ��豸
	//���ҵ�����ָ�����豸�������豸�Ѿ��������ʱ�����˳����ҡ�
	//����ָ���һ���豸������MemberIndex��Ϊ0��
	MemberIndex = 0;
	while (1)
	{
		Result = SetupDiEnumDeviceInterfaces(hDevInfoSet,					//����SetupDiEnumDeviceInterfaces���豸��Ϣ�����л�ȡ���ΪMemberIndex���豸��Ϣ��
			NULL,
			&HidGuid,
			MemberIndex,
			&DevInterfaceData);
		if (Result == FALSE) {
			printf("Could not find the device!\n");
			break;											//�����ȡ��Ϣʧ�ܣ���˵���豸�Ѿ�������ϣ��˳�ѭ����	
		}
		MemberIndex++;														//��MemberIndexָ����һ���豸
		Result = SetupDiGetDeviceInterfaceDetail(hDevInfoSet,
			&DevInterfaceData,
			NULL,
			NULL,
			&RequiredSize,
			NULL);
		pDevDetailData_local = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(RequiredSize);//Ȼ�󣬷���һ����СΪRequiredSize�����������������豸��ϸ��Ϣ��
		if (pDevDetailData_local == NULL)											//����ڴ治�㣬��ֱ�ӷ��ء�
		{
			printf("�ڴ治��!\n");
			SetupDiDestroyDeviceInfoList(hDevInfoSet);
			return FALSE;
		}
		pDevDetailData_local->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);		//������pDevDetailData_local��cbSizeΪ�ṹ��Ĵ�С��ע��ֻ�ǽṹ���С�����������滺��������

																				//Ȼ���ٴε���SetupDiGetDeviceInterfaceDetail��������ȡ�豸����ϸ��Ϣ����ε�������ʹ�õĻ������Լ���������С��
		Result = SetupDiGetDeviceInterfaceDetail(hDevInfoSet,
			&DevInterfaceData,
			pDevDetailData_local,
			RequiredSize,
			NULL,
			NULL);
		//���豸·�����Ƴ�����Ȼ�����ٸո�������ڴ档

		//�������ʧ�ܣ��������һ���豸��
		if (Result == FALSE) continue;

		//������óɹ�����ʹ�ò�����д���ʵ�CreateFile��������ȡ�豸�����ԣ�����VID��PID���汾�ŵȡ�
		//����һЩ��ռ�豸������USB���̣���ʹ�ö����ʷ�ʽ���޷��򿪵ģ�
		//��ʹ�ò�����д���ʵĸ�ʽ�ſ��Դ���Щ�豸���Ӷ���ȡ�豸�����ԡ�
		hDevHandle = CreateFile(pDevDetailData_local->DevicePath,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);


		//����򿪳ɹ������ȡ�豸���ԡ�
		if (hDevHandle != INVALID_HANDLE_VALUE)
		{
			//��ȡ�豸�����Բ�������DevAttributes�ṹ����
			Result = HidD_GetAttributes(hDevHandle, &DevAttributes);
			if (Result == FALSE) continue;		//��ȡʧ�ܣ�������һ��
												//�����ȡ�ɹ����������е�VID��PID�Լ��豸�汾����������Ҫ��
												//���бȽϣ������һ�µĻ�����˵������������Ҫ�ҵ��豸��
												//printf("VID=%d,PID=%d,Size=%d\n",DevAttributes.VendorID,DevAttributes.ProductID,DevAttributes.VersionNumber,DevAttributes.Size);
			if (DevAttributes.VendorID == Vid_Open&&DevAttributes.ProductID == Pid_Open)
			{
				//HidD_SetNumInputBuffers(hDevHandle,512);
				CloseHandle(hDevHandle);
				//hWrite = CreateFile(pDevDetailData->DevicePath,
				//	GENERIC_WRITE,
				//	FILE_SHARE_READ | FILE_SHARE_WRITE,
				//	NULL,
				//	OPEN_EXISTING,
				//	FILE_ATTRIBUTE_NORMAL,
				//	NULL);
				//hRead = CreateFile(pDevDetailData->DevicePath,
				//	GENERIC_READ,
				//	FILE_SHARE_READ | FILE_SHARE_WRITE,
				//	NULL,
				//	OPEN_EXISTING,
				//	FILE_ATTRIBUTE_NORMAL,//|FILE_FLAG_OVERLAPPED,
				//	NULL);
				//printf("Find Device!\tVID=0x%X\tPID=0x%X\n", DevAttributes.VendorID, DevAttributes.ProductID);
				//HidD_SetNumInputBuffers(hRead, 512);
				//return TRUE;
				pDevDetailData[ExistDevice] = pDevDetailData_local;
				ExistDevice++;
			}

			else
			{
				CloseHandle(hDevHandle);
			}
		}
		free(pDevDetailData);
	}
	CloseHandle(hDevHandle);
	return FALSE;
}
void GHid::Close()
{
	if(hRead!=INVALID_HANDLE_VALUE)
		CloseHandle(hRead);
	if(hWrite!=INVALID_HANDLE_VALUE)
		CloseHandle(hRead);
	if(hDevHandle!=INVALID_HANDLE_VALUE)
		CloseHandle(hDevHandle);
	hRead = INVALID_HANDLE_VALUE;
	hWrite = INVALID_HANDLE_VALUE;
}
DWORD GHid::Read(BYTE* Buffer)
{
	unsigned long	numBytesReturned;
	unsigned char	inbuffer[65];		
	BOOL			bResult;
	bResult = ReadFile(	hRead,							
			&inbuffer[0],						
				ReadLength+1,					//���ucDataLength��ֵΪ64�Ļ�,Capabilities.InputReportByteLength��Ϊ65
			&numBytesReturned,					
			NULL);

	if(bResult==1)
	{
		for (int i = 0; i < ReadLength; i++)
		{
			Buffer[i] = inbuffer[i+1];
		}
		return TRUE;
	}
	return FALSE;
}
BOOL GHid::Write(BYTE* Buffer)
{
	unsigned long	numBytesReturned;
	unsigned char   outbuffer[65];		/* output buffer */
	BOOL			bResult;
	OVERLAPPED		HidOverlapped;
	HidOverlapped.hEvent = CreateEvent(NULL, TRUE, TRUE, _T(""));
	HidOverlapped.Offset = 0;
	HidOverlapped.OffsetHigh = 0;
	outbuffer[0] = 0;	/* this is used as the report ID */
	for(DWORD i=0;i<ReadLength;i++)		//copy data form ucDataBuffer
		outbuffer[i+1]	= Buffer[i];
	bResult = WriteFile(	hWrite, 
								outbuffer, 
								ReadLength+1, 
								&numBytesReturned, 
								(LPOVERLAPPED) &HidOverlapped);
	
	if(bResult==FALSE)
	{
		printf("Wrong in write��");
		DWORD LastError=GetLastError();
		//���Ƿ������IO����
		if((LastError==ERROR_IO_PENDING)||(LastError==ERROR_SUCCESS))
		{
			return TRUE;
		}
		else
		{

			if(LastError==1)
			{
				printf("���豸��֧��WriteFile������");
			}
			return FALSE;
		}
	}
	return TRUE;

}