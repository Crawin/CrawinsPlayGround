#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#pragma comment(lib, "ole32.lib")

class CHotKey
{
private:
	static int ref_cnt;
public:
	CHotKey();
	virtual ~CHotKey();
	virtual bool execute() = 0;
};

class CMIC : public CHotKey
{
private:
	BOOL m_bMicMute;

	IMMDeviceEnumerator* pEnumerator = nullptr;
	IMMDevice* pDevice = nullptr;
	IAudioEndpointVolume* pEndpointVolume = nullptr;
public:
	CMIC();
	virtual ~CMIC();
	void MuteMicrophone();
	virtual bool execute() override;
};