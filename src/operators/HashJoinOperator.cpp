
#include "HashJoinOperator.hpp"

namespace simpledb {

    HashJoinOperator::HashJoinOperator(std::unique_ptr<Operator> inLeft, std::unique_ptr<Operator> inRight, uint64_t registerIdLeft, uint64_t registerIdRight, size_t(*hash)(const Register&), bool (*equal)(const Register&, const Register&)) : Operator(), _inLeft(std::move(inLeft)), _inRight(std::move(inRight)), _registerIdLeft(registerIdLeft), _registerIdRight(registerIdRight), _hashTable(0, hash, equal), _rightTuple(), _tableIterator(), _tuple() {
    }

    void HashJoinOperator::open() {
        // build phase: put all tuples from left in hash table
        _inLeft->open();
        while (_inLeft->next()) {
            std::vector<std::unique_ptr < Register>> tuple = _inLeft->getOutput();
            std::vector<Register> hashValueTuple(tuple.cbegin(), tuple.cend());
            _hashTable.emplace(Register(tuple[_registerIdLeft]), std::move(hashValueTuple));
        }
        _inLeft->close();

        // open right input
        _inRight->open();
    }

    bool HashJoinOperator::next() {
        // do we still have output tuples from last hash table lookup?
        if (!_rightTuple.empty()) {
            joinTuples();
            return true;
        }

        // get next tuple from right and probe hash table
        while (_inRight->next()) {
            _rightTuple = _inRight->getOutput();
            auto range = _hashTable.equal_range(*_rightTuple[_registerIdRight].get());

            // save probe result
            _tableIterator = range.first;
            _tableIteratorEnd = range.second;

            // empty result?: try with next tuple from right
            if (_tableIterator == _tableIteratorEnd) {
                continue;
            }

            joinTuples();
            return true;
        }
        return false;
    }

    std::vector<std::unique_ptr<Register>> HashJoinOperator::getOutput() {
        return std::move(_tuple);
    }

    void HashJoinOperator::close() {
        _rightTuple.clear();
        _inRight->close();
    }

    void HashJoinOperator::joinTuples() {
        // join tuples
        const std::vector<Register> &leftTuple = _tableIterator->second;
        _tuple.clear();
        _tuple.reserve(leftTuple.size() + _rightTuple.size());

        // left side
        for (const Register &reg : leftTuple) {
            _tuple.push_back(std::unique_ptr<Register>(new Register(reg)));
        }

        // right side
        for (const std::unique_ptr<Register> &reg : _rightTuple) {
            _tuple.push_back(std::unique_ptr<Register>(new Register(reg)));
        }

        // advance iterator
        ++_tableIterator;
        if (_tableIterator == _tableIteratorEnd) {
            _rightTuple.clear();
        }
    }
}
