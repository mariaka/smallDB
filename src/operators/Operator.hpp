
#ifndef SIMPLEDB_OPERATORS_OPERATOR_HPP
#define SIMPLEDB_OPERATORS_OPERATOR_HPP

#include "Register.hpp"

#include <memory>
#include <vector>

namespace simpledb {

    class Operator {
    public:
        Operator() = default;
        virtual ~Operator() = default;

        Operator(const Operator& orig) = delete;
        Operator& operator=(const Operator& orig) = delete;

        /**
         * Opens the operator.
         */
        virtual void open() = 0;

        /**
         * Calculates the next tuple.
         * @return true, if a tuple could be calculated; false, if there are no more tuples
         */
        virtual bool next() = 0;

        /**
         * Gets the tuple from the operator.
         * This is a destructive operation. It can only be called once after next() was called and returned true.
         * @return a vector of pointers to registers, representing the tuple
         */
        virtual std::vector<std::unique_ptr<Register>> getOutput() = 0;

        /**
         * Closes the operator.
         */
        virtual void close() = 0;

    private:
    };
}

#endif	/* SIMPLEDB_OPERATORS_OPERATOR_HPP */
