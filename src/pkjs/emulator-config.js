var defaultSettings = require('./default-settings.auto');

function encode_value(value, fallback) {
  return encodeURIComponent(value !== undefined ? value : fallback);
}

module.exports = function build_emulator_config_url(settings, palette_mode) {
  var fallback_settings = defaultSettings[palette_mode] || defaultSettings.color;

  return 'http://localhost:8765/emulator-config.html' +
    '?slow=' + encode_value(settings.SETTING_SLOW_VERSION, fallback_settings.SETTING_SLOW_VERSION ? 1 : 0) +
    '&bg=' + encode_value(settings.SETTING_BG_COLOR, fallback_settings.SETTING_BG_COLOR) +
    '&face=' + encode_value(settings.SETTING_FACE_COLOR, fallback_settings.SETTING_FACE_COLOR) +
    '&splitFace=' + encode_value(settings.SETTING_SPLIT_FACE_COLORS, fallback_settings.SETTING_SPLIT_FACE_COLORS ? 1 : 0) +
    '&face1=' + encode_value(settings.SETTING_FACE_COLOR_1, fallback_settings.SETTING_FACE_COLOR_1) +
    '&face2=' + encode_value(settings.SETTING_FACE_COLOR_2, fallback_settings.SETTING_FACE_COLOR_2) +
    '&face3=' + encode_value(settings.SETTING_FACE_COLOR_3, fallback_settings.SETTING_FACE_COLOR_3) +
    '&face4=' + encode_value(settings.SETTING_FACE_COLOR_4, fallback_settings.SETTING_FACE_COLOR_4) +
    '&line=' + encode_value(settings.SETTING_LINE_COLOR, fallback_settings.SETTING_LINE_COLOR) +
    '&faceMix=' + encode_value(settings.SETTING_FACE_MIX_WITH_BACKGROUND, fallback_settings.SETTING_FACE_MIX_WITH_BACKGROUND ? 1 : 0) +
    '&lineMix=' + encode_value(settings.SETTING_LINE_MIX_WITH_BACKGROUND, fallback_settings.SETTING_LINE_MIX_WITH_BACKGROUND ? 1 : 0) +
    '&splitLine=' + encode_value(settings.SETTING_SPLIT_LINE_COLORS, fallback_settings.SETTING_SPLIT_LINE_COLORS ? 1 : 0) +
    '&backLine=' + encode_value(settings.SETTING_BACK_LINE_COLOR, fallback_settings.SETTING_BACK_LINE_COLOR) +
    '&sideLine=' + encode_value(settings.SETTING_SIDE_LINE_COLOR, fallback_settings.SETTING_SIDE_LINE_COLOR) +
    '&palette=' + encode_value(palette_mode, 'color');
};
