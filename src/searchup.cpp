#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

struct SearchOptions {
    enum class Mode { Text, Filename, Regex };
    enum class Output { Normal, Json, FilesOnly, Count };

    Mode mode = Mode::Text;
    Output output = Output::Normal;
    std::string pattern;
    std::vector<std::string> paths;
    std::vector<std::string> extensions;
    std::vector<std::string> exclude_dirs;
    bool case_insensitive = false;
    bool show_line_numbers = true;
    bool include_hidden = false;
    bool include_binary = false;
    int context_before = 0;
    int context_after = 0;
    int max_depth = -1;
    uintmax_t max_size = 0;
    std::regex compiled_regex;
    bool has_regex = false;
};

struct MatchResult {
    std::string file;
    uintmax_t line = 0;
    std::string content;
};

struct FileResult {
    std::string file;
    std::vector<MatchResult> matches;
};

struct SearchReport {
    std::string pattern;
    std::string mode;
    uintmax_t total_matches = 0;
    uintmax_t files_searched = 0;
    std::vector<FileResult> results;
};

struct CollectedLine {
    uintmax_t number;
    std::string content;
    bool is_match;
};

static std::string to_lower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

static bool is_binary_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return true;
    char buf[4096];
    file.read(buf, sizeof(buf));
    auto count = file.gcount();
    for (decltype(count) i = 0; i < count; ++i) {
        if (buf[i] == '\0') return true;
    }
    return false;
}

static bool str_contains(const std::string& haystack, const std::string& needle,
                         bool case_insensitive) {
    if (case_insensitive) {
        return to_lower(haystack).find(to_lower(needle)) != std::string::npos;
    }
    return haystack.find(needle) != std::string::npos;
}

static std::string escape_json(const std::string& s) {
    std::string result;
    result.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '"':  result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n";  break;
            case '\r': result += "\\r";  break;
            case '\t': result += "\\t";  break;
            default:   result += c;
        }
    }
    return result;
}

static bool matches_filename(const std::string& filename, const SearchOptions& opts) {
    if (opts.mode == SearchOptions::Mode::Regex && opts.has_regex) {
        return std::regex_search(filename, opts.compiled_regex);
    }
    return str_contains(filename, opts.pattern, opts.case_insensitive);
}

static bool matches_content(const std::string& line, const SearchOptions& opts) {
    if (opts.mode == SearchOptions::Mode::Regex && opts.has_regex) {
        return std::regex_search(line, opts.compiled_regex);
    }
    return str_contains(line, opts.pattern, opts.case_insensitive);
}

static bool has_extension(const std::string& filename,
                          const std::vector<std::string>& extensions) {
    if (extensions.empty()) return true;
    auto dot_pos = filename.rfind('.');
    if (dot_pos == std::string::npos) return false;
    std::string ext = to_lower(filename.substr(dot_pos + 1));
    for (const auto& e : extensions) {
        if (ext == to_lower(e)) return true;
    }
    return false;
}

static std::vector<CollectedLine> read_file_with_context(
    const std::string& path, const SearchOptions& opts) {
    std::vector<CollectedLine> all_lines;
    std::ifstream file(path);
    if (!file) return all_lines;
    std::string line_buf;
    uintmax_t line_num = 0;
    while (std::getline(file, line_buf)) {
        ++line_num;
        all_lines.push_back(
            {line_num, line_buf, matches_content(line_buf, opts)});
    }
    return all_lines;
}

static std::vector<MatchResult> extract_matches_with_context(
    const std::vector<CollectedLine>& lines, const SearchOptions& opts) {
    std::vector<MatchResult> results;
    std::unordered_set<uintmax_t> included_lines;

    for (size_t i = 0; i < lines.size(); ++i) {
        if (!lines[i].is_match) continue;

        int64_t start = static_cast<int64_t>(i) - opts.context_before;
        if (start < 0) start = 0;
        uintmax_t end = i + static_cast<size_t>(opts.context_after);
        if (end >= lines.size()) end = lines.size() - 1;

        for (auto j = static_cast<uintmax_t>(start); j <= end; ++j) {
            if (included_lines.find(lines[j].number) == included_lines.end()) {
                included_lines.insert(lines[j].number);
                results.push_back({std::string{}, lines[j].number, lines[j].content});
            }
        }
    }
    return results;
}

