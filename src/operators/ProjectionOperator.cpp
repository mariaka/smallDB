
#include "ProjectionOperator.hpp"

namespace simpledb {

    ProjectionOperator::ProjectionOperator(std::unique_ptr<Operator> in, std::vector<uint64_t> registerIds) : Operator(), _in(std::move(in)), _registerIds(std::move(registerIds)), _tuple() {
    }

    void ProjectionOperator::open() {
        _in->open();
    }

    bool ProjectionOperator::next() {
        if (_in->next()) {
            std::vector<std::unique_ptr < Register>> tuple = _in->getOutput();
            _tuple.clear();
            _tuple.reserve(_registerIds.size());

            for (uint64_t i = 0; i < _registerIds.size(); ++i) {
                _tuple.emplace_back(std::move(tuple[_registerIds[i]]));
            }
            return true;
        }
        return false;
    }

    std::vector<std::unique_ptr<Register>> ProjectionOperator::getOutput() {
        return std::move(_tuple);
    }

    void ProjectionOperator::close() {
        _in->close();
    }

    /*
    SelectionOperator::SelectionOperator(std::unique_ptr<Operator> in, uint64_t registerId, Register constant, bool (*equal)(const Register&, const Register&)) : _in(std::move(in)), _registerId(registerId), _constant(std::move(constant)), _equal(equal), _tuple() {
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
    }*/
}
