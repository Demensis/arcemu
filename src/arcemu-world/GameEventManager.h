// Own GameEvent Manager class
// Copyright (C) 2011 HellGate-WoW

#ifndef __GAMEVENTMGR_H
#define __GAMEVENTMGR_H

struct EventInfo
{
	// these are loaded from DB
	uint32 start_time;	// UNIX time of the next event start
	uint32 length;		// length in minutes!!	
	uint32 occurence;	// 0 to not repeat event, otherwise in minutes
	char* name;

	// this is calculated or used in core only
	uint32 end_time;
	bool active;
};

// used for backuping original values
struct EventScript
{
	// these are loaded from DB
	uint32 sql_id;
	uint8 type;
	uint32 data_1;
	uint32 data_2;
	uint32 data_3;
	uint32 mapid;
};

enum GamEventScriptTypes
{
	CREATURE_CHANGE_SCRIPTED_CHANGE = 0,
	CREATURE_CHANGE_EMOTE,
	CREATURE_CHANGE_DISPLAYID,
	CREATURE_CHANGE_WEAPON,
	CREATURE_CHANGE_REACT,
	CREATURE_CAST_SPELL_ON_EVENT_START,
	CREATURE_CAST_SPELL_ON_EVENT_STOP,
	CREATURE_CHANGE_UPDATE_FIELD,
	CREATURE_CHANGE_DESPAWN,
};

typedef std::map<uint32, EventInfo*> GameEventMap;
typedef std::map<uint32, std::map<uint32, std::list<uint32>>> GameEventSpawnMap;
typedef std::map<uint32, set<EventScript*>> GameEventScriptMap;

class SERVER_DECL GameEventMgr :  public Singleton < GameEventMgr >
{
private:

	// private storages
	GameEventMap m_GameEventMap; // contains basic info about all events
	GameEventSpawnMap m_creaturespawns;
	GameEventSpawnMap m_gameobjectspawns;
	GameEventScriptMap m_GameEventScriptBackup;

public:

	// constructor & destructor
	GameEventMgr();
	~GameEventMgr();

	// we are going to check all events by this
	void CheckForEvents();

	// method for loading all events
	void LoadEvents();
	void LoadEventsProc(QueryResultVector & results);

	// saves event #id to  the database
	bool SaveEvent(uint32 id);

	// Activate and Deactivate events
	void StartEvent(uint32 id);
	void FinishEvent(uint32 id);

	// Some check methods, which can be used everywhere...
	bool IsEventActive(uint32 id)
	{
		GameEventMap::iterator itr = CheckAndReturnEvent( id );
		if( itr == m_GameEventMap.end() )
			return false;
		else if( itr->second->active == false )
			return false;
		else
			return true;
	}

private:
	// This will load all objects and scripts from database (for given event id)
	void ProcessObjectsAndScripts(uint32 id);
	void ProcessObjectsAndScriptsProc(QueryResultVector & results, uint32 id);

	// methods for spawning and despawning
	void SpawnEvent(uint32 id);
	void DespawnEvent(uint32 id);

	void DoScript(uint32 event_id, uint32 sql_id, uint8 type, uint32 data1, uint32 data2, uint32 data3, char * say, uint32 mapid);
	void ScriptedBackToOrig(uint32 event_id);

	// Some needed functions
	ARCEMU_INLINE GameEventMap::iterator CheckAndReturnEvent(uint32 id)
	{
		GameEventMap::iterator _event = m_GameEventMap.find(id);
		if( _event == m_GameEventMap.end() )
			return m_GameEventMap.end();
		else
			return _event;
	}

};

#define sGameEventMgr GameEventMgr::getSingleton()

#endif
