#include <cmath>
#include <limits>
#include <iostream>
#include <string>
#include <functional>
#include <sstream>
#include <iomanip>
#include <numbers>
#include <vector>

template<typename T>
struct MathOp
{
    virtual T get() const = 0;
    virtual int order() const = 0;
    virtual MathOp<T>* rearranged(int child, MathOp<T>* output) const = 0;
    virtual std::ostream& to_stream(std::ostream& stream, int parent_order) const = 0;

    virtual MathOp<T>* solve_for(MathOp<T>* op, MathOp<T>* output) const
    {
        return op == this ? output : nullptr;
    }

    std::ostream& to_stream_paren(std::ostream& stream, int parent_order) const
    {
        bool parens = parent_order <= order();

        if (parens)
        {
            stream << '(';
        }

        to_stream(stream, parent_order);

        if (parens)
        {
            stream << ')';
        }

        return stream;
    }

    friend std::ostream& operator<<(std::ostream &stream, const MathOp<T> &op)
    {
        return op.to_stream_paren(stream, std::numeric_limits<int>::max());
    }

    virtual ~MathOp() { }
};

template<typename T>
struct MathOpSymbol : public MathOp<T>
{
    MathOpSymbol(std::string symbol, T c)
        : symbol(symbol), c(c)
    { }

    T get() const override { return c; }
    int order() const override { return 0; }
    MathOp<T>* rearranged(int, MathOp<T>*) const override { return nullptr; }
    std::ostream& to_stream(std::ostream& stream, int parent_order) const { return stream << symbol; }

protected:
    T c;

private:
    std::string symbol;
};

template<typename T>
struct MathOpSymbolPi : public MathOpSymbol<T> { MathOpSymbolPi() : MathOpSymbol<T>("π", M_PI) { }; };

template<typename T>
struct MathOpSymbolE : public MathOpSymbol<T> { MathOpSymbolE() : MathOpSymbol<T>("e", M_E) { }; };

template<typename T>
struct MathOpSymbolSqrt2 : public MathOpSymbol<T> { MathOpSymbolSqrt2() : MathOpSymbol<T>("√(2)", M_SQRT2) { }; };

template<typename T>
struct MathOpVariable: public MathOpSymbol<T>
{
    MathOpVariable(std::string symbol, T c = 0)
        : MathOpSymbol<T>(symbol, c)
    { }

    void set(T x) { this->c = x; }
};

template<typename T>
struct MathOpConstantValue : public MathOpSymbol<T>
{
    MathOpConstantValue(T c)
        : MathOpSymbol<T>(to_string(c), c)
    { }

private:
    std::string to_string(T x)
    {
        std::stringstream ss;
        ss << x;
        return ss.str();
    }
};


template<typename T, typename U>
struct MathUnaryOp : public MathOp<T>
{
    MathUnaryOp(MathOp<T>* x, int order, std::string symbol)
        : x(x), o(order), symbol(symbol)
    { }

    T get() const override { return f(x->get()); }
    int order() const override { return o; }

    MathOp<T>* solve_for(MathOp<T>* op, MathOp<T>* output) const override
    {
        auto x = this->rearranged(0, output);

        return this->x->solve_for(op, x);
    }

    std::ostream& to_stream(std::ostream& stream, int parent_order) const
    {
        return stream << symbol << '(' << *x << ')';
    }

protected:
    MathOp<T>* x;
    int o;
    std::string symbol;
    U f;
};

template<typename T, typename U>
struct MathBinaryOp : public MathOp<T>
{
    MathBinaryOp(MathOp<T>* lhs, MathOp<T>* rhs, int order, std::string symbol)
        : lhs(lhs), rhs(rhs), o(order), symbol(symbol)
    { }

    T get() const override { return f(lhs->get(), rhs->get()); }
    int order() const override { return o; }

    MathOp<T>* solve_for(MathOp<T>* op, MathOp<T>* output) const override
    {
        auto output_lhs = this->rearranged(0, output);

        auto solved_lhs = this->lhs->solve_for(op, output_lhs);
        if (solved_lhs != nullptr)
        {
            return solved_lhs;
        }

        auto output_rhs = this->rearranged(1, output);

        return this->rhs->solve_for(op, output_rhs);
    }

