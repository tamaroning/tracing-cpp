// Copyright(c) 2025, Raiki Tamura.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// Tracing header file.
// See example.cc for usage example.

#ifndef TRACING_H
#define TRACING_H

#include <cstdint>
#include <format>
#include <functional>
#include <iostream>
#include <source_location>
#include <sstream>
#include <string>

namespace tracing {

// Log levels.
enum class Level : uint32_t {
  Trace = 0,
  Debug,
  Info,
  Warning,
  Error,
};

inline bool operator<(Level lhs, Level rhs) noexcept {
  return static_cast<uint32_t>(lhs) < static_cast<uint32_t>(rhs);
}

// Record class to hold log messages and levels.
class Record {
public:
  Record(Level level, std::string &&args, const std::source_location &loc)
      : level(level), args(std::move(args)), loc(loc) {}

  Level get_level() const noexcept { return level; }
  const std::string &get_args() const noexcept { return args; }
  const std::source_location &get_loc() const noexcept { return loc; }

private:
  Level level;
  std::string args;
  std::source_location loc;
};

class Config {
public:
  Config(const Config &other)
      : level(other.level), formatter(other.formatter) {}
  Config(Config &&other)
      : level(other.level), formatter(std::move(other.formatter)) {}
  Config &operator=(const Config &other) = default;
  Config &operator=(Config &&other) = default;

private:
  Level level{Level::Warning};
  std::optional<std::function<void(std::ostream &, Record &)>> formatter;
};

inline void default_formatter(std::ostream &ofs, Record &record) {
  std::stringstream ss;
  ss << "[";
  switch (record.get_level()) {
  case Level::Trace:
    ss << "\033[37mTRACE\033[0m"; // gray
    break;
  case Level::Debug:
    ss << "\033[34mDEBUG\033[0m"; // blue
    break;
  case Level::Info:
    ss << "\033[32mINFO\033[0m"; // green
    break;
  case Level::Warning:
    ss << "\033[33mWARNING\033[0m"; // yellow
    break;
  case Level::Error:
    ss << "\033[31mERROR\033[0m"; // red
    break;
  }
  ss << " " << record.get_loc().file_name() << ":" << record.get_loc().line()
     << ":" << record.get_loc().column() << "] " << record.get_args()
     << std::endl;
  ofs << ss.str();
};

class Builder {
public:
  // Create a builder with default settings.
  Builder() : level(Level::Warning), formatter(default_formatter) {}

  // Create a builder with settings specified by the environment variable.
  static Builder from_env(std::string_view name) {
    Builder b;

    const char *cpp_log = std::getenv(name.data());
    if (cpp_log != nullptr) {
      std::string log_level{cpp_log};
      std::transform(log_level.begin(), log_level.end(), log_level.begin(),
                     [](unsigned char c) { return std::tolower(c); });
      if (log_level == "trace") {
        b.level = Level::Trace;
      } else if (log_level == "debug") {
        b.level = Level::Debug;
      } else if (log_level == "info") {
        b.level = Level::Info;
      } else if (log_level == "warning") {
        b.level = Level::Warning;
      } else if (log_level == "error") {
        b.level = Level::Error;
      }
    }
    return b;
  }

  void init() { g_builder = std::move(*this); }

  template <typename... Args> friend struct trace;
  template <typename... Args> friend struct debug;
  template <typename... Args> friend struct info;
  template <typename... Args> friend struct warning;
  template <typename... Args> friend struct error;

private:
  Builder(Level level, std::function<void(std::ostream &, Record &)> formatter)
      : level(level), formatter(formatter) {}

  Builder(const Builder &other) = delete;
  Builder(Builder &&other) = default;
  Builder &operator=(const Builder &other) = delete;
  Builder &operator=(Builder &&other) = default;

  Level level{Level::Warning};
  std::function<void(std::ostream &, Record &)> formatter;

  // Global builder instance.
  static Builder g_builder;
};

// Initialize the global builder instance.
inline Builder Builder::g_builder = Builder{};

template <typename... Args> struct trace {
  trace(std::format_string<Args...> fmt, Args &&...args,
        const std::source_location &loc = std::source_location::current()) {
    if (Builder::g_builder.level > Level::Debug) {
      return;
    }

    std::string message =
        std::format(fmt, std::forward<decltype(args)>(args)...);
    Record record{Level::Trace, std::move(message), loc};
    Builder::g_builder.formatter(std::cout, record);
  }
};
// deduction guide
template <typename... Args>
trace(std::format_string<Args...> fmt, Args &&...) -> trace<Args...>;

template <typename... Args> struct debug {
  debug(std::format_string<Args...> fmt, Args &&...args,
        const std::source_location &loc = std::source_location::current()) {
    if (Builder::g_builder.level > Level::Debug) {
      return;
    }

    std::string message =
        std::format(fmt, std::forward<decltype(args)>(args)...);
    Record record{Level::Debug, std::move(message), loc};
    Builder::g_builder.formatter(std::cout, record);
  }
};
// deduction guide
template <typename... Args>
debug(std::format_string<Args...> fmt, Args &&...) -> debug<Args...>;

template <typename... Args> struct info {
  info(std::format_string<Args...> fmt, Args &&...args,
       const std::source_location &loc = std::source_location::current()) {
    if (Builder::g_builder.level > Level::Info) {
      return;
    }

    std::string message =
        std::format(fmt, std::forward<decltype(args)>(args)...);
    Record record{Level::Info, std::move(message), loc};
    Builder::g_builder.formatter(std::cout, record);
  }
};
// deduction guide
template <typename... Args>
info(std::format_string<Args...> fmt, Args &&...) -> info<Args...>;

template <typename... Args> struct warning {
  warning(std::format_string<Args...> fmt, Args &&...args,
          const std::source_location &loc = std::source_location::current()) {
    if (Builder::g_builder.level > Level::Warning) {
      return;
    }

    std::string message =
        std::format(fmt, std::forward<decltype(args)>(args)...);
    Record record{Level::Warning, std::move(message), loc};
    Builder::g_builder.formatter(std::cout, record);
  }
};
// deduction guide
template <typename... Args>
warning(std::format_string<Args...> fmt, Args &&...) -> warning<Args...>;

template <typename... Args> struct error {
  error(std::format_string<Args...> fmt, Args &&...args,
        const std::source_location &loc = std::source_location::current()) {
    if (Builder::g_builder.level > Level::Error) {
      return;
    }

    std::string message =
        std::format(fmt, std::forward<decltype(args)>(args)...);
    Record record{Level::Error, std::move(message), loc};
    Builder::g_builder.formatter(std::cout, record);
  }
};
// deduction guide
template <typename... Args>
error(std::format_string<Args...> fmt, Args &&...) -> error<Args...>;

} // namespace tracing

#endif
