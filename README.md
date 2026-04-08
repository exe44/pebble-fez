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

## Install

Install to a Basalt emulator:

```sh
pebble install --emulator basalt
```

Install to a phone-connected watch target:

```sh
pebble install --phone <ip>
```

## Tested Platforms

- aplite
- basalt
- chalk
- diorite
- emery
- flint
- gabbro
