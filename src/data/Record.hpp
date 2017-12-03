
#ifndef SIMPLEDB_DATA_RECORD_HPP
#define SIMPLEDB_DATA_RECORD_HPP

#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>

namespace simpledb {

    /**
     * Simple record implementation to hold data.
     */
    class Record {
    public:
        /**
         * Constructs a new record.
         * @param length the length of the data in bytes
         * @param data a pointer to the data
         */
        Record(uint64_t length, const char* const data);

        /**
         * Constructs a new empty record.
         */
        Record();

        /**
         * Move constructs a record from another record.
         * @param orig the original record
         */
        Record(Record&& orig);

        ~Record() = default;

        Record(const Record& orig) = delete;
        Record& operator=(const Record& orig) = delete;

        /**
         * @return the length of the record in bytes
         */
        uint64_t length() const {
            return _length;
        }

        /**
         * @return a pointer the data of the record
         */
        const char* getData() const {
            return _data.get();
        }


    private:
        uint64_t _length; // the length of the data
        std::unique_ptr<char[] > _data; // pointer to the data
    };
}

#endif	/* SIMPLEDB_DATA_RECORD_HPP */
