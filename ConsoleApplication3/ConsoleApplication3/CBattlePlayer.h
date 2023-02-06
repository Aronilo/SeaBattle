#pragma once
#include "CAquatory.h"
#include "CShip.h"
#include <vector>

#include "winsock2.h" 

class CBattlePlayer
{
public:
	CBattlePlayer();
	~CBattlePlayer();

	CAquatory m_Aqua;
	vector <CShip*> m_Ships;
	int n_Decks[4];

	SOCKET m_sock;
	bool PrepareShips();
	bool ShipsAreReady();
	bool DoMove();

	bool PrepareFromFile();
	void Prepare();

	void wait();

	string PrintAqua(bool flag);

	bool Try2PlaceShip(int iDeck, string cells);
	void Message(string str);

	string recieve();

	CBattlePlayer *m_pAnotherPlayer;

	bool IsAlive();
};

