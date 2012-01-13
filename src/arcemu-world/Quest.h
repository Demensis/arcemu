/*
 * ArcEmu MMORPG Server
 * Copyright (C) 2005-2007 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008-2011 <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef WOWSERVER_QUEST_H
#define WOWSERVER_QUEST_H

using namespace std;

enum QUEST_STATUS
{
    QMGR_QUEST_NOT_AVAILABLE					= 0x00,	// There aren't any quests available.		| "No Mark"
    QMGR_QUEST_AVAILABLELOW_LEVEL				= 0x01,	// Quest available, and your level isn't enough.| "Gray exclamation Mark !"
    QMGR_QUEST_CHAT								= 0x02,	// Quest available it shows a talk balloon.	| "No Mark"
    QMGR_QUEST_REPEATABLE_FINISHED_LOWLEVEL		= 0x03,
    QMGR_QUEST_REPEATABLE_LOWLEVEL				= 0x04,
	QMGR_QUEST_NOT_FINISHED						= 0x05,	// Quest isn't finished yet.		| "Gray Question ? Mark"
	QMGR_QUEST_REPEATABLE_FINISHED				= 0x06,	// Quest repeatable					| "Blue Question ? Mark"
	QMGR_QUEST_REPEATABLE						= 0x07,	// Quest daily and weekly			| "Blue exclamation ! Mark" 
	QMGR_QUEST_AVAILABLE						= 0x08,	// Quest available, and your level is enough	| "Yellow exclamation ! Mark"
	QMGR_QUEST_FINISHED2						= 0x09,	// Quest has been finished.			| "Yellow Question  ? Mark" (has no minimap icon) seems that these are used for method = 0 but not sure
	QMGR_QUEST_FINISHED							= 0x0A,	// Quest has been finished.			| "Yellow Question  ? Mark" (has minimap icon)

};

enum QUESTGIVER_QUEST_TYPE
{
    QUESTGIVER_QUEST_START  = 0x01,
    QUESTGIVER_QUEST_END	= 0x02,
};

enum QUEST_TYPE
{
	QUEST_TYPE_ELITE               = 1,
	QUEST_TYPE_LIFE                = 21,
	QUEST_TYPE_PVP                 = 41,
	QUEST_TYPE_RAID                = 62,
	QUEST_TYPE_DUNGEON             = 81,
	QUEST_TYPE_WORLD_EVENT         = 82,
	QUEST_TYPE_LEGENDARY           = 83,
	QUEST_TYPE_ESCORT              = 84, // not all quests has this, use QUEST_CUSTOM_FLAG_ESCORT_QUEST instead
	QUEST_TYPE_HEROIC              = 85,
	QUEST_TYPE_RAID_10             = 88,
	QUEST_TYPE_RAID_25             = 89
};

enum QUEST_FLAG
{
	QUEST_FLAG_NONE					= 0x00000000,
	QUEST_FLAG_STAY_ALIVE			= 0x00000001,   // probably used for quests where u need to protect someone  
    QUEST_FLAG_PARTY_ACCEPT			= 0x00000002,   // If player in party, all players that can accept this quest will receive confirmation box to accept quest CMSG_QUEST_CONFIRM_ACCEPT/SMSG_QUEST_CONFIRM_ACCEPT
	QUEST_FLAG_EXPLORATION			= 0x00000004,	// explore is needed to complete this Q
	QUEST_FLAG_SHARABLE				= 0x00000008,	// quest can be shared
	QUEST_FLAG_UNUSED				= 0x00000010,	// as of 3.3.2 unused
	QUEST_FLAG_EPIC					= 0x00000020,	// as of 3.3.2 only 2 Quests
	QUEST_FLAG_RAID					= 0x00000040,	// as of 3.3.3 around 523 quests
	QUEST_FLAG_TBC					= 0x00000080,	// only if tbc is allowed on that acc
	QUEST_FLAG_UNK1					= 0x00000100,	// something to do with quests which require items only?
	QUEST_FLAG_HIDDEN_REWARDS		= 0x00000200,	// hidden rewards, shown only in SMSG_QUESTGIVER_OFFER_REWARD
	QUEST_FLAG_AUTO_REWARDED		= 0x00000400,	// this quests doesnt appear into quest log, they automatically offer reward
	QUEST_FLAG_TBC_RACES			= 0x00000800,	// quests in draenei and BE start zone
	QUEST_FLAG_DAILY				= 0x00001000,	// daily quest
	QUEST_FLAG_FLAGS_PVP			= 0x00002000,	// activates pvp on accept
	QUEST_FLAG_WEEKLY				= 0x00008000,	// weekly quest
	QUEST_FLAG_AUTO_COMPLETE		= 0x00010000,	// quest is autocomplete (?) not handled by core
	QUEST_FLAG_AUTO_ACCEPT			= 0x00080000,	// start zone quests, these are auto added into player's quest log
};

enum SpecialFlags
{
	// These are custom
	QUEST_SPECIAL_FLAG_REPEATABLE			= 0x00000001, // simple repeatable quest
	QUEST_SPECIAL_FLAG_TIMED				= 0x00000002, // timed quests
	QUEST_SPECIAL_FLAG_ESCORT_QUEST			= 0x00000004, // this just call to scripts
	QUEST_SPECIAL_FLAG_SCRIPTED				= 0x00000008, // set this flag, so quest is not complete even there is no requirements to be completed
};

enum FAILED_REASON
{
    FAILED_REASON_FAILED			= 0,
    FAILED_REASON_INV_FULL			= 4,
    FAILED_REASON_DUPE_ITEM_FOUND   = 17,
};

enum INVALID_REASON
{
	INVALID_REASON_DONT_HAVE_LEVEL			= 1,	// You are not high enough level for that quest.
	INVALID_REASON_DONT_HAVE_RACE			= 6,	// That quest is not available to your race.
	INVALID_REASON_COMPLETED_QUEST			= 7,	// You have completed that quest.
	INVALID_REASON_HAVE_TIMED_QUEST			= 12,	// You can only be on one timed quest at a time.
	INVALID_REASON_HAVE_QUEST				= 13,	// You are already on that quest.
	INVALID_REASON_DONT_HAVE_EXP_ACCOUNT	= 16,	// This quest requires an expansion enabled account.
	INVALID_REASON_HAVE_QUEST2				= 18,	// You are already on that quest.
	INVALID_REASON_DONT_HAVE_REQ_ITEMS		= 21,	// You don't have the required items with you. Check storage.
	INVALID_REASON_DONT_HAVE_REQ_MONEY		= 23,	// You don't have enough money for that quest.
	INVALID_REASON_REACHED_DAILY_LIMIT		= 26,	// You have already completed 25 daily quests today.
	INVALID_REASON_REACHED_TIRED_LIMIT		= 27,	// You cannot complete quests once you have reached tired time.
	INVALID_REASON_DAILY_COMPLETED_TODAY	= 29,	// You have completed that daily quest today.
};

enum QUEST_SHARE
{
    QUEST_SHARE_MSG_SHARING_QUEST			= 0,
    QUEST_SHARE_MSG_CANT_TAKE_QUEST			= 1,
    QUEST_SHARE_MSG_ACCEPT_QUEST			= 2,
    QUEST_SHARE_MSG_REFUSE_QUEST			= 3,
	QUEST_SHARE_MSG_BUSY					= 4,
	QUEST_SHARE_MSG_LOG_FULL				= 5,
	QUEST_SHARE_MSG_HAVE_QUEST				= 6,
    QUEST_SHARE_MSG_FINISH_QUEST			= 7,
    QUEST_SHARE_MSG_CANT_BE_SHARED_TODAY	= 8, //VLack: the following 4 messages (from 8 to 11) are unused on ArcEmu, but for completeness I have included them here, maybe we'll need them later...
    QUEST_SHARE_MSG_SHARING_TIMER_EXPIRED	= 9,
    QUEST_SHARE_MSG_NOT_IN_PARTY			= 10,
    QUEST_SHARE_MSG_DIFFERENT_SERVER_DAILY	= 11,
};

#define MAX_REQUIRED_QUEST_ITEM 6

enum QUEST_METHOD
{
	QUEST_METHOD_NO_OBJECTIVES				= 0,
	QUEST_METHOD_UNK1						= 1, // all old pvp quests for classic wow BGs ?
	QUEST_METHOD_NORMAL_QUEST				= 2
};

enum QUEST_MOB_TYPES
{
    QUEST_MOB_TYPE_CREATURE = 1,
    QUEST_MOB_TYPE_GAMEOBJECT = 2,
};

enum QuestCompletionStatus
{
	QUEST_INCOMPLETE = 0,
	QUEST_COMPLETE   = 1,
	QUEST_FAILED     = 2
};


class QuestScript;
#pragma pack(push,1)
struct Quest
{
	uint32 id;
	uint32 method;
	int32 zone_or_sort; // negative sort, positive is zone
	uint32 quest_flags;
	uint32 min_level;
	int32 questlevel;
	uint32 type;
	uint32 required_races;
	uint32 required_class;
	uint32 required_tradeskill;
	uint32 required_tradeskill_value;
	uint32 required_rep_faction[2];
	int32 required_rep_value[2];
	uint32 requiredtitleid;

	uint32 time;
	uint32 special_flags;

	uint32 next_quest_id;

	uint32 srcitem;
	uint32 srcspell;

	char* title;
	char* details;
	char* objectives;
	char* completiontext;
	char* incompletetext;
	char* endtext;
	char * completedtext;

	char* objectivetexts[4];

	uint32 required_item[ MAX_REQUIRED_QUEST_ITEM ];
	uint32 required_itemcount[ MAX_REQUIRED_QUEST_ITEM ];

	int32 required_mob[4]; //positive is NPC, negative is GO
	uint32 required_mobcount[4];
	uint32 ReqSourceId[4];
	uint32 ReqSourceCount[4];
	uint32 required_spell[4];
	uint32 required_emote[4];

	uint32 reward_choiceitem[6];
	uint32 reward_choiceitemcount[6];

	uint32 reward_item[4];
	uint32 reward_itemcount[4];

	uint32 reward_repfaction[5];
	int32 reward_repvalue[5];
	uint32 reward_repvalueoverride[5];
	int32 reward_replimit[5]; // rep limit should be lower than 0? maybe for some optional quests

	int32 reward_money; // negativemeans required money 
	uint32 reward_xp;
	uint32 reward_spell;
	uint32 effect_on_player;

	uint32 MailTemplateId;
	uint32 MailDelaySecs;
	uint32 MailSendItem;

	uint32 point_mapid;
	float point_x;
	float point_y;
	uint32 point_opt;

	uint32 rew_money_at_max_level;
	uint32 required_triggers[4];
	char * x_or_y_quest_string;
	char * required_quests;
	char * required_quests_in_q_log;
	char * none_of_quests;
	uint32 receive_items[4];
	uint32 receive_itemcount[4];

	uint32 bonushonor;
	uint32 bonushonorraw;
	uint32 bonusarenapoints;
	uint32 rewardtitleid;
	uint32 rewardtalents;
	uint32 suggestedplayers;
	uint32 requiredplayerkillcount;
	uint32 unk1;

	/* emotes */
	uint32 detailemotecount;
	uint32 detailemote[4];
	uint32 detailemotedelay[4];
	uint32 completionemotecount;
	uint32 completionemote[4];
	uint32 completionemotedelay[4];
	uint32 completeemote;
	uint32 incompleteemote;
	uint32 RewXPId;
	uint32 event_id;

	/* this marks the end of the fields loaded from db - don't remove the comment please */

	uint32 count_required_mob;
	uint32 count_requiredtriggers;
	uint32 count_receiveitems;
	uint32 count_reward_choiceitem;
	uint32 count_required_item;
	uint32 required_mobtype[4];
	uint32 count_reward_item;
	set<uint32> quest_list;
	set<uint32> none_of_quests_list;
	set<uint32> required_q_log_quest_list;
	set<uint32> required_quests_list;

	QuestScript* pQuestScript;

	///////////////////////////////////////////////////////////////////
	//bool HasFlag( uint32 flag )
	//  Tells if the quest has a specific flag.
	//
	//
	//Parameters:
	//  uint32 flag  -  flag to check
	//
	//Return Value
	//  Returns true if the quest has this flag.
	//  Returns false if the quest doesn't have this flag.
	//
	///////////////////////////////////////////////////////////////////
	bool HasFlag(uint32 flag)
	{
		if((quest_flags & flag) != 0)
			return true;
		else
			return false;
	}

	bool HasSpecialFlag(uint32 flag)
	{
		if((this->special_flags & flag) != 0)
			return true;
		else
			return false;
	}
};
#pragma pack(pop)

