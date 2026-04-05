# pebble-fez

Pebble watch face inspired by the logo of wonderful game "[FEZ](http://fezgame.com/)"!

![pebble-fez](https://user-images.githubusercontent.com/158320/177231014-a57d8be7-8365-4556-a1a7-4eac3cb373e1.jpg)

https://user-images.githubusercontent.com/158320/177231264-657d05bc-3842-4868-8295-6872371aaf27.mp4

[GitHub Releases](https://github.com/exe44/pebble-fez/releases)

## Build

This project now uses the modern Pebble project layout with `package.json` and `wscript`.

```sh
pebble build
```

The compiled bundle will be generated at `build/pebble-fez.pbw`.

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
