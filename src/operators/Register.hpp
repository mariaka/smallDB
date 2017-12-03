
#ifndef SIMPLEDB_OPERATORS_REGISTER_HPP
#define SIMPLEDB_OPERATORS_REGISTER_HPP

#include "data/Record.hpp"

#include <cstdint>
#include <memory>
#include <utility>

namespace simpledb {

    class Register {
    public:

        Register(Record& record, uint64_t length, uint64_t offset);
        ~Register() = default;

        Register(const Register& orig);
        Register& operator=(const Register& orig) = delete;

        explicit Register(const std::unique_ptr<Register>& orig);

        Register(Register&& orig);
        Register& operator=(Register&& orig) = delete;

        uint64_t length() const;
        const char* getData() const;

    private:
        uint64_t _length;
        std::unique_ptr<char [] > _data;
    };
}

#endif	/* SIMPLEDB_OPERATORS_REGISTER_HPP */

