// Own GameEvent Manager class
// Copyright (C) 2011 HellGate-WoW

#include "StdAfx.h"

initialiseSingleton(GameEventMgr);

GameEventMgr::GameEventMgr()
{
	// do we need it? no..
}

GameEventMgr::~GameEventMgr()
{
	for(GameEventMap::iterator itr = m_GameEventMap.begin(); itr != m_GameEventMap.end(); itr++)
	{
		// the event struct has been created with new, so we have to delete it here.
		delete itr->second;
	}
	m_GameEventMap.clear();

	// enough?
	m_creaturespawns.clear();
	m_gameobjectspawns.clear();
}

void GameEventMgr::LoadEvents()
{
	Log.Notice("GameEvent","Started loading events...");

	// select all events
	AsyncQuery* queries = new AsyncQuery(new SQLClassCallbackP0<GameEventMgr>(this, &GameEventMgr::LoadEventsProc));
	queries->AddQuery("SELECT * FROM game_event;");
	WorldDatabase.QueueAsyncQuery(queries);
}

void GameEventMgr::LoadEventsProc(QueryResultVector & results)
{
	// store all loaded events

	// we won't continue if we don't have any event
	if( results.size() != 1 || results[0].result == NULL )
	{
		Log.Warning("GameEvent","There are no events");
		return;
	}

	// UNXITIME is UTC, but we want to think only in localtime
	uint32 current_time = UNIXTIME + (sWorld.GMTTimeZone*60*60);

	QueryResult *qry = results[0].result;
	do 
	{
		Field * f = qry->Fetch();
		uint32 event_id = f[0].GetUInt32();
		EventInfo * ev = new EventInfo();
		ev->start_time = f[1].GetUInt32();
		ev->length = f[2].GetUInt32();
		ev->occurence = f[3].GetUInt32();
		ev->name = strdup(f[4].GetString());
		ev->active = false;

		// set the end_time...
		ev->end_time = ev->start_time + ev->length;

		// calculate next start_time if it's needed
		if( ev->end_time < current_time )
		{
			// if it's non-repeatable event, check if it already happened so we don't need to do anything with it
			if( ev->occurence == 0 )
			{
				delete ev;
				continue;
			}

			// easiest way?
			uint32 tdiff = (current_time - ev->end_time) / ev->occurence;
			ev->start_time += tdiff * ev->occurence;

			// set the end_time again
			ev->end_time = ev->start_time + ev->length;

			// save it to the database
			WorldDatabase.Execute("UPDATE game_event SET start_time = '%u' WHERE event_id = '%u';", ev->start_time, event_id);
		}

		// store them
		m_GameEventMap.insert( make_pair(event_id, ev) );

	} while( qry->NextRow() );

	Log.Success("GameEvent","%u events loaded", m_GameEventMap.size());
}

// Remember, this could be used only after core fully loads all events!
bool GameEventMgr::SaveEvent(uint32 id)
{
	//check for ID first
	GameEventMap::iterator _event = CheckAndReturnEvent(id);
	if( _event == m_GameEventMap.end() )
		return false;

	std::stringstream ss;

	ss << "UPDATE game_event SET ";
	ss << "start_time = '" << _event->second->start_time << "', ";
	ss << "length = '" << _event->second->length << "', ";
	ss << "occurence = '" << _event->second->occurence << "' ";
	ss << "WHERE event_id = '" << id << "';";

	WorldDatabase.Execute(ss.str().c_str());

	return true;
}

// this method is used to check which event to activate etc.
void GameEventMgr::CheckForEvents()
{
	GameEventMap::iterator itr = m_GameEventMap.begin();
	uint32 current_time = UNIXTIME + (sWorld.GMTTimeZone*60*60);

	for( itr; itr != m_GameEventMap.end(); itr++ )
	{
		EventInfo * ev = itr->second;

		//   current time > start time        current time < end time            not active yet
		if( (ev->start_time < current_time) && (ev->end_time > current_time) && (ev->active == false) )
		{
			StartEvent(itr->first);
		}
		//            is active            end time < current time
		else if( (ev->active == true) && (ev->end_time < current_time) )
		{
			FinishEvent(itr->first);
		}
		// let's call this dirty bug fix, in very small percent of all performed actions it may happen
		else if( (ev->active == false) && ev->end_time < current_time )
		{
			// re-calculate start time :<
			// REMEMBER that in this case we must use UNIXTIME, because we use UNIXTIME relative to your time zone only for checks
			uint32 tdiff = (current_time - ev->end_time) / ev->occurence;
			ev->start_time += tdiff * ev->occurence;

			// set the end_time again
			ev->end_time = ev->start_time + ev->length;
			sGameEventMgr.SaveEvent( itr->first );
		}

		// otherwise do nothing
	}
}

