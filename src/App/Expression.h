/***************************************************************************
 *   Copyright (c) Eivind Kvedalen (eivind@kvedalen.name) 2015             *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <string>
#include <boost/tuple/tuple.hpp>
#include <Base/Exception.h>
#include <Base/Unit.h>
#include <App/PropertyLinks.h>
#include <App/ObjectIdentifier.h>
#include <Base/BaseClass.h>
#include <Base/Quantity.h>
#include <set>
#include <deque>
#include <App/Range.h>

namespace Base {
class XMLReader;
}

namespace App  {

class DocumentObject;
class Expression;
class Document;

typedef std::map<App::DocumentObject*, std::map<std::string, std::vector<ObjectIdentifier> > > ExpressionDeps;

class AppExport ExpressionVisitor {
public:
    virtual ~ExpressionVisitor() {}
    virtual void visit(Expression * e) = 0;
    virtual void setExpressionChanged() {}
    virtual int changed() const { return 0;}
    virtual void reset() {}
    virtual App::PropertyLinkBase* getPropertyLink() {return 0;}

protected:
    void getIdentifiers(Expression &e, std::set<App::ObjectIdentifier> &); 
    void getDeps(Expression &e, ExpressionDeps &); 
    void getDepObjects(Expression &e, std::set<App::DocumentObject*> &, std::vector<std::string> *); 
    bool adjustLinks(Expression &e, const std::set<App::DocumentObject*> &inList);
    bool renameDocument(Expression &e, const std::string &oldName, const std::string &newName);
    bool renameObjectIdentifier(Expression &e,const std::map<ObjectIdentifier,ObjectIdentifier> &paths,
            const ObjectIdentifier &path);
    bool updateElementReference(Expression &e, App::DocumentObject *feature,bool reverse);
    void importSubNames(Expression &e, const std::map<std::string,std::string> &subNameMap);
    void updateLabelReference(Expression &e, App::DocumentObject *obj, 
            const std::string &ref, const char *newLabel);
    void moveCells(Expression &e, const CellAddress &address, int rowCount, int colCount);
};

template<class P> class ExpressionModifier : public ExpressionVisitor {
public:
    ExpressionModifier(P & _prop)
        : prop(_prop),_changed(0) 
    { 
        propLink = dynamic_cast<App::PropertyLinkBase*>(&prop);
    }

    virtual ~ExpressionModifier() { }

    virtual void setExpressionChanged() override{
        ++_changed;
        if (!signaller)
            signaller = boost::shared_ptr<typename AtomicPropertyChangeInterface<P>::AtomicPropertyChange>(AtomicPropertyChangeInterface<P>::getAtomicPropertyChange(prop));
    }

    virtual int changed() const override { return _changed; }

    virtual void reset() override {_changed = 0;}

    virtual App::PropertyLinkBase* getPropertyLink() override {return propLink;}

protected:
    P & prop;
    App::PropertyLinkBase *propLink;
    boost::shared_ptr<typename AtomicPropertyChangeInterface<P>::AtomicPropertyChange> signaller;
    int _changed;
};

/**
  * Base class for expressions.
  *
  */

class AppExport Expression : public Base::BaseClass {
    TYPESYSTEM_HEADER();

public:

    Expression(const App::DocumentObject * _owner);
    virtual ~Expression();

    Expression(const Expression&) = delete;
    void operator=(const Expression &)=delete;

    virtual bool isTouched() const { return false; }

    Expression * eval() const;

    std::string toString(bool persistent=false, bool checkPriority=false, int indent=0) const;

    Expression *copy() const;

    static Expression * parse(const App::DocumentObject * owner, const std::string& buffer);

    virtual int priority() const;

    virtual int jump() const {return 0;}

    void getIdentifiers(std::set<App::ObjectIdentifier> &) const;
    std::set<App::ObjectIdentifier> getIdentifiers() const;

    void getDeps(ExpressionDeps &deps) const;
    ExpressionDeps getDeps() const;

    std::set<App::DocumentObject*> getDepObjects(std::vector<std::string> *labels=0) const;
    void getDepObjects(std::set<App::DocumentObject*> &, std::vector<std::string> *labels=0) const;

