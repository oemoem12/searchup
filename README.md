<p align="center">
  <a href="https://www.npmjs.com/package/@catodm/searchup"><img src="https://img.shields.io/npm/v/@catodm/searchup?style=flat-square" alt="npm version"/></a>
  <a href="https://github.com/oemoem12/searchup/blob/master/LICENSE"><img src="https://img.shields.io/npm/l/@catodm/searchup?style=flat-square" alt="MIT License"/></a>
  <a href="https://github.com/oemoem12/searchup/actions"><img src="https://img.shields.io/github/actions/workflow/status/oemoem12/searchup?style=flat-square" alt="CI"/></a>
</p>

<h1 align="center">SearchUp</h1>

<p align="center">
  A high-performance, lightweight search utility written in <strong>C++17</strong><br/>
  Designed for both humans and AI agents • Zero external dependencies
</p>

---

## Features

| Feature | Flag | Description |
|---------|------|-------------|
| Full-text search | `-t` | Search file contents |
| Filename search | `-f` | Search file/directory names |
| Regex search | `-r` | ECMAScript regular expressions |
| JSON output | `--json` | Structured output for AI parsing |
| Context lines | `-A / -B / -C` | Lines before/after match |
| Extension filter | `-e` | Filter by file extension |
| Depth limit | `--depth` | Control recursion depth |

## Install

```bash
# npm (recommended)
npm install -g @catodm/searchup

# Build from source
git clone https://github.com/oemoem12/searchup.git
cd searchup
make -j$(nproc)
sudo make install
```

## Usage

```bash
# Basic full-text search
searchup -t "function" src/

# Regex with context
searchup -r -C 2 "class\s+\w+" .

# JSON output for AI consumption
searchup --json "TODO" .

# Filename search
searchup -f "config" /etc

# Count matches per file
searchup -c -t "FIXME" .
```

## Project Structure

```
searchup-skill/
├── SKILL.md              # Skill definition (Trae IDE)
├── scripts/
│   ├── searchup.cpp      # C++17 core engine
│   └── searchup.js       # Node.js wrapper (npm bin)
├── Makefile              # GNU Make build
├── CMakeLists.txt        # CMake build support
├── package.json          # npm package
├── LICENSE               # MIT License
└── README.md             # You are here
```

## AI-Friendly JSON Output

```json
{
  "pattern": "TODO",
  "mode": "text",
  "total_matches": 42,
  "files_searched": 156,
  "results": [
    {
      "file": "src/main.cpp",
      "matches": [
        { "line": 10, "content": "// TODO: refactor" }
      ]
    }
  ]
}
```

## Requirements

- **C++17** compiler (GCC 8+, Clang 7+)
- **Node.js >= 14** (for npm install)
- **POSIX OS** (Linux, macOS, WSL)

## License

Copyright © 2025 [catodm](https://github.com/catodm). Licensed under the **[MIT License](LICENSE)**.

```
MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```