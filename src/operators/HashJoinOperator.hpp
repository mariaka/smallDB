
#ifndef SIMPLEDB_OPERATORS_HASHJOINOPERATOR_HPP
#define SIMPLEDB_OPERATORS_HASHJOINOPERATOR_HPP

#include "Operator.hpp"
#include "Register.hpp"

#include <cstdint>
#include <memory>
#include <vector>
#include <unordered_map>

namespace simpledb {

    class HashJoinOperator : public Operator {
    public:
        HashJoinOperator(std::unique_ptr<Operator> inLeft, std::unique_ptr<Operator> inRight, uint64_t registerIdLeft, uint64_t registerIdRight, size_t(*hash)(const Register&), bool (*equal)(const Register&, const Register&));
        virtual ~HashJoinOperator() = default;

        HashJoinOperator(const HashJoinOperator& orig) = delete;
        HashJoinOperator& operator=(const HashJoinOperator& orig) = delete;

        virtual void open();
        virtual bool next();
        virtual std::vector<std::unique_ptr<Register>> getOutput();
        virtual void close();

    private:
        std::unique_ptr<Operator> _inLeft;
        std::unique_ptr<Operator> _inRight;
        uint64_t _registerIdLeft;
        uint64_t _registerIdRight;
        //size_t(*_hash)(const Register&);
        //bool (*_equal)(const Register&, const Register&);
        std::unordered_multimap<Register, std::vector<Register>, size_t(*)(const Register&), bool(*)(const Register&, const Register&) > _hashTable;

        std::vector<std::unique_ptr<Register>> _rightTuple;
        decltype(_hashTable)::const_iterator _tableIterator;
        decltype(_hashTable)::const_iterator _tableIteratorEnd;

        std::vector<std::unique_ptr<Register>> _tuple;

        void joinTuples();
    };
}

#endif	/* SIMPLEDB_OPERATORS_HASHJOINOPERATOR_HPP */
