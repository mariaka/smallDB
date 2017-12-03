

#ifndef SIMPLEDB_SCHEMA_SCHEMA_HPP
#define SIMPLEDB_SCHEMA_SCHEMA_HPP

#include "Relation.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <utility>
#include <vector>

namespace simpledb {

    /**
     * Stores all information about the database schema.
     * Is able to serialize and deserialize all schema information.
     * This class is for data storage only. All members are public.
     */
    class Schema {
    public:
        /**
         * Constructs a new empty schema.
         */
        Schema() = default;

        /**
         * Constructs a schema deserialized from an input stream.
         * @param in the input stream to deserialize the schema from
         */
        explicit Schema(std::istream& in);
        ~Schema() = default;

        Schema(const Schema& orig) = delete;
        Schema& operator=(const Schema& orig) = delete;

        /**
         * Serializes the schema to the output stream.
         * @param out the output stream
         */
        void serialize(std::ostream& out) const;

        /**
         * Compares the schema to another schema.
         * The schemas are considered equal, if they hold exactly the same information.
         * @param other the other schema for comparison
         * @return true, if the schemas are equal; fals, otherwise
         */
        bool operator==(const Schema& other) const;

        /**
         * the relations of the schema
         */
        std::vector<Relation> relations;

    private:
        /**
         * Deserializes the schema from an input stream.
         * @param in the input stream to deserialize the schema from
         */
        void deserialize(std::istream& in);
    };
}

#endif /* SIMPLEDB_SCHEMA_SCHEMA_HPP */
