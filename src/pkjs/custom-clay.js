module.exports = function(minified) {
  var clayConfig = this;
  var bw_platforms = ['aplite', 'diorite', 'flint'];
  var palettes = {
    color: [
      '55ff00', 'aaff55', 'ffff55', 'ffffaa',
      'aaffaa', '55ff55', '00ff00', 'aaff00', 'ffff00', 'ffaa55', 'ffaaaa',
      '55ffaa', '00ff55', '00aa00', '55aa00', 'aaaa55', 'aaaa00', 'ffaa00', 'ff5500', 'ff5555',
      'aaffff', '00ffaa', '00aa55', '55aa55', '005500', '555500', 'aa5500', 'ff0000', 'ff0055',
      '55aaaa', '00aaaa', '005555', 'ffffff', '000000', 'aa5555', 'aa0000',
      '55ffff', '00ffff', '00aaff', '0055aa', 'aaaaaa', '555555', '550000', 'aa0055', 'ff55aa',
      '55aaff', '0055ff', '0000ff', '0000aa', '000055', '550055', 'aa00aa', 'ff00aa', 'ffaaff',
      '5555aa', '5555ff', '5500ff', '5500aa', 'aa00ff', 'ff00ff', 'ff55ff',
      'aaaaff', 'aa55ff', 'aa55aa'
    ],
    bw: ['000000', 'aaaaaa', 'ffffff']
  };

  function get_palette() {
    var watch_info = clayConfig.meta.activeWatchInfo;
    var firmware = watch_info && watch_info.firmware;
    var platform = watch_info && watch_info.platform;

    if ((firmware && firmware.major === 2) || bw_platforms.indexOf(platform) !== -1) {
      return palettes.bw;
    }

    return palettes.color;
  }

  function pick_random_color() {
    var palette = get_palette();
    var index = Math.floor(Math.random() * palette.length);

    return parseInt(palette[index], 16);
  }

  function sync_split_line_color_fields() {
    var enabled = this.get();
    var back = clayConfig.getItemByMessageKey('SETTING_BACK_LINE_COLOR');
    var side = clayConfig.getItemByMessageKey('SETTING_SIDE_LINE_COLOR');

    if (enabled) {
      back.show();
      back.enable();
      side.show();
      side.enable();
      return;
    }

    back.hide();
    back.disable();
    side.hide();
    side.disable();
  }

  function randomize_colors() {
    var targets = clayConfig.getItemById('randomize-targets').get();
    var split_toggle = clayConfig.getItemByMessageKey('SETTING_SPLIT_LINE_COLORS');
    var should_randomize_bg = !!targets[0];
    var should_randomize_face = !!targets[1];
    var should_randomize_line = !!targets[2];
    var line_color;

    if (should_randomize_bg) {
      clayConfig.getItemByMessageKey('SETTING_BG_COLOR').set(pick_random_color());
    }

    if (should_randomize_face) {
      clayConfig.getItemByMessageKey('SETTING_FACE_COLOR').set(pick_random_color());
    }

    if (!should_randomize_line) {
      return;
    }

    line_color = pick_random_color();
    clayConfig.getItemByMessageKey('SETTING_LINE_COLOR').set(line_color);

    if (split_toggle.get()) {
      clayConfig.getItemByMessageKey('SETTING_BACK_LINE_COLOR').set(pick_random_color());
      clayConfig.getItemByMessageKey('SETTING_SIDE_LINE_COLOR').set(pick_random_color());
      return;
    }

    clayConfig.getItemByMessageKey('SETTING_BACK_LINE_COLOR').set(line_color);
    clayConfig.getItemByMessageKey('SETTING_SIDE_LINE_COLOR').set(line_color);
  }

  clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
    var split_toggle = clayConfig.getItemByMessageKey('SETTING_SPLIT_LINE_COLORS');
    var randomize_button = clayConfig.getItemById('randomize-colors');

    sync_split_line_color_fields.call(split_toggle);
    split_toggle.on('change', sync_split_line_color_fields);
    randomize_button.$element.select('button').on('click', randomize_colors);
  });
};
