#ifndef __PLAYER_H_
#define  __PLAYER_H_

#include "objects/object_player.h"


// Класс, хранящий ссылки наобъектыы игрока и управляющий ими
class Player
{
public:
	
	ObjPlayer* first;		// Первый игрок
	char* first_proto;		// Прототип первого игрока
	ObjPlayer* second;		// Второй игрок
	char* second_proto;		// Прототип второго игрока
	ObjPlayer* current;		// Текущий игрок
	
	
	Vector2 revivePoint;	// Точка, в которой игрок будет оживлен

	bool change_blocked;	// Блокировка переключения между игроками

	int onChangePlayerProcessor;	// Луа-обработчик смены игрока
	int onPlayerDeathProcessor;		// Луа-обработчик смерти игрока

	Player()
	{
		first = NULL;
		first_proto = NULL;
		second = NULL;
		second_proto = NULL;
		current = NULL;

		change_blocked = false;
		onChangePlayerProcessor = LUA_NOREF;
		onPlayerDeathProcessor = LUA_NOREF;

	}

	~Player()
	{
		DELETEARRAY(first_proto);
		DELETEARRAY(second_proto);
		SCRIPT::RemoveFromRegistry(this->onChangePlayerProcessor);
		SCRIPT::RemoveFromRegistry(this->onPlayerDeathProcessor);
	}

	void Update();
	void Changer( bool forced, bool immediate );

	void BlockChange();
	void UnblockChange();

	void OnDying(ObjPlayer* plr);

	ObjPlayer* Create(const char* proto_name, Vector2 coord, const char* start_anim);

	ObjPlayer* Revive(int num, const char* start_anim);
	
	ObjPlayer* GetByNum(int num);
	
};


#endif // __PLAYER_H_