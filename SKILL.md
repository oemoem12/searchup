---
name: "searchup-skill"
description: "High-performance C++ search utility for files and text. Invoke when user needs fast full-text search, regex search, filename search, or codebase scanning beyond the capability of built-in grep/find tools. Especially useful for large codebases and AI-driven code analysis."
---

# SearchUp — AI-Friendly C++ Search Engine

A high-performance search utility written in C++17, designed specifically for AI agents to search files, directories, and text content with maximum speed and flexibility.

## Quick Start

Before first use, build the binary:

```bash
cd .trae/skills/searchup-skill
make -j$(nproc)
```

Or use CMake:

```bash
cd .trae/skills/searchup-skill
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

The compiled binary will be at `.trae/skills/searchup-skill/build/searchup`.

## Usage

```
searchup [OPTIONS] <pattern> [path...]
```

### Search Modes

| Mode | Flag | Description |
|------|------|-------------|
| Full-text search | `-t` (default) | Search file contents for pattern |
| Filename search | `-f` | Search file/directory names matching pattern |
| Regex search | `-r` | Use regular expressions for pattern matching |

### Output Modes

| Flag | Description |
|------|-------------|
| `--json` | Output results as JSON for easy AI parsing |
| `-l` | Only list filenames with matches |
| `-c` | Show match count per file |
| `-n` | Show line numbers (default in text mode) |

### Filters

| Flag | Description |
|------|-------------|
| `-i` | Case-insensitive search |
| `-e <ext>` | Only search files with given extension (e.g., `-e cpp -e h`) |
| `-x <dir>` | Exclude directory from search (repeatable) |
| `--depth <N>` | Maximum recursion depth (default: unlimited) |
| `--max-size <N>` | Skip files larger than N bytes |
| `--hidden` | Include hidden files and directories |
| `--binary` | Include binary files (skip by default) |

### Context

| Flag | Description |
|------|-------------|
| `-A <N>` | Show N lines after each match |
| `-B <N>` | Show N lines before each match |
| `-C <N>` | Show N lines of context around each match |

### Examples

```bash
# Full-text search for "main" in all .cpp files, case-insensitive
searchup -i -t -e cpp "main" src/

# Regex search with 3 lines of context
searchup -r -C 3 "class\s+\w+" .

# Find all files named "config*" 
searchup -f "config" /etc

# JSON output for AI consumption (default search mode)
searchup --json "TODO" src/

# Count matches per file
searchup -c -t "FIXME" .

# List only files containing the pattern
searchup -l -t "include" src/

# Search with depth limit and size filter
searchup -t --depth 3 --max-size 1048576 "function" .
```

## JSON Output Format

When using `--json`, the output is structured as:

```json
{
  "pattern": "search_term",
  "mode": "text",
  "total_matches": 42,
  "files_searched": 156,
  "results": [
    {
      "file": "src/main.cpp",
      "matches": [
        { "line": 10, "content": "int main(int argc, char** argv) {" },
        { "line": 25, "content": "  return main_loop();" }
      ]
    }
  ]
}
```

## AI Integration

This skill is designed to be invoked by AI agents when:
- Built-in grep/find performance is insufficient for large codebases
- Structured (JSON) output is needed for programmatic processing
- Complex search patterns with multiple filters are required
- Fast, C++-native performance is desired over script-based search
- The agent needs to scan a codebase with fine-grained control over search depth, file sizes, and output format

## Building Dependencies

- C++17 compatible compiler (GCC 8+, Clang 7+)
- CMake 3.14+ (optional, for CMake build)
- POSIX-compliant OS (Linux, macOS, WSL)
- Standard library only — no external dependencies