    Expression *importSubNames(const std::map<std::string,std::string> &nameMap) const;

    Expression *updateLabelReference(App::DocumentObject *obj, 
            const std::string &ref, const char *newLabel) const;

    bool adjustLinks(const std::set<App::DocumentObject*> &inList);

    virtual Expression *simplify() const { return copy(); }

    void visit(ExpressionVisitor & v);

    class Exception : public Base::Exception {
    public:
        Exception(const char *sMessage) : Base::Exception(sMessage) { }
    };

    App::DocumentObject *  getOwner() const { return owner; }

    boost::any getValueAsAny() const;

    struct AppExport Component {
        ObjectIdentifier::Component comp;
        Expression *e1;
        Expression *e2;

        Component()
            :e1(0),e2(0)
        {}

        Component(const std::string &n)
            :comp(ObjectIdentifier::SimpleComponent(n)),e1(0),e2(0)
        {}

        Component(Expression *e1, Expression *e2=0, bool isRange=false)
            :e1(e1),e2(e2)
        {
            if(isRange) 
                comp = ObjectIdentifier::RangeComponent(0);
        }

        Component(const ObjectIdentifier::Component &comp, Expression *e1=0, Expression *e2=0)
            :comp(comp),e1(e1),e2(e2)
        {}
    };

    virtual void addComponent(const Component &component);

    static void finalize();

protected:
    virtual bool _isIndexable() const {return false;}
    virtual Expression *_copy() const = 0;
    virtual void _toString(std::ostringstream &ss, bool persistent, int indent) const = 0;
    virtual void _getDeps(ExpressionDeps &) const  {}
    virtual void _getDepObjects(std::set<App::DocumentObject*> &, std::vector<std::string> *) const  {}
    virtual void _getIdentifiers(std::set<App::ObjectIdentifier> &) const  {}
    virtual bool _adjustLinks(const std::set<App::DocumentObject*> &, ExpressionVisitor &) {return false;}
    virtual bool _updateElementReference(App::DocumentObject *,bool,ExpressionVisitor &) {return false;}
    virtual bool _renameDocument(const std::string &, const std::string &, ExpressionVisitor &) {return false;}
    virtual void _importSubNames(const std::map<std::string,std::string> &) {}
    virtual void _updateLabelReference(App::DocumentObject *, const std::string &, const char *) {}
    virtual bool _renameObjectIdentifier(const std::map<ObjectIdentifier,ObjectIdentifier> &, 
                                         const ObjectIdentifier &, ExpressionVisitor &) {return false;}
    virtual void _moveCells(const CellAddress &, int, int, ExpressionVisitor &) {}
    virtual boost::any _getValueAsAny() const = 0;
    virtual Expression *_eval() const {return 0;}
    virtual void _visit(ExpressionVisitor &) {}

    Expression *fromAny(boost::any) const;

    friend ExpressionVisitor;

protected:
    App::DocumentObject * owner; /**< The document object used to access unqualified variables (i.e local scope) */

    std::vector<Component> components;
};

/**
  * Part of an expressions that contains a unit.
  *
  */

class  AppExport UnitExpression : public Expression {
    TYPESYSTEM_HEADER();
public:
    UnitExpression(const App::DocumentObject *_owner = 0, const Base::Quantity & _quantity = Base::Quantity(), const std::string & _unitStr = std::string());

    virtual Expression * simplify() const;

    void setUnit(const Base::Quantity &_quantity);

    double getValue() const { return quantity.getValue(); }

    const Base::Unit & getUnit() const { return quantity.getUnit(); }

    const Base::Quantity & getQuantity() const { return quantity; }

    const std::string getUnitString() const { return unitStr; }

    double getScaler() const { return quantity.getValue(); }

protected:
    virtual void _toString(std::ostringstream &ss, bool persistent, int indent) const;
    virtual Expression *_copy() const;
    virtual Expression *_eval() const;
    virtual boost::any _getValueAsAny() const;

protected:
    Base::Quantity quantity;
    std::string unitStr; /**< The unit string from the original parsed string */
};

