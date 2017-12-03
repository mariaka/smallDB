
#include "TableScanOperator.hpp"

namespace simpledb {

    TableScanOperator::TableScanOperator(Relation& relation, SPSegment& segment) : Operator(), _relation(relation), _segment(segment), _it(), _tuple() {
    }

    void TableScanOperator::open() {
        _it = _segment.range();
    }

    bool TableScanOperator::next() {
        if (_it == nullptr || !_it->isValid()) {
            return false;
        }

        Record record = **_it;
        _tuple.clear();
        _tuple.reserve(_relation.attributes.size());

        uint64_t offset = 0;
        for (const auto &i : _relation.attributes) {
            _tuple.push_back(std::unique_ptr<Register>(new Register(record, i.length, offset)));
            offset += i.length;
        }

        ++(*_it);

        return true;
    }

    std::vector<std::unique_ptr<Register>> TableScanOperator::getOutput() {
        return std::move(_tuple);
    }

    void TableScanOperator::close() {
        _it.reset();
    }
}
