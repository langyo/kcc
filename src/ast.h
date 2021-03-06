/*
 * ast.h
 *
 *  Created on: 2018/7/6
 *      Author: xiaoc
 */

#ifndef AST_H_
#define AST_H_

#include "kcc.h"
#include "lex.h"
#include "format.h"

namespace kcc {
    struct Token;

    class Visitor;

    struct SourcePos {
        int line;
        int col;
        const char *filename;

        SourcePos() = default;

        SourcePos(const char *_filename, int a, int b) {
            line = a;
            col = b;
            filename = _filename;
        }
    };

    class Type;

    struct Value {
        enum Type {
            None = 0,
            Int = 1,
            Float = 2,
            Register = 4,
            Imm = 8,
            Mem = 16,
        } type;
        int offset;
        double fImm;
        int iImm;

        Value() {
            type = Type::None;
        }

        Value(Type t, int o) : type(t), offset(o) {}

        explicit Value(int i) {
            type = static_cast<Type>(Type::Imm | Type::Int);
            iImm = i;
        }

        explicit Value(double i) {
            type = static_cast<Type>(Type::Imm | Type::Float);
            fImm = i;
        }

        int getAddress() const {
            assert(isMemObj());
            return offset;
        }

        int getReg() const {
            assert(isRegister());
            return offset;
        }
        int getImm()const{
            assert(isImm());
            return iImm;
        }
        bool isRegister() const { return type & Type::Register; }

        bool isFloat() const { return type & Type::Float; }

        bool isInt() const { return type & Type::Int; }

        bool isMemObj() const { return type & Type::Mem; }

        bool isImm() const { return type & Type::Imm; }

        bool isNone() const { return type == Type::None; }

        static Value makeIReg(int i) {
            return Value(static_cast<Type>(Type::Int | Type::Register), i);
        }

        static Value makeFReg(int i) {
            return Value(static_cast<Type>(Type::Float | Type::Register), i);
        }

        static Value makeIMem(int i) {
            return Value(static_cast<Type>(Type::Int | Type::Mem), i);
        }

        static Value makeFMem(int i) {
            return Value(static_cast<Type>(Type::Int | Type::Mem), i);
        }

    };

    struct Record {
        Type *type;
        Value addr;
        Value reg;
        bool isGlobal;

        Record() {
            type = nullptr;
        }
    };

    class AST {
    protected:
        std::vector<AST *> children;
        Token content;
        AST *parent;
        Record record;

        virtual void linkRec();

    public:
        bool isFloat;
        bool isGlobal;
        unsigned int scale;

        void setType(Type *ty) {
            record.type = ty;
        }

        void setAddr(const Value &a) {
            record.addr = a;
        }

        Value getAddr() const { return record.addr; }

        Value getReg() const { return record.reg; }

        void setReg(const Value &r) { record.reg = r; }

        Type *getType() const {
            return record.type;
        }

        SourcePos pos;

        AST();

        void setContent(const Token &t) {
            content = t;
        }

        virtual std::string str(int depth = 0) const;

        virtual std::string info() const;

        virtual const std::string kind() const { return std::string(); };

        inline AST *first() const {
            return children.at(0);
        }

        inline AST *second() const {
            return children.at(1);
        }

        inline AST *third() const {
            return children.at(2);
        }

        inline AST *forth() const {
            return children.at(3);
        }

        inline void add(AST *t) {
            children.push_back(t);
        }

        inline int size() const {
            return children.size();
        }

        inline const Token &getToken() const {
            return content;
        }

        inline std::vector<AST *>::reverse_iterator rbegin() {
            return children.rbegin();
        }

        inline std::vector<AST *>::reverse_iterator rend() {
            return children.rend();
        }

        inline std::vector<AST *>::iterator begin() {
            return children.begin();
        }

        inline std::vector<AST *>::iterator end() {
            return children.end();
        }

        void set(int i, AST *ast) {
            children[i] = ast;
        }

        AST *get(int i) {
            return children.at(i);
        }

