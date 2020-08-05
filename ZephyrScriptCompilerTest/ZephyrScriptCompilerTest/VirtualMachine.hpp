#pragma once
#include "BytecodeCommon.hpp"
#include "Chunk.hpp"

#include <stack>


//-----------------------------------------------------------------------------------------------
class VirtualMachine
{
public:
	VirtualMachine() {}

	void Interpret( const Chunk& chunk );

private:
	std::stack<Value> m_stack;
};
