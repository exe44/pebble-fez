#!/usr/bin/env node

const http = require('http');
const template = require('../src/pkjs/emulator-config-template.auto');

const port = 8765;

http.createServer((request, response) => {
  if (request.url.split('?')[0] !== '/emulator-config.html') {
    response.writeHead(404, { 'Content-Type': 'text/plain' });
    response.end('Not Found');
    return;
  }

  response.writeHead(200, { 'Content-Type': 'text/html; charset=utf-8' });
  response.end(template);
}).listen(port, '127.0.0.1', () => {
  console.log(`Emulator config server listening on http://localhost:${port}/emulator-config.html`);
});
