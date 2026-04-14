#!/usr/bin/env node

const fs = require('fs');
const path = require('path');

const repoRoot = path.resolve(__dirname, '..');
const sourcePath = path.join(repoRoot, 'config', 'default-settings.json');
const jsOutputPath = path.join(repoRoot, 'src', 'pkjs', 'default-settings.auto.js');
const cOutputPath = path.join(repoRoot, 'src', 'c', 'app_settings_defaults.auto.h');
const SETTING_KEYS = [
  'SETTING_SLOW_VERSION',
  'SETTING_BG_COLOR',
  'SETTING_FACE_COLOR',
  'SETTING_FACE_MIX_WITH_BACKGROUND',
  'SETTING_LINE_COLOR',
  'SETTING_LINE_MIX_WITH_BACKGROUND',
  'SETTING_SPLIT_LINE_COLORS',
  'SETTING_BACK_LINE_COLOR',
  'SETTING_SIDE_LINE_COLOR'
];

function loadSource() {
  return JSON.parse(fs.readFileSync(sourcePath, 'utf8'));
}

function parseBool(value) {
  return value ? 'true' : 'false';
}

function parseColor(value) {
  return parseInt(String(value).replace(/^#|^0x/i, ''), 16) & 0xFFFFFF;
}

function normalizeSettings(raw) {
  return {
    SETTING_SLOW_VERSION: !!raw.SETTING_SLOW_VERSION,
    SETTING_BG_COLOR: parseColor(raw.SETTING_BG_COLOR),
    SETTING_FACE_COLOR: parseColor(raw.SETTING_FACE_COLOR),
    SETTING_FACE_MIX_WITH_BACKGROUND: !!raw.SETTING_FACE_MIX_WITH_BACKGROUND,
    SETTING_LINE_COLOR: parseColor(raw.SETTING_LINE_COLOR),
    SETTING_LINE_MIX_WITH_BACKGROUND: !!raw.SETTING_LINE_MIX_WITH_BACKGROUND,
    SETTING_SPLIT_LINE_COLORS: !!raw.SETTING_SPLIT_LINE_COLORS,
    SETTING_BACK_LINE_COLOR: parseColor(raw.SETTING_BACK_LINE_COLOR),
    SETTING_SIDE_LINE_COLOR: parseColor(raw.SETTING_SIDE_LINE_COLOR)
  };
}

function formatJsValue(value) {
  if (typeof value === 'boolean') {
    return value ? 'true' : 'false';
  }

  return `0x${value.toString(16).padStart(6, '0').toUpperCase()}`;
}

function buildJsProfile(name, settings) {
  const lines = SETTING_KEYS.map((key) => `    ${key}: ${formatJsValue(settings[key])}`);

  return `  ${name}: {\n${lines.join(',\n')}\n  }`;
}

function buildJs(profiles) {
  return `module.exports = {\n` +
    `${buildJsProfile('color', profiles.color)},\n` +
    `${buildJsProfile('bw', profiles.bw)}\n` +
    `};\n`;
}

function buildHeaderProfile(name, settings) {
  return SETTING_KEYS.map((key) => {
    const value = settings[key];
    const formatted = typeof value === 'boolean'
      ? parseBool(value)
      : `0x${value.toString(16).padStart(6, '0').toUpperCase()}`;

    return `#define DEFAULT_${key} ${formatted}`;
  }).join('\n');
}

function buildHeader(profiles) {
  return `#pragma once\n\n` +
    `#if defined(PBL_BW)\n` +
    `${buildHeaderProfile('bw', profiles.bw)}\n` +
    `#else\n` +
    `${buildHeaderProfile('color', profiles.color)}\n` +
    `#endif\n`;
}

function writeFile(outputPath, content) {
  fs.writeFileSync(outputPath, content);
  console.log(path.relative(repoRoot, outputPath));
}

function main() {
  const source = loadSource();
  const profiles = {
    color: normalizeSettings(source.color),
    bw: normalizeSettings(source.bw)
  };

  writeFile(jsOutputPath, buildJs(profiles));
  writeFile(cOutputPath, buildHeader(profiles));
}

main();
