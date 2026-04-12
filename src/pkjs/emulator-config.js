var template = require('./emulator-config-template.auto');

function encode_value(value, fallback) {
  return encodeURIComponent(value !== undefined ? value : fallback);
}

module.exports = function build_emulator_config_url(settings, palette_mode) {
  return 'data:text/html;charset=utf-8,' + encodeURIComponent(template) +
    '?slow=' + encode_value(settings.SETTING_SLOW_VERSION, 0) +
    '&bg=' + encode_value(settings.SETTING_BG_COLOR, 0x000000) +
    '&face=' + encode_value(settings.SETTING_FACE_COLOR, 0xFFAA00) +
    '&line=' + encode_value(settings.SETTING_LINE_COLOR, 0xFFFFFF) +
    '&faceMix=' + encode_value(settings.SETTING_FACE_MIX_WITH_BACKGROUND, 0) +
    '&lineMix=' + encode_value(settings.SETTING_LINE_MIX_WITH_BACKGROUND, 0) +
    '&splitLine=' + encode_value(settings.SETTING_SPLIT_LINE_COLORS, 0) +
    '&backLine=' + encode_value(settings.SETTING_BACK_LINE_COLOR, settings.SETTING_LINE_COLOR || 0xFFFFFF) +
    '&sideLine=' + encode_value(settings.SETTING_SIDE_LINE_COLOR, settings.SETTING_LINE_COLOR || 0xFFFFFF) +
    '&palette=' + encode_value(palette_mode, 'color');
};
