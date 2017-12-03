
#ifndef SIMPLEDB_OPERATORS_PRINTOPERATOR_HPP
#define SIMPLEDB_OPERATORS_PRINTOPERATOR_HPP

#include "Operator.hpp"
#include "Register.hpp"

#include <cassert>
#include <memory>
#include <ostream>
#include <utility>
#include <vector>

namespace simpledb {

    class PrintOperator : public Operator {
    public:
        PrintOperator(std::unique_ptr<Operator> in, std::ostream& out, std::vector<void (*)(std::ostream&, const Register&) > print);
        virtual ~PrintOperator() = default;

        PrintOperator(const PrintOperator& orig) = delete;
        PrintOperator& operator=(const PrintOperator& orig) = delete;

        virtual void open();
        virtual bool next();
        virtual std::vector<std::unique_ptr<Register>> getOutput();
        virtual void close();

    private:
        std::unique_ptr<Operator> _in;
        std::ostream& _out;
        std::vector<void (*)(std::ostream&, const Register&) > _print;
    };
}

#endif	/* SIMPLEDB_OPERATORS_PRINTOPERATOR_HPP */

