#pragma once
#include "BytecodeCommon.hpp"
#include "Chunk.hpp"

#include <stack>


//-----------------------------------------------------------------------------------------------
class VirtualMachine
{
public:
	VirtualMachine() {}

	eInterpretResult Interpret( const Chunk& chunk );
	eInterpretResult Interpret( const std::string& file );

private:
	// TODO: Write my own stack that does this
	void PushValue( Value val );
	Value PopValue();

	void PushBinaryOp( Value a, Value b, eOpCode opCode );

private:
	std::stack<Value> m_stack;
};