    std::ostream& to_stream(std::ostream& stream, int parent_order) const
    {
        lhs->to_stream_paren(stream, order());
        stream << symbol;
        rhs->to_stream_paren(stream, order());

        return stream;
    }

protected:
    MathOp<T>* lhs;
    MathOp<T>* rhs;
    int o;
    std::string symbol;
    U f;
};

template <typename T>
struct raises : public std::binary_function<T, T, T>
{
    T operator()(T x, T y) const
    {
        return std::pow(x, y);
    }
};

template <typename T>
struct logarithm : public std::unary_function<T, T>
{
    T operator()(T x) const
    {
        return std::log(x);
    }
};

template <typename T>
struct square_root : public std::unary_function<T, T>
{
    T operator()(T x) const
    {
        return std::sqrt(x);
    }
};

template<typename T> struct MathOpPow;

template<typename T>
struct MathOpSqrt : public MathUnaryOp<T, square_root<T>>
{
    MathOpSqrt(MathOp<T>* x): MathUnaryOp<T, square_root<T>>(x, 2, "√") { }

    MathOp<T>* rearranged(int child, MathOp<T>* output) const override
    {
        auto two = new MathOpConstantValue<T>(2);
        return new MathOpPow<T>(output, two); 
    }
};

template<typename T>
struct MathOpLog : public MathUnaryOp<T, logarithm<T>>
{
    MathOpLog(MathOp<T>* x): MathUnaryOp<T, logarithm<T>>(x, 2, "log") { }

    MathOp<T>* rearranged(int child, MathOp<T>* output) const override
    {
        auto e = new MathOpSymbolE<T>();
        return new MathOpPow<T>(e, output);
    }
};

template<typename T> struct MathOpDiv;

template<typename T>
struct MathOpPow : public MathBinaryOp<T, raises<T>>
{
    MathOpPow(MathOp<T>* lhs, MathOp<T>* rhs): MathBinaryOp<T, raises<T>>(lhs, rhs, 2, " ^ ") { }

    MathOp<T>* rearranged(int child, MathOp<T>* output) const override
    {
        if (child == 0)
        {
            auto one = new MathOpConstantValue<T>(1);
            auto inv = new MathOpDiv<T>(one, this->rhs);

            return new MathOpPow(this->lhs, inv);
        }

        auto out_log = new MathOpLog<T>(output);
        auto lhs_log = new MathOpLog<T>(this->lhs);

        return new MathOpDiv<T>(out_log, lhs_log); 
    }
};

template<typename T>
struct MathOpMul : public MathBinaryOp<T, std::multiplies<T>>
{
    MathOpMul(MathOp<T>* lhs, MathOp<T>* rhs): MathBinaryOp<T, std::multiplies<T>>(lhs, rhs, 10, " * ") { }

    MathOp<T>* rearranged(int child, MathOp<T>* output) const override
    {
        return new MathOpDiv<T>(output, child == 0 ? this->rhs : this->lhs);
    }
};

template<typename T>
struct MathOpDiv : public MathBinaryOp<T, std::divides<T>>
{
    MathOpDiv(MathOp<T>* lhs, MathOp<T>* rhs) : MathBinaryOp<T, std::divides<T>>(lhs, rhs, 10, " / ") { }

    MathOp<T>* rearranged(int child, MathOp<T>* output) const override
    {
        return child == 0 
            ? (MathOp<T>*) new MathOpMul<T>(output, this->rhs)
            : (MathOp<T>*) new MathOpDiv<T>(this->lhs, output);
    }
};

template<typename T> struct MathOpSub;

template<typename T>
struct MathOpAdd : public MathBinaryOp<T, std::plus<T>>
{
    MathOpAdd(MathOp<T>* lhs, MathOp<T>* rhs) : MathBinaryOp<T, std::plus<T>>(lhs, rhs, 100, " + ") { }

    MathOp<T>* rearranged(int child, MathOp<T>* output) const override
    {
        return new MathOpSub<T>(output, child == 0 ? this->rhs : this->lhs);
    }
};