/**
  * Class implementing a number with an optional unit
  */

class AppExport NumberExpression : public UnitExpression {
    TYPESYSTEM_HEADER();
public:
    NumberExpression(const App::DocumentObject *_owner = 0, const Base::Quantity & quantity = Base::Quantity());

    void negate();

protected:
    virtual void _toString(std::ostringstream &ss, bool persistent, int indent) const;
    virtual Expression *_copy() const;
};

class AppExport ConstantExpression : public NumberExpression {
    TYPESYSTEM_HEADER();
public:
    ConstantExpression(const App::DocumentObject *_owner = 0, std::string _name = "", const Base::Quantity &_quantity = Base::Quantity());

    std::string getName() const { return name; }

    virtual Expression *simplify() const;

protected:
    virtual Expression *_eval() const;
    virtual boost::any _getValueAsAny() const;
    virtual void _toString(std::ostringstream &ss, bool persistent, int indent) const;
    virtual Expression *_copy() const;

    std::string name; /**< Constant's name */
};

class AppExport BooleanExpression : public NumberExpression {
    TYPESYSTEM_HEADER();
public:
    BooleanExpression(const App::DocumentObject *_owner = 0, bool _value = false);

protected:
    virtual Expression *_copy() const;
};


/**
  * Class implementing an infix expression.
  *
  */

class AppExport OperatorExpression : public UnitExpression {
    TYPESYSTEM_HEADER();
public:
    enum Operator {
        NONE,
        ADD,
        SUB,
        MUL,
        DIV,
        MOD,
        POW,
        EQ,
        NEQ,
        LT,
        GT,
        LTE,
        GTE,
        UNIT,
        NEG,
        POS,
        AND_OP,
        AND_OP2,
        OR_OP,
        OR_OP2,
    };
    OperatorExpression(const App::DocumentObject *_owner = 0, Expression * _left = 0, Operator _op = NONE, Expression * _right = 0);

    virtual ~OperatorExpression();

    virtual bool isTouched() const;

    virtual Expression * simplify() const;

    virtual int priority() const;

    Operator getOperator() const { return op; }

    Expression * getLeft() const { return left; }

    Expression * getRight() const { return right; }

protected:
    boost::any _calc(boost::any l, boost::any r) const;
    Expression *_calc(Expression *l, Expression *r) const;
    Expression *_calc(Expression *l) const;

    virtual void _visit(ExpressionVisitor & v);
    virtual void _toString(std::ostringstream &ss, bool persistent, int indent) const;
    virtual Expression *_copy() const;
    virtual Expression *_eval() const;
    virtual boost::any _getValueAsAny() const;

    virtual bool isCommutative() const;

    virtual bool isLeftAssociative() const;

    virtual bool isRightAssociative() const;

    Operator op;        /**< Operator working on left and right */
    Expression * left;  /**< Left operand */
    Expression * right; /**< Right operand */
};

class AppExport AssignmentExpression : public Expression {
    TYPESYSTEM_HEADER();

public:
    AssignmentExpression(const App::DocumentObject *_owner = 0, const std::string &name = std::string(),
            OperatorExpression::Operator _op = OperatorExpression::NONE, Expression * _right = 0);
    AssignmentExpression(const App::DocumentObject *_owner, const std::vector<std::string> &names,
            Expression * _right);
    virtual ~AssignmentExpression();

    void add(Expression *right);

    virtual bool isTouched() const;

    boost::any apply(bool needReturn) const;

protected:
    virtual void _visit(ExpressionVisitor & v);
    virtual void _toString(std::ostringstream &ss, bool persistent, int indent) const;
    virtual Expression *_copy() const;
    virtual boost::any _getValueAsAny() const;

protected:
    OperatorExpression::Operator op;
    std::vector<std::string> names;
    std::vector<Expression*> exprs;
};

class AppExport ConditionalExpression : public Expression {
    TYPESYSTEM_HEADER();
public:
    ConditionalExpression(const App::DocumentObject *_owner = 0, Expression * _condition = 0,Expression * _trueExpr = 0,  Expression * _falseExpr = 0);

    virtual ~ConditionalExpression();