static std::vector<MatchResult> extract_matches_only(
    const std::vector<CollectedLine>& lines) {
    std::vector<MatchResult> results;
    for (const auto& l : lines) {
        if (l.is_match) {
            results.push_back({std::string{}, l.number, l.content});
        }
    }
    return results;
}

static bool should_skip_dir(const std::string& dirname,
                            const std::vector<std::string>& exclude_dirs) {
    for (const auto& ex : exclude_dirs) {
        if (dirname == ex) return true;
    }
    return false;
}

static bool should_skip_entry(const fs::directory_entry& entry,
                              const SearchOptions& opts) {
    auto name = entry.path().filename().string();
    if (!opts.include_hidden && !name.empty() && name[0] == '.') return true;
    if (entry.is_directory()) return false;
    if (!has_extension(name, opts.extensions)) return true;
    if (opts.max_size > 0) {
        std::error_code ec;
        auto size = fs::file_size(entry.path(), ec);
        if (!ec && size > opts.max_size) return true;
    }
    return false;
}

static void search_directory(const fs::path& dir, SearchReport& report,
                             const SearchOptions& opts, int current_depth) {
    if (opts.max_depth >= 0 && current_depth > opts.max_depth) return;

    std::error_code ec;
    auto it = fs::directory_iterator(dir, ec);
    if (ec) return;

    for (const auto& entry : it) {
        if (entry.is_directory()) {
            auto dirname = entry.path().filename().string();
            if (should_skip_dir(dirname, opts.exclude_dirs)) continue;
            if (!opts.include_hidden && !dirname.empty() && dirname[0] == '.')
                continue;
            search_directory(entry.path(), report, opts, current_depth + 1);
            continue;
        }

        if (!entry.is_regular_file()) continue;
        if (should_skip_entry(entry, opts)) continue;

        auto path_str = entry.path().string();
        auto filename = entry.path().filename().string();

        if (opts.mode == SearchOptions::Mode::Filename) {
            if (matches_filename(filename, opts)) {
                report.files_searched++;
                report.total_matches++;
                FileResult fr;
                fr.file = path_str;
                fr.matches.push_back({path_str, 0, filename});
                report.results.push_back(fr);
            }
            continue;
        }

        if (!opts.include_binary && is_binary_file(path_str)) continue;

        auto all_lines = read_file_with_context(path_str, opts);
        bool has_match = false;
        for (const auto& l : all_lines) {
            if (l.is_match) { has_match = true; break; }
        }

        if (!has_match) continue;

        report.files_searched++;

        std::vector<MatchResult> matches;
        if (opts.context_before > 0 || opts.context_after > 0) {
            matches = extract_matches_with_context(all_lines, opts);
        } else {
            matches = extract_matches_only(all_lines);
        }

        if (matches.empty()) continue;

        FileResult fr;
        fr.file = path_str;
        for (auto& m : matches) {
            m.file = path_str;
        }
        fr.matches = matches;
        report.total_matches +=
            (opts.context_before > 0 || opts.context_after > 0)
                ? matches.size()
                : matches.size();
        report.results.push_back(fr);
    }
}

static void output_normal(const SearchReport& report, const SearchOptions& opts) {
    for (const auto& fr : report.results) {
        if (report.results.size() > 1 || opts.output == SearchOptions::Output::Normal) {
            std::cout << fr.file << ":\n";
        }
        for (const auto& m : fr.matches) {
            if (opts.show_line_numbers) {
                std::cout << "  " << m.line << ": " << m.content << "\n";
            } else {
                std::cout << "  " << m.content << "\n";
            }
        }
    }
}

static void output_files_only(const SearchReport& report) {
    for (const auto& fr : report.results) {
        std::cout << fr.file << "\n";
    }
}

static void output_count(const SearchReport& report) {
    for (const auto& fr : report.results) {
        std::cout << fr.file << ": " << fr.matches.size() << "\n";
    }
}

