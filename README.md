# pebble-fez

Pebble watch face inspired by the logo of wonderful game "[FEZ](http://fezgame.com/)"!

![pebble-fez](https://user-images.githubusercontent.com/158320/177231014-a57d8be7-8365-4556-a1a7-4eac3cb373e1.jpg)

https://user-images.githubusercontent.com/158320/177231264-657d05bc-3842-4868-8295-6872371aaf27.mp4

[GitHub Releases](https://github.com/exe44/pebble-fez/releases)

## Build

This project now uses the modern Pebble project layout with `package.json` and `wscript`.

```sh
npm install
pebble build
```

The compiled bundle will be generated at `build/pebble-fez.pbw`.

If this is your first checkout or `package.json` / `package-lock.json` changed, run `npm install` before building to install the JavaScript dependencies used by the configuration page.

## C Modules

- `src/c/main.c`: app lifecycle and module coordination
- `src/c/app_settings.[hc]`: persisted settings and color helpers
- `src/c/camera_controller.[hc]`: camera transition state and view matrix updates
- `src/c/clock_digits.[hc]`: time-to-digit conversion and diff logic
- `src/c/digit_renderer.[hc]`: digit layout, layer management, projection, and drawing
- `src/c/math_helper.[hc]`: vector and matrix helpers
- `src/c/poly_data.h`: static digit mesh data

## Test on an Emulator

Build the project first; this also generates the emulator configuration page:

```sh
pebble build
```

Then start the local configuration server once and leave it running in a separate terminal:

```sh
npm run serve:emulator-config
```

Install to the desired platform. For example, Pebble Time uses `basalt`:

```sh
pebble install --emulator basalt
```

Open the actual configuration flow in a browser with the Pebble SDK tool:

```sh
pebble emu-app-config --emulator basalt
```

Keep only one emulator configuration page open at a time.

Other useful platforms are `aplite`, `chalk`, `emery`, and `gabbro`. Stop the local configuration server with `Ctrl-C` when finished.

## Test on a Physical Watch

The default current workflow uses the Pebble mobile app's Dev Connect cloud relay. On the phone, open **Devices**, select the overflow menu, enable **Dev Connect**, and sign in with GitHub. Then sign in to the CLI with the same GitHub account:

```sh
pebble login
```

Build, install, and view logs:

```sh
pebble build
pebble install --cloudpebble --logs
pebble logs --cloudpebble
```

Pebble Time 2 uses the `emery` platform and is included in the generated bundle. Open FEZ's settings from the mobile app to test the normal Clay configuration page; this does not require `npm run serve:emulator-config`.

For the legacy local Wi-Fi developer connection, enable **Developer Connection** in the mobile app, note its Server IP, then use `pebble install --phone <ip>` and `pebble logs --phone <ip>`.

## Tested Platforms

- aplite
- basalt
- chalk
- diorite
- emery
- flint
- gabbro
