
#ifndef SIMPLEDB_SCHEMA_ATTRIBUTE_HPP
#define SIMPLEDB_SCHEMA_ATTRIBUTE_HPP

#include <cstdint>
#include <iostream>
#include <string>

namespace simpledb {

    /**
     * Stores all information about a database attribute.
     * Is able to serialize and deserialize all attribute information.
     * This class is for data storage only. All members are public.
     */
    class Attribute {
    public:

        /**
         * Attribute data types.
         */
        enum class Type : int {
            Integer = 0, // int64_t
            Char = 1
        };

        /**
         * Attribute data type lengths.
         */
        static constexpr uint64_t TYPE_LENGTH[] = {
            sizeof (int64_t),
            0
        };

        static constexpr uint64_t TypeLength(Type type) {
            return TYPE_LENGTH[static_cast<int> (type)];
        }

        /**
         * Constructs a new attribute.
         * @param name the name
         * @param type the type
         * @param length the length (applies only to certain datatypes)
         * @param notNull the not null flag
         */
        Attribute(const std::string& name, Attribute::Type type, uint64_t length, bool notNull);

        /**
         * Constructs an attribute by deserializing it from an input stream.
         */
        explicit Attribute(std::istream& in);
        ~Attribute() = default;

        Attribute(const Attribute& orig) = delete;
        Attribute& operator=(const Attribute& orig) = delete;
        Attribute(Attribute&& orig) = default;
        Attribute& operator=(Attribute&& orig) = default;

        /**
         * Serializes the attribute to an output stream.
         * @param out the output stream
         */
        void serialize(std::ostream& out) const;

        /**
         * Compares the attribute to another attribute.
         * The attribute are considered equal, if they hold exactly the same information.
         * @param other the other attribute for comparison
         * @return true, if the attributes are equal; fals, otherwise
         */
        bool operator==(const Attribute& other) const;


        std::string name; // the name
        Attribute::Type type; // the datatype
        uint64_t length; // the length
        bool notNull; // the not null flag

    private:
        /**
         * Deserializes an attribute from an input stream.
         * @param in the input stream
         */
        void deserialize(std::istream& in);
    };
}

#endif /* SIMPLEDB_SCHEMA_ATTRIBUTE_HPP */
