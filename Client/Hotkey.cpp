#include <iostream>
#include "Hotkey.h"

int Hotkey::ref_cnt = 0;

Hotkey::Hotkey()
{
    HRESULT hr;
    if (ref_cnt == 0) {
        hr = CoInitialize(NULL);  // COM ���̺귯�� �ʱ�ȭ
        if (FAILED(hr)) {
            std::cerr << "COM initialization failed!" << std::endl;
            return;
        }
        std::cout << "COM ���̺귯�� �ʱ�ȭ" << std::endl;
    }
    ++ref_cnt;
}

Hotkey::~Hotkey()
{
    if (ref_cnt == 1) {
        CoUninitialize();  // COM ����
        std::cout << "COM ����" << std::endl;
    }
    --ref_cnt;
}

MIC::MIC()
{
    HRESULT hr;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (FAILED(hr)) {
        std::cerr << "IMMDeviceEnumerator ���� ����" << std::endl;
        return;
    }

    hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eMultimedia, &pDevice);
    if (FAILED(hr)) {
        std::cerr << "�⺻ ����ũ ��ġ ã�� ����" << std::endl;
        pEnumerator->Release();
        return;
    }

    hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&pEndpointVolume);
    if (FAILED(hr)) {
        std::cerr << "IAudioEndpointVolume Ȱ��ȭ ����" << std::endl;
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
        std::cerr << "����ũ ���Ұ� ���� ����" << std::endl;
    }
    else {
        std::cout << "����ũ " << (!mic_mute ? "���Ұŵ�" : "���Ұ� ������") << std::endl;
        mic_mute = !mic_mute;
    }
}
