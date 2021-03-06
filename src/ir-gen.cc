//
// Created by xiaoc on 2018/9/17.
//

#include "ir-gen.h"

using namespace kcc;

void kcc::IRGenerator::visit(kcc::For *aFor) {

}

void kcc::IRGenerator::visit(kcc::Identifier *identifier) {
    if (identifier->isGlobal) {
        emit(Opcode::loadGlobal, identifier->getReg(), identifier->tok());
    } else
        emit(Opcode::load, identifier->getReg(), identifier->getAddr());
}

void kcc::IRGenerator::visit(kcc::While *aWhile) {
    int begin = (int) ir().size();
    aWhile->cond()->accept(this);
    auto cond = aWhile->cond()->getReg();
    int branchIdx = (int) ir().size();
    emit(Opcode::branch, cond);
    aWhile->body()->accept(this);
    emit(Opcode::jmp, Value(begin));
    patch(branchIdx, Opcode::branch, cond, Value(branchIdx + 1), Value((int)ir().size()));
}

void kcc::IRGenerator::visit(kcc::Block *block) {
    for (auto i:*block) {
        i->accept(this);
    }
}

void kcc::IRGenerator::visit(kcc::TopLevel *level) {
    visitAll(level);
}

void kcc::IRGenerator::visit(kcc::If *anIf) {
    anIf->cond()->accept(this);
    auto cond = anIf->cond()->getReg();
    int branchIdx = (int)ir().size();
    emit(Opcode::branch, cond);
    anIf->body()->accept(this);
    int jmpIdx = (int) ir().size();
    emit(Opcode::jmp, Value(0));
    int a = (int) ir().size();
    if (anIf->size() == 3) {
        anIf->elsePart()->accept(this);
    }
    patch(branchIdx, Opcode::branch, cond, Value(branchIdx + 1), Value(a));
    patch(jmpIdx, Opcode::jmp, Value((int)ir().size()));

}

void kcc::IRGenerator::visit(kcc::TernaryExpression *expression) {

}

void kcc::IRGenerator::visit(kcc::Number *number) {
    if (number->isFloat)
        emit(Opcode::fconst, number->getReg(), Value(std::stod(number->tok())));
    else
        emit(Opcode::iconst, number->getReg(), Value(std::stoi(number->tok())));
}

void kcc::IRGenerator::visit(kcc::Return *aReturn) {
    aReturn->first()->accept(this);
    emit(Opcode::ret, aReturn->first()->getReg());
}

void kcc::IRGenerator::visit(kcc::Empty *empty) {

}

void kcc::IRGenerator::visit(kcc::PrimitiveType *type) {

}

void kcc::IRGenerator::visit(kcc::PointerType *type) {

}

void kcc::IRGenerator::visit(kcc::ArrayType *type) {

}

void kcc::IRGenerator::visit(kcc::ArgumentExepressionList *list) {
    for (auto i:*list) {
        i->accept(this);
    }
}

void kcc::IRGenerator::visit(kcc::FuncDefArg *arg) {

}

void kcc::IRGenerator::visit(kcc::FuncDef *def) {
    auto funcName = def->name();
    funcs.emplace_back(Function(funcName,    def->frameSize));
    def->block()->accept(this);
}

void kcc::IRGenerator::visit(kcc::CallExpression *expression) {
    auto arg = expression->arg();
    arg->accept(this);
    int a = 0, b = 0;
    for (auto i:*arg) {
        if (i->isFloat) {
            emit(Opcode::pushf, i->getReg(), Value(a++));
        } else {
            emit(Opcode::pushi, i->getReg(), Value(b++));
        }
    }
    auto callee = expression->callee();
    if(callee->kind() == Identifier().kind() && callee->isGlobal) {
        emit(Opcode::callGlobal,callee->tok());
    }
}

void kcc::IRGenerator::visit(kcc::CastExpression *expression) {

}

