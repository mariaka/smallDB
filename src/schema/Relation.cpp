
#include "Relation.hpp"

namespace simpledb {

    Relation::Relation(const std::string& name, uint64_t segmentId) : name(name), segmentId(segmentId), attributes(), primaryKeys() {
    }

    Relation::Relation(const std::string& name) : Relation(name, 0) {
    }

    Relation::Relation(std::istream& in) {
        deserialize(in);
    }

    void Relation::serialize(std::ostream& out) const {
        // serialize name
        out.write(name.c_str(), sizeof (decltype(name)::value_type) * (name.length() + 1));

        // serialize segment id
        out.write(reinterpret_cast<const char*> (&segmentId), sizeof (segmentId));

        {
            // serialize number of attributes
            auto size = attributes.size();
            out.write(reinterpret_cast<const char*> (&size), sizeof (size));

            // serialize attributes
            for (Attribute const &attr : attributes) {
                attr.serialize(out);
            }
        }
        {
            // serialize number of primary keys
            auto size = primaryKeys.size();
            out.write(reinterpret_cast<const char*> (&size), sizeof (size));

            // serialize primary keys
            out.write(reinterpret_cast<const char*> (primaryKeys.data()), sizeof (decltype(primaryKeys)::value_type) * size);
        }
    }

    bool Relation::operator ==(const Relation& other) const {
        return (
                this->name.compare(other.name) == 0 &&
                this->segmentId == other.segmentId &&
                std::equal(this->attributes.cbegin(), this->attributes.cend(), other.attributes.cbegin()) &&
                std::equal(this->primaryKeys.cbegin(), this->primaryKeys.cend(), other.primaryKeys.cbegin())
                );
    };

    void Relation::deserialize(std::istream& in) {
        // deserialize name
        std::getline(in, name, '\0');

        // deserialize segment id
        in.read(reinterpret_cast<char*> (&segmentId), sizeof (segmentId));

        {
            // deserialize number of attributes
            decltype(attributes)::size_type size;
            in.read(reinterpret_cast<char*> (&size), sizeof (size));

            // deserialize attributes
            attributes.reserve(size);
            for (decltype(attributes)::size_type i = 0; i < size; ++i) {
                attributes.emplace_back(in);
            }
        }
        {
            // deserialize number of primary keys
            decltype(primaryKeys)::size_type size;
            in.read(reinterpret_cast<char*> (&size), sizeof (size));

            // deserialize primary keys
            primaryKeys.resize(size);
            in.read(reinterpret_cast<char*> (primaryKeys.data()), sizeof (decltype(primaryKeys)::value_type) * size);
        }
    }
}
