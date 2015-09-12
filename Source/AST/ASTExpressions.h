#pragma once

#include "AST.h"

namespace AST
{
	template<typename Type>
	struct Literal : public Type::Expression
	{
		typedef typename Type::NativeType NativeType;
		NativeType value;
		Literal(NativeType inValue) : Expression(Op::lit), value(inValue) {}
	};

	template<typename Class>
	struct Error : public Expression<Class>, public ErrorRecord
	{
		Error(std::string&& inMessage)
		: Expression(Op::error), ErrorRecord(std::move(inMessage))
		{}
	};

	struct GetVariable : public Expression<AnyClass>
	{
		uintptr_t variableIndex;
		GetVariable(AnyOp inOp,TypeClassId inTypeClass,uintptr_t inVariableIndex) : Expression(inOp,inTypeClass), variableIndex(inVariableIndex) {}
	};
	
	struct SetVariable : public Expression<AnyClass>
	{
		UntypedExpression* value;
		uintptr_t variableIndex;
		SetVariable(Op op,TypeClassId inTypeClass,UntypedExpression* inValue,uintptr_t inVariableIndex)
		: Expression(op,inTypeClass), value(inValue), variableIndex(inVariableIndex) {}
	};

	template<typename Class>
	struct Load : public Expression<Class>
	{
		bool isFarAddress;
		bool isAligned;
		Expression<IntClass>* address;
		TypeId memoryType;

		Load(typename Class::Op op,bool inIsFarAddress,bool inIsAligned,Expression<IntClass>* inAddress,TypeId inMemoryType)
		: Expression(op), isFarAddress(inIsFarAddress), isAligned(inIsAligned), address(inAddress), memoryType(inMemoryType) {}
	};
	
	template<typename Class>
	struct Unary : public Expression<Class>
	{
		Expression<Class>* operand;
		Unary(typename Class::Op op,Expression<Class>* inOperand): Expression(op), operand(inOperand)
		{}
	};
	
	template<typename Class>
	struct Store : public Expression<Class>
	{
		bool isFarAddress;
		bool isAligned;
		Expression<IntClass>* address;
		TypedExpression value;
		TypeId memoryType;

		Store(bool inIsFarAddress,bool inIsAligned,Expression<IntClass>* inAddress,TypedExpression inValue,TypeId inMemoryType)
		: Expression(Class::Op::store), isFarAddress(inIsFarAddress), isAligned(inIsAligned), address(inAddress), value(inValue), memoryType(inMemoryType) {}
	};

	template<typename Class>
	struct Binary : public Expression<Class>
	{
		Expression<Class>* left;
		Expression<Class>* right;

		Binary(typename Class::Op op,Expression<Class>* inLeft,Expression<Class>* inRight)
		: Expression(op), left(inLeft), right(inRight)
		{}
	};

	struct Comparison : public Expression<BoolClass>
	{
		TypeId operandType;
		UntypedExpression* left;
		UntypedExpression* right;

		Comparison(Op op,TypeId inOperandType,UntypedExpression* inLeft,UntypedExpression* inRight)
		: Expression(op), operandType(inOperandType), left(inLeft), right(inRight) {}
	};

	template<typename Class>
	struct Cast : public Expression<Class>
	{
		typedef typename Class::Op Op;

		TypedExpression source;

		Cast(Op op,TypedExpression inSource)
		: Expression(op), source(inSource) {}
	};
	
	struct Call : public Expression<AnyClass>
	{
		uintptr_t functionIndex;
		UntypedExpression** parameters;
		Call(AnyOp op,TypeClassId inTypeClass,uintptr_t inFunctionIndex,UntypedExpression** inParameters)
		: Expression(op,inTypeClass), functionIndex(inFunctionIndex), parameters(inParameters) {}
	};

