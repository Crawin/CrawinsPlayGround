#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#pragma comment(lib, "ole32.lib")

class Hotkey
{
private:
	static int ref_cnt;
public:
	Hotkey();
	virtual ~Hotkey();
};

class MIC : public Hotkey
{
private:
	BOOL mic_mute;

	IMMDeviceEnumerator* pEnumerator = nullptr;
	IMMDevice* pDevice = nullptr;
	IAudioEndpointVolume* pEndpointVolume = nullptr;
public:
	MIC();
	virtual ~MIC();
	void MuteMicrophone();
};