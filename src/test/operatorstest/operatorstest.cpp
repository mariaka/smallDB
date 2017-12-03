#include "buffer.hpp"
#include "data.hpp"
#include "file.hpp"
#include "operators.hpp"
#include "schema.hpp"
#include "segment.hpp"

#include <cassert>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <sys/stat.h>

using namespace simpledb;

Record createEmployee(int64_t id, int64_t countryId, int64_t age, const std::string &name, uint64_t nameLength) {
    assert(nameLength >= name.length() - 1);
    char buffer[sizeof (id) + sizeof (countryId) + sizeof (age) + nameLength];
    std::string paddedName = name;
    paddedName.append(nameLength - (name.length() - 1), ' ');

    *reinterpret_cast<int64_t*> (buffer) = id;
    *reinterpret_cast<int64_t*> (buffer + sizeof (id)) = countryId;
    *reinterpret_cast<int64_t*> (buffer + sizeof (id) + sizeof (countryId)) = age;
    std::memcpy(buffer + sizeof (id) + sizeof (countryId) + sizeof (age), paddedName.data(), paddedName.length());
    return Record(sizeof (buffer), buffer);
}

Record createCountry(int64_t id, const std::string &name, uint64_t nameLength) {
    assert(nameLength >= name.length() - 1);
    char buffer[sizeof (id) + nameLength];
    std::string paddedName = name;
    paddedName.append(nameLength - (name.length() - 1), ' ');

    *reinterpret_cast<int64_t*> (buffer) = id;
    std::memcpy(buffer + sizeof (int64_t), paddedName.data(), paddedName.length());
    return Record(sizeof (buffer), buffer);
}

int main(int argc, char** argv) {
    // create tmp dir
    const char *tmpDir = "/tmp/operatorstest/";
    {
        if (mkdir(tmpDir, 0700) < 0) {
            std::perror("Could not create tmp directory");
        }
    }

    {
        // setup managers
        std::shared_ptr<FileManager> fm = std::make_shared<FileManager>(tmpDir);
        std::shared_ptr<BufferManager> bm = std::make_shared<BufferManager>(tmpDir, 100);
        std::shared_ptr<SegmentManager> sm = std::make_shared<SegmentManager>(tmpDir, bm, fm);

        // setup segments
        uint64_t employeeSegmentId = sm->create();
        SPSegment employeeSegment(employeeSegmentId, sm, bm);
        uint64_t countrySegmentId = sm->create();
        SPSegment countrySegment(countrySegmentId, sm, bm);

        // create schema
        Schema schema;

        schema.relations.emplace_back("employee", 1);
        auto employee = --schema.relations.end();
        employee->attributes.emplace_back("id", Attribute::Type::Integer, Attribute::TypeLength(Attribute::Type::Integer), true);
        employee->attributes.emplace_back("country_id", Attribute::Type::Integer, Attribute::TypeLength(Attribute::Type::Integer), true);
        employee->attributes.emplace_back("age", Attribute::Type::Integer, Attribute::TypeLength(Attribute::Type::Integer), true);
        employee->attributes.emplace_back("name", Attribute::Type::Char, 20, false);
        employee->primaryKeys.emplace_back(0);
        employee->primaryKeys.emplace_back(1);

        schema.relations.emplace_back("country", 2);
        auto country = --schema.relations.end();
        country->attributes.emplace_back("country_id", Attribute::Type::Integer, Attribute::TypeLength(Attribute::Type::Integer), true);
        country->attributes.emplace_back("name", Attribute::Type::Char, 20, false);
        country->primaryKeys.emplace_back(0);

        // create data
        employeeSegment.insert(createEmployee(0, 0, 42, std::string("Max Mustermann"), 20));
        employeeSegment.insert(createEmployee(1, 0, 18, std::string("Jon Doe"), 20));
        employeeSegment.insert(createEmployee(2, 0, 18, std::string("Jeff Dean"), 20));
        employeeSegment.insert(createEmployee(3, 1, 18, std::string("Chuck Norris"), 20));

        countrySegment.insert(createCountry(0, "Deutschland", 20));
        countrySegment.insert(createCountry(1, "Oesterreich", 20));

        // create operators
        std::unique_ptr<TableScanOperator> employeeScan(new TableScanOperator(schema.relations[0], employeeSegment));

        const int64_t const18 = 18;
        Record constRecord18(sizeof (const18), reinterpret_cast<const char*> (&const18));
        std::unique_ptr<SelectionOperator> employeeAgeSelection(new SelectionOperator(std::move(employeeScan), 2, Register(constRecord18, constRecord18.length(), 0), Subscripts::equal<Attribute::Type::Integer>));

        std::unique_ptr<TableScanOperator> countryScan(new TableScanOperator(schema.relations[1], countrySegment));

        std::unique_ptr<HashJoinOperator> join(new HashJoinOperator(std::move(employeeAgeSelection), std::move(countryScan), 1, 0, Subscripts::hash<Attribute::Type::Integer>, Subscripts::equal<Attribute::Type::Integer>));

        std::vector<uint64_t> countryNameProjectionIds{0, 2, 3, 5};
        std::unique_ptr<ProjectionOperator> countryNameProjection(new ProjectionOperator(std::move(join), std::move(countryNameProjectionIds)));

        std::vector<void (*)(std::ostream&, const Register&) > printSubscripts{Subscripts::print<Attribute::Type::Integer>, Subscripts::print<Attribute::Type::Integer>, Subscripts::print<Attribute::Type::Char>, Subscripts::print<Attribute::Type::Char>};
        std::unique_ptr<PrintOperator> print(new PrintOperator(std::move(countryNameProjection), std::cout, std::move(printSubscripts)));

        // use operators
        print->open();
        while (print->next());
        print->close();

        // delete segments
        sm->remove(employeeSegmentId);
        sm->remove(countrySegmentId);
    }

    // delete tmp files and dir
    if (std::remove((std::string(tmpDir) + "segments").c_str()) < 0) {
        std::perror("Could not delete temp folder");
    }
    if (std::remove(tmpDir) < 0) {
        std::perror("Could not delete temp folder");
    }

    std::cout << std::endl;
    std::cout << "exptected output (in any order): " << std::endl;
    std::cout << "1 | 18 | Jon Doe              | Deutschland         " << std::endl;
    std::cout << "2 | 18 | Jeff Dean            | Deutschland         " << std::endl;
    std::cout << "3 | 18 | Chuck Norris         | Oesterreich         " << std::endl;

    return (EXIT_SUCCESS);
}
