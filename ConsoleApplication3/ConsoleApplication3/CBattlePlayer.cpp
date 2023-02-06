#include "CBattlePlayer.h"
#include "windows.h"
#include <fstream>
#include <iostream>


#define TEST
CBattlePlayer::CBattlePlayer()
{
	m_sock = 0;
	m_pAnotherPlayer = NULL;
	n_Decks[0] = 2;
	n_Decks[1] = 0;
	n_Decks[2] = 0;
	n_Decks[3] = 0;
}

CBattlePlayer::~CBattlePlayer()
{
	for (size_t i = 0; i < m_Ships.size(); i++)
	{
		delete m_Ships[i];
	}

	m_Ships.clear();

}
void CBattlePlayer::Message(string str)
{
	str = "\n" + str + "\n";
	send(m_sock, str.c_str(), str.size() + 1, 0);
}

bool CBattlePlayer::PrepareFromFile() {

	Message("Введите имя файла!");

	string filename = recieve();

	ifstream in(filename);

	if (!in.is_open()) {
		return false;
	}

	int Decks[4];

	for (int i = 0; i < 4; i++) {
		in >> Decks[i];
	}

	return true;
}

void CBattlePlayer::Prepare() {
	int Decks[4];

	for (int i = 0; i < 4; i++) {
		Decks[i] = n_Decks[i];
	}

	Message("Самостоятельная расстановка!");

	string answer;

	while (true)
	{
		Sleep(100);

		Message(PrintAqua(false));

		Sleep(100);
		answer = "Осталось разместить\n";
		for (int i = 0; i < 4; i++) {
			if (Decks[i] != 0) {
				answer += to_string(Decks[i]) + " корабля размера " + to_string(i + 1) + "\n";
			}
		}

		Message(answer);

		Sleep(100);

		Message("Какого размера корабль поставить?");

		string iDeck = recieve();

		Message("Введите клетки в формате A1A2A3A4");

		string cells = recieve();

		if (Try2PlaceShip(iDeck[0] - '0', cells))
		{
			Message("Хорошо!");
			Decks[iDeck[0] - '0' - 1]--;
		}
		else
		{
			Message("Ошибка в расположении корабля!");
		}
		Sleep(100);

		if (ShipsAreReady()) {
			Message("Ваши корабли готовы!");
			break;
		}
		else {
			Message("Продолжаем расстановку кораблей!");
		}
	}
}

bool CBattlePlayer::PrepareShips()
{
	Message("Выберите способ расстановки кораблей!\n1 - Самостоятельно\n2 - Из файла\n");

	// Ответ (1 или 2)
	string move = recieve();

	if (move == "1") {
		Prepare();
	} else if (move == "2") {
		PrepareFromFile();
	}
	else {
		exit(0);
	}
	
	return true;
}

void CBattlePlayer::wait() {
	Sleep(100);
	Message("Ожидание хода другого игрока!");
}

bool CBattlePlayer::DoMove()
{
	Sleep(100);
	Message("Ваш ход!");

	Sleep(100);

	Message(PrintAqua(true));

	// Клетка
	string cell = recieve();

	CShip *ship=NULL;
	if (m_pAnotherPlayer->m_Aqua.TestShip(cell, &ship))
	{
		Sleep(100);

		Message("Попадание!");

		Sleep(100);

		if (!ship->Alive())
		{
			Message("Корабль потоплен!");

			Sleep(100);

			if (!m_pAnotherPlayer->IsAlive())
			{
				Message("Вы выиграли");
				m_pAnotherPlayer->Message("Вы проиграли");
				return true;
			}
			else {
				Message("Игра продолжается!");
			}
		}
		else {
			Message("Корабль повреждён!");
		}
		
		DoMove();

	}
	else {
		Message("Мимо(");
	}
	
	return true;
}

bool CBattlePlayer::ShipsAreReady()
{
	int Decks[4];

	for (int i = 0; i < 4; i++) {
		Decks[i] = 0;
	}
	
	for (size_t i = 0; i < m_Ships.size(); i++)
	{
		if (!m_Ships[i]->IsPlaced())
			return false;
		else {
			Decks[m_Ships[i]->m_Deck - 1]++;
		}
	}

	bool flag = true;

	for (int i = 0; i < 4; i++) {
		if (n_Decks[i] != Decks[i]) {
			flag = false;
		}
	}

	return flag;
}
bool CBattlePlayer::Try2PlaceShip(int iDeck, string cells)
{	
	CShip *s = new CShip(iDeck);

	for (int i = 0; i < iDeck; i++)
	{
		CCell *pCell= NULL;
		if (m_Aqua.Try2PlaceDeck(cells.c_str() + i * 2, &pCell, s))
		{
			s->m_pCells.push_back(pCell);
			pCell->m_pShip = s;
		}
		else
		{
			s->ClearCells();
			delete s;
			return false;
		}
	}

	m_Ships.push_back(s);

	return true;
}
string CBattlePlayer::recieve()
{
	char buff[5000];
	int nsize = 0;
	nsize = recv(m_sock, &buff[0], sizeof(buff), 0);
	if (nsize == -1)
		nsize = 0;
	buff[nsize] = 0;

	return buff;
}
bool CBattlePlayer::IsAlive()
{
	for (size_t i = 0; i < m_Ships.size(); i++)
	{
		if (m_Ships[i]->Alive())
			return true;
	}

	return false;
}


string CBattlePlayer::PrintAqua(bool flag) {

	string str = "1 2 3 4 5 6 7 8 9 10 + ";

	if (flag) {
		str += "1 2 3 4 5 6 7 8 9 10";
	}

	str += "\n\n";

	string buf;

	// 01 02 03 04 .. 10  A  01 02 03

	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			str += m_Aqua.m_Cells[i][j].PrintOwn() + ' ';
		}
		str += " " + string(1, 'A' + i) + " ";
		if (flag) {
			for (int j = 0; j < 10; j++)
			{
				str += m_pAnotherPlayer->m_Aqua.m_Cells[i][j].PrintForeign() + ' ';
			}
		}
		str += "\n";
	}

	str += "\n";

	return str;
}