        virtual ~AST();

        virtual void accept(Visitor *vis);

        virtual void link();

        AST *getParent() const { return parent; }

        const std::string &tok() const { return getToken().tok; }

        std::string getPos() const {
            return format("{}:{}:{}", pos.filename, pos.line, pos.col);
        }
    };

    const char *printstr(AST *ast);

    class BinaryExpression : public AST {
    public:
        explicit BinaryExpression(const Token &t) {
            content = t;
            scale = 1;
        }

        BinaryExpression() { scale = 1; }

        const std::string kind() const override { return "BinaryExpression"; }

        void accept(Visitor *) override;

        AST *lhs() const { return first(); }

        AST *rhs() const { return second(); }
    };

    class PostfixExpr : public AST {
    public:
        explicit PostfixExpr(const Token &t) { content = t; }

        PostfixExpr() = default;

        const std::string kind() const override { return "PostfixExpr"; }

        void accept(Visitor *) override;
    };

    class UnaryExpression : public AST {
    public:
        explicit UnaryExpression(const Token &t) { content = t; }

        UnaryExpression() = default;

        const std::string kind() const override { return "UnaryExpression"; }

        void accept(Visitor *) override;

        AST *expr() const { return first(); }
    };

    class TernaryExpression : public AST {
    public:
        TernaryExpression() = default;

        const std::string kind() const override { return "TernaryExpression"; }

        void accept(Visitor *) override;
    };

    class Identifier : public AST {
    public:
        Identifier() = default;

        explicit Identifier(const Token &t) { content = t; }

        const std::string kind() const override { return "Identifier"; }

        void accept(Visitor *) override;
    };

    class Number : public AST {
    public:
        Number() {}

        explicit Number(const Token &t) { content = t; }

        const std::string kind() const override { return "Number"; }

        void accept(Visitor *) override;

        int getInt() {
            std::istringstream in(tok());
            int i;
            in >> i;
            return i;
        }

        double getFloat() {
            std::istringstream in(tok());
            double i;
            in >> i;
            return i;
        }
    };

    class Literal : public AST {
    public:
        explicit Literal(const Token &t) { content = t; }

        const std::string kind() const override { return "Literal"; }

        void accept(Visitor *) override;
    };

    class CastExpression : public AST {
    public:
        CastExpression() = default;

        const std::string kind() const override { return "CastExpression"; }

        void accept(Visitor *) override;
    };

    class IndexExpression : public AST {
    public:
        const std::string kind() const override { return "IndexExpression"; }

        void accept(Visitor *) override;
    };

    class ArgumentExepressionList;

    class CallExpression : public AST {
    public:
        const std::string kind() const override { return "CallExpression"; }

        void accept(Visitor *) override;

        AST *callee() const { return first(); }

        ArgumentExepressionList *arg() const { return (ArgumentExepressionList *) second(); }
    };

    class ArgumentExepressionList : public AST {
    public:
        const std::string kind() const override { return "ArgumentExepressionList"; }

        void accept(Visitor *) override;
    };

    class Type : public AST {
    public:
        virtual bool isPrimitive() const { return false; }

        virtual bool isArray() const { return false; }

        virtual bool isPointer() const { return false; }

        virtual std::string repr() const { return std::string(); }
    };

    class PrimitiveType : public Type {
    public:
        explicit PrimitiveType(const Token &t) { content = t; }

        const std::string kind() const override { return "PrimitiveType"; }

        void accept(Visitor *) override;

        bool isPrimitive() const override { return true; }

        std::string repr() const { return tok(); }
    };

    class PointerType : public Type {
    public:
        explicit PointerType() {}

        const std::string kind() const override { return "PointerType"; }

        void accept(Visitor *) override;

        bool isPointer() const override { return true; }

        Type *ptrTo() const { return (Type *) first(); }

        std::string repr() const { return ((Type *) first())->repr().append("*"); }
    };

    class ArrayType : public Type {
        int arrSize;
    public:
        explicit ArrayType(int size = -1) {
            arrSize = size;
        }

