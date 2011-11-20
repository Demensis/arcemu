/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2009 ArcEmu Team <http://www.arcemu.org/>
 * Copyright (C) 2008-2009 Sun++ Team <http://www.sunscripting.com/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Setup.h"
#include "../Common/EasyFunctions.h"

class ScourgeGryphonOne : public GossipScript
{
	public:
		void GossipHello(Object* pObject, Player* plr)
		{
			if(plr->GetQuestLogForEntry(12670) ||  plr->HasFinishedQuest(12670))
			{
				TaxiPath* path = sTaxiMgr.GetTaxiPath(1053);
				plr->TaxiStart(path, 26308, 0);
				// remove Dominion Over Acherus, no need to check if target has this aura, actual check is done befor removing
				plr->RemoveAllAuraById(51721);
			}
		}
};

class ScourgeGryphonTwo : public GossipScript
{
	public:
		void GossipHello(Object* pObject, Player* plr)
		{
			if(plr->GetQuestLogForEntry(12670) ||  plr->HasFinishedQuest(12670))
			{
				TaxiPath* path = sTaxiMgr.GetTaxiPath(1054);
				plr->TaxiStart(path, 26308, 0);
			}
		}
};

// The Endless Hunger script for both GO and Unworthy Initiate

#define CN_INITIATE_1				29519
#define CN_INITIATE_2				29565
#define CN_INITIATE_3				29567
#define CN_INITIATE_4				29520
#define CN_INITIATE_5				29566

    //SPELL_CHAINED_PESANT_LH        54602,              // not used. possible it determine side, where to go get "weapon"
    //SPELL_CHAINED_PESANT_RH        54610,
#define    SPELL_CHAINED_PESANT_CHEST      54612
#define    SPELL_CHAINED_PESANT_BREATH     54613
#define    SPELL_INITIATE_VISUAL           51519

#define    SPELL_BLOOD_STRIKE              52374
#define    SPELL_DEATH_COIL                52375
#define    SPELL_ICY_TOUCH                 52372
#define    SPELL_PLAGUE_STRIKE             52373

#define    NPC_ANCHOR                      29521
#define    FACTION_MONSTER                 16

#define    PHASE_INACTIVE                  0
#define    PHASE_STANDUP                   1
#define    PHASE_REACH_ARMORY              2
#define    PHASE_ACTIVATE                  3
#define    PHASE_ATTACK_PLAYER             4

struct DisplayToSpell
{
	uint32 displayid;
	uint32 spellid;
};

DisplayToSpell m_DisplayToSpell[] =
{
    {25354, 51520},                                         // human M
    {25355, 51534},                                         // human F
    {25356, 51538},                                         // dwarf M
    {25357, 51541},                                         // draenei M
    {25358, 51535},                                         // nelf M
    {25359, 51539},                                         // gnome M
    {25360, 51536},                                         // nelf F
    {25361, 51537},                                         // dwarf F
    {25362, 51540},                                         // gnome F
    {25363, 51542},                                         // draenei F
    {25364, 51543},                                         // orc M
    {25365, 51546},                                         // troll M
    {25366, 51547},                                         // tauren M
    {25367, 51549},                                         // forsaken M
    {25368, 51544},                                         // orc F
    {25369, 51552},                                         // belf F
    {25370, 51545},                                         // troll F
    {25371, 51548},                                         // tauren F
    {25372, 51550},                                         // forsaken F
    {25373, 51551}                                          // belf M
};

static Location m_ArmoryLocations[] = 
{
	{ 2461.047363f, -5570.428223f, 415.664490f, 1.735095f },
	{ 2472.961670f, -5575.300781f, 415.429169f, 0.368502f },
	{ 2474.302246f, -5576.693848f, 415.394287f, 1.012529f },
	{ 2480.435547f, -5584.607422f, 415.663696f, 0.328443f },
	{ 2482.882324f, -5590.556152f, 415.663605f, 0.383420f },
	{ 2448.824219f, -5612.786621f, 415.664154f, 3.540719f },
	{ 2447.006104f, -5611.062500f, 415.664154f, 4.298627f },
	{ 2440.834229f, -5604.432617f, 415.664154f, 4.243647f },
	{ 2437.441895f, -5590.024414f, 415.663879f, 2.822077f },
	{ 2439.216797f, -5585.857422f, 415.663879f, 2.802442f },
};