static void output_json(const SearchReport& report) {
    std::cout << "{\n";
    std::cout << "  \"pattern\": \"" << escape_json(report.pattern) << "\",\n";
    std::cout << "  \"mode\": \"" << escape_json(report.mode) << "\",\n";
    std::cout << "  \"total_matches\": " << report.total_matches << ",\n";
    std::cout << "  \"files_searched\": " << report.files_searched << ",\n";
    std::cout << "  \"results\": [\n";
    for (size_t i = 0; i < report.results.size(); ++i) {
        const auto& fr = report.results[i];
        std::cout << "    {\n";
        std::cout << "      \"file\": \"" << escape_json(fr.file) << "\",\n";
        std::cout << "      \"matches\": [\n";
        for (size_t j = 0; j < fr.matches.size(); ++j) {
            const auto& m = fr.matches[j];
            std::cout << "        { \"line\": " << m.line
                      << ", \"content\": \"" << escape_json(m.content) << "\" }";
            if (j + 1 < fr.matches.size()) std::cout << ",";
            std::cout << "\n";
        }
        std::cout << "      ]\n";
        std::cout << "    }";
        if (i + 1 < report.results.size()) std::cout << ",";
        std::cout << "\n";
    }
    std::cout << "  ]\n";
    std::cout << "}\n";
}

static void print_usage(const char* prog) {
    std::cerr << "SearchUp — AI-Friendly C++ Search Engine\n\n";
    std::cerr << "Usage: " << prog << " [OPTIONS] <pattern> [path...]\n\n";
    std::cerr << "Search Modes:\n";
    std::cerr << "  -t          Full-text search (default)\n";
    std::cerr << "  -f          Filename search\n";
    std::cerr << "  -r          Regex search\n\n";
    std::cerr << "Output Modes:\n";
    std::cerr << "  --json      JSON output for AI parsing\n";
    std::cerr << "  -l          List filenames only\n";
    std::cerr << "  -c          Show match count per file\n";
    std::cerr << "  -n          Show line numbers (default) / --no-n to disable\n\n";
    std::cerr << "Filters:\n";
    std::cerr << "  -i          Case-insensitive search\n";
    std::cerr << "  -e <ext>    Filter by file extension (repeatable)\n";
    std::cerr << "  -x <dir>    Exclude directory (repeatable)\n";
    std::cerr << "  --depth <N> Max recursion depth\n";
    std::cerr << "  --max-size <N> Skip files larger than N bytes\n";
    std::cerr << "  --hidden    Include hidden files/directories\n";
    std::cerr << "  --binary    Include binary files\n\n";
    std::cerr << "Context:\n";
    std::cerr << "  -A <N>      Show N lines after each match\n";
    std::cerr << "  -B <N>      Show N lines before each match\n";
    std::cerr << "  -C <N>      Show N lines of context around each match\n\n";
    std::cerr << "Examples:\n";
    std::cerr << "  " << prog << " -i -t -e cpp \"main\" src/\n";
    std::cerr << "  " << prog << " -r -C 3 \"class\\\\s+\\\\w+\" .\n";
    std::cerr << "  " << prog << " --json \"TODO\" src/\n";
}