    virtual bool isTouched() const;

    virtual Expression *simplify() const;

    virtual int priority() const;

protected:
    virtual void _visit(ExpressionVisitor & v);
    virtual void _toString(std::ostringstream &ss, bool persistent, int indent) const;
    virtual Expression *_copy() const;
    virtual Expression *_eval() const;
    virtual boost::any _getValueAsAny() const;

    Expression * condition;  /**< Condition */
    Expression * trueExpr;  /**< Expression if abs(condition) is > 0.5 */
    Expression * falseExpr; /**< Expression if abs(condition) is < 0.5 */
};

/**
  * Class implementing various functions, e.g sin, cos, etc.
  *
  */

class AppExport FunctionExpression : public UnitExpression {
    TYPESYSTEM_HEADER();
public:
    FunctionExpression(const App::DocumentObject *_owner=0, int f=0,
            const std::vector<Expression *> &_args = {});

    virtual ~FunctionExpression();

    virtual bool isTouched() const;

    virtual Expression * simplify() const;

    int type() const {return f;}

protected:
    virtual void _visit(ExpressionVisitor & v);
    virtual void _toString(std::ostringstream &ss, bool persistent, int indent) const;
    virtual Expression *_copy() const;
    virtual Expression *_eval() const;
    virtual boost::any _getValueAsAny() const;
    Expression *evalAggregate() const;

    int f;        /**< Function to execute */
    std::vector<Expression *> args; /** Arguments to function*/
};

/**
  * Class implementing a reference to a property. If the name is unqualified,
  * the owner of the expression is searched. If it is qualified, the document
  * that contains the owning document object is searched for other document
  * objects to search. Both labels and internal document names are searched.
  *
  */

class AppExport VariableExpression : public Expression {
    TYPESYSTEM_HEADER();
public:
    VariableExpression(const App::DocumentObject *_owner = 0, ObjectIdentifier _var = ObjectIdentifier());

    ~VariableExpression();

    virtual bool isTouched() const;

    std::string name() const { return var.getPropertyName(); }

    const ObjectIdentifier &getPath() const { return var; }

    void setPath(const ObjectIdentifier & path);

    virtual void addComponent(const Component &component);

protected:
    virtual bool _isIndexable() const;
    virtual void _toString(std::ostringstream &ss, bool persistent, int indent) const;
    virtual Expression *_copy() const;
    virtual void _getDeps(ExpressionDeps &) const;
    virtual void _getDepObjects(std::set<App::DocumentObject*> &, std::vector<std::string> *) const;
    virtual void _getIdentifiers(std::set<App::ObjectIdentifier> &) const;
    virtual bool _adjustLinks(const std::set<App::DocumentObject*> &, ExpressionVisitor &);
    virtual void _importSubNames(const std::map<std::string,std::string> &);
    virtual void _updateLabelReference(App::DocumentObject *, const std::string &, const char *);
    virtual bool _updateElementReference(App::DocumentObject *,bool,ExpressionVisitor &);
    virtual bool _renameDocument(const std::string &, const std::string &, ExpressionVisitor &);
    virtual bool _renameObjectIdentifier(const std::map<ObjectIdentifier,ObjectIdentifier> &, 
                                         const ObjectIdentifier &, ExpressionVisitor &);
    virtual void _moveCells(const CellAddress &, int, int, ExpressionVisitor &);
    virtual boost::any _getValueAsAny() const;

protected:

    ObjectIdentifier var; /**< Variable name  */
};

/**
  * Class implementing a callable expression with named arguments and optional trailing accessor
  */

class AppExport CallableExpression : public Expression {
    TYPESYSTEM_HEADER();
public:
    CallableExpression(const App::DocumentObject *_owner = 0, Expression *expr=0, 
            const std::vector<std::pair<std::string,Expression *> > &_args = {});

    static Expression *create(const App::DocumentObject *owner, const ObjectIdentifier &path,
            const std::vector<std::pair<std::string,Expression *> > &args = {});

    static Py::Object evaluate(const App::DocumentObject *owner, 
            const std::string &expr, PyObject *tuple, PyObject *kwds);

