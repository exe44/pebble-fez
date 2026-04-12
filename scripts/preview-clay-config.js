#!/usr/bin/env node

const fs = require('fs');
const path = require('path');
const Module = require('module');

const repoRoot = path.resolve(__dirname, '..');
const bwPlatforms = new Set(['aplite', 'diorite', 'flint']);

function getArg(name, fallback) {
  const prefix = `--${name}=`;
  const exactIndex = process.argv.indexOf(`--${name}`);

  if (exactIndex !== -1 && exactIndex + 1 < process.argv.length) {
    return process.argv[exactIndex + 1];
  }

  const inline = process.argv.find((arg) => arg.indexOf(prefix) === 0);
  if (inline) {
    return inline.slice(prefix.length);
  }

  return fallback;
}

function buildWatchInfo(platform) {
  return {
    platform: platform,
    firmware: {
      major: bwPlatforms.has(platform) ? 2 : 3,
      minor: 0
    }
  };
}

function loadSavedSettings(settingsPath) {
  if (!settingsPath) {
    return {};
  }

  try {
    return JSON.parse(fs.readFileSync(settingsPath, 'utf8'));
  } catch (error) {
    console.error(`Failed to read settings from ${settingsPath}`);
    console.error(error.message);
    process.exit(1);
  }
}

function main() {
  const platform = getArg('platform', 'basalt');
  const outputPath = path.resolve(repoRoot, getArg('out', `build/clay-preview-${platform}.html`));
  const settingsPathArg = getArg('settings', '');
  const settingsPath = settingsPathArg ? path.resolve(repoRoot, settingsPathArg) : '';
  const savedSettings = loadSavedSettings(settingsPath);
  const originalLoad = Module._load;

  Module._load = function(request, parent, isMain) {
    if (request === 'message_keys') {
      return {};
    }

    return originalLoad.apply(this, arguments);
  };

  global.Pebble = {
    platform: 'pypkjs',
    getActiveWatchInfo: function() {
      return buildWatchInfo(platform);
    },
    getAccountToken: function() {
      return '';
    },
    getWatchToken: function() {
      return '';
    },
    addEventListener: function() {}
  };

  global.localStorage = {
    getItem: function(key) {
      if (key === 'clay-settings') {
        return JSON.stringify(savedSettings);
      }

      return null;
    },
    setItem: function() {}
  };

  const Clay = require(path.join(repoRoot, 'node_modules/@rebble/clay/dist/js/index.js'));
  const clayConfig = require(path.join(repoRoot, 'src/pkjs/config'));
  const customClay = require(path.join(repoRoot, 'src/pkjs/custom-clay'));
  const clay = new Clay(clayConfig, customClay, { autoHandleEvents: false });
  const activeWatchInfo = buildWatchInfo(platform);

  clay.meta = {
    activeWatchInfo: activeWatchInfo,
    accountToken: '',
    watchToken: '',
    userData: {}
  };
  const previewUrl = clay.generateUrl();
  const hashIndex = previewUrl.indexOf('#');

  if (hashIndex === -1) {
    console.error('Failed to generate Clay preview HTML');
    process.exit(1);
  }

  fs.mkdirSync(path.dirname(outputPath), { recursive: true });
  fs.writeFileSync(outputPath, decodeURIComponent(previewUrl.slice(hashIndex + 1)));

  console.log(`Clay preview generated for ${platform}`);
  console.log(outputPath);
}

main();
