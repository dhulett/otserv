//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
//
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


#ifndef __TOWN_H__
#define __TOWN_H__

#include <string>
#include <list>
#include <map>

#include "position.h"

class Town
{
public:
	Town(unsigned long _townid)
	{
		townid = _townid;
	}
	
	~Town(){};
	
	const Position& getTemplePosition() const {return posTemple;};
	const std::string& getName() const {return townName;};

	void setTemplePos(const Position& pos) {posTemple = pos;};
	void setName(std::string _townName) {townName = _townName;};
	unsigned long getTownID() const {return townid;};

private:
	unsigned long townid;
	std::string townName;
	Position posTemple;
};

typedef std::map<unsigned long, Town*> TownMap;

class Towns
{
public:
	static Towns& getInstance()
	{
		static Towns instance;
		return instance;
	}

	bool addTown(unsigned long _townid, Town* town)
	{
		TownMap::iterator it = townMap.find(_townid);
		
		if(it != townMap.end()){
			return false;
		}

		townMap[_townid] = town;
		return true;
	}
	
	Town* getTown(std::string townname)
	{
		for(TownMap::iterator it = townMap.begin(); it != townMap.end(); ++it){
			if(stricmp(it->second->getName().c_str(), townname.c_str()) == 0){
				return it->second;
			}
		}

		return NULL;
	}

	Town* getTown(unsigned long _townid)
	{
		TownMap::iterator it = townMap.find(_townid);
		
		if(it != townMap.end()){
			return it->second;
		}

		return NULL;
	}

private:
	TownMap townMap;
};

#endif