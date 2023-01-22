# minidsp_portaudio
personal recording setup for FFT analysis of MEMS microphones input

i've modified the portaudio paex_record.c program to work with the mini-dsp UMA-8.  the UMA-8 is a multi-microphone MEMS array and just two of the microphones are taken to input into the FFT and also for recording output

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