	struct CallIndirect : public Expression<AnyClass>
	{
		uintptr_t tableIndex;
		Expression<IntClass>* functionIndex; // must be I32
		UntypedExpression** parameters;
		CallIndirect(TypeClassId inTypeClass,uintptr_t inTableIndex,Expression<IntClass>* inFunctionIndex,UntypedExpression** inParameters)
		: Expression(AnyOp::callIndirect,inTypeClass), tableIndex(inTableIndex), functionIndex(inFunctionIndex), parameters(inParameters) {}
	};

	// Used to coerce an expression result to void.
	struct DiscardResult : public Expression<VoidClass>
	{
		TypedExpression expression;

		DiscardResult(TypedExpression inExpression): Expression(Op::discardResult), expression(inExpression) {}
	};

	struct Nop : public Expression<VoidClass>
	{
		Nop(): Expression(Op::nop) {}
	};
	
	// Each unique branch target has a BranchTarget allocated in the module's arena, so you can identify them by pointer.
	struct BranchTarget
	{
		TypeId type;
		BranchTarget(TypeId inType): type(inType) {}
	};

	struct SwitchArm
	{
		uint64_t key;
		UntypedExpression* value; // The type of the switch for the final arm, void for all others.
	};

	template<typename Class>
	struct Switch : public Expression<Class>
	{
		TypedExpression key;
		uintptr_t defaultArmIndex;
		size_t numArms;
		SwitchArm* arms;
		BranchTarget* endTarget;

		Switch(TypedExpression inKey,uintptr_t inDefaultArmIndex,size_t inNumArms,SwitchArm* inArms,BranchTarget* inEndTarget)
		: Expression(Op::switch_), key(inKey), defaultArmIndex(inDefaultArmIndex), numArms(inNumArms), arms(inArms), endTarget(inEndTarget) {}
	};
	
	template<typename Class>
	struct IfElse : public Expression<Class>
	{
		Expression<BoolClass>* condition;
		Expression<Class>* thenExpression;
		Expression<Class>* elseExpression;

		IfElse(Expression<BoolClass>* inCondition,Expression<Class>* inThenExpression,Expression<Class>* inElseExpression)
		:	Expression(Op::ifElse)
		,	condition(inCondition)
		,	thenExpression(inThenExpression)
		,	elseExpression(inElseExpression)
		{}
	};

	template<typename Class>
	struct Label : public Expression<Class>
	{
		BranchTarget* endTarget;
		Expression<Class>* expression;

		Label(BranchTarget* inEndTarget,Expression<Class>* inExpression)
		: Expression(Op::label), endTarget(inEndTarget), expression(inExpression) {}
	};

	template<typename Class>
	struct Loop : public Expression<Class>
	{
		Expression<VoidClass>* expression;
		
		BranchTarget* breakTarget;
		BranchTarget* continueTarget;

		Loop(Expression<VoidClass>* inExpression,BranchTarget* inBreakTarget,BranchTarget* inContinueTarget)
		: Expression(Op::loop), expression(inExpression), breakTarget(inBreakTarget), continueTarget(inContinueTarget) {}
	};

	template<typename Class>
	struct Branch : public Expression<Class>
	{
		BranchTarget* branchTarget;
		UntypedExpression* value; // The type is determined by the branch target. If the type is void, it may be null.

		Branch(BranchTarget* inBranchTarget,UntypedExpression* inValue)
		: Expression(Op::branch), branchTarget(inBranchTarget), value(inValue) {}
	};

	template<typename Class>
	struct Return : public Expression<Class>
	{
		// The value to return. The type is implied by the return type of the function.
		// If the return type is Void, then it will be null.
		UntypedExpression* value;

		Return(UntypedExpression* inValue): Expression(Op::ret), value(inValue) {}
	};

	template<typename Class>
	struct Sequence : public Expression<Class>
	{
		Expression<VoidClass>* voidExpression;
		Expression<Class>* resultExpression;

		Sequence(Expression<VoidClass>* inVoidExpression,Expression<Class>* inResultExpression)
		:	Expression(Op::sequence)
		,	voidExpression(inVoidExpression)
		,	resultExpression(inResultExpression)
		{}
	};
}