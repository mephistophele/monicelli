#ifndef NODES_HPP
#define NODES_HPP

/*
 * Monicelli: an esoteric language compiler
 * 
 * Copyright (C) 2014 Stefano Sanfilippo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Emitter.hpp"
#include "Pointers.hpp"

#include "location.hh"

#include <functional>
#include <unordered_set>
#include <boost/optional.hpp>

#define maybe_return(val) \
    if ((val) != nullptr) return *(val); else return boost::none;

namespace monicelli {

enum class Type {
    INT,
    CHAR,
    FLOAT,
    BOOL,
    DOUBLE,
    VOID,
    UNKNOWN
};

std::ostream& operator<<(std::ostream&, Type const&);

enum class Operator {
    PLUS, MINUS, TIMES, DIV,
    SHL, SHR,
    LT, GT, GTE, LTE, EQ
};

std::ostream& operator<<(std::ostream&, Operator const&);

class Localizable {
public:
    void setLocation(location const& l) {
        loc = l;
    }

    location const& getLocation() const {
        return loc;
    }

private:
    location loc;
};

class Emittable: public Localizable {
public:
    virtual ~Emittable() {}
    virtual bool emit(Emitter *emitter) const = 0;
};


class Statement: virtual public Emittable {};

class Expression: virtual public Emittable {};

class SimpleExpression: public Expression {};

class SemiExpression: public Localizable {
public:
    SemiExpression(Operator op, Expression *l): op(op), left(l) {}

    Expression const& getLeft() const {
        return *left;
    }

    Operator getOperator() const {
        return op;
    }

private:
    Operator op;
    Pointer<Expression> left;
};


class Id: public SimpleExpression {
public:
    explicit Id(std::string *c): value(c) {}
    explicit Id(char const* c) {
        value = Pointer<std::string>(new std::string(c));
    }
    explicit Id(std::string const& c) {
        value = Pointer<std::string>(new std::string(c));
    }

    virtual bool emit(Emitter *emitter) const {
        return emitter->emit(*this);
    }

    std::string const& getValue() const {
        return *value;
    }

private:
    Pointer<std::string> value;
};

static inline
bool operator==(Id const& a, Id const& b) {
    return a.getValue() == b.getValue();
}


class Number: public SimpleExpression {};

class Integer: public Number {
public:
    Integer(long i): value(i) {}

    virtual bool emit(Emitter *emitter) const {
        return emitter->emit(*this);
    }

    long getValue() const {
        return value;
    }

private:
    long value;
};


class Float: public Number {
public:
    Float(double f): value(f) {}

    virtual bool emit(Emitter *emitter) const {
        return emitter->emit(*this);
    }

    double getValue() const {
        return value;
    }

private:
    double value;
};


class Return: public Statement {
public:
    explicit Return(Expression *e): expression(e) {}

    virtual bool emit(Emitter *emitter) const {
        return emitter->emit(*this);
    }

    boost::optional<Expression const&> getExpression() const {
        maybe_return(expression);
    }

private:
    Pointer<Expression> expression;
};


class Loop: public Statement {
public:
    Loop(PointerList<Statement> *b, Expression *c): body(b), condition(c) {}

    virtual bool emit(Emitter *emitter) const {
        return emitter->emit(*this);
    }

    PointerList<Statement> const& getBody() const {
        return *body;
    }

    Expression const& getCondition() const {
        return *condition;
    }

private:
    Pointer<PointerList<Statement>> body;
    Pointer<Expression> condition;
};


class VarDeclaration: public Statement {
public:
    VarDeclaration(Id *n, Type t, bool p, Expression *i):
        name(n), point(p), init(i), type(t) {}

    virtual bool emit(Emitter *emitter) const {
        return emitter->emit(*this);
    }

    Id const& getId() const {
        return *name;
    }

    bool isPointer() const {
        return point;
    }

    boost::optional<Expression const&> getInitializer() const {
        maybe_return(init);
    }

    Type getType() const {
        return type;
    }

private:
    Pointer<Id> name;
    bool point;
    Pointer<Expression> init;
    Type type;
};


class Assignment: public Statement {
public:
    Assignment(Id *n, Expression *v): name(n), value(v) {}

    virtual bool emit(Emitter *emitter) const {
        return emitter->emit(*this);
    }

    Id const& getName() const {
        return *name;
    }

    Expression const& getValue() const {
        return *value;
    }

private:
    Pointer<Id> name;
    Pointer<Expression> value;
};


class Print: public Statement {
public:
    explicit Print(Expression *e): expression(e) {}

    virtual bool emit(Emitter *emitter) const {
        return emitter->emit(*this);
    }

    Expression const& getExpression() const {
        return *expression;
    }

private:
    Pointer<Expression> expression;
};


class Input: public Statement {
public:
    explicit Input(Id *v): variable(v) {}

    virtual bool emit(Emitter *emitter) const {
        return emitter->emit(*this);
    }

    Id const& getVariable() const {
        return *variable;
    }

private:
    Pointer<Id> variable;
};


class Abort: public Statement {
public:
    virtual bool emit(Emitter *emitter) const {
        return emitter->emit(*this);
    }
};


class Assert: public Statement {
public:
    explicit Assert(Expression *e): expression(e) {}

    virtual bool emit(Emitter *emitter) const {
        return emitter->emit(*this);
    }

    Expression const& getExpression() const {
        return *expression;
    }

private:
    Pointer<Expression> expression;
};


class FunctionCall: public Statement, public Expression {
public:
    FunctionCall(Id *n, PointerList<Expression> *a): name(n), args(a) {}

    virtual bool emit(Emitter *emitter) const {
        return emitter->emit(*this);
    }

    Id const& getName() const {
        return *name;
    }

    PointerList<Expression> const& getArgs() const {
        return *args;
    }

private:
    Pointer<Id> name;
    Pointer<PointerList<Expression>> args;
};

class BranchCase: public Localizable {
public:
    BranchCase(SemiExpression *c, PointerList<Statement> *b): condition(c), body(b) {}

    SemiExpression const& getCondition() const {
        return *condition;
    }

    PointerList<Statement> const& getBody() const {
        return *body;
    }

private:
    Pointer<SemiExpression> condition;
    Pointer<PointerList<Statement>> body;
};


class Branch: public Statement {
public:
    class Body: public Localizable {
    public:
        Body(PointerList<BranchCase> *c, PointerList<Statement> *e = nullptr): cases(c), els(e) {}

        PointerList<BranchCase> const& getCases() const {
            return *cases;
        }

        boost::optional<PointerList<Statement> const&> getElse() const {
            maybe_return(els);
        }

    private:
        Pointer<PointerList<BranchCase>> cases;
        Pointer<PointerList<Statement>> els;
    };

    Branch(Id *v, Branch::Body *b): var(v), body(b) {}

    virtual bool emit(Emitter *emitter) const {
        return emitter->emit(*this);
    }

    Id const& getVar() const {
        return *var;
    }

    Branch::Body const& getBody() const {
        return *body;
    }

private:
    Pointer<Id> var;
    Pointer<Branch::Body> body;
};


Function *makeMain(PointerList<Statement> *body);

class FunArg: public Localizable {
public:
    FunArg(Id *n, Type t, bool p): name(n), type(t), pointer(p) {}

    Id const& getName() const {
        return *name;
    }

    Type getType() const {
        return type;
    }

    bool isPointer() const {
        return pointer;
    }

private:
    Pointer<Id> name;
    Type type;
    bool pointer;
};


class FunctionPrototype: public Emittable {
public:
    FunctionPrototype(Id *n, Type r, PointerList<FunArg> *a):
        name(n), type(r), args(a) {}

    virtual bool emit(Emitter *emitter) const {
        return emitter->emit(*this);
    }

    Id const& getName() const {
        return *name;
    }

    Type getType() const {
        return type;
    }

    PointerList<FunArg> const& getArgs() const {
        return *args;
    }

private:
    Pointer<Id> name;
    Type type;
    Pointer<PointerList<FunArg>> args;
};

static inline
bool operator==(const FunctionPrototype &a, const FunctionPrototype &b) {
    return a.getName() == b.getName();
}

static inline
size_t hash_value(const monicelli::FunctionPrototype &e) {
    return std::hash<std::string>()(e.getName().getValue());
}

class Function: public Emittable {
public:
    Function(FunctionPrototype *p, PointerList<Statement> *b):
        prototype(p), body(b) {}

    virtual bool emit(Emitter *emitter) const {
        return emitter->emit(*this);
    }

    FunctionPrototype const& getPrototype() const {
        return *prototype;
    }

    PointerList<Statement> const& getBody() const {
        return *body;
    }

private:
    Pointer<FunctionPrototype> prototype;
    Pointer<PointerList<Statement>> body;
};


class Module: public Emittable {
public:
    enum ModuleType {
        SYSTEM, USER
    };

    Module(const std::string &n, ModuleType s): name(n), type(s) {}

    virtual bool emit(Emitter *emitter) const {
        return emitter->emit(*this);
    }

    std::string const& getName() const {
        return name;
    }

    ModuleType getType() const {
        return type;
    }

private:
    std::string name;
    ModuleType type;
};

static inline
bool operator==(const Module &a, const Module &b) {
    return (a.getName() == b.getName()) && (a.getType() == b.getType());
}

static inline
size_t hash_value(const monicelli::Module &e) {
    return std::hash<std::string>()(e.getName()) ^ std::hash<bool>()(e.getType());
}

class Program: public Emittable {
public:
    virtual bool emit(Emitter *emitter) const {
        return emitter->emit(*this);
    }

    void setMain(Function *m) {
        main = Pointer<Function>(m);
    }

    void addFunction(Function *f) {
        functions.push_back(f);
    }

    void addModule(Module *m) {
        modules.insert(m);
    }

    boost::optional<Function const&> getMain() const {
        maybe_return(main);
    }

    PointerList<Function> const& getFunctions() const {
        return functions;
    }

    PointerSet<Module> const& getModules() const {
        return modules;
    }

private:
    Pointer<Function> main;
    PointerList<Function> functions;
    PointerSet<Module> modules;
};


class BinaryExpression: public Expression {
public:
    BinaryExpression(Expression *l, Operator op, Expression *r):
        left(l), op(op), right(r) {}

    virtual bool emit(Emitter *emitter) const {
        return emitter->emit(*this);
    }

    Expression const& getLeft() const {
        return *left;
    }

    Expression const& getRight() const {
        return *right;
    }

    Operator getOperator() const {
        return op;
    }

private:
    Pointer<Expression> left;
    Operator op;
    Pointer<Expression> right;
};


class ExpLt: public BinaryExpression {
public:
    ExpLt(Expression *l, Expression *r): BinaryExpression(l, Operator::LT, r) {}
};


class ExpGt: public BinaryExpression {
public:
    ExpGt(Expression *l, Expression *r): BinaryExpression(l, Operator::GT, r) {}
};


class ExpLte: public BinaryExpression {
public:
    ExpLte(Expression *l, Expression *r): BinaryExpression(l, Operator::LTE, r) {}
};


class ExpGte: public BinaryExpression {
public:
    ExpGte(Expression *l, Expression *r): BinaryExpression(l, Operator::GTE, r) {}
};


class ExpPlus: public BinaryExpression {
public:
    ExpPlus(Expression *l, Expression *r): BinaryExpression(l, Operator::PLUS, r) {}

};


class ExpMinus: public BinaryExpression {
public:
    ExpMinus(Expression *l, Expression *r): BinaryExpression(l, Operator::MINUS, r) {}
};


class ExpTimes: public BinaryExpression {
public:
    ExpTimes(Expression *l, Expression *r): BinaryExpression(l, Operator::TIMES, r) {}
};


class ExpDiv: public BinaryExpression {
public:
    ExpDiv(Expression *l, Expression *r): BinaryExpression(l, Operator::DIV, r) {}
};


class ExpShl: public BinaryExpression {
public:
    ExpShl(Expression *l, Expression *r): BinaryExpression(l, Operator::SHL, r) {}
};


class ExpShr: public BinaryExpression {
public:
    ExpShr(Expression *l, Expression *r): BinaryExpression(l, Operator::SHR, r) {}
};

class SemiExpEq: public SemiExpression {
public:
    SemiExpEq(Expression *l): SemiExpression(Operator::EQ, l) {}
};


class SemiExpLt: public SemiExpression {
public:
    SemiExpLt(Expression *l): SemiExpression(Operator::LT, l) {}
};


class SemiExpGt: public SemiExpression {
public:
    SemiExpGt(Expression *l): SemiExpression(Operator::GT, l) {}
};


class SemiExpLte: public SemiExpression {
public:
    SemiExpLte(Expression *l): SemiExpression(Operator::LTE, l) {}
};


class SemiExpGte: public SemiExpression {
public:
    SemiExpGte(Expression *l): SemiExpression(Operator::GTE, l) {}
};


class SemiExpPlus: public SemiExpression {
public:
    SemiExpPlus(Expression *l): SemiExpression(Operator::PLUS, l) {}
};


class SemiExpMinus: public SemiExpression {
public:
    SemiExpMinus(Expression *l): SemiExpression(Operator::MINUS, l) {}
};


class SemiExpTimes: public SemiExpression {
public:
    SemiExpTimes(Expression *l): SemiExpression(Operator::TIMES, l) {}
};


class SemiExpDiv: public SemiExpression {
public:
    SemiExpDiv(Expression *l): SemiExpression(Operator::DIV, l) {}
};


class SemiExpShl: public SemiExpression {
public:
    SemiExpShl(Expression *l): SemiExpression(Operator::SHR, l) {}
};


class SemiExpShr: public SemiExpression {
public:
    SemiExpShr(Expression *l): SemiExpression(Operator::SHL, l) {}
};

} // namespace

#undef maybe_return

#endif

