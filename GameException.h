#pragma once

#include <exception>
#include <string>

// Custom exception class for game related errors
class GameException : public std::exception {
private:
    std::string message;

public:
    explicit GameException(const std::string& msg) : message(msg) {}

    const char* what() const noexcept override {
        return message.c_str();
    }
};

// Exception for invalid card operations
class InvalidCardException : public GameException {
public:
    explicit InvalidCardException(const std::string& msg = "Invalid card operation")
        : GameException(msg) {}
};

// Exception for board initialization errors
class BoardInitializationException : public GameException {
public:
    explicit BoardInitializationException(const std::string& msg = "Board initialization failed")
        : GameException(msg) {}
};

// Exception for file IO errors
class FileIOException : public GameException {
public:
    explicit FileIOException(const std::string& msg = "File IO operation failed")
        : GameException(msg) {}
};
