
#ifndef SIMPLEDB_SCHEMA_RELATION_HPP
#define	SIMPLEDB_SCHEMA_RELATION_HPP

#include "Attribute.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace simpledb {

    /**
     * Stores all information about a database relation.
     * Is able to serialize and deserialize all relation information.
     * This class is for data storage only. All members are public.
     */
    class Relation {
    public:
        /**
         * Constructs a new relation.
         * @param name the name of the relation
         * @param segmentId the segment id to store the data of the relation
         */
        Relation(const std::string& name, uint64_t segmentId);

        /**
         * Constructs a new relation with invalid segment id.
         * @param name the name of the relation
         */
        explicit Relation(const std::string& name);

        /**
         * Constructs a relation by deserializing it from an input stream.
         * @param in the input stream
         */
        explicit Relation(std::istream& in);
        ~Relation() = default;

        Relation(const Relation& orig) = delete;
        Relation& operator=(const Relation& orig) = delete;
        Relation(Relation&& orig) = default;
        Relation& operator=(Relation&& orig) = default;

        /**
         * Serializes the relation to an output stream.
         * @param out the output stream
         */
        void serialize(std::ostream& out) const;

        /**
         * Compares the relation to another relation.
         * The relations are considered equal, if they hold exactly the same information.
         * @param other the other relation for comparison
         * @return true, if the relations are equal; fals, otherwise
         */
        bool operator==(const Relation& other) const;


        std::string name; // the name
        uint64_t segmentId; // the segment id to store the data of the relation
        std::vector<Attribute> attributes; // the attributes
        std::vector<uint64_t> primaryKeys; // the ids of the attributes starting from 0 that are primary keys

    private:
        /**
         * Deserializes the relation from an input stream.
         * @param in the input stream
         */
        void deserialize(std::istream& in);
    };
}

#endif /* SIMPLEDB_SCHEMA_RELATION_HPP */