class AcherusSoulPrison : GameObjectAIScript
{
	public:
		AcherusSoulPrison(GameObject * goinstance) : GameObjectAIScript(goinstance) {}
		static GameObjectAIScript * Create(GameObject* GO)
		{
			return new AcherusSoulPrison(GO);
		}

		void OnActivate(Player* pPlayer)
		{
			QuestLogEntry * en = pPlayer->GetQuestLogForEntry( 12848 );
			if( !en || en->GetMobCount(0) != 0 )
				return;

			float SSX = pPlayer->GetPositionX();
			float SSY = pPlayer->GetPositionY();
			float SSZ = pPlayer->GetPositionZ();

			Creature * pCreature = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(SSX, SSY, SSZ);

			if(!pCreature || !pCreature->isAlive())
				return;

			if(pCreature->GetEntry() == CN_INITIATE_1 || 
			   pCreature->GetEntry() == CN_INITIATE_2 ||
			   pCreature->GetEntry() == CN_INITIATE_3 || 
			   pCreature->GetEntry() == CN_INITIATE_4 ||
			   pCreature->GetEntry() == CN_INITIATE_5 )
			{
				pCreature->SetStandState(uint8(STANDSTATE_STAND));
				float Facing = pCreature->calcRadAngle(pCreature->GetPositionX(), pCreature->GetPositionY(), pPlayer->GetPositionX(), pPlayer->GetPositionY());
				pCreature->SetFacing(Facing);

				// this way we have refference to our player
				pCreature->m_escorter = pPlayer;

				// do all other things directly in creature's AI class, it's more comfortable
				pCreature->GetScript()->RegisterAIUpdateEvent(1000);
			}
		}
};

class UnworthyInitiate : public MoonScriptCreatureAI
{
	public:
		MOONSCRIPT_FACTORY_FUNCTION(UnworthyInitiate, MoonScriptCreatureAI);
		UnworthyInitiate(Creature * c) : MoonScriptCreatureAI(c)
		{
			AddSpell(SPELL_BLOOD_STRIKE, Target_Current, 75.0f, 0.0f, 4000);
			AddSpell(SPELL_DEATH_COIL, Target_Current, 75.f, 0.0f, 6000);
			AddSpell(SPELL_ICY_TOUCH, Target_Current, 75.f, 0.0f, 2000);
			AddSpell(SPELL_PLAGUE_STRIKE, Target_Current, 75.f, 0.0f, 5000);
			state = PHASE_INACTIVE;
		}

		void OnLoad()
		{
			// attach nearest prison and cast spell on it
			float SSX = _unit->GetPositionX();
			float SSY = _unit->GetPositionY();
			float SSZ = _unit->GetPositionZ();
			anchor = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(SSX, SSY, SSZ, NPC_ANCHOR);
			if( anchor )
			{
				anchor->CastSpell(_unit, SPELL_CHAINED_PESANT_BREATH, false);
				_unit->CastSpell(_unit, SPELL_CHAINED_PESANT_CHEST, false);
			}

			ParentClass::OnLoad();
		}

