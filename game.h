//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
// class representing the gamestate
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////


#ifndef __OTSERV_GAME_H__
#define __OTSERV_GAME_H__

#include <queue>
#include <vector>
#include <set>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "spawn.h"
#include "position.h"
#include "item.h"
#include "container.h"
#include "magic.h"
#include "map.h"
#include "templates.h"
#include "scheduler.h"

class Creature;   // see creature.h
class Player;
class Monster;
class Npc;
class Commands;
class lessSchedTask;
class SchedulerTask;

#define STACKPOS_MOVE -1
#define STACKPOS_LOOK -2
#define STACKPOS_USE -3

/** State of a creature at a given time
  * Keeps track of all the changes to be able to send to the client
	*/

class CreatureState {
public:
	CreatureState() {};
	~CreatureState() {};

	Position pos;
	int damage;
	int manaDamage;
	bool drawBlood;
	std::vector<Creature*> attackerlist;
};

typedef std::vector< std::pair<Creature*, CreatureState> > CreatureStateVec;
typedef std::map<Tile*, CreatureStateVec> CreatureStates;

/** State of the game at a given time
  * Keeps track of all the changes to be able to send to the client
	*/

class Game;

class GameState {
public:
	GameState(Game *game, const Range &range);
	~GameState();

	void onAttack(Creature* attacker, const Position& pos, const MagicEffectClass* me);
	void onAttack(Creature* attacker, const Position& pos, Creature* attackedCreature);
	const CreatureStateVec& getCreatureStateList(Tile* tile) {return creaturestates[tile];};
	const SpectatorVec& getSpectators() {return spectatorlist;}

	/*bool isRemoved(Creature* creature)
	{
		std::list<Creature*>::iterator it = std::find(removedList.begin(), removedList.end(), creature);
		return (it != removedList.end());
	}
	*/
protected:
	void addCreatureState(Tile* tile, Creature* attackedCreature, int damage, int manaDamage, bool drawBlood);
	void onAttackedCreature(Tile* tile, Creature* attacker, Creature* attackedCreature, int damage, bool drawBlood);
	Game *game;

	SpectatorVec spectatorlist;
	CreatureStates creaturestates;
	//std::list<Creature*> removedList;
};

enum world_type_t{
	WORLD_TYPE_NO_PVP,
	WORLD_TYPE_PVP,
	WORLD_TYPE_PVP_ENFORCED
};

enum game_state_t{
	GAME_STATE_NORMAL,
	GAME_STATE_CLOSED,
	GAME_STATE_SHUTDOWN
};

enum lightState_t{
	LIGHT_STATE_DAY,
	LIGHT_STATE_NIGHT,
	LIGHT_STATE_SUNSET,
	LIGHT_STATE_SUNRISE,
};

/**
  * Main Game class.
  * This class is responsible to controll everything that happens
  */

class Game {
public:
	Game();
	~Game();
	
	/**
	  * Load a map.
	  * \param filename Mapfile to load
	  * \param filekind Kind of the map, BIN SQL or TXT
	  * \returns Int 0 built-in spawns, 1 needs xml spawns, 2 needs sql spawns, -1 if got error
	  */
	int loadMap(std::string filename, std::string filekind);
	
	/**
	  * Get the map size - info purpose only
	  * \param a the referenced witdh var
	  * \param b the referenced height var
	  */
	void getMapDimensions(int& a, int& b) {
		a = map->mapwidth;  
		b = map->mapheight;  
		return;
	}
  
	void setWorldType(world_type_t type);
	world_type_t getWorldType() const {return worldType;}
	//const std::string& getSpawnFile() {return map->spawnfile;}

	Cylinder* internalGetCylinder(Player* player, const Position& pos);
	Thing* internalGetThing(Player* player, const Position& pos, int32_t index);
	const Position& internalGetPosition(Player* player, const Position& pos);

	/**
	  * Get a single tile of the map.
	  * \returns A Pointer to the tile
		*/
	Tile* getTile(unsigned short _x, unsigned short _y, unsigned char _z);

	/**
	  * Returns a creature based on the unique creature identifier
	  * \param id is the unique creature id to get a creature pointer to
	  * \returns A Creature pointer to the creature
	  */
	Creature* getCreatureByID(unsigned long id);

	/**
	  * Returns a player based on the unique creature identifier
	  * \param id is the unique player id to get a player pointer to
	  * \returns A Pointer to the player
	  */
	Player* getPlayerByID(unsigned long id);

	/**
	  * Returns a creature based on a string name identifier
	  * \param s is the name identifier
	  * \returns A Pointer to the creature
	  */
	Creature* getCreatureByName(const std::string& s);

