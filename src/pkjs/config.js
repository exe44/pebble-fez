module.exports = [
  {
    type: 'heading',
    defaultValue: 'FEZ Settings',
    size: 2
  },
  {
    type: 'text',
    defaultValue: 'Adjust animation and colors.'
  },
  {
    type: 'section',
    items: [
      {
        type: 'toggle',
        messageKey: 'SETTING_SLOW_VERSION',
        label: 'Slow animation',
        defaultValue: false
      },
      {
        type: 'color',
        messageKey: 'SETTING_BG_COLOR',
        label: 'Background color',
        defaultValue: '000000',
        allowGray: true
      }
    ]
  },
  {
    type: 'section',
    items: [
      {
        type: 'color',
        messageKey: 'SETTING_FACE_COLOR',
        label: 'Face color',
        defaultValue: 'ffaa00',
        allowGray: true,
        description: 'Digit fill color.'
      },
      {
        type: 'toggle',
        messageKey: 'SETTING_FACE_MIX_WITH_BACKGROUND',
        label: 'Face mix with background',
        defaultValue: false
      }
    ]
  },
  {
    type: 'section',
    items: [
      {
        type: 'color',
        messageKey: 'SETTING_LINE_COLOR',
        label: 'Line color',
        defaultValue: 'ffffff',
        allowGray: true,
        description: 'Used for all lines when split is off.'
      },
      {
        type: 'toggle',
        messageKey: 'SETTING_LINE_MIX_WITH_BACKGROUND',
        label: 'Line mix with background',
        defaultValue: false
      },
      {
        type: 'toggle',
        messageKey: 'SETTING_SPLIT_LINE_COLORS',
        label: 'Split line colors',
        description: 'Enable separate back and side colors.',
        defaultValue: false
      },
      {
        type: 'color',
        messageKey: 'SETTING_BACK_LINE_COLOR',
        label: 'Back line color',
        defaultValue: 'ffffff',
        allowGray: true
      },
      {
        type: 'color',
        messageKey: 'SETTING_SIDE_LINE_COLOR',
        label: 'Side line color',
        defaultValue: 'ffffff',
        allowGray: true
      }
    ]
  },
  {
    type: 'section',
    items: [
      {
        type: 'checkboxgroup',
        id: 'randomize-targets',
        label: 'Randomize targets',
        defaultValue: [false, true, true],
        options: ['Background', 'Face', 'Line'],
        description: 'Choose which colors to randomize.'
      },
      {
        type: 'button',
        id: 'randomize-colors',
        defaultValue: 'Randomize colors',
        description: 'Colors apply after you save.',
        primary: false
      }
    ]
  },
  {
    type: 'submit',
    defaultValue: 'Save'
  }
];