void kcc::IRGenerator::visit(kcc::IndexExpression *expression) {

}

void kcc::IRGenerator::visit(kcc::Declaration *declaration) {

}

void kcc::IRGenerator::visit(kcc::DeclarationList *list) {

}

void kcc::IRGenerator::visit(kcc::Literal *literal) {
    emit(Opcode::sconst, literal->getReg(), literal->tok());
}

void kcc::IRGenerator::visit(kcc::BinaryExpression *expression) {
    expression->rhs()->accept(this);

    auto &op = expression->tok();
    if (op == "=") {
        if (expression->lhs()->isFloat && !expression->rhs()->isFloat) {
            emit(Opcode::cvti2f, expression->rhs()->getReg(), expression->rhs()->getReg());
        }
        if (!expression->lhs()->isFloat && expression->rhs()->isFloat) {
            emit(Opcode::cvtf2i, expression->rhs()->getReg(), expression->rhs()->getReg());
        }
        if (expression->lhs()->kind() == Identifier().kind()) {
            emit(Opcode::store, expression->lhs()->getAddr(), expression->rhs()->getReg());
        }
    } else {
        expression->lhs()->accept(this);
        if (expression->lhs()->isFloat || expression->rhs()->isFloat) {
            if (!expression->lhs()->isFloat) {
                emit(Opcode::cvti2f, expression->lhs()->getReg(),
                     expression->lhs()->getReg());
            }
            if (!expression->rhs()->isFloat) {
                emit(Opcode::cvti2f, expression->rhs()->getReg(),
                     expression->rhs()->getReg());
            }
            Opcode opcode;
            if (op == "+") {
                opcode = Opcode::fadd;
            } else if (op == "-") {
                opcode = Opcode::fsub;
            } else if (op == "*") {
                opcode = Opcode::fmul;
            } else if (op == "/") {
                opcode = Opcode::fdiv;
            } else if (op == "<") {
                opcode = Opcode::fl;
            } else if (op == "<=") {
                opcode = Opcode::fle;
            } else if (op == ">") {
                opcode = Opcode::fg;
            } else if (op == ">=") {
                opcode = Opcode::fge;
            } else if (op == "==") {
                opcode = Opcode::fe;
            } else if (op == "!=") {
                opcode = Opcode::fne;
            }
            emit(opcode, expression->getReg(),
                 expression->lhs()->getReg(),
                 expression->rhs()->getReg());
        } else {
            Opcode opcode;
            if (op == "+") {
                opcode = Opcode::iadd;
            } else if (op == "-") {
                opcode = Opcode::isub;
            } else if (op == "*") {
                opcode = Opcode::imul;
            } else if (op == "/") {
                opcode = Opcode::idiv;
            } else if (op == "<") {
                opcode = Opcode::il;
            } else if (op == "<=") {
                opcode = Opcode::ile;
            } else if (op == ">") {
                opcode = Opcode::ig;
            } else if (op == ">=") {
                opcode = Opcode::ige;
            } else if (op == "==") {
                opcode = Opcode::ie;
            } else if (op == "!=") {
                opcode = Opcode::ine;
            }
            emit(opcode, expression->getReg(),
                 expression->lhs()->getReg(),
                 expression->rhs()->getReg());
        }
    }
}

void kcc::IRGenerator::visit(kcc::UnaryExpression *expression) {

}

void kcc::IRGenerator::pre(kcc::AST *ast) {

}

void kcc::IRGenerator::visit(kcc::Enum *anEnum) {

}

void kcc::IRGenerator::visit(kcc::FuncType *type) {

}

void kcc::IRGenerator::visit(kcc::PostfixExpr *expr) {

}

void kcc::IRGenerator::visit(kcc::FuncArgType *type) {

}

void IRGenerator::buildSSA() {
    for (auto func:funcs) {
        auto cfg = func.generateCFG();
        cfg->buildSSA();
        cfg->dump();
    }
}