    virtual ~CallableExpression();

    virtual bool isTouched() const;

protected:
    virtual void _visit(ExpressionVisitor & v);
    virtual bool _isIndexable() const { return true; }
    virtual Expression *_eval() const;
    virtual boost::any _getValueAsAny() const;
    virtual void _toString(std::ostringstream &ss, bool persistent, int indent) const;
    virtual Expression *_copy() const;

protected:
    Expression *expr;
    std::string name;
    int ftype;
    std::vector<std::pair<std::string,Expression *> > args;
};

/**
  * Class implementing a string. Used to signal either a genuine string or
  * a failed evaluation of an expression.
  */
class AppExport StringExpression : public Expression {
    TYPESYSTEM_HEADER();
public:
    StringExpression(const App::DocumentObject *_owner = 0, 
            const std::string & _text = std::string(), bool r_literal=false);

    virtual const std::string &getText() const { return text; }

    bool rLiteral() const {return r_literal;}

protected:
    virtual bool _isIndexable() const { return true; }
    virtual Expression * _copy() const;
    virtual void _toString(std::ostringstream &ss, bool persistent, int indent) const;
    virtual boost::any _getValueAsAny() const;

protected:

    std::string text; /**< Text string */
    bool r_literal;
};

//////////////////////////////////////////////////////////////////////

class AppExport RangeExpression : public App::Expression {
    TYPESYSTEM_HEADER();
public:
    RangeExpression(const App::DocumentObject * _owner = 0, const std::string & begin = std::string(), const std::string & end = std::string());

    virtual ~RangeExpression() { }

    virtual bool isTouched() const;

    Range getRange() const;

protected:
    virtual void _toString(std::ostringstream &, bool, int) const;
    virtual Expression *_copy() const;
    virtual void _getDeps(ExpressionDeps &) const;
    virtual bool _renameObjectIdentifier(const std::map<ObjectIdentifier,ObjectIdentifier> &, 
                                         const ObjectIdentifier &, ExpressionVisitor &);
    virtual void _moveCells(const CellAddress &, int, int, ExpressionVisitor &);
    virtual boost::any _getValueAsAny() const;

protected:
    std::string begin;
    std::string end;
};

//////////////////////////////////////////////////////////////////////

class AppExport PyObjectExpression : public Expression {
    TYPESYSTEM_HEADER();

public:
    PyObjectExpression(const App::DocumentObject * _owner=0, PyObject *pyobj=0);
    ~PyObjectExpression();

    Py::Object getPyObject() const;

    void setPyObject(Py::Object pyobj);
    void setPyObject(PyObject *pyobj);

protected:
    virtual boost::any _getValueAsAny() const;
    virtual void _toString(std::ostringstream &,bool, int) const;
    virtual Expression *_copy() const;

protected:
    PyObject *pyObj;
};

//////////////////////////////////////////////////////////////////////

class AppExport ComprehensionExpression : public Expression {
    TYPESYSTEM_HEADER();

public:
    ComprehensionExpression(const App::DocumentObject * _owner=0, 
            const std::vector<std::string> &names={}, Expression *expr=0);

    virtual ~ComprehensionExpression();

    virtual bool isTouched() const;

    void setExpr(Expression *key, Expression *value=0, bool isList=true);

    void add(const std::vector<std::string> &names, Expression *expr);

    void setCondition(Expression *cond);

protected:
    virtual void _visit(ExpressionVisitor & v);
    virtual bool _isIndexable() const {return true;}
    virtual boost::any _getValueAsAny() const;
    virtual void _toString(std::ostringstream &, bool, int) const;
    virtual Expression *_copy() const;

    void _calc(Py::Object &res, size_t index) const;

protected:
    Expression *key;
    Expression *value;
    std::vector<std::pair<std::vector<std::string>, Expression*> > comps;
    Expression *condition;
    bool list;
};

//////////////////////////////////////////////////////////////////////

class AppExport ListExpression : public Expression {
    TYPESYSTEM_HEADER();

public:
    ListExpression(const App::DocumentObject * _owner=0, 
            const std::vector<std::pair<std::string, Expression*> > &items={});
    virtual ~ListExpression();