template<typename T>
struct MathOpSub : public MathBinaryOp<T, std::minus<T>>
{
    MathOpSub(MathOp<T>* lhs, MathOp<T>* rhs) : MathBinaryOp<T, std::minus<T>>(lhs, rhs, 100, " - ") { }

    MathOp<T>* rearranged(int child, MathOp<T>* output) const override
    {
        return child == 0 
            ? (MathOp<T>*) new MathOpAdd<T>(output, this->rhs)
            : (MathOp<T>*) new MathOpSub<T>(this->lhs, output);
    }
};

template<typename T>
struct Fraction
{
    T numerator;
    T denominator;

    bool isnan()      const { return std::isnan(numerator) || std::isnan(denominator); }
    bool isintegral() const { return denominator == 1.0; }
    T    fractional() const { return numerator / denominator; }

    Fraction(T numerator, T denominator)
        : numerator(numerator), denominator(denominator)
    { }

    static Fraction<T> quiet_NaN()
    {
        return Fraction<T>(std::numeric_limits<T>::quiet_NaN(), std::numeric_limits<T>::quiet_NaN());
    }

    static Fraction<T> find(T value, T max_error, int iters)
    {
        Fraction<T> lower(0, 1);
        Fraction<T> upper(1, 1);

        T integral;
        T fractional = std::modf(value, &integral);

        if (fractional == 0.0)
        {
            return Fraction<T>(integral, 1);
        }

        while (iters--)
        {
            Fraction<T> middle(lower.numerator + upper.numerator, lower.denominator + upper.denominator);
            T test = middle.fractional();

            if (std::abs(test - fractional) <= max_error)
            {
                return Fraction<T>(middle.numerator + integral * middle.denominator, middle.denominator);
            }

            if (fractional > test)
            {
                lower = middle;
            }
            else
            {
                upper = middle;
            }
        }

        return Fraction<T>::quiet_NaN();
    }
};

template<typename T>
struct UsefulFraction
{
    std::string format;
    std::function<T(T)> operate;
};

int main(int, char**)
{
    MathOpVariable<double> a("a", 21);
    MathOpVariable<double> b("b", 2);
    MathOpSymbolPi<double> c;
    MathOpAdd<double> d(&b, &c);
    MathOpMul<double> e(&a, &d);

    MathOpPow<double> f(&c, &e);

    MathOpSqrt<double> z(&f);

    std::cout << z << " = " << z.get() << '\n';

    MathOpConstantValue<double> output(z.get());
    MathOp<double>* q = z.solve_for(&a, &output);

    std::cout << a << " = " << *q << '\n';//" = " << q->get() << '\n';


    return 0 ;
    std::array<UsefulFraction<double>, 7> uses
    {
        (UsefulFraction<double> {  "(%g) / (%g)",  [](double x) { return x; }}),

        (UsefulFraction<double> { "π(%g) / (%g)",  [](double x) { return x / M_PI; }}),
        (UsefulFraction<double> {  "(%g) / (π%g)", [](double x) { return x * M_PI; }}),

        (UsefulFraction<double> { "e(%g) / (%g)",  [](double x) { return x / M_E; }}),
        (UsefulFraction<double> {  "(%g) / (e%g)", [](double x) { return x * M_E; }}),

        (UsefulFraction<double> { "√2⋅(%g) / (%g)",  [](double x) { return x / M_SQRT2; }}),
        (UsefulFraction<double> {  "(%g) / (√2⋅%g)", [](double x) { return x * M_SQRT2; }}),
    };

    double value = 15.0 / 4.0;//2.0 / 3.0 / std::sin(M_PI / 4);//2.0 / 3.0 * M_PI;//19.0 / 30.0 * M_PI;

    for(auto i: uses)
    {
        auto x = i.operate(value);
        auto f = Fraction<double>::find(x, 1E-30, 1000);
        if (!f.isnan() && f.numerator < 1000)
        {
            printf(i.format.c_str(), f.numerator, f.denominator);
            printf("\n");
            break;
        }
    }

    return 0;
}
