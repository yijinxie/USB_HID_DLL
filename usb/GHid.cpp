#include "stdafx.h"
#include "GHid.h"
#include "stdio.h"
#include "malloc.h"
GHid::GHid(void)
{
	ReadLength = 64;//此处需要更改
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
	GUID HidGuid;														//定义一个DEVINFO的句柄hDevInfoSet来保存获取到的设备信息集合句柄。
	HDEVINFO hDevInfoSet;												//定义MemberIndex，表示当前搜索到第几个设备，0表示第一个设备。
	DWORD MemberIndex;													//DevInterfaceData，用来保存设备的驱动接口信息
	SP_DEVICE_INTERFACE_DATA DevInterfaceData;							//定义一个BOOL变量，保存函数调用是否返回成功
	BOOL Result;														//定义一个RequiredSize的变量，用来接收需要保存详细信息的缓冲长度。
	DWORD RequiredSize;													//定义一个指向设备详细信息的结构体指针。
	PSP_DEVICE_INTERFACE_DETAIL_DATA	pDevDetailData_local;					//定义一个用来保存打开设备的句柄。//定义一个HIDD_ATTRIBUTES的结构体变量，保存设备的属性。
	HIDD_ATTRIBUTES DevAttributes;
	DevInterfaceData.cbSize = sizeof(DevInterfaceData);					//对DevInterfaceData结构体的cbSize初始化为结构体大小													
	DevAttributes.Size = sizeof(DevAttributes);							//对DevAttributes结构体的Size初始化为结构体大小	
	HidD_GetHidGuid(&HidGuid);											//调用HidD_GetHidGuid函数获取HID设备的GUID，并保存在HidGuid中。
	hDevInfoSet = SetupDiGetClassDevs(&HidGuid,							//根据HidGuid来获取设备信息集合。其中Flags参数设置为DIGCF_DEVICEINTERFACE|DIGCF_PRESENT，前者表示使用的GUID为接口类GUID，后者表示只列举正在使用的设备，因为我们这里只查找已经连接上的设备。返回的句柄保存在hDevinfo中。注意设备信息集合在使用完毕后，要使用函数SetupDiDestroyDeviceInfoList销毁，不然会造成内存泄漏。
		NULL,
		NULL,
		DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);


	//然后对设备集合中每个设备进行列举，检查是否是我们要找的设备
	//当找到我们指定的设备，或者设备已经查找完毕时，就退出查找。
	//首先指向第一个设备，即将MemberIndex置为0。
	MemberIndex = 0;
	while (1)
	{
		Result = SetupDiEnumDeviceInterfaces(hDevInfoSet,					//调用SetupDiEnumDeviceInterfaces在设备信息集合中获取编号为MemberIndex的设备信息。
			NULL,
			&HidGuid,
			MemberIndex,
			&DevInterfaceData);
		if (Result == FALSE) {
			printf("Could not find the device!\n");
			break;											//如果获取信息失败，则说明设备已经查找完毕，退出循环。	
		}
		MemberIndex++;														//将MemberIndex指向下一个设备
		Result = SetupDiGetDeviceInterfaceDetail(hDevInfoSet,
			&DevInterfaceData,
			NULL,
			NULL,
			&RequiredSize,
			NULL);
		pDevDetailData_local = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(RequiredSize);//然后，分配一个大小为RequiredSize缓冲区，用来保存设备详细信息。
		if (pDevDetailData_local == NULL)											//如果内存不足，则直接返回。
		{
			printf("内存不足!\n");
			SetupDiDestroyDeviceInfoList(hDevInfoSet);
			return FALSE;
		}
		pDevDetailData_local->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);		//并设置pDevDetailData_local的cbSize为结构体的大小（注意只是结构体大小，不包括后面缓冲区）。

																				//然后再次调用SetupDiGetDeviceInterfaceDetail函数来获取设备的详细信息。这次调用设置使用的缓冲区以及缓冲区大小。
		Result = SetupDiGetDeviceInterfaceDetail(hDevInfoSet,
			&DevInterfaceData,
			pDevDetailData_local,
			RequiredSize,
			NULL,
			NULL);
		//将设备路径复制出来，然后销毁刚刚申请的内存。

		//如果调用失败，则查找下一个设备。
		if (Result == FALSE) continue;

		//如果调用成功，则使用不带读写访问的CreateFile函数来获取设备的属性，包括VID、PID、版本号等。
		//对于一些独占设备（例如USB键盘），使用读访问方式是无法打开的，
		//而使用不带读写访问的格式才可以打开这些设备，从而获取设备的属性。
		hDevHandle = CreateFile(pDevDetailData_local->DevicePath,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);


		//如果打开成功，则获取设备属性。
		if (hDevHandle != INVALID_HANDLE_VALUE)
		{
			//获取设备的属性并保存在DevAttributes结构体中
			Result = HidD_GetAttributes(hDevHandle, &DevAttributes);
			if (Result == FALSE) continue;		//获取失败，查找下一个
												//如果获取成功，则将属性中的VID、PID以及设备版本号与我们需要的
												//进行比较，如果都一致的话，则说明它就是我们要找的设备。
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
				ReadLength+1,					//如果ucDataLength的值为64的话,Capabilities.InputReportByteLength就为65
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
		printf("Wrong in write！");
		DWORD LastError=GetLastError();
		//看是否是真的IO挂起
		if((LastError==ERROR_IO_PENDING)||(LastError==ERROR_SUCCESS))
		{
			return TRUE;
		}
		else
		{

			if(LastError==1)
			{
				printf("该设备不支持WriteFile函数。");
			}
			return FALSE;
		}
	}
	return TRUE;

}