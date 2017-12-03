
#include "Attribute.hpp"

namespace simpledb {

    constexpr uint64_t Attribute::TYPE_LENGTH[];

    Attribute::Attribute(const std::string& name, Attribute::Type type, uint64_t length, bool notNull) : name(name), type(type), length(length), notNull(notNull) {
    }

    Attribute::Attribute(std::istream& in) {
        deserialize(in);
    }

    void Attribute::serialize(std::ostream& out) const {
        // serialize name
        out.write(name.c_str(), sizeof (decltype(name)::value_type) * (name.length() + 1));

        // serialize type, length and not null
        out.write(reinterpret_cast<const char*> (&type), sizeof (type));
        out.write(reinterpret_cast<const char*> (&length), sizeof (length));
        out.write(reinterpret_cast<const char*> (&notNull), sizeof (notNull));
    }

    bool Attribute::operator ==(const Attribute& other) const {
        return (
                this->name.compare(other.name) == 0 &&
                this->type == other.type &&
                this->length == other.length &&
                this->notNull == other.notNull
                );
    };

    void Attribute::deserialize(std::istream& in) {
        // deserialize name
        std::getline(in, name, '\0');

        // deserialize type, length and not null
        in.read(reinterpret_cast<char*> (&type), sizeof (type));
        in.read(reinterpret_cast<char*> (&length), sizeof (length));
        in.read(reinterpret_cast<char*> (&notNull), sizeof (notNull));
    }
}
