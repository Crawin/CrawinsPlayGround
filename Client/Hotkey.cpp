#include <iostream>
#include "Hotkey.h"

int CHotKey::ref_cnt = 0;

CHotKey::CHotKey()
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

CHotKey::~CHotKey()
{
    if (ref_cnt == 1) {
        CoUninitialize();  // COM 해제
        std::cout << "COM 해제" << std::endl;
    }
    --ref_cnt;
}

CMIC::CMIC()
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
    m_bMicMute = false;
    hr = pEndpointVolume->GetMute(&m_bMicMute);
}

CMIC::~CMIC()
{
    std::cout << "MIC 해제\n";
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

void CMIC::MuteMicrophone()
{
    HRESULT hr;
    hr = pEndpointVolume->SetMute(!m_bMicMute, NULL);
    if (FAILED(hr)) {
        std::cerr << "마이크 음소거 설정 실패" << std::endl;
    }
    else {
        std::cout << "마이크 " << (!m_bMicMute ? "음소거됨" : "음소거 해제됨") << std::endl;
        m_bMicMute = !m_bMicMute;
    }
}

bool CMIC::execute()
{
    MuteMicrophone();
    return true;
}
