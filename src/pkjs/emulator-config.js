var template = require('./emulator-config-template.auto');

function encode_value(value, fallback) {
  return encodeURIComponent(value !== undefined ? value : fallback);
}

module.exports = function build_emulator_config_url(settings, palette_mode) {
  return 'data:text/html;charset=utf-8,' + encodeURIComponent(template) +
    '?slow=' + encode_value(settings.SETTING_SLOW_VERSION, 0) +
    '&fg=' + encode_value(settings.SETTING_FG_COLOR, 0xFFFFFF) +
    '&bg=' + encode_value(settings.SETTING_BG_COLOR, 0x000000) +
    '&accent=' + encode_value(settings.SETTING_ACCENT_COLOR, 0xFFAA00) +
    '&line=' + encode_value(settings.SETTING_LINE_COLOR_MODE, 0) +
    '&face=' + encode_value(settings.SETTING_FACE_COLOR_MODE, 2) +
    '&palette=' + encode_value(palette_mode, 'color');
};
