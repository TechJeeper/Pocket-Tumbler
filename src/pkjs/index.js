// Clay generates the config page (matches the Pebble app style) and
// sends settings to the watch automatically.
var Clay = require('@rebble/clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);
