
'use strict';

// Hierarchical node.js configuration with command-line arguments, environment
// variables, and files.
const nconf = module.exports = require('nconf');
const path = require('path');

nconf
  // 1. Command-line arguments
  .argv()
  // 2. Environment variables
  .env([
	'DS_URL',
	'DS_IP',
	'DS_PORT'
  ])
  // 3. Config file
  .file({ file: path.join(__dirname, 'config.json') })
  // 4. Defaults
  .defaults({
	  'DS_URL': '0.udp.particle.io',
	  'DS_IP':'107.20.71.170',
	  'DS_PORT':5684
  });

// Check for required settings
// checkConfig('AUTH_TOKEN');


function checkConfig (setting) {
  if (!nconf.get(setting)) {
    throw new Error(`You must set ${setting} as an environment variable or in config.json!`);
  }
}
