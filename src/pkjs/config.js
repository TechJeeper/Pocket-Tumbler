module.exports = [
  {
    type: 'heading',
    defaultValue: 'Pocket Tumbler'
  },
  {
    type: 'text',
    defaultValue: 'Walk to polish. Shake to tumble.'
  },

  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Goals' },
      {
        type: 'select',
        messageKey: 'WeeklyGoal',
        label: 'Weekly step goal',
        description: 'Hit the goal for a 10% polish bonus.',
        defaultValue: '2',
        options: [
          { label: '20,000 steps', value: '0' },
          { label: '35,000 steps', value: '1' },
          { label: '50,000 steps', value: '2' },
          { label: '70,000 steps', value: '3' }
        ]
      },
      {
        type: 'select',
        messageKey: 'WeeksToGem',
        label: 'Weeks to gem',
        description: 'How many goal-weeks of walking to fully polish a rock.',
        defaultValue: '1',
        options: [
          { label: '3 weeks', value: '0' },
          { label: '5 weeks', value: '1' },
          { label: '8 weeks', value: '2' }
        ]
      }
    ]
  },

  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Options' },
      {
        type: 'toggle',
        messageKey: 'ShakeBonus',
        label: 'Shake bonus',
        description: 'IMU tumbling adds a little polish (capped daily).',
        defaultValue: true
      },
      {
        type: 'toggle',
        messageKey: 'Vibe',
        label: 'Vibrate',
        description: 'Buzz on weekly goal and gem reveal.',
        defaultValue: true
      }
    ]
  },

  {
    type: 'submit',
    defaultValue: 'Save'
  }
];
