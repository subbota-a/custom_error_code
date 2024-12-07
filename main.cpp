#include <cassert>
#include <expected>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <iostream>
#include <magic_enum.hpp>
#include <ranges>
#include <string>
#include <utility>
#include <vector>
#include <version>

enum class ErrorLow { FileNotFound, PermissionDenied };
enum class ErrorMid { ConfigLoadFailed };
enum class ErrorHigh { OperationFailed };

template<typename Enum>
struct fmt::formatter<Enum, std::enable_if_t<std::is_enum_v<Enum>, char>> : fmt::formatter<std::string_view> {
    auto format(const Enum &value, fmt::format_context &ctx) const {
        return fmt::formatter<std::string_view>::format(magic_enum::enum_name(value), ctx);
    }
};

template<typename E> struct ErrorInfo {
    E code;
    std::string details;
    std::vector<std::string> previous_errors;

    ErrorInfo(E c, std::string details = {}, std::vector<std::string> prev = {})
        : code(c), details(std::move(details)), previous_errors(std::move(prev)) {}

    std::string message() const {
        if (details.empty())
            return fmt::format("{}", code);
        else
            return fmt::format("{}({})", code, details);
    }
};

template<typename E> struct fmt::formatter<ErrorInfo<E>, char> {
    constexpr auto parse(fmt::format_parse_context &ctx) { return ctx.begin(); }
    auto format(const ErrorInfo<E> &value, fmt::format_context &ctx) const {
        auto out = fmt::format_to(ctx.out(), "{}", value.code);
        if (!value.details.empty()) {
            out = fmt::format_to(out, "({})", value.details);
        }
        return fmt::format_to(out, " {}", std::views::reverse(value.previous_errors));
    }
};

template<typename E> static ErrorInfo<E> make(E code, std::string msg) { return ErrorInfo<E>(code, std::move(msg)); }

// Преобразование ошибки нижнего уровня в верхний уровень
// Добавляем сообщение нижнего уровня в начало цепочки предыдущих ошибок
template<typename E, typename E2> static ErrorInfo<E> make(E code, std::string msg, ErrorInfo<E2> &&prev) {
    prev.previous_errors.push_back(prev.message());
    return ErrorInfo<E>(code, msg, std::move(prev.previous_errors));
}

// Низкоуровневая функция
std::expected<int, ErrorInfo<ErrorLow>> low_level_open_file(const std::string &filename) {
    bool file_found = false;
    if (!file_found) {
        return std::unexpected(make(ErrorLow::FileNotFound, "filename=" + filename));
    }
    return 42;
}

// Средний уровень
std::expected<std::string, ErrorInfo<ErrorMid>> load_config() {
    auto res = low_level_open_file("/path/to/config.json");
    if (!res) {
        // Преобразуем низкоуровневую ошибку в среднеуровневую
        return std::unexpected(make(ErrorMid::ConfigLoadFailed, "config details", std::move(res.error())));
    }
    return std::string("config data");
}

// Высокий уровень
std::expected<void, ErrorInfo<ErrorHigh>> perform_operation() {
    auto cfg = load_config();
    if (!cfg) {
        // Преобразуем среднеуровневую ошибку в высокоуровневую
        return std::unexpected(make(ErrorHigh::OperationFailed, "my details", std::move(cfg.error())));
    }

    // ... логика операции
    return {};
}

void handle_request() {
    auto result = perform_operation();
    if (!result) {
        fmt::println(stderr, "Top-level error: {}", result.error());
    } else {
        std::cout << "Success!\n";
    }
}

int main() {
    handle_request();
    return 0;
}
