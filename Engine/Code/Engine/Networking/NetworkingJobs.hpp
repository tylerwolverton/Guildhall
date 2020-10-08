#pragma once
#include "Engine/Core/JobSystem.hpp"


//-----------------------------------------------------------------------------------------------
class UDPReadJob : public Job
{
public:
	UDPReadJob() {}
	virtual ~UDPReadJob() {}

	virtual void Execute() override;				// Called by worker thread
	virtual void ClaimJobCallback() override;		// Called by client on its thread

private:
	float m_result = 0.f;
};