class QuestScript;
#define CALL_QUESTSCRIPT_EVENT(obj, func) if(TO< QuestLogEntry* >(obj)->GetQuest()->pQuestScript != NULL) TO< QuestLogEntry* >(obj)->GetQuest()->pQuestScript->func

#define MAX_QUEST_LOG_SIZE 25

class SERVER_DECL QuestLogEntry : public EventableObject
{
		friend class QuestMgr;

	public:

		QuestLogEntry();
		~QuestLogEntry();

		ARCEMU_INLINE Quest* GetQuest() { return m_quest; };
		void Init(Quest* quest, Player* plr, uint32 slot);

		bool CanBeFinished();
		void Complete();
		void SaveToDB(QueryBuffer* buf);
		bool LoadFromDB(Field* fields);
		void UpdatePlayerFields();

		void SetTrigger(uint32 i);
		void SetMobCount(uint32 i, uint32 count);
		void IncrementMobCount(uint32 i);

		bool IsUnitAffected(Unit* target);
		ARCEMU_INLINE bool IsCastQuest() { return iscastquest;}
		ARCEMU_INLINE bool IsEmoteQuest() { return isemotequest; }
		void AddAffectedUnit(Unit* target);
		void ClearAffectedUnits();

		void SetSlot(int32 i);
		void Finish();


