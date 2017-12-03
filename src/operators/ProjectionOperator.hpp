
#ifndef SIMPLEDB_OPERATORS_PROJECTIONOPERATOR_HPP
#define SIMPLEDB_OPERATORS_PROJECTIONOPERATOR_HPP

#include "Operator.hpp"
#include "Register.hpp"

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace simpledb {

    class ProjectionOperator : public Operator {
    public:
        ProjectionOperator(std::unique_ptr<Operator> in, std::vector<uint64_t> registerIds);
        virtual ~ProjectionOperator() = default;

        ProjectionOperator(const ProjectionOperator& orig) = delete;
        ProjectionOperator& operator=(const ProjectionOperator& orig) = delete;

        virtual void open();
        virtual bool next();
        virtual std::vector<std::unique_ptr<Register>> getOutput();
        virtual void close();

    private:
        std::unique_ptr<Operator> _in;
        std::vector<uint64_t> _registerIds;

        std::vector<std::unique_ptr<Register>> _tuple;
    };
}

#endif	/* SIMPLEDB_OPERATORS_PROJECTIONOPERATOR_HPP */
