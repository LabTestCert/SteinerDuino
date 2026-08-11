# SteinerDuino

Firmware for the ESP32 that runs the Steiner tunnel logger/controller (data
consumed by [`../SteinerGate`](../SteinerGate)). Generic ESP32 dev board
(CP2102 USB-UART bridge), connects as `/dev/ttyUSB0` on Linux.

## Toolchain (Debian)

We build/upload with **arduino-cli** (no Arduino IDE installed on this
machine). Installed to `~/.local/bin/arduino-cli`.

- Board core: **esp32:esp32 pinned to version 2.0.17** (NOT the latest 3.x).
  The sketch uses the old LEDC PWM API (`ledcSetup`/`ledcAttachPin`/
  `ledcWrite(channel, ...)`), which was removed in core 3.x in favor of
  `ledcAttach`/`ledcWrite(pin, ...)`. Do not run
  `arduino-cli core upgrade` without first porting `PID_Setup()` /
  `PID_control()` to the new API.
- FQBN: `esp32:esp32:esp32` ("ESP32 Dev Module" — generic board profile;
  works for basically any CP2102/CH340-based ESP32 devkit and matches this
  board's pinout).
- `directories.user` is configured (`~/.arduino15/arduino-cli.yaml`) to
  point at this repo, so arduino-cli picks up libraries from `./libraries/`
  directly — no separate `~/Arduino/libraries` copy needed.

## Repo layout

| Path | Description |
| --- | --- |
| `FT_V9.1_SD_Ignore/FT_V9.1_SD_Ignore.ino` | Main firmware sketch. Folder name must match the `.ino` filename — this is an arduino-cli/Arduino requirement, not a stylistic choice. |
| `libraries/` | All libraries the sketch depends on, including two in-house ones: `AnalogSensor/` (supersampling + calibration) and `Sensirion_SDP6x_pressure_sens-master/` (pressure sensor driver). Everything else (RTClib, Adafruit GFX/SSD1306, ADS1115_WE, PID, arduino-timer, Pushbutton) is vendored here too rather than relying on Arduino's Library Manager. |

## Compile

```bash
cd ~/Code/SteinerDuino
arduino-cli compile --fqbn esp32:esp32:esp32 ./FT_V9.1_SD_Ignore
```

## Upload

**`../SteinerGate`'s background logger holds `/dev/ttyUSB0` open** (the
`steinergate-watchdog` systemd service, running on this same machine, owns
the serial port so it can log tunnel data continuously — see
[`../SteinerGate/src/LoggingService/README.md`](../SteinerGate/src/LoggingService/README.md)).
Its Worker process needs to be paused before `esptool` can do the
bootloader handshake, or the upload just fails/hangs with port-contention
errors. Use `upload.sh` for this rather than a bare `arduino-cli upload` —
it stops `steinergate-watchdog`, uploads, and restarts it afterward no
matter how the upload turns out (success, failure, or Ctrl-C):

```bash
cd ~/Code/SteinerDuino
./upload.sh
```

Optional args if you need to override the defaults:
`./upload.sh [sketch_dir] [port] [fqbn]` — e.g.
`./upload.sh ./FT_V9.1_SD_Ignore /dev/ttyUSB0 esp32:esp32:esp32`.

Stopping/starting `steinergate-watchdog` doesn't require `sudo` for this
user (verified — polkit already allows it for the active session). If that
ever changes, the two calls in `upload.sh` (`systemctl stop`/`systemctl
start`) are the only place `sudo` would need to be added.

To upload without the wrapper (e.g. the service is already stopped for
some other reason), confirm the board's port first with
`arduino-cli board list`, then:

```bash
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 ./FT_V9.1_SD_Ignore
```

## Serial monitor

Same port-contention issue as upload applies here — stop `steinergate-watchdog`
first (`systemctl stop steinergate-watchdog`), or the monitor and the logger
Worker will fight over the port. Restart the service (`systemctl start
steinergate-watchdog`) when done.

```bash
arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=115200
```

Exit with `Ctrl-C`. The sketch also accepts commands over serial:
`VERSION` and `SETTIME:YYYY-MM-DD HH:MM:SS`.

## Notes

- User must be in the `dialout` group for serial access (already the case
  on this machine). If you ever see a permissions error on `/dev/ttyUSB0`,
  check that BRLTTY (Debian's braille-display service, `brltty.service`)
  isn't running — it's known to grab CP2102/CH340 USB-serial devices out
  from under you. It's installed but inactive on this machine.
