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
        type: 'select',
        messageKey: 'SETTING_FG_COLOR',
        label: 'Foreground color',
        defaultValue: '1',
        options: [
          {
            label: 'White',
            value: '1'
          },
          {
            label: 'Black',
            value: '0'
          }
        ]
      },
      {
        type: 'select',
        messageKey: 'SETTING_BG_COLOR',
        label: 'Background color',
        defaultValue: '0',
        options: [
          {
            label: 'Black',
            value: '0'
          },
          {
            label: 'White',
            value: '1'
          }
        ]
      }
    ]
  },
  {
    type: 'submit',
    defaultValue: 'Save'
  }
];
