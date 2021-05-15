#include "Game/Scripting/ZephyrInterpreter.hpp"
#include "Game/Scripting/ZephyrVirtualMachine.hpp"


//-----------------------------------------------------------------------------------------------
void ZephyrInterpreter::InterpretStateBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk, 
													 ZephyrValueMap* globalVariables, 
													 Entity* parentEntity,
													 ZephyrValueMap* stateVariables )
{
	ZephyrVirtualMachine vm;
	vm.InterpretBytecodeChunk( bytecodeChunk, globalVariables, parentEntity, nullptr, stateVariables );
}


//-----------------------------------------------------------------------------------------------
void ZephyrInterpreter::InterpretEventBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk, 
													 ZephyrValueMap* globalVariables,
													 Entity* parentEntity, 
													 EventArgs* eventArgs, 
													 ZephyrValueMap* stateVariables )
{
	ZephyrVirtualMachine vm;
	vm.InterpretBytecodeChunk( bytecodeChunk, globalVariables, parentEntity, eventArgs, stateVariables );
}