void GameEventMgr::StartEvent(uint32 id)
{
	GameEventMap::iterator _event = CheckAndReturnEvent(id);
	if( _event == m_GameEventMap.end() )
	{
		sLog.outError("System tried to start event %u, but this event does not exist!", id);
		return;
	}

	// already running
	if( _event->second->active == true )
		return;

	// spawn etc. here
	ProcessObjectsAndScripts( id );
	sHookInterface.OnGameEventStart( id );

	// set as running
	_event->second->active = true;

	sLog.outDebug("Event %s (ID: %u) has just started.", _event->second->name, id);
}

void GameEventMgr::FinishEvent(uint32 id)
{
	GameEventMap::iterator _event = CheckAndReturnEvent(id);
	if( _event == m_GameEventMap.end() )
	{
		sLog.outError("System tried to finish event %u, but this event does not exist!", id);
		return;
	}

	// already finished
	if( _event->second->active != true )
		return;

	// despawn etc. here
	DespawnEvent( id );
	ScriptedBackToOrig( id );
	sHookInterface.OnGameEventFinish( id );

	// set as non active
	_event->second->active = false;

	// update next start time
	if( _event->second->occurence != 0 )
	{
		_event->second->start_time += _event->second->occurence;
		_event->second->end_time = _event->second->start_time + _event->second->length;
	}

	// if user wants to save event everytime, just save it
	if( sWorld.optimized_game_event_saving == true )
	{
		sGameEventMgr.SaveEvent( id );
	}

	sLog.outDebug("Event %s (ID: %u) has just stopped.", _event->second->name, id);
}

void GameEventMgr::ProcessObjectsAndScripts(uint32 id)
{
	Log.Success("GameEvent", "Started loading datas from database for event %u", id);

	AsyncQuery* queries = new AsyncQuery(new SQLClassCallbackP1<GameEventMgr, uint32>(this, &GameEventMgr::ProcessObjectsAndScriptsProc, id));
	queries->AddQuery("SELECT gs.*,cs.map FROM game_event_scripts gs JOIN creature_spawns cs ON (gs.sql_id = cs.id) WHERE gs.event_id = %u;", id);
	queries->AddQuery("SELECT * FROM game_event_creature_waypoints WHERE event_id = %u;", id);
	queries->AddQuery("SELECT * FROM game_event_creature_spawns WHERE event_id = %u;", id);
	queries->AddQuery("SELECT * FROM game_event_gameobject_spawns WHERE event_id = %u;", id);
	WorldDatabase.QueueAsyncQuery(queries);
}

