import json
import os.path
import subprocess

top = '.'
out = 'build'


def options(ctx):
    ctx.load('pebble_sdk')


def configure(ctx):
    ctx.load('pebble_sdk')


def _generate_emulator_config_template():
    html_path = 'src/pkjs/emulator-config.html'
    css_path = 'src/pkjs/emulator-config.css'
    js_path = 'src/pkjs/emulator-config-page.js'
    output_path = 'src/pkjs/emulator-config-template.auto.js'

    with open(html_path, 'r') as html_file:
        html = html_file.read()

    with open(css_path, 'r') as css_file:
        css = css_file.read()

    with open(js_path, 'r') as js_file:
        js = js_file.read()

    template = html.replace('__EMULATOR_CONFIG_CSS__', css)
    template = template.replace('__EMULATOR_CONFIG_JS__', js)

    output = 'module.exports = ' + json.dumps(template) + ';\n'

    with open(output_path, 'w') as output_file:
        output_file.write(output)


def _generate_default_settings():
    subprocess.check_call(['node', 'scripts/generate-default-settings.js'])


def build(ctx):
    ctx.load('pebble_sdk')
    _generate_default_settings()
    _generate_emulator_config_template()

    binaries = []
    cached_env = ctx.env

    for platform in ctx.env.TARGET_PLATFORMS:
        ctx.env = ctx.all_envs[platform]
        ctx.set_group(ctx.env.PLATFORM_NAME)
        app_elf = '{}/pebble-app.elf'.format(ctx.env.BUILD_DIR)
        ctx.pbl_build(source=ctx.path.ant_glob('src/c/**/*.c'), target=app_elf, bin_type='app')
        binaries.append({'platform': platform, 'app_elf': app_elf})

    ctx.env = cached_env

    ctx.set_group('bundle')
    ctx.pbl_bundle(
        binaries=binaries,
        js=ctx.path.ant_glob([
            'src/pkjs/**/*.js',
            'src/pkjs/**/*.json',
            'src/common/**/*.js',
        ]),
        js_entry_file='src/pkjs/index.js')
