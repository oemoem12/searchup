---
name: "searchup-skill"
description: "High-performance C++ search utility for files and text. Invoke when user needs fast full-text search, regex search, filename search, web page search, or codebase scanning beyond the capability of built-in grep/find tools. Especially useful for large codebases and AI-driven code analysis."
---

# SearchUp — AI-Friendly C++ Search Engine

A high-performance search utility written in **C++17**, designed specifically for AI agents to search files, directories, web pages, and text content with maximum speed and flexibility.

## AI Agent Quick Reference

The pre-compiled binary is at:

```
/home/cat/Documents/trae_projects/trae/.trae/skills/searchup-skill/build/searchup
```

If the binary does not exist, build it first:

```bash
cd /home/cat/Documents/trae_projects/trae/.trae/skills/searchup-skill && make -j$(nproc)
```

### Invocation Pattern

```
RunCommand: /home/cat/Documents/trae_projects/trae/.trae/skills/searchup-skill/build/searchup [OPTIONS] <pattern> [path...]
```

### Recommended AI Workflows

**1. Find where a symbol is defined (structured output):**

```bash
/home/cat/Documents/trae_projects/trae/.trae/skills/searchup-skill/build/searchup --json -t "class MyClass" /path/to/project
```

**2. Find all files containing a pattern:**

```bash
/home/cat/Documents/trae_projects/trae/.trae/skills/searchup-skill/build/searchup -l -t "TODO" /path/to/project
```

**3. Regex search with context:**

```bash
/home/cat/Documents/trae_projects/trae/.trae/skills/searchup-skill/build/searchup -r -C 3 "function\\s+\\w+\\(" /path/to/project
```

**4. Search specific file types only:**

```bash
/home/cat/Documents/trae_projects/trae/.trae/skills/searchup-skill/build/searchup -t -e cpp -e h "include" /path/to/project
```

**5. Find files by name:**

```bash
/home/cat/Documents/trae_projects/trae/.trae/skills/searchup-skill/build/searchup -f "config" /path/to/project
```

**6. Count matches per file:**

```bash
/home/cat/Documents/trae_projects/trae/.trae/skills/searchup-skill/build/searchup -c -i -t "fixme" /path/to/project
```

**7. Search web page content:**

```bash
/home/cat/Documents/trae_projects/trae/.trae/skills/searchup-skill/build/searchup --web -i "searchup" https://github.com/oemoem12/searchup
```

**8. Search web page with JSON output:**

```bash
/home/cat/Documents/trae_projects/trae/.trae/skills/searchup-skill/build/searchup --web --json "C++" https://example.com
```

### When to Use This Skill vs Built-in Tools

| Scenario | Use searchup | Use built-in Grep/Glob |
|----------|-------------|----------------------|
| Need JSON output for parsing | ✅ | ❌ |
| Large codebase (10k+ files) | ✅ | ❌ |
| Complex multi-filter search | ✅ | ❌ |
| Search web page content | ✅ | ❌ |
| Quick single-file lookup | ❌ | ✅ |
| Simple filename glob | ❌ | ✅ |

## Search Modes

| Mode | Flag | Description |
|------|------|-------------|
| Full-text search | `-t` (default) | Search file contents for pattern |
| Filename search | `-f` | Search file/directory names matching pattern |
| Regex search | `-r` | Use regular expressions for pattern matching |
| Web search | `--web` | Fetch and search web page content (HTML auto-stripped) |

## Output Modes

| Flag | Description |
|------|-------------|
| `--json` | Output results as JSON for easy AI parsing |
| `-l` | Only list filenames with matches |
| `-c` | Show match count per file |
| `-n` | Show line numbers (default in text mode) |

## Filters

| Flag | Description |
|------|-------------|
| `-i` | Case-insensitive search |
| `-e <ext>` | Only search files with given extension (e.g., `-e cpp -e h`) |
| `-x <dir>` | Exclude directory from search (repeatable) |
| `--depth <N>` | Maximum recursion depth (default: unlimited) |
| `--max-size <N>` | Skip files larger than N bytes |
| `--hidden` | Include hidden files and directories |
| `--binary` | Include binary files (skip by default) |

## Context

| Flag | Description |
|------|-------------|
| `-A <N>` | Show N lines after each match |
| `-B <N>` | Show N lines before each match |
| `-C <N>` | Show N lines of context around each match |

## Web Search Details

When using `--web` mode:
- Paths are treated as URLs instead of local paths
- HTML tags, scripts, styles, and entities are automatically stripped
- Requires `curl` to be installed on the system
- Supports all output modes (`--json`, `-l`, `-c`, `-n`) and context (`-A/-B/-C`)
- Supports `-i` (case-insensitive) and `-r` (regex) for web search

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

## Directory Structure

```
searchup-skill/
├── SKILL.md              # This file — skill definition
├── scripts/
│   ├── searchup.cpp      # C++17 core search engine (~530 lines)
│   └── searchup.js       # Node.js CLI wrapper (npm bin entry)
├── build/
│   └── searchup          # Pre-compiled binary (ready to use)
├── Makefile              # Build system (make / make install)
├── CMakeLists.txt        # CMake build support
├── package.json          # npm package config
├── LICENSE               # MIT License
└── README.md             # GitHub project readme
```

## Build (first time only)

```bash
cd /home/cat/Documents/trae_projects/trae/.trae/skills/searchup-skill && make -j$(nproc)
```

## Building Dependencies

- C++17 compatible compiler (GCC 8+, Clang 7+)
- POSIX-compliant OS (Linux, macOS, WSL)
- `curl` (required for `--web` mode)
- Standard library only — no external dependencies