void GameEventMgr::ProcessObjectsAndScriptsProc(QueryResultVector & results, uint32 id)
{
	// process scripts
	if(QueryResult *query = results[0].result)
	{
		do
		{
			Field * f = query->Fetch();
			uint32 sql_id = f[1].GetUInt32();
			uint8 type = f[2].GetUInt8();
			uint32 data1 = f[3].GetUInt32();
			uint32 data2 = f[4].GetUInt32();
			uint32 data3 = f[5].GetUInt32();
			char * say = strdup(f[6].GetString());
			uint32 mapid = f[7].GetUInt32();

			DoScript(id, sql_id, type, data1, data2, data3, say, mapid);
		}
		while(query->NextRow());
	}

	// waypoints -- the created objects will be deleted automatically by creature class on despawn
	std::map<uint32, std::map<uint32, std::list<WayPoint *>>> waypoints;
	if(QueryResult *query = results[1].result)
	{
		do
		{
			Field * f = query->Fetch();

			WayPoint *wp = new WayPoint;
			//uint32 event = f[0].GetUInt32();
			uint32 spawnid = f[1].GetUInt32();
			wp->id = f[2].GetUInt32();
			wp->x = f[3].GetFloat();
			wp->y = f[4].GetFloat();
			wp->z = f[5].GetFloat();
			wp->waittime = f[6].GetUInt32();
			wp->flags = f[7].GetUInt32();
			wp->forwardemoteoneshot = f[8].GetBool();
			wp->forwardemoteid = f[9].GetUInt32();
			wp->backwardemoteoneshot = f[10].GetBool();
			wp->backwardemoteid = f[11].GetUInt32();
			wp->forwardskinid = f[12].GetUInt32();
			wp->backwardskinid = f[13].GetUInt32();

			waypoints[id][spawnid].push_back(wp);
		}
		while(query->NextRow());
	}

	// creature spawns
	if(QueryResult *query = results[2].result)
	{
		do
		{
			Field * f = query->Fetch();

			CreatureSpawn spawn;
			//uint32 event = f[0].GetUInt32();
			uint32 spawn_id = f[1].GetUInt32();
			spawn.id = 0;
			spawn.entry = f[2].GetUInt32();
			uint32 mapid = f[3].GetUInt32();
			spawn.x = f[4].GetFloat();
			spawn.y = f[5].GetFloat();
			spawn.z = f[6].GetFloat();
			spawn.o = f[7].GetFloat();
			spawn.form = NULL;
			spawn.movetype = f[8].GetUInt8();
			spawn.displayid = f[9].GetUInt32();
			spawn.factionid = f[10].GetUInt32();
			spawn.flags = f[11].GetUInt32();
			spawn.bytes0 = f[12].GetUInt32();
			spawn.bytes1 = f[13].GetUInt32();
			spawn.bytes2 = f[14].GetUInt32();
			spawn.emote_state = f[15].GetUInt32();
			//uint32 npc_respawn_link = f[17].GetUInt32();
			spawn.channel_spell = f[17].GetUInt16();
			spawn.channel_target_go = f[18].GetUInt32();
			spawn.channel_target_creature = f[19].GetUInt32();
			spawn.stand_state = f[20].GetUInt16();
			spawn.death_state = f[21].GetUInt32();
			spawn.MountedDisplayID = f[22].GetUInt32();
			spawn.Item1SlotDisplay = f[23].GetUInt32();
			spawn.Item2SlotDisplay = f[24].GetUInt32();
			spawn.Item3SlotDisplay = f[25].GetUInt32();
			spawn.CanFly = f[26].GetUInt32();
			spawn.phase = f[27].GetUInt32();
			if(spawn.phase == 0) spawn.phase = 0xFFFFFFFF;

			CreatureProto* proto = CreatureProtoStorage.LookupEntry(spawn.entry);
			CreatureInfo* info = CreatureNameStorage.LookupEntry(spawn.entry);
			if(proto == NULL || info == NULL)
			{
				continue;
			}

			// get mapmgr for spawn
			MapMgr * mgr = sInstanceMgr.GetMapMgr(mapid);
			if(mgr == NULL)
			{
				// So this is a really interesting situation!
				Log.Success("GameEvent","Failed to spawn creature spawn %u for event %u on nonexistant map %u", spawn.id, id, mapid);
				continue;
			}

			// spawn the creature
			//Creature * crt = mgr->GetInterface()->SpawnCreature(&spawn, true);
			Creature * crt = mgr->CreateCreature(spawn.entry);
			if(crt == NULL)
			{
				// we didnt succeed creating the creature! Print a warning and go on
 				Log.Success("GameEvent","Failed to spawn creature spawn %u for event %u", spawn.id, id);
				continue;
			}

			crt->Load(&spawn, 0, mgr->GetMapInfo());

			crt->spawnid = 0;
			crt->m_spawn = NULL;

			// add waypoints
			crt->m_custom_waypoint_map = new WayPointMap();
			for(std::list<WayPoint *>::iterator itr = waypoints[id][spawn_id].begin(); itr != waypoints[id][spawn_id].end(); itr++)
			{
				// use custom waypoint map, so we avoid saving it to database
				crt->m_custom_waypoint_map->push_back(*itr);
			}

			crt->GetAIInterface()->SetWaypointMap(crt->m_custom_waypoint_map);
			crt->AddToWorld(mgr);

			m_creaturespawns[id][mapid].push_back(crt->GetUIdFromGUID());
		}
		while(query->NextRow());
	}

	// gameobject spawns
	if(QueryResult *query = results[3].result)
	{
		do
		{
			Field * f = query->Fetch();

			GOSpawn spawn;
			//uint32 event = f[0].GetUInt32();
			uint32 spawn_id = f[1].GetUInt32();
			// generate new ingame sql id
			spawn.id = /*objmgr.GenerateCreatureSpawnID()*/0;
			spawn.entry = f[2].GetUInt32();
			uint32 mapid = f[3].GetUInt32();
			spawn.x = f[4].GetFloat();
			spawn.y = f[5].GetFloat();
			spawn.z = f[6].GetFloat();
			spawn.facing = f[7].GetFloat();
			spawn.o = f[8].GetFloat();
			spawn.o1 = f[9].GetFloat();
			spawn.o2 = f[10].GetFloat();
			spawn.o3 = f[11].GetFloat();
			spawn.state = f[12].GetUInt32();
			spawn.flags = f[13].GetUInt32();
			spawn.faction = f[14].GetUInt32();
			spawn.scale = f[15].GetFloat();
			//uint32 state_npc_link = fields[16].GetUInt32();
			spawn.phase = f[17].GetUInt32();
			if(spawn.phase == 0) spawn.phase = 0xFFFFFFFF;
			spawn.overrides = f[18].GetUInt32();

			// get mapmgr for spawn
			MapMgr * mgr = sInstanceMgr.GetMapMgr(mapid);
			if(mgr == NULL)
			{
				// So this is a really interesting situation!
				Log.Success("GameEvent","Failed to spawn gameobject spawn %u for event %u on nonexistant map %u", spawn.id, id, mapid);
				continue;
			}

			// spawn the creature
			GameObject * go = mgr->CreateGameObject(spawn.entry);
			if(go == NULL)
			{
				// we didnt succeed creating the creature! Print a warning and go on
 				Log.Success("GameEvent","Failed to spawn gameobject spawn %u for event %u", spawn.id, id);
				continue;
			}

			go->Load(&spawn);

			go->m_spawn = NULL;

			go->AddToWorld(mgr);

			m_gameobjectspawns[id][mapid].push_back(go->GetUIdFromGUID());
		}
		while(query->NextRow());
	}

	Log.Success("GameEvent","event %u spawned.", id);
}

