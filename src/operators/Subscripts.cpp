
#include "Subscripts.hpp"

namespace simpledb {

    template <>
    void Subscripts::print<Attribute::Type::Integer>(std::ostream& out, const Register& reg) {
        out << *(reinterpret_cast<const int64_t *> (reg.getData()));
    }

    template <>
    void Subscripts::print<Attribute::Type::Char>(std::ostream& out, const Register& reg) {
        out.write(reg.getData(), reg.length());
    }
}