	/**
	  * Returns a player based on a string name identifier
	  * \param s is the name identifier
	  * \returns A Pointer to the player
	  */
	Player* getPlayerByName(const std::string& s);

	/**
	  * Starts an event.
	  * \returns A unique identifier for the event
	  */
	unsigned long addEvent(SchedulerTask*);

	/**
	  * Stop an event.
	  * \param eventid unique identifier to an event
	  */
	bool stopEvent(unsigned long eventid);

	/**
	  * Place Creature on the map.
	  * Adds the Creature to playersOnline and to the map
	  * \param creature Creature to place on the map
	  * \param isLogin Is true if the player is logging into the game
	  * \param forceLogin If true, placing the creature will not fail becase of obstacles (creatures/chests)
	  */
	bool placeCreature(const Position &pos, Creature* creature, bool isLogin = true, bool forceLogin = false);

	/**
		* Remove Creature from the map.
		* Removes the Creature the map
		* \param c Creature to remove
		*/
	bool removeCreature(Creature* creature, bool isLogout = true);

	uint32_t getPlayersOnline();
	uint32_t getMonstersOnline();
	uint32_t getNpcsOnline();
	uint32_t getCreaturesOnline();

	void getWorldLightInfo(LightInfo& lightInfo);

	void getSpectators(const Range& range, SpectatorVec& list);

	void thingMove(Player* player, const Position& fromPos, uint16_t itemId, uint8_t fromStackpos,
		const Position& toPos, uint8_t count);

	void moveCreature(Player* player, Cylinder* fromCylinder, Cylinder* toCylinder,
	Creature* moveCreature);
	ReturnValue moveCreature(Creature* creature, Direction direction);

	ReturnValue internalCreatureMove(Creature* creature, Cylinder* fromCylinder, Cylinder* toCylinder);

	void moveItem(Player* player, Cylinder* fromCylinder, Cylinder* toCylinder, int32_t index,
		Item* item, uint32_t count, uint16_t itemId);
	
	ReturnValue internalMoveItem(Cylinder* fromCylinder, Cylinder* toCylinder, int32_t index,
		Item* item, uint32_t count, uint32_t flags = 0);

	ReturnValue internalAddItem(Cylinder* toCylinder, Item* item, int32_t index = INDEX_WHEREEVER,
		uint32_t flags = 0, bool test = false);
	ReturnValue internalRemoveItem(Item* item, int32_t count = -1,  bool test = false);

	Item* transformItem(Item* item, uint16_t newtype, int32_t count = -1);
	ReturnValue internalTeleport(Thing *thing, const Position& newPos);

	/**
		* Creature wants to turn.
		* \param creature Creature pointer
		* \param dir Direction to turn to
		*/
	bool internalCreatureTurn(Creature* creature, Direction dir);

	/**
	  * Creature wants to say something.
	  * \param creature Creature pointer
	  * \param type Type of message
	  * \todo document types
	  * \param text The text to say
	  */
	bool internalCreatureSay(Creature* creature, SpeakClasses type, const std::string& text);

	bool internalCreatureChangeOutfit(Creature* creature);

	bool internalMonsterYell(Monster* monster, const std::string& text);
	
	//battle system
	bool internalCreatureSaySpell(Creature *creature, const std::string &text);
	bool creatureMakeMagic(Creature *creature, const Position& centerpos, const MagicEffectClass* me);
	bool creatureThrowRune(Creature *creature, const Position& centerpos, const MagicEffectClass& me);
	//bool creatureCastSpell(Creature *creature, const Position& centerpos, const MagicEffectClass& me);
	//battle system

	//Implementation of player invoked events
	bool playerWhisper(Player* player, const std::string& text);
	bool playerYell(Player* player, std::string& text);
	bool playerSpeakTo(Player* player, SpeakClasses type, const std::string& receiver, const std::string& text);
	bool playerBroadcastMessage(Player* player, const std::string& text);
	bool playerTalkToChannel(Player* player, SpeakClasses type, const std::string& text, unsigned short channelId);
	bool playerAutoWalk(Player* player, std::list<Direction>& path);

	bool playerUseItemEx(Player* player, const Position& fromPos, uint8_t fromStackPos, uint16_t fromItemId,
		const Position& toPos, uint8_t toStackPos, uint16_t toItemId);
	bool playerUseItem(Player* player, const Position& pos, uint8_t stackpos, uint8_t index, uint16_t itemId);
	bool playerUseBattleWindow(Player* player, const Position& fromPos, uint8_t fromStackPos,
		uint32_t creatureId, uint16_t itemId);
	bool playerRotateItem(Player* player, const Position& pos, uint8_t stackpos, const uint16_t itemId);
	bool playerWriteItem(Player* player, Item* item, const std::string& text);

