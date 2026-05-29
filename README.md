# SearchUp — AI-Friendly C++ Search Engine

A high-performance, lightweight search utility written in **C++17** — designed for both humans and AI agents. No external dependencies.

```bash
searchup [OPTIONS] <pattern> [path...]
```

## Features

| Mode | Flag | Description |
|------|------|-------------|
| Full-text search | `-t` | Search file contents |
| Filename search | `-f` | Search file/directory names |
| Regex search | `-r` | ECMAScript regular expressions |
| JSON output | `--json` | Structured output for AI parsing |
| Context | `-A / -B / -C` | Lines before/after match |
| Streaming | — | No buffering, instant results |

## Quick Start

```bash
# Build
make -j$(nproc)

# Search
./build/searchup -t "function" src/
./build/searchup -r -C 2 "class\s+\w+" .
./build/searchup --json "TODO" .
```

## Install

```bash
make install
# → /usr/local/bin/searchup
```

## Build Requirements

- C++17 compiler (GCC 8+, Clang 7+)
- POSIX OS (Linux, macOS, WSL)
- Zero external libraries

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

## License

MIT