int main(int argc, char* argv[]) {
    SearchOptions opts;
    std::vector<std::string> positional;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        }
        if (arg == "-t") {
            opts.mode = SearchOptions::Mode::Text;
        } else if (arg == "-f") {
            opts.mode = SearchOptions::Mode::Filename;
        } else if (arg == "-r") {
            opts.mode = SearchOptions::Mode::Regex;
        } else if (arg == "--json") {
            opts.output = SearchOptions::Output::Json;
        } else if (arg == "-l") {
            opts.output = SearchOptions::Output::FilesOnly;
        } else if (arg == "-c") {
            opts.output = SearchOptions::Output::Count;
        } else if (arg == "-n") {
            opts.show_line_numbers = true;
        } else if (arg == "--no-n") {
            opts.show_line_numbers = false;
        } else if (arg == "-i") {
            opts.case_insensitive = true;
        } else if (arg == "--hidden") {
            opts.include_hidden = true;
        } else if (arg == "--binary") {
            opts.include_binary = true;
        } else if (arg == "-e" && i + 1 < argc) {
            opts.extensions.push_back(argv[++i]);
        } else if (arg == "-x" && i + 1 < argc) {
            opts.exclude_dirs.push_back(argv[++i]);
        } else if (arg == "-A" && i + 1 < argc) {
            opts.context_after = std::stoi(argv[++i]);
        } else if (arg == "-B" && i + 1 < argc) {
            opts.context_before = std::stoi(argv[++i]);
        } else if (arg == "-C" && i + 1 < argc) {
            opts.context_before = opts.context_after = std::stoi(argv[++i]);
        } else if (arg == "--depth" && i + 1 < argc) {
            opts.max_depth = std::stoi(argv[++i]);
        } else if (arg == "--max-size" && i + 1 < argc) {
            opts.max_size = std::stoull(argv[++i]);
        } else if (!arg.empty() && arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << "\n";
            print_usage(argv[0]);
            return 1;
        } else {
            positional.push_back(arg);
        }
    }

    if (positional.empty()) {
        print_usage(argv[0]);
        return 1;
    }

    opts.pattern = positional[0];
    for (size_t i = 1; i < positional.size(); ++i) {
        opts.paths.push_back(positional[i]);
    }
    if (opts.paths.empty()) {
        opts.paths.push_back(".");
    }

    if (opts.mode == SearchOptions::Mode::Regex) {
        try {
            auto flags = std::regex::ECMAScript;
            if (opts.case_insensitive) flags |= std::regex::icase;
            opts.compiled_regex = std::regex(opts.pattern, flags);
            opts.has_regex = true;
        } catch (const std::regex_error& e) {
            std::cerr << "Invalid regex pattern: " << e.what() << "\n";
            return 1;
        }
    }

    SearchReport report;
    report.pattern = opts.pattern;
    switch (opts.mode) {
        case SearchOptions::Mode::Text:   report.mode = "text";   break;
        case SearchOptions::Mode::Filename: report.mode = "filename"; break;
        case SearchOptions::Mode::Regex:  report.mode = "regex";  break;
    }

    for (const auto& p : opts.paths) {
        fs::path path(p);
        std::error_code ec;
        if (!fs::exists(path, ec)) {
            std::cerr << "Path not found: " << p << "\n";
            continue;
        }

        if (fs::is_directory(path, ec)) {
            search_directory(path, report, opts, 0);
        } else if (fs::is_regular_file(path, ec)) {
            auto path_str = path.string();
            auto filename = path.filename().string();

            if (opts.mode == SearchOptions::Mode::Filename) {
                if (matches_filename(filename, opts)) {
                    report.files_searched++;
                    report.total_matches++;
                    FileResult fr;
                    fr.file = path_str;
                    fr.matches.push_back({path_str, 0, filename});
                    report.results.push_back(fr);
                }
            } else {
                if (!has_extension(filename, opts.extensions)) continue;
                if (!opts.include_binary && is_binary_file(path_str)) {
                    std::cerr << "Skipping binary file: " << path_str << "\n";
                    continue;
                }

                auto all_lines = read_file_with_context(path_str, opts);
                bool has_match = false;
                for (const auto& l : all_lines) {
                    if (l.is_match) { has_match = true; break; }
                }
                if (!has_match) continue;

                report.files_searched++;
                std::vector<MatchResult> matches;
                if (opts.context_before > 0 || opts.context_after > 0) {
                    matches = extract_matches_with_context(all_lines, opts);
                } else {
                    matches = extract_matches_only(all_lines);
                }

                if (!matches.empty()) {
                    FileResult fr;
                    fr.file = path_str;
                    for (auto& m : matches) m.file = path_str;
                    fr.matches = matches;
                    report.total_matches +=
                        (opts.context_before > 0 || opts.context_after > 0)
                            ? matches.size()
                            : matches.size();
                    report.results.push_back(fr);
                }
            }
        }
    }

    switch (opts.output) {
        case SearchOptions::Output::Normal:    output_normal(report, opts);   break;
        case SearchOptions::Output::Json:      output_json(report);           break;
        case SearchOptions::Output::FilesOnly: output_files_only(report);     break;
        case SearchOptions::Output::Count:     output_count(report);          break;
    }

    return 0;
}