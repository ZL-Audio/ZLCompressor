# ZLCompressor

LICENSE and CODE are available at [https://github.com/ZL-Audio/ZLCompressor](https://github.com/ZL-Audio/ZLCompressor)

# Changelog

## 0.4.0

New Features

- add meter panel
- add inf range button
- add negative lookahead (i.e., side-chain delay)

Bug fixes and Improvements

- improve UI layout
- improve magnitude analyzer display
- improve FFT analyzer display

## 0.3.2

Bug fixes and Improvements

- improve magnitude analyzer display
- improve FFT analyzer display

## 0.3.1

New Features

- add `Inflate`, `Expand` and `Shape` directions (see the manual for details)
- add UI scaling control & credit info

Bug fixes and Improvements

- fix plugin crashes when it does not have UI preset folder permission
- fix incorrect rms display
- fix displayed values when they are close to zero
- fix incorrect copy/paste from ZL Equalizer 2
- improve DSP performance

Other Changes

- support VST3 parameter context menu
- change several parameter ranges
- change icons

## 0.2.1

New Features

- add spectrum frozen (hold the mouse over the spectrum)
- add multiple-band selection
- add multiple-band copy/paste

Bug fixes and Improvements

- fix wrong 6dB slope choice when using mouse wheel
- improve over-sampling performance (greatly)

## 0.2.0

New Features

- add tooltips
- add side-chain filter solo (double-click on the dragger)
- add soft clipper

Bug fixes and Improvements

- fix potential overflow in `Vocal` style
- improve stability

## 0.1.2

New Features

- add UI setting panel
- add two stereo modes (see #20)
- add filter slope mouse wheel control (+ command/ctrl)
- add filter invert/copy/paste right click panel
- add threshold/ratio mousewheel control (on the analyzer)

Bug fixes and Improvements

- fix side wet slider width
- fix analyzer shaking when bypassed/paused
- improve slider mousewheel behaviour (see #21)
- improve FFT analyzer display

Others

- add Windows ARM64 release

## 0.1.1

New Features

- add compression style `Vocal`
- add RMS release and RMS parameters
- add loudness match

Bug fixes and Improvements

- fix delayed magnitude analyzer
- fix incorrect cumulative RMS display
- improve resizable corner dragger