		//////////////////////////////////////////////////////////////
		//void Fail( bool timerexpired )
		//  Marks the quest as failed
		//
		//Parameter(s)
		//  bool timerexpired  -  true if the reason for failure is
		//                        timer expiration.
		//
		//Return Value
		//  None
		//
		//
		//////////////////////////////////////////////////////////////
		void Fail( bool timerexpired );


		/////////////////////////////////////////////////////////////
		//bool HasFailed()
		//  Tells if the Quest has failed.
		//
		//Parameter(s)
		//  None
		//
		//Return Value
		//  Returns true if the quest has failed.
		//  Returns false otherwise.
		//
		//
		/////////////////////////////////////////////////////////////
		bool HasFailed(){
			if( completed == QUEST_FAILED )
				return true;
			else
				return false;
		}


		void SendQuestComplete();
		void SendUpdateAddKill(uint32 i);
		ARCEMU_INLINE uint32 GetMobCount(uint32 i) { return m_mobcount[i]; }
		ARCEMU_INLINE uint32 GetExploredAreas(uint32 i) { return m_explored_areas[i]; }

		ARCEMU_INLINE uint32 GetBaseField(uint32 slot)
		{
			return PLAYER_QUEST_LOG_1_1 + (slot * 5);
		}
		ARCEMU_INLINE int32 GetSlot() { return m_slot; }

	private:
		uint32 completed;

		bool mInitialized;
		bool mDirty;

		Quest* m_quest;
		Player* m_plr;

		uint32 m_mobcount[4];
		uint32 m_explored_areas[4];

		std::set<uint64> m_affected_units;
		bool iscastquest;
		bool isemotequest;

		uint32 expirytime;
		int32 m_slot;
};

#endif
