#!/usr/bin/env node

const { spawnSync } = require('child_process');
const path = require('path');
const fs = require('fs');

const root = path.resolve(__dirname, '..');
const binary = path.join(root, 'build', 'searchup');

if (!fs.existsSync(binary)) {
    const make = spawnSync('make', ['-C', root], { stdio: 'inherit' });
    if (make.status !== 0) {
        process.exit(make.status || 1);
    }
}

const args = process.argv.slice(2);
const result = spawnSync(binary, args, { stdio: 'inherit' });
process.exit(result.status || 0);