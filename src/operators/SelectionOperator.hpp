
#ifndef SIMPLEDB_OPERATORS_SELECTIONOPERATOR_HPP
#define SIMPLEDB_OPERATORS_SELECTIONOPERATOR_HPP

#include "Operator.hpp"
#include "Register.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace simpledb {

    class SelectionOperator : public Operator {
    public:
        SelectionOperator(std::unique_ptr<Operator> in, uint64_t registerId, Register constant, bool (*equal)(const Register&, const Register&));
        virtual ~SelectionOperator() = default;

        SelectionOperator(const SelectionOperator& orig) = delete;
        SelectionOperator& operator=(const SelectionOperator& orig) = delete;

        virtual void open();
        virtual bool next();
        virtual std::vector<std::unique_ptr<Register>> getOutput();
        virtual void close();

    private:
        std::unique_ptr<Operator> _in;
        uint64_t _registerId;
        Register _constant;
        bool (*_equal)(const Register&, const Register&);
        std::vector<std::unique_ptr<Register>> _tuple;
    };
}

#endif	/* SIMPLEDB_OPERATORS_SELECTIONOPERATOR_HPP */