    virtual bool isTouched() const;

    void addItem(const std::pair<std::string,Expression *> &item);

protected:
    virtual void _visit(ExpressionVisitor & v);
    virtual bool _isIndexable() const {return true;}
    virtual boost::any _getValueAsAny() const;
    virtual void _toString(std::ostringstream &, bool, int) const;
    virtual Expression *_copy() const;

protected:
    std::vector<std::pair<bool,Expression*> > items;
};

//////////////////////////////////////////////////////////////////////

class AppExport TupleExpression : public ListExpression {
    TYPESYSTEM_HEADER();

public:
    TupleExpression(const App::DocumentObject * _owner, const std::pair<std::string,Expression *> &tem);
    TupleExpression(const App::DocumentObject * _owner=0, 
            const std::vector<std::pair<std::string, Expression*> > &items={});

protected:
    virtual boost::any _getValueAsAny() const;
    virtual Expression *_copy() const;
    virtual void _toString(std::ostringstream &, bool, int) const;
};

//////////////////////////////////////////////////////////////////////

class AppExport DictExpression : public Expression {
    TYPESYSTEM_HEADER();

public:
    DictExpression(const App::DocumentObject * _owner=0, Expression *key=0, Expression *value=0);
    virtual ~DictExpression();

    virtual bool isTouched() const;

    void addItem(Expression *key, Expression *value);

protected:
    virtual void _visit(ExpressionVisitor & v);
    virtual bool _isIndexable() const {return true;}
    virtual boost::any _getValueAsAny() const;
    virtual void _toString(std::ostringstream &, bool, int) const;
    virtual Expression *_copy() const;

protected:
    std::vector<std::pair<Expression*,Expression*> > items;
};

//////////////////////////////////////////////////////////////////////

class AppExport IDictExpression : public Expression {
    TYPESYSTEM_HEADER();

public:
    IDictExpression(const App::DocumentObject * _owner=0, 
            const std::string &key=std::string(), Expression *value=0);

    virtual ~IDictExpression();

    virtual bool isTouched() const;

    void addItem(const std::string &key, Expression *value);

protected:
    virtual void _visit(ExpressionVisitor & v);
    virtual bool _isIndexable() const {return true;}
    virtual boost::any _getValueAsAny() const;
    virtual void _toString(std::ostringstream &, bool, int) const;
    virtual Expression *_copy() const;

protected:
    std::vector<std::pair<std::string,Expression*> > items;
};

/////////////////////////////////////////////////////////////////

class AppExport BaseStatement : public Expression {
    TYPESYSTEM_HEADER();
public:
    enum JumpType {
        JUMP_NONE,
        JUMP_RETURN,
        JUMP_BREAK,
        JUMP_CONTINUE,
    };
    BaseStatement(const App::DocumentObject *owner);
protected:
    virtual boost::any _getValueAsAny() const;
};

/////////////////////////////////////////////////////////////////

class AppExport JumpStatement : public BaseStatement {
    TYPESYSTEM_HEADER();

public:

    JumpStatement(const App::DocumentObject *_owner = 0, JumpType type=JUMP_NONE, Expression *expr = 0);
    virtual ~JumpStatement();

    virtual bool isTouched() const;
    virtual int jump() const;
    virtual Expression *simplify() const;

protected:
    virtual void _visit(ExpressionVisitor & v);
    virtual void _toString(std::ostringstream &ss, bool persistent, int indent) const;
    virtual Expression *_copy() const;
    virtual Expression *_eval() const;

protected:
    JumpType type;
    Expression *expr;
};

/////////////////////////////////////////////////////////////////

class AppExport IfStatement : public BaseStatement {
    TYPESYSTEM_HEADER();

public:
    IfStatement(const App::DocumentObject *_owner=0, Expression *condition=0, Expression *statement=0);
    virtual ~IfStatement();

    void addElseIf(Expression *condition, Expression *statement);
    void addElse(Expression *statement);

