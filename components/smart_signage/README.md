# Smart Signage Component

This directory contains the **Smart Signage** ESPHome component.  It
combines multiple subsystems – a radar sensor, an inertial
measurement unit (IMU), an RGB LED, a speaker and a configuration UI –
to build an interactive safety sign.  The design is intended to run
on the Seeed XIAO ESP32‑S3 board but can be adapted to other
platforms by updating the pin definitions in `include/board.h`.

## Features

* **Radar presence detection** using the LD2410 sensor.  A simple
  Kalman filter smooths distance readings and the controller only
  triggers when the filtered value is below the configured detection
  radius.
* **IMU based fall detection** via an MPU6500 accelerometer/gyro.
  When the device tilt exceeds a configurable angle for a number of
  consecutive samples the controller reports a fall.
* **Audio playback** over I²S using the Helix MP3 decoder.  Profiles
  defined in `config.yaml` map events (start, detected, fallen,
  rose, etc.) to playlists of audio files and optional repeat delays.
* **LED control** with steady on/off levels and programmable
  breathing effects.  Brightness is adjustable from the dashboard.
* **User interface** exposed through ESPHome.  A set of
  `select` and `number` entities let you pick profiles, assign the
  knob function, set session length, detection radius, volume and
  LED brightness.  A `button` entity starts a session, and a
  dedicated hardware button on GPIO3 mirrors this action.

## Project structure

* `include/` – header files for the finite state machines (FSMs),
  hardware abstraction layers and utility classes.
* `audio_fsm.h`, `led_fsm.h`, `imu_fsm.h`, `radar_fsm.h`,
  `ctrl_fsm.h` – declarative state machine definitions built
  using Boost.SML.  The corresponding `.cpp` files implement
  guards and actions.
* `user_intf.cpp/h` – code that parses `config.yaml`, populates
  the ESPHome dashboard entities and writes updated values into
  non‑volatile storage.
* `smart_signage.cpp/h` – the ESPHome component that wires
  everything together.  It creates the FreeRTOS tasks for each
  interface, mounts the filesystem, configures the user interface
  and kicks off the controller FSM.
* `smart_signage.yaml` – an example ESPHome configuration file.
* `config.yaml` – profile definitions mapping high‑level events to
  audio sequences and LED effects.

## Quick start

1. **Install ESPHome**.  You can use the Python package (`pip install
   esphome`) or a Docker image.  See the [ESPHome
   documentation](https://esphome.io/guides/installing_esphome.html)
   for details.
2. **Obtain the sources**.  Clone or download this repository.  The
   `components/smart_signage` folder should reside alongside your
   ESPHome YAML.  The provided `smart_signage.yaml` uses
   `external_components` to locate the component automatically.
3. **Add your audio assets**.  Place MP3 files under
   `data/audio/<profile>/` matching the paths referenced in
   `config.yaml`.  For example, the `wet_floor` profile looks for
   `/audio/wet_floor/start.mp3`, so your local file should be
   `data/audio/wet_floor/start.mp3`.
4. **Set Wi‑Fi credentials**.  Create a `secrets.yaml` file or edit
   `smart_signage.yaml` to supply `wifi_ssid` and `wifi_password`.
5. **Build and flash**.  From the project directory run

   ```bash
   esphome run project/cpy/smart_signage.yaml
   ```

   or, using Docker:

   ```bash
   docker run --rm -v "$PWD":/config -it ghcr.io/esphome/esphome run project/cpy/smart_signage.yaml
   ```

   ESPHome will compile the firmware, upload it to the XIAO ESP32‑S3
   and make the device available on your network.
6. **Interact with the dashboard**.  After flashing, browse to the
   device's web UI (printed in the ESPHome logs) or integrate it into
   Home Assistant via the API.  Select a profile, adjust the
   parameters and press **Start**.  You can also press the physical
   button on GPIO3 to start a session.

## Customising profiles

Profiles are defined in `config.yaml`.  Each profile has an `id`, a
human‑readable `name` and a set of `events`.  The events map onto
audio sequences and LED effects.  Use these to tailor the behaviour
for different scenarios (e.g. wet floor warning, general alarm,
welcome messages).  You can add, remove or modify profiles without
changing the firmware – just update the YAML and reboot.

## Troubleshooting

* If you encounter a **Config Json parse failed** error in the logs,
  verify that `config.yaml` is valid YAML and that the file is
  included via `profileConfig` in `smart_signage.yaml`.
* Make sure the audio files exist on the device filesystem and that
  their paths match the entries in `config.yaml`.
* When modifying hardware pins consult `include/board.h` for the
  default assignments.  You can override these definitions by
  altering the header or providing your own pin constants at compile
  time via `build_flags` in the YAML.

## License

This project is provided as‑is under an open source license.  See
the repository root for details.