#include <iostream>
#include "Hotkey.h"

int Hotkey::ref_cnt = 0;

Hotkey::Hotkey()
{
    HRESULT hr;
    if (ref_cnt == 0) {
        hr = CoInitialize(NULL);  // COM 라이브러리 초기화
        if (FAILED(hr)) {
            std::cerr << "COM initialization failed!" << std::endl;
            return;
        }
        std::cout << "COM 라이브러리 초기화" << std::endl;
    }
    ++ref_cnt;
}

Hotkey::~Hotkey()
{
    if (ref_cnt == 1) {
        CoUninitialize();  // COM 해제
        std::cout << "COM 해제" << std::endl;
    }
    --ref_cnt;
}

MIC::MIC()
{
    HRESULT hr;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (FAILED(hr)) {
        std::cerr << "IMMDeviceEnumerator 생성 실패" << std::endl;
        return;
    }

    hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eMultimedia, &pDevice);
    if (FAILED(hr)) {
        std::cerr << "기본 마이크 장치 찾기 실패" << std::endl;
        pEnumerator->Release();
        return;
    }

    hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&pEndpointVolume);
    if (FAILED(hr)) {
        std::cerr << "IAudioEndpointVolume 활성화 실패" << std::endl;
        pDevice->Release();
        pEnumerator->Release();
        return;
    }
    mic_mute = false;
    hr = pEndpointVolume->GetMute(&mic_mute);
}

MIC::~MIC()
{
    if (pEndpointVolume) {
        pEndpointVolume->Release();
        pEndpointVolume = nullptr;
    }

    if (pDevice) {
        pDevice->Release();
        pDevice = nullptr;
    }

    if (pEnumerator) {
        pEnumerator->Release();
        pEnumerator = nullptr;
    }
}

void MIC::MuteMicrophone()
{
    HRESULT hr;
    hr = pEndpointVolume->SetMute(!mic_mute, NULL);
    if (FAILED(hr)) {
        std::cerr << "마이크 음소거 설정 실패" << std::endl;
    }
    else {
        std::cout << "마이크 " << (!mic_mute ? "음소거됨" : "음소거 해제됨") << std::endl;
        mic_mute = !mic_mute;
    }
}