    virtual bool isTouched() const;

protected:
    virtual void _visit(ExpressionVisitor & v);
    virtual void _toString(std::ostringstream &ss, bool persistent, int indent) const override;
    virtual Expression *_copy() const;
    virtual Expression *_eval() const;

protected:
    std::vector<std::pair<Expression *, Expression *> > exprs;
};

/////////////////////////////////////////////////////////////////

class AppExport WhileStatement : public BaseStatement {
    TYPESYSTEM_HEADER();

public:
    WhileStatement(const App::DocumentObject *_owner=0, Expression *condition=0, Expression *statement=0);
    virtual ~WhileStatement();

    virtual bool isTouched() const;

protected:
    virtual void _visit(ExpressionVisitor & v);
    virtual void _toString(std::ostringstream &ss, bool persistent, int indent) const;
    virtual Expression *_copy() const;
    virtual Expression *_eval() const;

protected:
    Expression *condition;
    Expression *statement;
};

/////////////////////////////////////////////////////////////////

class AppExport ForStatement : public BaseStatement {
    TYPESYSTEM_HEADER();

public:
    ForStatement(const App::DocumentObject *_owner = 0, const std::vector<std::string> &names = {},
            Expression *value=0, Expression *statement=0);
    virtual ~ForStatement();

    void addElse(Expression *expr);

    virtual bool isTouched() const;

protected:
    virtual void _visit(ExpressionVisitor & v);
    virtual void _toString(std::ostringstream &ss, bool persistent, int indent) const;
    virtual Expression *_copy() const;
    virtual Expression *_eval() const;

protected:
    std::vector<std::string> names;
    Expression *value;
    Expression *statement;
    Expression *else_expr;
};

/////////////////////////////////////////////////////////////////

class AppExport SimpleStatement : public BaseStatement {
    TYPESYSTEM_HEADER();

public:
    SimpleStatement(const App::DocumentObject *_owner = 0, Expression *expr=0);
    virtual ~SimpleStatement();

    virtual bool isTouched() const;
    void add(Expression *expr);

protected:
    virtual void _visit(ExpressionVisitor & v);
    virtual void _toString(std::ostringstream &ss, bool persistent, int indent) const;
    virtual Expression *_copy() const;
    virtual Expression *_eval() const;

protected:
    std::vector<Expression*> exprs;
};

/////////////////////////////////////////////////////////////

class AppExport Statement : public SimpleStatement {
    TYPESYSTEM_HEADER();

public:
    Statement(const App::DocumentObject *_owner = 0, Expression *expr=0);
    virtual ~Statement();

protected:
    virtual void _toString(std::ostringstream &ss, bool persistent, int indent) const;
};

//////////////////////////////////////////////////////////////////////

namespace ExpressionParser {
AppExport Expression * parse(const App::DocumentObject *owner, const char *buffer);
AppExport UnitExpression * parseUnit(const App::DocumentObject *owner, const char *buffer);
AppExport ObjectIdentifier parsePath(const App::DocumentObject *owner, const char* buffer);
AppExport bool isTokenAnIndentifier(const std::string & str);
AppExport bool isTokenAUnit(const std::string & str);
AppExport std::vector<boost::tuple<int, int, std::string> > tokenize(const std::string & str);

/**
 * @brief The semantic_type class encapsulates the value in the parse tree during parsing.
 */

class semantic_type {
public:
  struct  {
    Base::Quantity scaler;
    std::string unitStr;
  } quantity;
  Expression * expr;
  ObjectIdentifier path;
  std::deque<ObjectIdentifier::Component> components;
  Expression::Component component;
  long long int ivalue;
  double fvalue;
  struct {
    std::string name;
    double fvalue;
  } constant;
  std::vector<Expression*> arguments;
  std::vector<std::pair<std::string,Expression*> > named_arguments;
  std::pair<std::string,Expression*> named_argument;
  std::vector<std::string> string_list;
  std::string string;
  ObjectIdentifier::String string_or_identifier;
  semantic_type() : expr(0), ivalue(0), fvalue(0) {}
};

}

/// Convenient class to mark begin of importing
class AppExport ExpressionImporter {
public:
    ExpressionImporter(Base::XMLReader &reader);
    ~ExpressionImporter();
};
}

#endif // EXPRESSION_H
