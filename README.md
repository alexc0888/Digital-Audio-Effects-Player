# Digital Audio Effects Player

**Digital Audio Effects Player** Digital Audio Effects Player is a custom-built audio playback system powered by an STM32F413ZH microcontroller, 
capable of applying real-time digital filters in the time-domain and visualizing the frequency spectrum. It supports playback of uncompressed .WAV files from an SD card and provides an interactive OLED menu system.


---

## Project Overview

The system consists of the following components:

- **Key Components**:
  - **STM32F413ZH Nucleo Board**: Cortex-M4F core, 96 MHz
  - **32 GB Micro SD Card**: Stores song files, has FatFS software layer on top to help define the contained data
  - **32×64 RGB LED Dot Matrix Display**: DMA engine transmits frame data over to GPIO pins, following the HUB75 protocol
  - **128x128 OLED Display**: Controlled via SPI interface for UI
  - **Potentiometers & Push Buttons**: Navigate the UI and set filter parameters
  - **12-bit DAC (Internal to STM32)**: Outputs audio data to external speaker
---

## Features

- **Audio Processing**: Real-time Bass Boost, Treble Boost, and Tremolo filters with adjustable parameters via potentiometers.
- **Live Frequency Spectrum**: 32-point FFT visualized on a 32x64 RGB LED Dot Matrix Display with interpolation for smooth animation
- **Interactive OLED Menu**: 128x128 SPI-driven OLED for track selection, managing settings, and playback progress
- **Optimizing with DMA Engine**: DMA-driven SD card reads*, DAC streaming, LED matrix rendering, and ADC sampling to minimize CPU load
- **WAV Parser**: Validates and extracts metadata from .WAV files before playback
- **Circular Buffer & Double Buffering**: Ensures smooth, uninterrupted playback and prevents CPU/DMA conflicts by following a producer/consumer paradigm with the circular buffer.

*While FatFS supports usage of DMA for SD card reads via SDIO, the given libraries force the CPU to spin-wait anyways until the payload is finished being delivered, so that an ACK response can be given. 
Messing too much with the FatFS layer was outside of the scope of this project. A potential solution would be to only use the SD card as tertiary storage. On song selection, the song should be loaded onto 
voltatile, secondary storage, such as a QSPI flash module. Then, a DMA engine could directly deliver payloads while the CPU is performing other work, enabling other work to be done in the loop. However, this was 
not done, since the issue was discovered late into development. Instead, we worked around the limitations we had, keeping the SD card in mind as the key bottleneck. Choosing an appropriate block size to read from each time, 
while optimizing other parts of the system proved to be enough to stream at the mandated 44.1 kHz sampling rate.

---

## Documentation 

You can learn more about the project by reading the report at [Documentation/Digital Audio Effects Player Final Report.pdf](https://github.com/alexc0888/Digital-Audio-Effects-Player/blob/master/Documentation/Digital%20Audio%20Effects%20Player%20Final%20Report.pdf)

---

## Demo

Check out a live demo here:  
[![Digital Audio Effects Player Demo Video](https://img.youtube.com/vi/3w7WjbBNveA/0.jpg)](https://www.youtube.com/watch?v=3w7WjbBNveA)

👉 [Watch the demo on YouTube](https://www.youtube.com/watch?v=3w7WjbBNveA)

---

## Contributors

- Alex Chitsazzadeh
- Aditya Sood
- Kerway Tsai
- Vinay Jagan
- Derek Rosales
- Zichen Zhu
- Austin Kinkade

---


## Acknowledgments

- STM32Cube HAL Drivers
- [FatFS File System from elm-chan](https://elm-chan.org/fsw/ff/)
- [CMSIS DSP Library](https://arm-software.github.io/CMSIS_5/DSP/html/index.html)

---
