#ifndef GRABER_RESULT_H
#define GRABER_RESULT_H

#include <QString>
#include <utility>

/**
 * Lightweight success/failure carrier used across repository & service layers.
 * Avoids exceptions for expected I/O failures while still carrying a message.
 */
template <typename T>
class Result {
public:
    static Result ok(T value) {
        Result r;
        r.ok_ = true;
        r.value_ = std::move(value);
        return r;
    }

    static Result fail(const QString &message) {
        Result r;
        r.ok_ = false;
        r.message_ = message;
        return r;
    }

    bool isOk() const { return ok_; }
    bool isFail() const { return !ok_; }
    explicit operator bool() const { return ok_; }

    const T &value() const { return value_; }
    T &value() { return value_; }
    T takeValue() { return std::move(value_); }

    const QString &message() const { return message_; }

private:
    bool ok_ = false;
    T value_{};
    QString message_;
};

/** Specialization for void operations (write / delete / etc.). */
template <>
class Result<void> {
public:
    static Result ok() {
        Result r;
        r.ok_ = true;
        return r;
    }

    static Result fail(const QString &message) {
        Result r;
        r.ok_ = false;
        r.message_ = message;
        return r;
    }

    bool isOk() const { return ok_; }
    bool isFail() const { return !ok_; }
    explicit operator bool() const { return ok_; }

    const QString &message() const { return message_; }

private:
    bool ok_ = false;
    QString message_;
};

using VoidResult = Result<void>;

#endif // GRABER_RESULT_H
