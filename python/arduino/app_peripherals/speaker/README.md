# Speaker peripheral

This brick allow you to play audio using Alsa Linux audio subsystem

## Usage

```python
from arduino.app_peripherals.speaker import Speaker

speak = Speaker(device='USB_SPEAKER_1')
speak.start()
# data is a byte array
speak.play(data)
speak.stop()
```

## Parameters

- `device`: (optional) ALSA device name (default: 'USB_SPEAKER_1'. It can be the real ALSA device name or USB_SPEAKER_1, USB_SPEAKER_2, ..)
- `sample_rate`: (optional) sampling frequency (default: 16000 Hz)
- `channels`: (optional) channels (default: 1)
- `format`: (optional) ALSA audio format (default: 'S16_LE')
- `periodsize`: (optional) ALSA period size in frames (default: None = hardware default)
  - For **real-time synthesis** (WaveGenerator, etc.): set to match generation block size to eliminate buffer mismatches
  - For **streaming/file playback**: leave as None for hardware-optimal buffering
  - Example: `periodsize=480` for 30ms blocks @ 16kHz (480 = 16000 × 0.03)
- `queue_maxsize`: (optional) application queue depth in blocks (default: 100)
  - Lower values (5-20): reduce latency for interactive audio
  - Higher values (50-200): provide stability for streaming
