
#include "Schema.hpp"

namespace simpledb {

    Schema::Schema(std::istream& in) {
        deserialize(in);
    }

    void Schema::serialize(std::ostream& out) const {
        {
            // serialize number of relations
            auto size = relations.size();
            out.write(reinterpret_cast<const char*> (&size), sizeof (size));

            // serialize relations
            for (Relation const &rel : relations) {
                rel.serialize(out);
            }
        }
    }

    bool Schema::operator ==(const Schema& other) const {
        if (this->relations.size() != other.relations.size()) {
            return false;
        }
        return (std::equal(this->relations.cbegin(), this->relations.cend(), other.relations.cbegin()));
    };

    void Schema::deserialize(std::istream& in) {
        {
            // deserialize number of relations
            decltype(relations)::size_type size;
            in.read(reinterpret_cast<char*> (&size), sizeof (size));

            // deserialize relations
            // TODO: use range constructor of vector to improve performance
            relations.reserve(size);
            for (decltype(relations)::size_type i = 0; i < size; ++i) {
                relations.emplace_back(in);
            }
        }
    }
}
