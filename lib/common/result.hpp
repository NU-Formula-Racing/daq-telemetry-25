#ifndef __RESULT_H__
#define __RESULT_H__

#include <sstream>
#include <vector>
#include <string>
#include <initializer_list>
#include <functional>

namespace common {

/// @brief A class reprsenting the result of a function, used for operations where you can fail, and
/// care about an error message
/// @tparam T The type of the the value
template <typename T>
class Result {
   public:
    /// @brief Create a result with some value -- no error
    /// @param value The value
    /// @return The result with a value
    static Result<T> ok(T value) { return Result<T>(value); }

    /// @brief Create a result with no value, but with an error message
    /// @param errorMessage The error message
    /// @return The result with an error message
    static Result<T> errorResult(std::string errorMessage) { return Result<T>(true, errorMessage); }

    /// @brief Ctor
    Result() : _error(true) {}

    /// @brief Cast to bool
    operator bool() const { return !_error; }

    /// @brief Equals operator
    Result<T> operator=(const Result<T>& other) {
        _value = other._value;
        _error = other._error;
        _errorMessage = other._errorMessage;
        return *this;
    }

    /// @brief Get the value in the result
    /// @return The value in the result -- could be garbage if it is an error
    T value() const { return _value; }

    /// @brief Checks if the result is an error
    /// @return If the result is an error
    bool isError() const { return _error; }

    /// @brief Get the error message
    /// @return The error message
    std::string error() const { return _errorMessage; }

   private:
    T _value;
    bool _error;
    std::string _errorMessage;

    Result(T value) : _value(value), _error(false), _errorMessage("") {}
    Result(bool error, std::string errorMessage) : _error(error), _errorMessage(errorMessage) {}
};

// template <typename OnError, typename... Fs>
// bool check(OnError onError, Fs&&... functions) {
//     // check if any of the functions return an error
//     bool error = false;
//     std::vector<std::string> errors;
//     std::initializer_list<int>{(error |= functions.isError(), 0)...};
//     std::initializer_list<int>{(errors.push_back(functions.error()), 0)...};

//     if (error) {
//         // concatenate the error messages into one string
//         std::string errorMessage;
//         for (const std::string& error : errors) {
//             if (error.empty()) {
//                 continue;
//             }
//             errorMessage += error;
//             errorMessage += "\n";
//         }

//         onError(errorMessage.c_str());

//         return true;
//     }

//     return false;
// }

// std::function<void(const char*)> defaultErrorCallback(std::ostream& stream) {
//     return [&stream](const char* error) { stream << "Error: " << error << std::endl; };
// }

}  // namespace common

#endif  // __RESULT_H__