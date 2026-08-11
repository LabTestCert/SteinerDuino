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

Confirm the board is on `/dev/ttyUSB0` first:

```bash
arduino-cli board list
```

Then:

```bash
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 ./FT_V9.1_SD_Ignore
```

(Combine both in one step with `arduino-cli compile --upload`.)

## Serial monitor

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
