var template = require('./emulator-config-template.auto');
var defaultSettings = require('./default-settings.auto');

function encode_value(value, fallback) {
  return encodeURIComponent(value !== undefined ? value : fallback);
}

module.exports = function build_emulator_config_url(settings, palette_mode) {
  var fallback_settings = defaultSettings[palette_mode] || defaultSettings.color;

  return 'data:text/html;charset=utf-8,' + encodeURIComponent(template) +
    '?slow=' + encode_value(settings.SETTING_SLOW_VERSION, fallback_settings.SETTING_SLOW_VERSION ? 1 : 0) +
    '&bg=' + encode_value(settings.SETTING_BG_COLOR, fallback_settings.SETTING_BG_COLOR) +
    '&face=' + encode_value(settings.SETTING_FACE_COLOR, fallback_settings.SETTING_FACE_COLOR) +
    '&line=' + encode_value(settings.SETTING_LINE_COLOR, fallback_settings.SETTING_LINE_COLOR) +
    '&faceMix=' + encode_value(settings.SETTING_FACE_MIX_WITH_BACKGROUND, fallback_settings.SETTING_FACE_MIX_WITH_BACKGROUND ? 1 : 0) +
    '&lineMix=' + encode_value(settings.SETTING_LINE_MIX_WITH_BACKGROUND, fallback_settings.SETTING_LINE_MIX_WITH_BACKGROUND ? 1 : 0) +
    '&splitLine=' + encode_value(settings.SETTING_SPLIT_LINE_COLORS, fallback_settings.SETTING_SPLIT_LINE_COLORS ? 1 : 0) +
    '&backLine=' + encode_value(settings.SETTING_BACK_LINE_COLOR, fallback_settings.SETTING_BACK_LINE_COLOR) +
    '&sideLine=' + encode_value(settings.SETTING_SIDE_LINE_COLOR, fallback_settings.SETTING_SIDE_LINE_COLOR) +
    '&palette=' + encode_value(palette_mode, 'color');
};
