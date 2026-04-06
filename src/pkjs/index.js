var Clay = require('@rebble/clay');
var messageKeys = require('message_keys');
var clayConfig = require('./config');
var buildEmulatorConfigUrl = require('./emulator-config');

var clay = new Clay(clayConfig, null, { autoHandleEvents: false });
var current_config_mode = 'clay';

function sanitize_settings(settings) {
  if (!isFinite(settings.SETTING_FG_COLOR)) {
    settings.SETTING_FG_COLOR = 0xFFFFFF;
  }

  if (!isFinite(settings.SETTING_BG_COLOR)) {
    settings.SETTING_BG_COLOR = 0x000000;
  }

  if (settings.SETTING_FG_COLOR === 0 || settings.SETTING_FG_COLOR === 1) {
    settings.SETTING_FG_COLOR = settings.SETTING_FG_COLOR === 0 ? 0x000000 : 0xFFFFFF;
  }

  if (settings.SETTING_BG_COLOR === 0 || settings.SETTING_BG_COLOR === 1) {
    settings.SETTING_BG_COLOR = settings.SETTING_BG_COLOR === 0 ? 0x000000 : 0xFFFFFF;
  }

  settings.SETTING_FG_COLOR = settings.SETTING_FG_COLOR & 0xFFFFFF;
  settings.SETTING_BG_COLOR = settings.SETTING_BG_COLOR & 0xFFFFFF;

  if (settings.SETTING_FG_COLOR === settings.SETTING_BG_COLOR) {
    settings.SETTING_BG_COLOR = settings.SETTING_FG_COLOR === 0x000000 ? 0xFFFFFF : 0x000000;
  }

  settings.SETTING_SLOW_VERSION = settings.SETTING_SLOW_VERSION ? 1 : 0;

  return settings;
}

function is_emulator() {
  return typeof Pebble === 'undefined' || Pebble.platform === 'pypkjs';
}

function get_emulator_palette_mode() {
  var watch_info = Pebble.getActiveWatchInfo && Pebble.getActiveWatchInfo();
  var platform = watch_info && watch_info.platform;
  var firmware = watch_info && watch_info.firmware;

  if (firmware && firmware.major === 2) {
    return 'bw';
  }

  if (platform === 'aplite' || platform === 'diorite' || platform === 'flint') {
    return 'bw';
  }

  return 'color';
}

function load_saved_settings() {
  var settings = {};

  try {
    settings = JSON.parse(localStorage.getItem('fez-settings')) || {};
  } catch (err) {
    console.log('Failed to parse saved settings', err);
  }

  return settings;
}

function save_settings(settings) {
  localStorage.setItem('fez-settings', JSON.stringify(settings));
}

function normalize_clay_settings(response) {
  var settings = clay.getSettings(response, false);

  return {
    SETTING_SLOW_VERSION: settings.SETTING_SLOW_VERSION.value ? 1 : 0,
    SETTING_FG_COLOR: parseInt(settings.SETTING_FG_COLOR.value, 10),
    SETTING_BG_COLOR: parseInt(settings.SETTING_BG_COLOR.value, 10)
  };
}

function normalize_emulator_settings(response) {
  var settings = JSON.parse(decodeURIComponent(response));

  return {
    SETTING_SLOW_VERSION: settings.SETTING_SLOW_VERSION ? 1 : 0,
    SETTING_FG_COLOR: parseInt(settings.SETTING_FG_COLOR, 10),
    SETTING_BG_COLOR: parseInt(settings.SETTING_BG_COLOR, 10)
  };
}

Pebble.addEventListener('showConfiguration', function() {
  var saved_settings = load_saved_settings();

  if (is_emulator()) {
    current_config_mode = 'emulator';
    Pebble.openURL(buildEmulatorConfigUrl(saved_settings, get_emulator_palette_mode()));
    return;
  }

  current_config_mode = 'clay';
  clay.setSettings(saved_settings);
  Pebble.openURL(clay.generateUrl());
});

Pebble.addEventListener('webviewclosed', function(e) {
  var settings;

  if (!e || !e.response) {
    return;
  }

  try {
    settings = current_config_mode === 'emulator' ?
      normalize_emulator_settings(e.response) :
      normalize_clay_settings(e.response);
  } catch (err) {
    console.log('Failed to parse config response', err);
    return;
  }

  settings = sanitize_settings(settings);
  save_settings(settings);

  Pebble.sendAppMessage({
    [messageKeys.SETTING_SLOW_VERSION]: settings.SETTING_SLOW_VERSION,
    [messageKeys.SETTING_FG_COLOR]: settings.SETTING_FG_COLOR,
    [messageKeys.SETTING_BG_COLOR]: settings.SETTING_BG_COLOR
  }, function() {
    console.log('Sent config data to Pebble');
  }, function(err) {
    console.log('Failed to send config data to Pebble');
    console.log(JSON.stringify(err));
  });
});
