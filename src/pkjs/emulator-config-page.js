(function() {
  var current_bg;
  var current_face;
  var current_line;
  var current_back_line;
  var current_side_line;
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

  function hex_to_lab(hex) {
    var r = parseInt(hex.slice(0, 2), 16) / 255;
    var g = parseInt(hex.slice(2, 4), 16) / 255;
    var b = parseInt(hex.slice(4, 6), 16) / 255;

    r = (r > 0.04045) ? Math.pow((r + 0.055) / 1.055, 2.4) : r / 12.92;
    g = (g > 0.04045) ? Math.pow((g + 0.055) / 1.055, 2.4) : g / 12.92;
    b = (b > 0.04045) ? Math.pow((b + 0.055) / 1.055, 2.4) : b / 12.92;

    var x = (r * 0.4124 + g * 0.3576 + b * 0.1805) / 0.95047;
    var y = (r * 0.2126 + g * 0.7152 + b * 0.0722) / 1.00000;
    var z = (r * 0.0193 + g * 0.1192 + b * 0.9505) / 1.08883;

    x = (x > 0.008856) ? Math.pow(x, 1 / 3) : (7.787 * x) + 16 / 116;
    y = (y > 0.008856) ? Math.pow(y, 1 / 3) : (7.787 * y) + 16 / 116;
    z = (z > 0.008856) ? Math.pow(z, 1 / 3) : (7.787 * z) + 16 / 116;

    return [(116 * y) - 16, 500 * (x - y), 200 * (y - z)];
  }

  function color_distance(left, right) {
    var left_lab = hex_to_lab(left);
    var right_lab = hex_to_lab(right);
    var dl = left_lab[0] - right_lab[0];
    var da = left_lab[1] - right_lab[1];
    var db = left_lab[2] - right_lab[2];

    return Math.sqrt((dl * dl) + (da * da) + (db * db));
  }

  function normalize_hex(value, fallback) {
    var raw = get_param(value, fallback);
    var parsed = parseInt(raw, 10);
    var normalized;

    if (isNaN(parsed)) {
      normalized = String(raw).replace(/^#|^0x/i, '').toLowerCase();
    } else {
      normalized = parsed.toString(16).toLowerCase();
    }

    while (normalized.length < 6) {
      normalized = '0' + normalized;
    }

    return normalized.slice(-6);
  }

  function get_param(name, fallback) {
    var matches = new RegExp('[?&]' + name + '=([^&]+)').exec(window.location.search);

    if (!matches) {
      return fallback;
    }

    return decodeURIComponent(matches[1]);
  }

  function get_return_to() {
    var matches = /[?&]return_to=([^&]+)/.exec(window.location.search);

    if (!matches) {
      return null;
    }

    return decodeURIComponent(matches[1]);
  }

  function render_palette(container_id, selected, on_select) {
    var palette_mode = get_param('palette', 'color');
    var colors = palettes[palette_mode] || palettes.color;
    var container = document.getElementById(container_id);

    container.className = colors.length <= 3 ? 'palette compact' : 'palette';
    container.innerHTML = '';

    colors.forEach(function(color) {
      var button = document.createElement('button');
      button.type = 'button';
      button.className = 'palette-button' + (color === selected ? ' selected' : '');
      button.style.setProperty('--swatch', '#' + color);
      button.title = '#' + color;
      button.setAttribute('aria-label', '#' + color);
      button.addEventListener('click', function() {
        on_select(color);
      });
      container.appendChild(button);
    });
  }

  function round_to_palette(color) {
    var palette_mode = get_param('palette', 'color');
    var colors = palettes[palette_mode] || palettes.color;
    var closest = colors[0];
    var closest_distance = color_distance(color, closest);

    colors.forEach(function(candidate) {
      var distance = color_distance(color, candidate);

      if (distance < closest_distance) {
        closest = candidate;
        closest_distance = distance;
      }
    });

    return closest;
  }

  function pick_random_color() {
    var palette_mode = get_param('palette', 'color');
    var colors = palettes[palette_mode] || palettes.color;
    var index = Math.floor(Math.random() * colors.length);

    return colors[index];
  }

  function is_bw_mode() {
    return get_param('palette', 'color') === 'bw';
  }

  function reset_bw_advanced_colors() {
    document.getElementById('face-mix').checked = false;
    document.getElementById('line-mix').checked = false;
    document.getElementById('split-line-colors').checked = false;
    current_back_line = current_line;
    current_side_line = current_line;
  }

  function sync_visible_sections() {
    var hide_advanced_colors = is_bw_mode();

    if (hide_advanced_colors) {
      reset_bw_advanced_colors();
    }

    document.getElementById('face-mix-row').className =
      hide_advanced_colors ? 'hidden' : '';
    document.getElementById('line-mix-row').className =
      hide_advanced_colors ? 'hidden' : '';
    document.getElementById('split-line-row').className =
      hide_advanced_colors ? 'hidden' : '';
    document.getElementById('randomize-card').className =
      hide_advanced_colors ? 'card hidden' : 'card';

    if (hide_advanced_colors) {
      document.getElementById('back-line-card').className = 'line-subsection hidden';
      document.getElementById('side-line-card').className = 'line-subsection hidden';
      return;
    }

    document.getElementById('back-line-card').className =
      document.getElementById('split-line-colors').checked ? 'line-subsection' : 'line-subsection hidden';
    document.getElementById('side-line-card').className =
      document.getElementById('split-line-colors').checked ? 'line-subsection' : 'line-subsection hidden';
  }

  function randomize_colors() {
    if (document.getElementById('randomize-bg').checked) {
      current_bg = pick_random_color();
    }

    if (document.getElementById('randomize-face').checked) {
      current_face = pick_random_color();
    }

    if (!document.getElementById('randomize-line').checked) {
      repaint();
      return;
    }

    current_line = pick_random_color();

    if (document.getElementById('split-line-colors').checked) {
      current_back_line = pick_random_color();
      current_side_line = pick_random_color();
    } else {
      current_back_line = current_line;
      current_side_line = current_line;
    }

    repaint();
  }

  function repaint() {
    render_palette('bg-palette', current_bg, function(color) {
      current_bg = color;
      repaint();
    });

    render_palette('face-palette', current_face, function(color) {
      current_face = color;
      repaint();
    });

    render_palette('line-palette', current_line, function(color) {
      current_line = color;
      repaint();
    });

    render_palette('back-line-palette', current_back_line, function(color) {
      current_back_line = color;
      repaint();
    });

    render_palette('side-line-palette', current_side_line, function(color) {
      current_side_line = color;
      repaint();
    });

    sync_visible_sections();
  }

  document.getElementById('slow').checked = get_param('slow', '0') === '1';
  current_bg = round_to_palette(normalize_hex('bg', '000000'));
  current_face = round_to_palette(normalize_hex('face', 'ffaa00'));
  current_line = round_to_palette(normalize_hex('line', 'ffffff'));
  current_back_line = round_to_palette(normalize_hex('backLine', current_line));
  current_side_line = round_to_palette(normalize_hex('sideLine', current_line));
  document.getElementById('face-mix').checked = get_param('faceMix', '0') === '1';
  document.getElementById('line-mix').checked = get_param('lineMix', '0') === '1';
  document.getElementById('split-line-colors').checked = get_param('splitLine', '0') === '1';
  document.getElementById('randomize-bg').checked = false;
  document.getElementById('randomize-face').checked = true;
  document.getElementById('randomize-line').checked = true;
  document.getElementById('split-line-colors').addEventListener('change', repaint);
  document.getElementById('randomize-colors').addEventListener('click', randomize_colors);
  repaint();

  document.getElementById('save').addEventListener('click', function() {
    var result = {
      SETTING_SLOW_VERSION: document.getElementById('slow').checked ? 1 : 0,
      SETTING_BG_COLOR: parseInt(current_bg, 16),
      SETTING_FACE_COLOR: parseInt(current_face, 16),
      SETTING_LINE_COLOR: parseInt(current_line, 16),
      SETTING_FACE_MIX_WITH_BACKGROUND: document.getElementById('face-mix').checked ? 1 : 0,
      SETTING_LINE_MIX_WITH_BACKGROUND: document.getElementById('line-mix').checked ? 1 : 0,
      SETTING_SPLIT_LINE_COLORS: document.getElementById('split-line-colors').checked ? 1 : 0,
      SETTING_BACK_LINE_COLOR: parseInt(current_back_line, 16),
      SETTING_SIDE_LINE_COLOR: parseInt(current_side_line, 16)
    };
    var return_to = get_return_to();
    var response = encodeURIComponent(JSON.stringify(result));

    if (return_to) {
      document.location = return_to + response;
      return;
    }

    document.location = 'pebblejs://close#' + response;
  });
}());
