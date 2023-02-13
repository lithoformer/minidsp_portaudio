# minidsp_portaudio
recording setup for FFT analysis of MEMS microphones input

i've modified the portaudio paex_record.c program to work with the mini-dsp UMA-8.  the UMA-8 is a multi-channel MEMS microphone array and only two of the microphones are taken to input into the FFT and also for recording output in stereo

hardware:

minidsp UMA-8:
https://www.minidsp.com/products/usb-audio-interface/uma-8-microphone-array

not shown:
(your playback interface)

software dependencies:

pffft for fft:
https://github.com/marton78/pffft

portaudio for audio i/o:
http://files.portaudio.com/download.html


to run the code, build portaudio and pffft as shown on their respective repos and import the binaries into your MSVC solution