void GameEventMgr::DespawnEvent(uint32 id)
{
	Log.Success("GameEvent","Started despawning event %u", id);

	// creatures
	for(std::map<uint32, std::list<uint32>>::iterator mitr = m_creaturespawns[id].begin(); mitr != m_creaturespawns[id].end(); mitr++)
	{
		for(std::list<uint32>::iterator gitr = mitr->second.begin(); gitr != mitr->second.end(); gitr++)
		{
			// find creature with current guid(*gitr) and despawn it
			MapMgr *mgr = sInstanceMgr.GetMapMgr(mitr->first);
			Creature *crt = mgr->GetCreature(*gitr);
			if(crt == NULL)
			{
				Log.Success("GameEvent","Failed to despawn creature with guid %u for event %u", *gitr, id);
			}
			else
			{
				// we don't need to delete waypoints as they are stored in m_custom_waypoint_map
				// and they are deleted automatically when creature is deleted

				crt->Despawn(0,0);
				Log.Success("GameEvent","creature with guid %u despawned from map %u for event %u", *gitr, mitr->first, id);
			}
		}
		mitr->second.clear();
	}

	// gameobjects
	for(std::map<uint32, std::list<uint32>>::iterator mitr = m_gameobjectspawns[id].begin(); mitr != m_gameobjectspawns[id].end(); mitr++)
	{
		for(std::list<uint32>::iterator gitr = mitr->second.begin(); gitr != mitr->second.end(); gitr++)
		{
			// find gameobject with current guid(*gitr) and despawn it
			MapMgr *mgr = sInstanceMgr.GetMapMgr(mitr->first);
			GameObject *go = mgr->GetGameObject(*gitr);
			if(go == NULL)
			{
				Log.Success("GameEvent","Failed to despawn gameobject with guid %u for event %u", *gitr, id);
			}
			else
			{
				go->Despawn(0,0);
				Log.Success("GameEvent","gameobject with guid %u despawned from map %u for event %u", *gitr, mitr->first, id);
			}
		}
		mitr->second.clear();
	}

	Log.Success("GameEvent","event %u despawned.", id);
}

