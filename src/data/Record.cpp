
#include "Record.hpp"

namespace simpledb {

    Record::Record(uint64_t length, const char* const data) : _length(length), _data(new char[length]) {
        if (data) {
            memcpy(_data.get(), data, length);
        }
    }

    Record::Record() : Record(0, nullptr) {
    }

    Record::Record(Record&& orig) : _length(orig._length), _data(std::forward<Record>(orig)._data) {
        orig._length = 0;
    }
}