


 

#pragma once
#include "Windows.h"
/* global constants */
extern "C" {
// This file is in the Windows DDK available from Microsoft.
#include "hidsdi.h"
#include <setupapi.h>
}


class GHid
{
public:
	GHid(void);
	~GHid(void);
	BOOL Open(WORD Pid_Open, WORD Vid_Open);
	void Close();
	DWORD Read(BYTE* Buffer);
	BOOL Write(BYTE* Buffer);
protected:
	BYTE ReadLength;
	HANDLE hDevHandle;
	HANDLE hRead;
	HANDLE hWrite;
};