void GameEventMgr::DoScript(uint32 event_id, uint32 sql_id, uint8 type, uint32 data1, uint32 data2, uint32 data3, char * say, uint32 mapid)
{
	MapMgr * mapmgr = sInstanceMgr.GetMapMgr( mapid );
	if( mapmgr == NULL )
		return;

	Creature * c = mapmgr->GetSqlIdCreature( sql_id );
	if( c == NULL )
		return;

	// create backup for original values
	EventScript * es = new EventScript();
	es->sql_id = sql_id;
	es->mapid = mapid;
	es->type = type;
	es->data_1 = 0; // null them out first!
	es->data_2 = 0;
	es->data_3 = 0;

	if( say )
	{
		c->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, say);
	}

	switch( type )
	{
		case CREATURE_CHANGE_SCRIPTED_CHANGE:
		{
			CALL_SCRIPT_EVENT(c, GameEventStart)(event_id);
		} break;
		case CREATURE_CHANGE_EMOTE:
		{
			// do not backup one-shoot emote
			c->Emote(EmoteType(data1));

			// backup emote state first
			es->data_2 = c->GetEmoteState();
			c->SetEmoteState(data2);

			// backup stand state
			es->data_3 = static_cast<uint32>(c->GetStandState());
			c->SetStandState(static_cast<uint8>(data3));
		} break;

		case CREATURE_CHANGE_DISPLAYID:
		{
			es->data_1 = c->GetDisplayId();
			c->SetDisplayId(data1);
			es->data_2 = c->GetNativeDisplayId();
			c->SetNativeDisplayId(data2);
			es->data_3 = c->GetMount();
			c->SetMount(data3);

			c->EventModelChange();
		} break;

		case CREATURE_CHANGE_WEAPON:
		{
			es->data_1 = c->GetEquippedItem(MELEE);
			es->data_2 = c->GetEquippedItem(OFFHAND);
			es->data_3 = c->GetEquippedItem(RANGED);
			c->SetEquippedItem(MELEE, data1);
			c->SetEquippedItem(OFFHAND, data2);
			c->SetEquippedItem(RANGED, data3);
		} break;

		case CREATURE_CHANGE_REACT:
		{
			es->data_1 = c->GetFaction();
			c->SetFaction(data1);
			es->data_2 = c->GetUInt32Value(UNIT_NPC_FLAGS);
			c->SetUInt32Value(UNIT_NPC_FLAGS, data2);
			es->data_3 = c->GetUInt32Value(UNIT_FIELD_FLAGS);
			c->SetUInt32Value(UNIT_FIELD_FLAGS, data3);
		} break;

		case CREATURE_CAST_SPELL_ON_EVENT_START:
		{
			SpellEntry * sp = dbcSpell.LookupEntryForced( data1 );
			if( sp == NULL )
				return;

			SpellCastTime * casttime = dbcSpellCastTime.LookupEntry(sp->CastingTimeIndex);
			Spell * spell = sSpellFactoryMgr.NewSpell(c, sp, false, NULL);

			SpellCastTargets t(0);

			// force self casting
			if( data2 )
			{
				t.m_unitTarget = c->GetGUID();
			}
			else
			{
				spell->GenerateTargets(&t);
				spell->m_targets = t;
			}

			if (objmgr.IsSpellDisabled(spell->GetProto()->Id) || spell->CanCast(false) != SPELL_CANCAST_OK || !spell->HasPower() || c->m_silenced || c->IsStunned() || c->IsFeared() )
			{
				delete spell;
				return;
			}

			if( casttime->CastTime > 0 )
				c->GetAIInterface()->StopMovement(casttime->CastTime);

			spell->prepare(&t);

		} break;

		case CREATURE_CAST_SPELL_ON_EVENT_STOP:
		{
			// this time just backup it, we will procez it on event end
			es->data_1 = data1;
			es->data_2 = data2;
		} break;

		case CREATURE_CHANGE_UPDATE_FIELD:
		{
			es->data_1 = data1;
			es->data_2 = c->GetUInt32Value(data1);
			c->SetUInt32Value(data1, data2);
		} break;

		case CREATURE_CHANGE_DESPAWN:
		{
			GameEventMap::iterator itr = CheckAndReturnEvent( event_id );
			if( itr == m_GameEventMap.end() )
				return;

			uint32 current_time = UNIXTIME + (sWorld.GMTTimeZone*60*60);
			// this is calculated in seconds and added 1 extra second as timer for spawn and despawn
			uint32 respawntime = itr->second->end_time - current_time + 1;
			// values here are in miliseconds
			c->Despawn(0, respawntime*1000);
			delete es;
			return;
		} break;
	}

	// insert event into storage
	GameEventScriptMap::iterator itr = m_GameEventScriptBackup.find(event_id);
	if( itr == m_GameEventScriptBackup.end() )
	{
		set< EventScript* > s;
		s.insert( es );
		m_GameEventScriptBackup.insert(make_pair(event_id, s));
	}
	else
	{
		itr->second.insert( es );
	}
}