        const std::string kind() const override { return "ArrayType"; }

        std::string info() const override;

        void accept(Visitor *) override;
    };

    class FuncArgType;

    class FuncType : public Type {
    public:
        explicit FuncType() {}

        const std::string kind() const override { return "FuncType"; }

        void accept(Visitor *) override;

        Type *ret() const { return (Type *) first(); }

        FuncArgType *arg() const { return (FuncArgType *) second(); }
    };

    class FuncArgType : public Type {
    public:
        explicit FuncArgType() {}

        const std::string kind() const override { return "FuncArgType"; }

        void accept(Visitor *) override;


    };

    class While : public AST {
    public:
        const std::string kind() const override { return "While"; }

        void accept(Visitor *) override;

        AST *cond() const { return first(); }

        AST *body() const { return second(); }
    };

    class If : public AST {
    public:
        const std::string kind() const override { return "If"; }

        void accept(Visitor *) override;

        AST *cond() const { return first(); }

        AST *body() const { return second(); }

        AST *elsePart() const { return third(); }
    };

    class Block : public AST {
    public:
        const std::string kind() const override { return "Block"; }

        void accept(Visitor *) override;
    };

    class TopLevel : public AST {
    public:
        const std::string kind() const override { return "TopLevel"; }

        void accept(Visitor *) override;
    };

    class DeclarationList : public AST {
    public:
        const std::string kind() const override { return "DeclarationList"; }

        void accept(Visitor *) override;
    };

    class Declaration : public AST {
    public:
        const std::string kind() const override { return "Declaration"; }

        void accept(Visitor *) override;

        Type *type() const { return (Type *) first(); }

        Identifier *identifier() const { return (Identifier *) second(); }

    };

    class FuncDefArg : public AST {
    public:

        const std::string kind() const override { return "FuncDefArg"; }

        void accept(Visitor *) override;

        FuncArgType *extractArgType();
    };

    class FuncDef : public AST {
    public:
        unsigned int frameSize;

        const std::string kind() const override { return "FuncDef"; }

        void accept(Visitor *) override;

        FuncType *extractCallSignature();

        const std::string &name() const { return second()->tok(); }

        FuncDefArg *arg() const {
            return (FuncDefArg *) third();
        }

        Block *block() const {
            return (Block *) forth();
        }
    };

    class Return : public AST {
    public:
        const std::string kind() const override { return "Return"; }

        void accept(Visitor *) override;
    };

    class For : public AST {
    public:
        const std::string kind() const override { return "For"; }

        void accept(Visitor *) override;

        AST *init() const { return first(); }

        AST *cond() const { return second(); }

        AST *step() const { return third(); }

        AST *body() const { return forth(); }
    };

    class Empty : public AST {
    public:
        const std::string kind() const override { return "Empty"; }

        void accept(Visitor *) override;
    };

    class Enum : public AST {
    public:
        const std::string kind() const override { return "Enum"; }

        void accept(Visitor *) override;
    };
}
template<>
struct Formatter<kcc::Value> {
    const char *str(kcc::Value i) {
        using kcc::Value;
        std::string ty;
        if (i.type & Value::Type::Register) {
            if (i.type & Value::Type::Int) {
                ty = format("i{}", i.offset);
            } else {
                assert(i.type & Value::Type::Float);
                ty = format("f{}", i.offset);
            }
        } else if (i.type & Value::Type::Imm) {
            if (i.type & Value::Type::Int) {
                ty = format("{}", i.iImm);
            } else {
                assert(i.type & Value::Type::Float);
                ty = format("{}", i.fImm);
            }
        } else if (i.type & Value::Type::Mem) {
            if (i.type & Value::Type::Int) {
                ty = format("i[{}]", i.offset);
            } else {
                assert(i.type & Value::Type::Float);
                ty = format("f[{}]", i.offset);
            }
        }
        return ty.c_str();
    }
};

#endif /* AST_H_ */
