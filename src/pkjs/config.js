module.exports = [
  {
    type: 'heading',
    defaultValue: 'FEZ Settings',
    size: 2
  },
  {
    type: 'text',
    defaultValue: 'Customize animation speed and the monochrome color pairing used by the watchface.'
  },
  {
    type: 'section',
    items: [
      {
        type: 'toggle',
        messageKey: 'SETTING_SLOW_VERSION',
        label: 'Slow animation',
        description: 'Use the original longer camera transition timing.',
        defaultValue: false
      },
      {
        type: 'color',
        messageKey: 'SETTING_FG_COLOR',
        label: 'Foreground color',
        defaultValue: 'ffffff',
        allowGray: true,
        description: 'On color watches this shows the Pebble color palette. Black-and-white platforms automatically fall back to their supported colors.'
      },
      {
        type: 'color',
        messageKey: 'SETTING_BG_COLOR',
        label: 'Background color',
        defaultValue: '000000',
        allowGray: true,
        description: 'On color watches this shows the Pebble color palette. Black-and-white platforms automatically fall back to their supported colors.'
      },
      {
        type: 'color',
        messageKey: 'SETTING_ACCENT_COLOR',
        label: 'Accent color',
        defaultValue: 'ffaa00',
        allowGray: true,
        description: 'An extra color you can use for contours or solid faces.'
      },
      {
        type: 'select',
        messageKey: 'SETTING_LINE_COLOR_MODE',
        label: 'Line color',
        defaultValue: '0',
        options: [
          {
            label: 'Foreground',
            value: '0'
          },
          {
            label: 'Background',
            value: '1'
          },
          {
            label: 'Accent',
            value: '2'
          },
          {
            label: 'FG + BG',
            value: '3'
          }
        ],
        description: 'Choose the contour color source.'
      },
      {
        type: 'select',
        messageKey: 'SETTING_FACE_COLOR_MODE',
        label: 'Face color',
        defaultValue: '2',
        options: [
          {
            label: 'Foreground',
            value: '0'
          },
          {
            label: 'Background',
            value: '1'
          },
          {
            label: 'Accent',
            value: '2'
          },
          {
            label: 'FG + BG',
            value: '3'
          }
        ],
        description: 'Choose the solid polygon color source.'
      }
    ]
  },
  {
    type: 'submit',
    defaultValue: 'Save'
  }
];
