#pragma once
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
class MultiplayerGame : public Game
{
public:
	MultiplayerGame() = default;
	~MultiplayerGame() = default;

	virtual void	Startup();
	virtual void	Update();
	virtual void	Render() const;
	//virtual void	Shutdown();

	virtual void	RestartGame();

	virtual void ShootEntity( EntityId shooterId, const Vec3& forwardVector, float shotRange, int damage ) override;

	virtual void AddPlayerScore( int playerNum, EntityId playerId ) override;
	virtual void UpdatePlayerScore( int playerNum, int newScore ) override;
	virtual void UpdatePlayerScoresForAllClients() override;
	virtual std::vector<int> GetPlayerScores() const override						{ return m_playerScores; }

private:
	std::map<EntityId, int> m_playerIdsToPlayerNums;
	std::vector<int> m_playerScores;
};
