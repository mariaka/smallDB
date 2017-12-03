
#include "schema.hpp"

#include <cassert>
#include <cstdio>
#include <fstream>

#include <sys/stat.h>

using namespace simpledb;

int main(int argc, char** argv) {

    // create tmp dir
    const char *tmpDir = "/tmp/schematest/";
    const char *tmpFile = "/tmp/schematest/schema";
    {
        if (::mkdir(tmpDir, 0700) < 0) {
            std::perror("Could not create tmp directory");
        }
    }

    {
        // create schema
        Schema schema;

        schema.relations.emplace_back("employee", 1);
        auto employee = --schema.relations.end();
        employee->attributes.emplace_back("id", Attribute::Type::Integer, Attribute::TypeLength(Attribute::Type::Integer), true);
        employee->attributes.emplace_back("country_id", Attribute::Type::Integer, Attribute::TypeLength(Attribute::Type::Integer), true);
        employee->attributes.emplace_back("name", Attribute::Type::Char, 100, false);
        employee->primaryKeys.emplace_back(0);
        employee->primaryKeys.emplace_back(1);

        schema.relations.emplace_back("country", 2);
        auto country = --schema.relations.end();
        country->attributes.emplace_back("country_id", Attribute::Type::Integer, Attribute::TypeLength(Attribute::Type::Integer), true);
        country->attributes.emplace_back("name", Attribute::Type::Char, 100, false);
        country->primaryKeys.emplace_back(0);

        // serialize schema
        {
            std::ofstream out(tmpFile, std::ofstream::binary | std::ofstream::trunc);
            assert(out.is_open());

            schema.serialize(out);
        }

        // deserialize schema
        std::ifstream in(tmpFile, std::ifstream::binary);
        assert(in.is_open());

        Schema schema2(in);

        // compare schema
        if (schema == schema2) {
            std::cout << "test successful" << std::endl;
        } else {
            std::cout << "error: schemas not equal" << std::endl;
        }
    }

    // delete temp file and dir
    if (std::remove(tmpFile) < 0) {
        std::perror("Could not delete temp file");
    }
    if (std::remove(tmpDir) < 0) {
        std::perror("Could not delete temp folder");
    }

    return 0;
}
