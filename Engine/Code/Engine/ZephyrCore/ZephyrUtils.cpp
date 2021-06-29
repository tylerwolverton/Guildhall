#include "Engine/ZephyrCore/ZephyrUtils.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"


//-----------------------------------------------------------------------------------------------
void CloneZephyrEventArgs( EventArgs& destArgs, const EventArgs& srcArgs )
{
	for ( auto const& keyValuePair : srcArgs.GetAllKeyValuePairs() )
	{
		if ( keyValuePair.second->Is<float>() )
		{
			destArgs.SetValue( keyValuePair.first, srcArgs.GetValue( keyValuePair.first, 0.f ) );
		}
		else if ( keyValuePair.second->Is<int>() )
		{
			destArgs.SetValue( keyValuePair.first, srcArgs.GetValue( keyValuePair.first, (EntityId)-1 ) );
		}
		else if ( keyValuePair.second->Is<double>() )
		{
			destArgs.SetValue( keyValuePair.first, (float)srcArgs.GetValue( keyValuePair.first, 0.0 ) );
		}
		else if ( keyValuePair.second->Is<bool>() )
		{
			destArgs.SetValue( keyValuePair.first, srcArgs.GetValue( keyValuePair.first, false ) );
		}
		else if ( keyValuePair.second->Is<Vec2>() )
		{
			destArgs.SetValue( keyValuePair.first, srcArgs.GetValue( keyValuePair.first, Vec2::ZERO ) );
		}
		else if ( keyValuePair.second->Is<Vec3>() )
		{
			destArgs.SetValue( keyValuePair.first, srcArgs.GetValue( keyValuePair.first, Vec3::ZERO ) );
		}
		else if ( keyValuePair.second->Is<std::string>()
				  || keyValuePair.second->Is<char*>() )
		{
			destArgs.SetValue( keyValuePair.first, srcArgs.GetValue( keyValuePair.first, "" ) );
		}
	}

	// Any other variables will be ignored since they have no ZephyrType equivalent
}


