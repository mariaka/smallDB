
#include "SelectionOperator.hpp"

namespace simpledb {

    SelectionOperator::SelectionOperator(std::unique_ptr<Operator> in, uint64_t registerId, Register constant, bool (*equal)(const Register&, const Register&)) : Operator(), _in(std::move(in)), _registerId(registerId), _constant(std::move(constant)), _equal(equal), _tuple() {
    }

    void SelectionOperator::open() {
        _in->open();
    }

    bool SelectionOperator::next() {
        while (_in->next()) {
            std::vector<std::unique_ptr < Register>> tuple = _in->getOutput();
            if (_equal(*tuple[_registerId].get(), _constant)) {
                _tuple = std::move(tuple);
                return true;
            }
        }
        return false;
    }

    std::vector<std::unique_ptr < Register >> SelectionOperator::getOutput() {
        return std::move(_tuple);
    }

    void SelectionOperator::close() {
        _in->close();
    }
}
