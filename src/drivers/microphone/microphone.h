#pragma once

class Microphone
{
public:
    Microphone();
    void init();
    void read();

private:
    const unsigned AdcFrequency{44100U};
    const unsigned long AdcClock{48'000'000UL};
    const unsigned FftSamples{1024U};
    const float ClockDivider{(static_cast<float>(AdcClock) / AdcFrequency) - 1};
};
