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
      }
    ]
  },
  {
    type: 'submit',
    defaultValue: 'Save'
  }
];
