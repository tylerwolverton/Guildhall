#include "Game/Scripting/ZephyrInterpreter.hpp"
#include "Game/Scripting/ZephyrVirtualMachine.hpp"


//-----------------------------------------------------------------------------------------------
void ZephyrInterpreter::InterpretStateBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk, 
													 ZephyrValueMap* globalVariables, 
													 Entity* parentEntity )
{
	ZephyrVirtualMachine vm;
	vm.InterpretStateBytecodeChunk( bytecodeChunk, globalVariables, parentEntity );
}


//-----------------------------------------------------------------------------------------------
void ZephyrInterpreter::InterpretEventBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk, 
													 ZephyrValueMap* globalVariables, 
													 Entity* parentEntity, 
													 EventArgs* eventArgs, 
													 ZephyrValueMap* stateVariables )
{
	ZephyrVirtualMachine vm;
	vm.InterpretEventBytecodeChunk( bytecodeChunk, globalVariables, parentEntity, eventArgs, stateVariables );
}
