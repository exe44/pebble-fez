(function() {
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

  document.getElementById('slow').checked = get_param('slow', '0') === '1';
  document.getElementById('fg').value = get_param('fg', '1');
  document.getElementById('bg').value = get_param('bg', '0');

  document.getElementById('save').addEventListener('click', function() {
    var result = {
      SETTING_SLOW_VERSION: document.getElementById('slow').checked ? 1 : 0,
      SETTING_FG_COLOR: parseInt(document.getElementById('fg').value, 10),
      SETTING_BG_COLOR: parseInt(document.getElementById('bg').value, 10)
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
