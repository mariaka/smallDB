
#include "Register.hpp"

namespace simpledb {

    Register::Register(Record& record, uint64_t length, uint64_t offset) : _length(length), _data(new char[length]) {
        std::memcpy(_data.get(), record.getData() + offset, length);
    }

    Register::Register(const Register& orig) : _length(orig.length()), _data(new char[_length]) {
        std::memcpy(_data.get(), orig.getData(), _length);
    }

    Register::Register(const std::unique_ptr<Register>& orig) : Register(*orig.get()) {
    }

    Register::Register(Register&& orig) : _length(std::move(orig._length)), _data(std::move(orig._data)) {
        orig._length = 0;
    }

    uint64_t Register::length() const {
        return _length;
    }

    const char* Register::getData() const {
        return _data.get();
    }
}