	bool playerRequestTrade(Player* player, const Position& pos, uint8_t stackpos,
		uint32_t playerId, uint16_t itemId);
	bool playerAcceptTrade(Player* player);
	bool playerLookInTrade(Player* player, bool lookAtCounterOffer, int index);
	bool playerCloseTrade(Player* player);
	bool playerSetAttackedCreature(Player* player, unsigned long creatureid);
	bool playerLookAt(Player* player, const Position& pos, uint16_t itemId, uint8_t stackpos);
	bool playerRequestAddVip(Player* player, const std::string& vip_name);
	bool playerTurn(Player* player, Direction dir);
	bool playerSay(Player* player, SpeakClasses type, const std::string& text);
	bool playerChangeOutfit(Player* player);
	bool playerSaySpell(Player *player, const std::string& text);

	void flushSendBuffers();
	void addPlayerBuffer(Player* p);
	void FreeThing(Thing* thing);

	std::list<Position> getPathTo(Creature* creature, Position start, Position to);
	void changeOutfitAfter(unsigned long id, int looktype, long time);
	void changeSpeed(unsigned long id, unsigned short speed);
	void changeLight(const Creature* creature);
	#ifdef __SKULLSYSTEM__
	void changeSkull(Player* creature, skulls_t new_skull);
	#endif
	void AddMagicEffectAt(const Position& pos, uint8_t type);
	
	game_state_t getGameState();
	void setGameState(game_state_t newstate){game_state = newstate;}

	/** Lockvar for Game. */
	OTSYS_THREAD_LOCKVAR gameLock;   
	bool isExecutingEvents;

protected:
	std::vector<Player*> BufferedPlayers;
	std::vector<Thing*> ToReleaseThings;

	//list of items that are in trading state, mapped to the player
	std::map<Item*, unsigned long> tradeItems;
	
	AutoList<Creature> listCreature;

	void changeOutfit(unsigned long id, int looktype);

	//battle system
	void creatureApplyDamage(Creature *creature, int damage, int &outDamage, int &outManaDamage);
	bool creatureOnPrepareAttack(Creature *creature, Position pos);
	bool creatureOnPrepareMagicAttack(Creature *creature, Position pos, const MagicEffectClass* me);
	void creatureMakeDamage(Creature *creature, Creature *attackedCreature, fight_t damagetype);
	void CreateDamageUpdate(Creature* player, Creature* attackCreature, int damage);
	void CreateManaDamageUpdate(Creature* player, Creature* attackCreature, int damage);
	//battle system

	OTSYS_THREAD_LOCKVAR eventLock;
	OTSYS_THREAD_SIGNALVAR eventSignal;

	static OTSYS_THREAD_RETURN eventThread(void *p);

#ifdef __DEBUG_CRITICALSECTION__
	static OTSYS_THREAD_RETURN monitorThread(void *p);
#endif

	struct GameEvent
	{
		__int64  tick;
		int      type;
		void*    data;
	};

	void checkPlayerWalk(unsigned long id);
	void checkCreature(unsigned long creatureid);
	void checkCreatureAttacking(unsigned long creatureid, unsigned long time);
	void checkDecay(int t);
	void checkSpawns(int t);
	void checkLight(int t);
	
	#define DECAY_INTERVAL  10000
	void startDecay(Item* item);
	struct decayBlock{
		long decayTime;
		std::list<Item*> decayItems;
	};
	std::list<decayBlock*> decayVector;
	
	static const int LIGHT_LEVEL_DAY = 250;
	static const int LIGHT_LEVEL_NIGHT = 40;
	static const int SUNSET = 1305;
	static const int SUNRISE = 430;
	int lightlevel;
	lightState_t light_state;
	int light_hour;
	int light_hour_delta;
	
	std::priority_queue<SchedulerTask*, std::vector<SchedulerTask*>, lessSchedTask > eventList;
	std::map<unsigned long, SchedulerTask*> eventIdMap;
	unsigned long eventIdCount;

	uint32_t max_players;
	world_type_t worldType;

	Map* map;
	
	std::vector<std::string> commandTags;
	void addCommandTag(std::string tag);
	void resetCommandTag();
	
	game_state_t game_state;

	friend class Commands;
	friend class Monster;
	friend class GameState;
	friend class Spawn;
	friend class SpawnManager;
	friend class ActionScript;
	friend class Actions;
};

#endif
