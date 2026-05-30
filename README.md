<p align="center">
  <img src="https://img.shields.io/npm/v/searchup?style=flat-square" alt="npm version"/>
  <img src="https://img.shields.io/npm/l/searchup?style=flat-square" alt="license"/>
  <img src="https://img.shields.io/npm/types/searchup?style=flat-square" alt="types"/>
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
npm install -g searchup

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

[MIT](LICENSE) © catodm