
#ifndef SIMPLEDB_OPERATORS_SUBSCRIPTS_HPP
#define SIMPLEDB_OPERATORS_SUBSCRIPTS_HPP

#include "Register.hpp"

#include "hash.hpp"
#include "schema/Attribute.hpp"

#include <cstddef>
#include <cstring>
#include <ostream>

namespace simpledb {

    class Subscripts {
    public:
        Subscripts() = delete;
        ~Subscripts() = delete;

        Subscripts(const Subscripts& orig) = delete;
        Subscripts& operator=(const Subscripts& orig) = delete;

        template <Attribute::Type T>
        static void print(std::ostream& out, const Register& reg);

        template <Attribute::Type T>
        static bool equal(const Register& r1, const Register& r2);

        template <Attribute::Type T>
        static size_t hash(const Register& reg);

    private:
    };

    template <>
    void Subscripts::print<Attribute::Type::Integer>(std::ostream& out, const Register& reg);

    template <>
    void Subscripts::print<Attribute::Type::Char>(std::ostream& out, const Register& reg);

    template <Attribute::Type T>
    bool Subscripts::equal(const Register& r1, const Register& r2) {
        if (r1.length() != r2.length()) {
            return false;
        }

        return (std::memcmp(r1.getData(), r2.getData(), r1.length()) == 0);
    }

    template <Attribute::Type T>
    size_t Subscripts::hash(const Register& reg) {
        return Hash()(reg.getData(), reg.length());
    }
}

#endif	/* SIMPLEDB_OPERATORS_SUBSCRIPTS_HPP */