void GameEventMgr::ScriptedBackToOrig(uint32 event_id)
{
	GameEventScriptMap::iterator event_backup = m_GameEventScriptBackup.find(event_id);
	if( event_backup == m_GameEventScriptBackup.end() )
		return;

	set<EventScript*>::iterator itr = event_backup->second.begin();
	for( itr; itr != event_backup->second.end(); itr++ )
	{
		uint32 mapid = (*itr)->mapid;
		uint32 sql_id = (*itr)->sql_id;
		uint8 type = (*itr)->type;
		uint32 data1 = (*itr)->data_1;
		uint32 data2 = (*itr)->data_2;
		uint32 data3 = (*itr)->data_3;

		MapMgr * mapmgr = sInstanceMgr.GetMapMgr( mapid );
		if( mapmgr == NULL )
			return;

		Creature * c = mapmgr->GetSqlIdCreature( sql_id );
		if( c == NULL )
			return;

		switch( type )
		{
			case CREATURE_CHANGE_SCRIPTED_CHANGE:
			{
				CALL_SCRIPT_EVENT(c, GameEventFinish)(event_id);
			} break;
			case CREATURE_CHANGE_EMOTE:
			{
				c->Emote(EmoteType(data1));
				c->SetEmoteState(data2);
				c->SetStandState(static_cast<uint8>(data3));
			} break;

			case CREATURE_CHANGE_DISPLAYID:
			{
				c->SetDisplayId(data1);
				c->SetNativeDisplayId(data2);
				c->SetMount(data3);

				c->EventModelChange();
			} break;

			case CREATURE_CHANGE_WEAPON:
			{
				c->SetEquippedItem(MELEE, data1);
				c->SetEquippedItem(OFFHAND, data2);
				c->SetEquippedItem(RANGED, data3);
			} break;

			case CREATURE_CHANGE_REACT:
			{
				c->SetFaction(data1);
				c->SetUInt32Value(UNIT_NPC_FLAGS, data2);
				c->SetUInt32Value(UNIT_FIELD_FLAGS, data3);
			} break;

			case CREATURE_CAST_SPELL_ON_EVENT_STOP:
			{
				SpellEntry * sp = dbcSpell.LookupEntryForced( data1 );
				if( sp == NULL )
					return;

				SpellCastTime * casttime = dbcSpellCastTime.LookupEntry(sp->CastingTimeIndex);
				Spell * spell = sSpellFactoryMgr.NewSpell(c, sp, false, NULL);

				SpellCastTargets t(0);

				// force self casting
				if( data2 )
				{
					t.m_unitTarget = c->GetGUID();
				}
				else
				{
					spell->GenerateTargets(&t);
					spell->m_targets = t;
				}

				if (objmgr.IsSpellDisabled(spell->GetProto()->Id) || spell->CanCast(false) != SPELL_CANCAST_OK || !spell->HasPower() || c->m_silenced || c->IsStunned() || c->IsFeared() )
				{
					delete spell;
					return;
				}

				if( casttime->CastTime > 0 )
					c->GetAIInterface()->StopMovement(casttime->CastTime);

				spell->prepare(&t);

			} break;

			case CREATURE_CHANGE_UPDATE_FIELD:
			{
				c->SetUInt32Value(data1, data2);
			} break;
		}
	}

	m_GameEventScriptBackup.erase(event_id);
}
