var Clay = require('@rebble/clay');
var clayConfig = require('./config');
var customClay = require('./custom-clay');
var defaultSettings = require('./default-settings.auto');
var buildEmulatorConfigUrl = require('./emulator-config');

var clay = new Clay(clayConfig, customClay, { autoHandleEvents: false });
var current_config_mode = 'clay';
var MESSAGE_KEYS = {
  SETTING_SLOW_VERSION: 0,
  SETTING_BG_COLOR: 1,
  SETTING_FACE_COLOR: 2,
  SETTING_LINE_COLOR: 3,
  SETTING_FACE_MIX_WITH_BACKGROUND: 4,
  SETTING_LINE_MIX_WITH_BACKGROUND: 5,
  SETTING_SPLIT_LINE_COLORS: 6,
  SETTING_BACK_LINE_COLOR: 7,
  SETTING_SIDE_LINE_COLOR: 8
};

function get_platform_palette_mode() {
  if (typeof Pebble === 'undefined') {
    return 'color';
  }

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

function clone_settings(settings) {
  return {
    SETTING_SLOW_VERSION: settings.SETTING_SLOW_VERSION,
    SETTING_BG_COLOR: settings.SETTING_BG_COLOR,
    SETTING_FACE_COLOR: settings.SETTING_FACE_COLOR,
    SETTING_LINE_COLOR: settings.SETTING_LINE_COLOR,
    SETTING_FACE_MIX_WITH_BACKGROUND: settings.SETTING_FACE_MIX_WITH_BACKGROUND,
    SETTING_LINE_MIX_WITH_BACKGROUND: settings.SETTING_LINE_MIX_WITH_BACKGROUND,
    SETTING_SPLIT_LINE_COLORS: settings.SETTING_SPLIT_LINE_COLORS,
    SETTING_BACK_LINE_COLOR: settings.SETTING_BACK_LINE_COLOR,
    SETTING_SIDE_LINE_COLOR: settings.SETTING_SIDE_LINE_COLOR
  };
}

function get_default_settings(palette_mode) {
  return clone_settings(defaultSettings[palette_mode] || defaultSettings.color);
}

function normalize_setting_value(setting, fallback) {
  if (setting && setting.value !== undefined) {
    return setting.value;
  }

  return fallback;
}

function sanitize_settings(settings, fallback_settings, palette_mode) {
  var mode = palette_mode || get_platform_palette_mode();
  var fallback = fallback_settings || get_default_settings(mode);

  if (settings.SETTING_SLOW_VERSION === undefined || settings.SETTING_SLOW_VERSION === null) {
    settings.SETTING_SLOW_VERSION = fallback.SETTING_SLOW_VERSION;
  }

  if (settings.SETTING_FACE_MIX_WITH_BACKGROUND === undefined || settings.SETTING_FACE_MIX_WITH_BACKGROUND === null) {
    settings.SETTING_FACE_MIX_WITH_BACKGROUND = fallback.SETTING_FACE_MIX_WITH_BACKGROUND;
  }

  if (settings.SETTING_LINE_MIX_WITH_BACKGROUND === undefined || settings.SETTING_LINE_MIX_WITH_BACKGROUND === null) {
    settings.SETTING_LINE_MIX_WITH_BACKGROUND = fallback.SETTING_LINE_MIX_WITH_BACKGROUND;
  }

  if (settings.SETTING_SPLIT_LINE_COLORS === undefined || settings.SETTING_SPLIT_LINE_COLORS === null) {
    settings.SETTING_SPLIT_LINE_COLORS = fallback.SETTING_SPLIT_LINE_COLORS;
  }

  if (!isFinite(settings.SETTING_BG_COLOR)) {
    settings.SETTING_BG_COLOR = fallback.SETTING_BG_COLOR;
  }

  if (!isFinite(settings.SETTING_FACE_COLOR)) {
    settings.SETTING_FACE_COLOR = fallback.SETTING_FACE_COLOR;
  }

  if (settings.SETTING_BG_COLOR === 0 || settings.SETTING_BG_COLOR === 1) {
    settings.SETTING_BG_COLOR = settings.SETTING_BG_COLOR === 0 ? 0x000000 : 0xFFFFFF;
  }

  if (settings.SETTING_FACE_COLOR === 0 || settings.SETTING_FACE_COLOR === 1) {
    settings.SETTING_FACE_COLOR = settings.SETTING_FACE_COLOR === 0 ? 0x000000 : 0xFFFFFF;
  }

  settings.SETTING_BG_COLOR = settings.SETTING_BG_COLOR & 0xFFFFFF;
  settings.SETTING_FACE_COLOR = settings.SETTING_FACE_COLOR & 0xFFFFFF;

  settings.SETTING_SLOW_VERSION = settings.SETTING_SLOW_VERSION ? 1 : 0;
  settings.SETTING_FACE_MIX_WITH_BACKGROUND = settings.SETTING_FACE_MIX_WITH_BACKGROUND ? 1 : 0;
  settings.SETTING_LINE_MIX_WITH_BACKGROUND = settings.SETTING_LINE_MIX_WITH_BACKGROUND ? 1 : 0;
  if (!isFinite(settings.SETTING_LINE_COLOR)) {
    settings.SETTING_LINE_COLOR = fallback.SETTING_LINE_COLOR;
  }

  if (!isFinite(settings.SETTING_BACK_LINE_COLOR)) {
    settings.SETTING_BACK_LINE_COLOR = fallback.SETTING_BACK_LINE_COLOR;
  }

  if (!isFinite(settings.SETTING_SIDE_LINE_COLOR)) {
    settings.SETTING_SIDE_LINE_COLOR = fallback.SETTING_SIDE_LINE_COLOR;
  }

  settings.SETTING_LINE_COLOR = settings.SETTING_LINE_COLOR & 0xFFFFFF;
  settings.SETTING_BACK_LINE_COLOR = settings.SETTING_BACK_LINE_COLOR & 0xFFFFFF;
  settings.SETTING_SIDE_LINE_COLOR = settings.SETTING_SIDE_LINE_COLOR & 0xFFFFFF;
  settings.SETTING_SPLIT_LINE_COLORS = settings.SETTING_SPLIT_LINE_COLORS ? 1 : 0;

  if (mode === 'bw') {
    settings.SETTING_FACE_MIX_WITH_BACKGROUND = 0;
    settings.SETTING_LINE_MIX_WITH_BACKGROUND = 0;
    settings.SETTING_SPLIT_LINE_COLORS = 0;
    settings.SETTING_BACK_LINE_COLOR = settings.SETTING_LINE_COLOR;
    settings.SETTING_SIDE_LINE_COLOR = settings.SETTING_LINE_COLOR;
  }

  return settings;
}

function is_emulator() {
  return typeof Pebble === 'undefined' || Pebble.platform === 'pypkjs';
}

function get_emulator_palette_mode() {
  return get_platform_palette_mode();
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
  var palette_mode = get_platform_palette_mode();
  var settings = clay.getSettings(response, false);
  var fallback_settings = get_default_settings(palette_mode);

  return {
    SETTING_SLOW_VERSION: normalize_setting_value(settings.SETTING_SLOW_VERSION, fallback_settings.SETTING_SLOW_VERSION) ? 1 : 0,
    SETTING_BG_COLOR: parseInt(normalize_setting_value(settings.SETTING_BG_COLOR, fallback_settings.SETTING_BG_COLOR), 10),
    SETTING_FACE_COLOR: parseInt(normalize_setting_value(settings.SETTING_FACE_COLOR, fallback_settings.SETTING_FACE_COLOR), 10),
    SETTING_LINE_COLOR: parseInt(normalize_setting_value(settings.SETTING_LINE_COLOR, fallback_settings.SETTING_LINE_COLOR), 10),
    SETTING_FACE_MIX_WITH_BACKGROUND: normalize_setting_value(settings.SETTING_FACE_MIX_WITH_BACKGROUND, fallback_settings.SETTING_FACE_MIX_WITH_BACKGROUND) ? 1 : 0,
    SETTING_LINE_MIX_WITH_BACKGROUND: normalize_setting_value(settings.SETTING_LINE_MIX_WITH_BACKGROUND, fallback_settings.SETTING_LINE_MIX_WITH_BACKGROUND) ? 1 : 0,
    SETTING_SPLIT_LINE_COLORS: normalize_setting_value(settings.SETTING_SPLIT_LINE_COLORS, fallback_settings.SETTING_SPLIT_LINE_COLORS) ? 1 : 0,
    SETTING_BACK_LINE_COLOR: parseInt(normalize_setting_value(settings.SETTING_BACK_LINE_COLOR, fallback_settings.SETTING_BACK_LINE_COLOR), 10),
    SETTING_SIDE_LINE_COLOR: parseInt(normalize_setting_value(settings.SETTING_SIDE_LINE_COLOR, fallback_settings.SETTING_SIDE_LINE_COLOR), 10)
  };
}

function normalize_emulator_settings(response) {
  var settings = JSON.parse(decodeURIComponent(response));

  return {
    SETTING_SLOW_VERSION: settings.SETTING_SLOW_VERSION ? 1 : 0,
    SETTING_BG_COLOR: parseInt(settings.SETTING_BG_COLOR, 10),
    SETTING_FACE_COLOR: parseInt(settings.SETTING_FACE_COLOR, 10),
    SETTING_LINE_COLOR: parseInt(settings.SETTING_LINE_COLOR, 10),
    SETTING_FACE_MIX_WITH_BACKGROUND: settings.SETTING_FACE_MIX_WITH_BACKGROUND ? 1 : 0,
    SETTING_LINE_MIX_WITH_BACKGROUND: settings.SETTING_LINE_MIX_WITH_BACKGROUND ? 1 : 0,
    SETTING_SPLIT_LINE_COLORS: settings.SETTING_SPLIT_LINE_COLORS ? 1 : 0,
    SETTING_BACK_LINE_COLOR: parseInt(settings.SETTING_BACK_LINE_COLOR, 10),
    SETTING_SIDE_LINE_COLOR: parseInt(settings.SETTING_SIDE_LINE_COLOR, 10)
  };
}

Pebble.addEventListener('showConfiguration', function() {
  var palette_mode = get_platform_palette_mode();
  var fallback_settings = get_default_settings(palette_mode);
  var saved_settings = load_saved_settings();
  var initial_settings = sanitize_settings(saved_settings, fallback_settings, palette_mode);

  if (is_emulator()) {
    current_config_mode = 'emulator';
    Pebble.openURL(buildEmulatorConfigUrl(initial_settings, palette_mode));
    return;
  }

  current_config_mode = 'clay';
  clay.setSettings(initial_settings);
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

  settings = sanitize_settings(settings, null, get_platform_palette_mode());
  save_settings(settings);

  Pebble.sendAppMessage({
    [MESSAGE_KEYS.SETTING_SLOW_VERSION]: settings.SETTING_SLOW_VERSION,
    [MESSAGE_KEYS.SETTING_BG_COLOR]: settings.SETTING_BG_COLOR,
    [MESSAGE_KEYS.SETTING_FACE_COLOR]: settings.SETTING_FACE_COLOR,
    [MESSAGE_KEYS.SETTING_LINE_COLOR]: settings.SETTING_LINE_COLOR,
    [MESSAGE_KEYS.SETTING_FACE_MIX_WITH_BACKGROUND]: settings.SETTING_FACE_MIX_WITH_BACKGROUND,
    [MESSAGE_KEYS.SETTING_LINE_MIX_WITH_BACKGROUND]: settings.SETTING_LINE_MIX_WITH_BACKGROUND,
    [MESSAGE_KEYS.SETTING_SPLIT_LINE_COLORS]: settings.SETTING_SPLIT_LINE_COLORS,
    [MESSAGE_KEYS.SETTING_BACK_LINE_COLOR]: settings.SETTING_BACK_LINE_COLOR,
    [MESSAGE_KEYS.SETTING_SIDE_LINE_COLOR]: settings.SETTING_SIDE_LINE_COLOR
  }, function() {
    console.log('Sent config data to Pebble');
  }, function(err) {
    console.log('Failed to send config data to Pebble');
    console.log(JSON.stringify(err));
  });
});
