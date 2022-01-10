#ifndef FINDVARIABLETRANSFORMER_H
#define FINDVARIABLETRANSFORMER_H

#include "algeblah.h"

namespace MathOps
{

template <typename T>
struct FindVariableTransformer : public Transformer<T>
{
    FindVariableTransformer(std::string symbol)
        : symbol(symbol)
    { }

    std::shared_ptr<MathOp<T>> visit(std::shared_ptr<MutableSymbol<T>> op) override { return nullptr; }
    std::shared_ptr<MathOp<T>> visit(std::shared_ptr<ConstantSymbol<T>> op) override { return nullptr; }
    std::shared_ptr<MathOp<T>> visit(std::shared_ptr<MutableValue<T>> op) override { return nullptr; }
    std::shared_ptr<MathOp<T>> visit(std::shared_ptr<ConstantValue<T>> op) override { return nullptr; }

    std::shared_ptr<MathOp<T>> visit(std::shared_ptr<OpVariable<T>> op) override
    {
        return op->get_symbol() == symbol ? op : nullptr;
    }

    std::shared_ptr<MathOp<T>> visit(std::shared_ptr<ValueVariable<T>> op) override
    {
        return op->get_symbol() == symbol ? op : nullptr;
    }

    std::shared_ptr<MathOp<T>> visit(std::shared_ptr<NamedConstant<T>> op) override
    {
        return op->get_symbol() == symbol ? op : nullptr;
    }

    std::shared_ptr<MathOp<T>> visit(std::shared_ptr<Sqrt<T>> op) override
    {
        return op->get_x()->transform(*this);
    }

    std::shared_ptr<MathOp<T>> visit(std::shared_ptr<Negate<T>> op) override
    {
        return op->get_x()->transform(*this);
    }

    std::shared_ptr<MathOp<T>> visit(std::shared_ptr<Square<T>> op) override
    {
        return op->get_x()->transform(*this);
    }

    std::shared_ptr<MathOp<T>> visit(std::shared_ptr<Log<T>> op) override
    {
        return op->get_x()->transform(*this);
    }

    std::shared_ptr<MathOp<T>> visit(std::shared_ptr<Sin<T>> op) override
    {
        return op->get_x()->transform(*this);
    }

    std::shared_ptr<MathOp<T>> visit(std::shared_ptr<ASin<T>> op) override
    {
        return op->get_x()->transform(*this);
    }

    std::shared_ptr<MathOp<T>> visit(std::shared_ptr<Cos<T>> op) override
    {
        return op->get_x()->transform(*this);
    }

    std::shared_ptr<MathOp<T>> visit(std::shared_ptr<ACos<T>> op) override
    {
        return op->get_x()->transform(*this);
    }

    std::shared_ptr<MathOp<T>> visit(std::shared_ptr<Tan<T>> op) override
    {
        return op->get_x()->transform(*this);
    }

    std::shared_ptr<MathOp<T>> visit(std::shared_ptr<ATan<T>> op) override
    {
        return op->get_x()->transform(*this);
    }

    std::shared_ptr<MathOp<T>> visit(std::shared_ptr<Pow<T>> op) override
    {
        auto lhs = op->get_lhs()->transform(*this);
        return lhs ? lhs : op->get_rhs()->transform(*this);
    }

    std::shared_ptr<MathOp<T>> visit(std::shared_ptr<Mul<T>> op) override
    {
        auto lhs = op->get_lhs()->transform(*this);
        return lhs ? lhs : op->get_rhs()->transform(*this);
    }

    std::shared_ptr<MathOp<T>> visit(std::shared_ptr<Div<T>> op) override
    {
        auto lhs = op->get_lhs()->transform(*this);
        return lhs ? lhs : op->get_rhs()->transform(*this);
    }

    std::shared_ptr<MathOp<T>> visit(std::shared_ptr<Add<T>> op) override
    {
        auto lhs = op->get_lhs()->transform(*this);
        return lhs ? lhs : op->get_rhs()->transform(*this);
    }

    std::shared_ptr<MathOp<T>> visit(std::shared_ptr<Sub<T>> op) override
    {
        auto lhs = op->get_lhs()->transform(*this);
        return lhs ? lhs : op->get_rhs()->transform(*this);
    }

private:
    std::string symbol;
};

} /* namespace MathOps */

#endif /* FINDVARIABLETRANSFORMER_H */