### Find Webcam on Windows

List devices:

```
ffmpeg -f dshow -list_devices true -i 0

...
[dshow @ 0000020a78e0d540] DirectShow video devices (some may be both video and audio devices)
[dshow @ 0000020a78e0d540]  "Hercules HD Sunset"
[dshow @ 0000020a78e0d540]     Alternative name "@device_pnp_\\?\usb#vid_06f8&pid_3017&mi_00#6&ea49103&0&0000#{65e8773d-8f56-11d0-a3b9-00a0c9223196}\global"
[dshow @ 0000020a78e0d540] DirectShow audio devices
[dshow @ 0000020a78e0d540]  "Microphone (Hercules HD Sunset Mic)"
[dshow @ 0000020a78e0d540]     Alternative name "@device_cm_{33D9A762-90C8-11D0-BD43-00A0C911CE86}\wave_{86EA91AF-AD0B-41A7-84A5-0D205C9C7BCC}"
[dshow @ 0000020a78e0d540]  "Microphone (Steam Streaming Microphone)"
[dshow @ 0000020a78e0d540]     Alternative name "@device_cm_{33D9A762-90C8-11D0-BD43-00A0C911CE86}\wave_{0CC86501-581A-49E6-9672-939EDE2B4093}"
0: Immediate exit requested
```

List option For select device:

```
ffmpeg -f dshow -list_options true -i video="Hercules HD Sunset"

...
[dshow @ 000002b07fe4d580] DirectShow video device options (from video devices)
[dshow @ 000002b07fe4d580]  Pin "Capture" (alternative pin name "0")
[dshow @ 000002b07fe4d580]   pixel_format=yuyv422  min s=640x480 fps=7.5 max s=640x480 fps=30
[dshow @ 000002b07fe4d580]   pixel_format=yuyv422  min s=640x480 fps=7.5 max s=640x480 fps=30
...
[dshow @ 000002b07fe4d580]   pixel_format=yuyv422  min s=1280x720 fps=7.5 max s=1280x720 fps=10
[dshow @ 000002b07fe4d580]   pixel_format=yuyv422  min s=1280x800 fps=7.5 max s=1280x800 fps=10
[dshow @ 000002b07fe4d580]   pixel_format=yuyv422  min s=1280x800 fps=7.5 max s=1280x800 fps=10
[dshow @ 000002b07fe4d580]   vcodec=mjpeg  min s=640x480 fps=7.5 max s=640x480 fps=30
[dshow @ 000002b07fe4d580]   vcodec=mjpeg  min s=640x480 fps=7.5 max s=640x480 fps=30
...
[dshow @ 000002b07fe4d580]   vcodec=mjpeg  min s=1280x800 fps=7.5 max s=1280x800 fps=30
[dshow @ 000002b07fe4d580]   vcodec=mjpeg  min s=1280x800 fps=7.5 max s=1280x800 fps=30
video=Hercules HD Sunset: Immediate exit requested
```

Acquisition with mjpeg:
```
ffmpeg -f dshow -vcodec mjpeg -i video="Hercules HD Sunset" OUTPUT_OPTIONS...
```