		void AIUpdate()
		{
			if( state == PHASE_INACTIVE )
			{
				timer = AddTimer(1000);
				state = PHASE_STANDUP;
			}
			else if( state == PHASE_STANDUP && IsTimerFinished(timer) )
			{
				_unit->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "You have committed a big mistake, demon");
				timer = AddTimer(5000);
				state = PHASE_REACH_ARMORY;
			}
			else if( state == PHASE_REACH_ARMORY && IsTimerFinished(timer) )
			{
				// test for nearest coords of armory
				uint32 nearest_entry = 0;
				uint32 nearest_distance = 1000;
				for(uint32 i = 0; i < sizeof(m_ArmoryLocations) / sizeof(Location); i++)
				{
					uint32 distance = _unit->CalcDistance(m_ArmoryLocations[i].x, m_ArmoryLocations[i].y, m_ArmoryLocations[i].z);

					if( distance < nearest_distance )
					{
						nearest_distance = distance;
						nearest_entry = i;
					}
				}
				
				// now go to nearest armory
				MoveTo(m_ArmoryLocations[nearest_entry].x, m_ArmoryLocations[nearest_entry].y, m_ArmoryLocations[nearest_entry].z, true);
				timer = AddTimer(2000);
				state = PHASE_ACTIVATE;
			}
			else if( state == PHASE_ACTIVATE && IsTimerFinished(timer) )
			{
				// face off the player
				float Facing = _unit->calcRadAngle(_unit->GetPositionX(), _unit->GetPositionY(), _unit->m_escorter->GetPositionX(), _unit->m_escorter->GetPositionY());
				_unit->SetFacing(Facing);

				// select suitable spell
				uint32 spell_to_cast = 0;
				for(uint32 i = 0; i < sizeof(m_DisplayToSpell) / sizeof(DisplayToSpell); i++)
				{
					if( m_DisplayToSpell[i].displayid == _unit->GetDisplayId() )
					{
						spell_to_cast = m_DisplayToSpell[i].spellid;
						break;
					}
				}

				// change look
				_unit->CastSpell(_unit, spell_to_cast, false);
				_unit->CastSpell(_unit, 48266, false); // blood presence
				timer = AddTimer(2000);
				state = PHASE_ATTACK_PLAYER;
			}
			else if( state == PHASE_ATTACK_PLAYER && IsTimerFinished(timer) )
			{
				_unit->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "And now you die!");
				state = -1; // we are fighting
				timer = AddTimer(1000);
			}
			else if( state == -1 && IsTimerFinished(timer) )
			{
				_unit->SetUInt32Value(UNIT_FIELD_FLAGS, 0);
				_unit->GetAIInterface()->setNextTarget(_unit->m_escorter);
				_unit->GetAIInterface()->AttackReaction(_unit->m_escorter, 1, 0);
			}

			ParentClass::AIUpdate();
		}

		void OnDied(Unit * mKiller)
		{
			if( !mKiller->IsPlayer() )
				return;

			// don't credit if any other player kills my initiate
			if( mKiller->GetGUID() != _unit->m_escorter->GetGUID() )
				return;

			Player * plr = TO_PLAYER(mKiller);
			QuestLogEntry * qle = plr->GetQuestLogForEntry(12848);
			if( !qle )
				return;

			qle->SetMobCount(0, 1);
			qle->SendUpdateAddKill(0);
			qle->UpdatePlayerFields();
			qle->SendQuestComplete();

			ParentClass::OnDied(mKiller);
		}

		void OnCombatStop(Unit * pTarget)
		{
			MoveToSpawnOrigin();
			_unit->SetUInt32Value(UNIT_FIELD_FLAGS, 33024);
			if( anchor )
			{
				anchor->CastSpell(_unit, SPELL_CHAINED_PESANT_BREATH, false);
				_unit->CastSpell(_unit, SPELL_CHAINED_PESANT_CHEST, false);
			}
			_unit->SetStandState(uint8(STANDSTATE_KNEEL));
			state = PHASE_INACTIVE;
			_unit->m_escorter = NULL;
			ParentClass::OnCombatStop(pTarget);
		}

		int32 timer;
		int8 state;
		Creature * anchor;
};


void SetupDeathKnight(ScriptMgr* mgr)
{
	GossipScript* SGO = new ScourgeGryphonOne();
	mgr->register_gossip_script(29488, SGO);
	GossipScript* SGT = new ScourgeGryphonTwo();
	mgr->register_gossip_script(29501, SGT);

	mgr->register_gameobject_script(191588, &AcherusSoulPrison::Create);
	mgr->register_gameobject_script(191577, &AcherusSoulPrison::Create);
	mgr->register_gameobject_script(191580, &AcherusSoulPrison::Create);
	mgr->register_gameobject_script(191581, &AcherusSoulPrison::Create);
	mgr->register_gameobject_script(191582, &AcherusSoulPrison::Create);
	mgr->register_gameobject_script(191583, &AcherusSoulPrison::Create);
	mgr->register_gameobject_script(191584, &AcherusSoulPrison::Create);
	mgr->register_gameobject_script(191585, &AcherusSoulPrison::Create);
	mgr->register_gameobject_script(191586, &AcherusSoulPrison::Create);
	mgr->register_gameobject_script(191587, &AcherusSoulPrison::Create);
	mgr->register_gameobject_script(191589, &AcherusSoulPrison::Create);
	mgr->register_gameobject_script(191590, &AcherusSoulPrison::Create);

	mgr->register_creature_script(CN_INITIATE_1, &UnworthyInitiate::Create);
	mgr->register_creature_script(CN_INITIATE_2, &UnworthyInitiate::Create);
	mgr->register_creature_script(CN_INITIATE_3, &UnworthyInitiate::Create);
	mgr->register_creature_script(CN_INITIATE_4, &UnworthyInitiate::Create);
	mgr->register_creature_script(CN_INITIATE_5, &UnworthyInitiate::Create);
}