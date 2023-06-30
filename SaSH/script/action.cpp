﻿#include "stdafx.h"
#include "interpreter.h"
#include "util.h"
#include "injector.h"
#include "map/mapanalyzer.h"

//action
int Interpreter::useitem(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	QString itemName;
	int target = 0;
	int min = 0, max = 14;
	if (!checkRange(TK, 1, &min, &max))
	{
		checkString(TK, 1, &itemName);

		if (itemName.isEmpty())
			return Parser::kArgError;
		target = -2;
		checkInt(TK, 2, &target);
		if (target == -2)
		{
			QString targetTypeName;
			checkString(TK, 2, &targetTypeName);
			if (targetTypeName.isEmpty())
			{
				target = 0;

			}
			else
			{
				QHash<QString, int> hash = {
					{ u8"自己", 0},
					{ u8"戰寵", injector.server->pc.battlePetNo},
					{ u8"騎寵", injector.server->pc.ridePetNo},
					{ u8"隊長", 6},

					{ u8"自己", 0},
					{ u8"战宠", injector.server->pc.battlePetNo},
					{ u8"骑宠", injector.server->pc.ridePetNo},
					{ u8"队长", 6},

					{ u8"self", 0},
					{ u8"battlepet", injector.server->pc.battlePetNo},
					{ u8"ride", injector.server->pc.ridePetNo},
					{ u8"leader", 6},
				};

				for (int i = 0; i < MAX_PET; ++i)
				{
					hash.insert(u8"寵物" + QString::number(i + 1), i + 1);
					hash.insert(u8"宠物" + QString::number(i + 1), i + 1);
					hash.insert(u8"pet" + QString::number(i + 1), i + 1);
				}

				for (int i = 1; i < MAX_PARTY; ++i)
				{
					hash.insert(u8"隊員" + QString::number(i), i + 1 + MAX_PET);
					hash.insert(u8"队员" + QString::number(i), i + 1 + MAX_PET);
					hash.insert(u8"teammate" + QString::number(i), i + 1 + MAX_PET);
				}

				if (!hash.contains(targetTypeName))
					return Parser::kArgError;

				target = hash.value(targetTypeName, -1);
				if (target < 0)
					return Parser::kArgError;
			}
		}
	}
	else
	{
		min += CHAR_EQUIPPLACENUM;
		max += CHAR_EQUIPPLACENUM;

		checkInt(TK, 2, &target);

		checkString(TK, 3, &itemName);

		if (itemName.isEmpty())
			return Parser::kArgError;

	}

	if (target > 100 || target == -1)
	{
		util::SafeVector<int> v;
		if (!injector.server->getItemIndexsByName(itemName, "", &v))
			return Parser::kNoChange;

		int totalUse = target - 100;

		for (const int& it : v)
		{
			ITEM item = injector.server->pc.item[it];
			int size = injector.server->pc.item[it].pile;
			for (int i = 0; i < size; ++i)
			{
				injector.server->useItem(it, 0);
				if (target != -1)
				{
					--totalUse;
					if (totalUse <= 0)
						return Parser::kNoChange;
				}
			}
		}
		return Parser::kNoChange;
	}

	int itemIndex = injector.server->getItemIndexByName(itemName);
	if (itemIndex == -1)
		return Parser::kNoChange;

	injector.server->useItem(itemIndex, target);
	return Parser::kNoChange;
}

int Interpreter::dropitem(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	QString tempName;
	checkString(TK, 1, &tempName);
	if (tempName.isEmpty())
		return Parser::kArgError;

	//指定丟棄白名單，位於白名單的物品不丟棄
	if (tempName == QString(u8"非"))
	{
		int min = 0, max = 14;
		if (!checkRange(TK, 2, &min, &max))
			return Parser::kArgError;

		min += CHAR_EQUIPPLACENUM;
		max += CHAR_EQUIPPLACENUM;

		QString itemNames;
		checkString(TK, 3, &itemNames);
		if (itemNames.isEmpty())
			return Parser::kArgError;

		QStringList itemNameList = itemNames.split(util::rexOR, Qt::SkipEmptyParts);
		if (itemNameList.isEmpty())
			return Parser::kArgError;

		util::SafeVector<int> indexs;
		for (int i = CHAR_EQUIPPLACENUM; i < MAX_ITEM; ++i)
		{
			ITEM item = injector.server->pc.item[i];
			for (const QString& name : itemNameList)
			{
				if (item.name == name)
				{
					if (!indexs.contains(i))
						indexs.append(i);
				}
				else if (name.startsWith(kFuzzyPrefix))
				{
					QString newName = name;
					newName.remove(0, 1);
					if (item.name.contains(newName))
					{
						if (!indexs.contains(i))
							indexs.append(i);
					}
				}
			}
		}

		//排除掉所有包含再indexs的
		for (int i = min; i <= max; ++i)
		{
			if (!indexs.contains(i))
				injector.server->dropItem(i);
		}

	}
	else
	{
		QString itemNames = tempName;

		QStringList itemNameList = itemNames.split(util::rexOR, Qt::SkipEmptyParts);
		if (itemNameList.isEmpty())
			return Parser::kArgError;

		//int size = itemNameList.size();
		for (const QString& name : itemNameList)
		{
			util::SafeVector<int> indexs;
			if (injector.server->getItemIndexsByName(name, "", &indexs))
			{
				for (const int it : indexs)
					injector.server->dropItem(it);
			}
		}

	}
	return Parser::kNoChange;
}

int Interpreter::playerrename(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	QString newName;
	checkString(TK, 1, &newName);

	injector.server->setPlayerFreeName(newName);

	return Parser::kNoChange;
}

int Interpreter::petrename(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	int petIndex = -1;
	checkInt(TK, 1, &petIndex);
	petIndex -= 1;

	QString newName;
	checkString(TK, 2, &newName);

	injector.server->setPetFreeName(petIndex, newName);

	return Parser::kNoChange;
}

int Interpreter::setpetstate(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	int petIndex = -1;
	checkInt(TK, 1, &petIndex);
	petIndex -= 1;
	if (petIndex < 0 || petIndex >= MAX_PET)
		return Parser::kArgError;

	QString stateStr;
	checkString(TK, 2, &stateStr);
	if (stateStr.isEmpty())
		stateStr = QString(u8"rest");

	PetState state = petStateMap.value(stateStr.toLower(), PetState::kRest);


	injector.server->setPetState(petIndex, state);

	return Parser::kNoChange;
}

int Interpreter::droppet(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	int petIndex = -1;
	checkInt(TK, 1, &petIndex);
	petIndex -= 1;

	QString petName;
	checkString(TK, 1, &petName);

	if (petIndex >= 0)
		injector.server->dropPet(petIndex);
	else if (!petName.isEmpty())
	{
		util::SafeVector<int> v;
		if (injector.server->getPetIndexsByName(petName, &v))
		{
			for (const int it : v)
				injector.server->dropPet(it);
		}
	}

	return Parser::kNoChange;
}

int Interpreter::buy(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	int itemIndex = -1;
	checkInt(TK, 1, &itemIndex);
	itemIndex -= 1;

	int count = 0;
	checkInt(TK, 2, &count);

	if (itemIndex < 0 || count <= 0)
		return Parser::kArgError;

	QString npcName;
	checkString(TK, 3, &npcName);

	if (npcName.isEmpty())
		injector.server->buy(itemIndex, count);
	else
	{
		mapunit_t unit;
		if (injector.server->findUnit(npcName, util::OBJ_NPC, &unit))
		{
			injector.server->buy(itemIndex, count, kDialogBuy, unit.id);
		}
	}

	return Parser::kNoChange;
}

int Interpreter::sell(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	QString name;
	checkString(TK, 1, &name);
	QStringList nameList = name.split(util::rexOR, Qt::SkipEmptyParts);
	if (nameList.isEmpty())
		return Parser::kArgError;

	QString npcName;
	checkString(TK, 2, &npcName);


	util::SafeVector<int> itemIndexs;
	for (const QString& it : nameList)
	{
		util::SafeVector<int> indexs;
		if (!injector.server->getItemIndexsByName(it, "", &indexs))
			continue;
		itemIndexs.append(indexs);
	}

	std::sort(itemIndexs.begin(), itemIndexs.end());
	auto it = std::unique(itemIndexs.begin(), itemIndexs.end());
	itemIndexs.erase(it, itemIndexs.end());

	if (npcName.isEmpty())
		injector.server->sell(itemIndexs);
	else
	{
		mapunit_t unit;
		if (injector.server->findUnit(npcName, util::OBJ_NPC, &unit))
		{
			injector.server->sell(itemIndexs, kDialogSell, unit.id);

		}
	}

	return Parser::kNoChange;
}

int Interpreter::sellpet(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	mapunit_t unit;
	if (!injector.server->findUnit("宠物店", util::OBJ_NPC, &unit))
	{
		if (!injector.server->findUnit("寵物店", util::OBJ_NPC, &unit))
			return Parser::kNoChange;
	}

	//int petIndex = -1;
	int min = 1, max = MAX_PET;
	if (!checkRange(TK, 1, &min, &max))
	{
		min = 0;
		checkInt(TK, 1, &min);
		if (min <= 0 || min > MAX_PET)
			return Parser::kArgError;
		max = min;
	}

	for (int petIndex = min; petIndex <= max; ++petIndex)
	{
		if (isInterruptionRequested())
			return Parser::kNoChange;

		if (injector.server.isNull())
			return Parser::kError;

		if (injector.server->pet[petIndex - 1].useFlag == 0)
			continue;

		bool bret = false;
		for (;;)
		{
			if (isInterruptionRequested())
				return Parser::kNoChange;

			if (injector.server.isNull())
				return Parser::kError;

			dialog_t dialog = injector.server->currentDialog.get();
			switch (dialog.seqno)
			{
			case 263:
			{
				//injector.server->IS_WAITFOR_DIALOG_FLAG = true;
				injector.server->press(BUTTON_YES, 263, unit.id);
				//waitfor(1000, [&injector]()->bool { return !injector.server->IS_WAITFOR_DIALOG_FLAG; });
				bret = true;
				break;
			}
			case 262:
			{
				//injector.server->IS_WAITFOR_DIALOG_FLAG = true;
				injector.server->press(petIndex, 262, unit.id);
				injector.server->press(BUTTON_YES, 263, unit.id);
				bret = true;
				//waitfor(1000, [&injector]()->bool { return !injector.server->IS_WAITFOR_DIALOG_FLAG; });
				break;
			}
			default:
			{
				//injector.server->IS_WAITFOR_DIALOG_FLAG = true;
				injector.server->press(3, 261, unit.id);
				injector.server->press(petIndex, 262, unit.id);
				injector.server->press(BUTTON_YES, 263, unit.id);
				bret = true;
				//waitfor(1000, [&injector]()->bool { return !injector.server->IS_WAITFOR_DIALOG_FLAG; });
				break;
			}
			}

			if (bret)
				break;

			//QThread::msleep(300);
		}
	}


	return Parser::kNoChange;
}

int Interpreter::make(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	QString ingreName;
	checkString(TK, 1, &ingreName);

	QStringList ingreNameList = ingreName.split(util::rexOR, Qt::SkipEmptyParts);
	if (ingreNameList.isEmpty())
		return Parser::kArgError;

	injector.server->craft(util::kCraftItem, ingreNameList);

	return Parser::kNoChange;
}

int Interpreter::cook(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	QString ingreName;
	checkString(TK, 1, &ingreName);

	QStringList ingreNameList = ingreName.split(util::rexOR, Qt::SkipEmptyParts);
	if (ingreNameList.isEmpty())
		return Parser::kArgError;

	injector.server->craft(util::kCraftFood, ingreNameList);

	return Parser::kNoChange;
}

int Interpreter::learn(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	int petIndex = 0;
	checkInt(TK, 1, &petIndex);
	if (petIndex <= 0 || petIndex >= 6)
		return Parser::kArgError;
	--petIndex;

	int skillIndex = 0;
	checkInt(TK, 2, &skillIndex);
	if (skillIndex <= 0)
		return Parser::kArgError;
	--skillIndex;


	int spot = 0;
	checkInt(TK, 3, &spot);
	if (spot <= 0 || spot > MAX_SKILL + 1)
		return Parser::kArgError;
	--spot;

	QString npcName;
	checkString(TK, 4, &npcName);


	int dialogid = -1;
	checkInt(TK, 5, &dialogid);

	if (npcName.isEmpty())
		injector.server->learn(skillIndex, petIndex, spot);
	else
	{
		mapunit_t unit;
		if (injector.server->findUnit(npcName, util::OBJ_NPC, &unit))
		{
			injector.server->learn(skillIndex, petIndex, spot, dialogid, unit.id);
		}
	}

	return Parser::kNoChange;
}

//group
int Interpreter::join(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	injector.server->setTeamState(true);

	return Parser::kNoChange;
}

int Interpreter::leave(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	injector.server->setTeamState(false);

	return Parser::kNoChange;
}

int Interpreter::kick(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	//0解隊 1-4隊員
	int partyIndex = -1;
	checkInt(TK, 1, &partyIndex);
	if (partyIndex < 0 || partyIndex >= MAX_PARTY)
	{
		QString exceptName;
		checkString(TK, 1, &exceptName);
		if (!exceptName.isEmpty())
		{
			QStringList list = exceptName.split(util::rexOR, Qt::SkipEmptyParts);
			if (list.isEmpty() || list.size() > 4)
				return Parser::kArgError;

			util::SafeVector<int> wrongIndex;

			bool bret = false;
			for (int i = 1; i < MAX_PARTY; ++i)
			{
				util::SafeHash<QString, QVariant> party = injector.server->hashparty.value(i);
				if (party.value("valid").toInt() == 0)
					continue;

				for (QString it : list)
				{
					bool isExact = true;
					QString newName = it.trimmed();
					if (newName.startsWith(kFuzzyPrefix))
					{
						newName = newName.mid(1);
						isExact = false;
					}

					if (isExact && party.value("name").toString() == newName)
					{
						bret = true;
					}
					else if (!isExact && party.value("name").toString().contains(newName))
					{
						bret = true;
					}

				}

				if (!bret)
				{
					wrongIndex.push_back(i);
				}
			}

			if (!wrongIndex.isEmpty())
			{
				for (int i : wrongIndex)
				{
					injector.server->kickteam(i);
				}
			}
		}
	}

	injector.server->kickteam(partyIndex);

	return Parser::kNoChange;
}

int Interpreter::usemagic(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	QString magicName;
	checkString(TK, 1, &magicName);
	if (magicName.isEmpty())
		return Parser::kArgError;

	int target = -1;
	checkInt(TK, 2, &target);
	if (target < 0)
	{
		QString targetTypeName;
		checkString(TK, 2, &targetTypeName);
		if (targetTypeName.isEmpty())
		{
			target = 0;

		}
		else
		{


			QHash<QString, int> hash = {
				{ u8"自己", 0},
				{ u8"戰寵", injector.server->pc.battlePetNo},
				{ u8"騎寵", injector.server->pc.ridePetNo},
				{ u8"隊長", 6},

				{ u8"自己", 0},
				{ u8"战宠", injector.server->pc.battlePetNo},
				{ u8"骑宠", injector.server->pc.ridePetNo},
				{ u8"队长", 6},

				{ u8"self", 0},
				{ u8"battlepet", injector.server->pc.battlePetNo},
				{ u8"ride", injector.server->pc.ridePetNo},
				{ u8"leader", 6},
			};

			for (int i = 0; i < MAX_PET; ++i)
			{
				hash.insert(u8"寵物" + QString::number(i + 1), i + 1);
				hash.insert(u8"宠物" + QString::number(i + 1), i + 1);
				hash.insert(u8"pet" + QString::number(i + 1), i + 1);
			}

			for (int i = 1; i < MAX_PARTY; ++i)
			{
				hash.insert(u8"隊員" + QString::number(i), i + 1 + MAX_PET);
				hash.insert(u8"队员" + QString::number(i), i + 1 + MAX_PET);
				hash.insert(u8"teammate" + QString::number(i), i + 1 + MAX_PET);
			}

			if (!hash.contains(targetTypeName))
				return Parser::kArgError;

			target = hash.value(targetTypeName, -1);
			if (target < 0)
				return Parser::kArgError;
		}
	}

	int magicIndex = injector.server->getMagicIndexByName(magicName);
	if (magicIndex < 0)
		return Parser::kArgError;

	injector.server->useMagic(magicIndex, target);

	return Parser::kNoChange;
}

int Interpreter::pickitem(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	QString dirStr;
	checkString(TK, 1, &dirStr);
	if (dirStr.isEmpty())
		return Parser::kArgError;

	if (dirStr.startsWith(u8"全"))
	{
		for (int i = 0; i < 7; ++i)
		{
			injector.server->setPlayerFaceDirection(i);
			injector.server->pickItem(i);
		}
	}
	else
	{
		DirType type = dirMap.value(dirStr, kDirNone);
		if (type == kDirNone)
			return Parser::kArgError;
		injector.server->setPlayerFaceDirection(type);
		injector.server->pickItem(type);
	}

	return Parser::kNoChange;
}

int Interpreter::depositgold(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	if (injector.server->pc.gold <= 0)
		return Parser::kNoChange;

	int gold;
	checkInt(TK, 1, &gold);

	int isPublic = 0;
	checkInt(TK, 2, &isPublic);

	if (gold <= 0)
		return Parser::kArgError;

	if (gold > injector.server->pc.gold)
		gold = injector.server->pc.gold;

	injector.server->depositGold(gold, isPublic > 0);

	return Parser::kNoChange;
}

int Interpreter::withdrawgold(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	int gold;
	checkInt(TK, 1, &gold);

	int isPublic = 0;
	checkInt(TK, 2, &isPublic);

	injector.server->withdrawGold(gold, isPublic > 0);

	return Parser::kNoChange;
}

util::SafeHash<int, ITEM> recordedEquip_;
int Interpreter::recordequip(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	if (!injector.server->getOnlineFlag())
		return Parser::kNoChange;

	recordedEquip_.clear();
	for (int i = 0; i < CHAR_EQUIPPLACENUM; ++i)
	{
		injector.server->announce(QObject::tr("record equip:[%1]%2").arg(i + 1).arg(injector.server->pc.item[i].name));
		recordedEquip_.insert(i, injector.server->pc.item[i]);
	}

	return Parser::kNoChange;
}

int Interpreter::wearequip(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	if (!injector.server->getOnlineFlag())
		return Parser::kNoChange;

	for (int i = 0; i < CHAR_EQUIPPLACENUM; ++i)
	{
		ITEM item = injector.server->pc.item[i];
		ITEM recordedItem = recordedEquip_.value(i);
		if (recordedItem.useFlag == 0 || recordedItem.name.isEmpty())
			continue;

		if (item.name == recordedItem.name && item.memo == recordedItem.memo)
			continue;

		int itemIndex = injector.server->getItemIndexByName(recordedItem.name);
		if (itemIndex == -1)
			continue;

		injector.server->useItem(itemIndex, 0);
	}

	return Parser::kNoChange;
}

int Interpreter::unwearequip(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	int part = CHAR_EQUIPNONE;

	if (!checkInt(TK, 1, &part) || part < 1)
	{
		QString partStr;
		checkString(TK, 1, &partStr);
		if (partStr.isEmpty())
			return Parser::kArgError;

		if (partStr.toLower() == "all" || partStr.toLower() == QString("全部"))
		{
			part = 100;
		}
		else
		{
			part = equipMap.value(partStr.toLower(), CHAR_EQUIPNONE);
			if (part == CHAR_EQUIPNONE)
				return Parser::kArgError;
		}
	}
	else
		--part;
	if (part < 100)
	{
		int spotIndex = injector.server->getItemEmptySpotIndex();
		if (spotIndex == -1)
			return Parser::kNoChange;

		injector.server->swapItem(part, spotIndex);
	}
	else
	{
		util::SafeVector<int> v;
		if (!injector.server->getItemEmptySpotIndexs(&v))
			return Parser::kNoChange;

		for (int i = 0; i < CHAR_EQUIPPLACENUM; ++i)
		{
			if (v.isEmpty())
				break;

			int itemIndex = v.takeFirst();

			injector.server->swapItem(i, itemIndex);
		}
	}

	return Parser::kNoChange;
}

int Interpreter::petequip(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	int petIndex = -1;
	if (!checkInt(TK, 1, &petIndex))
		return Parser::kArgError;

	if (petIndex < 0 || petIndex >= MAX_PET)
		return Parser::kArgError;

	QString itemName;
	checkString(TK, 2, &itemName);
	if (itemName.isEmpty())
		return Parser::kArgError;

	int itemIndex = injector.server->getItemIndexByName(itemName);
	if (itemIndex != -1)
		injector.server->petitemswap(petIndex, itemIndex, -1);

	return Parser::kNoChange;
}

int Interpreter::petunequip(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	int part = CHAR_EQUIPNONE;

	int petIndex = -1;
	if (!checkInt(TK, 1, &petIndex))
		return Parser::kArgError;

	if (petIndex < 0 || petIndex >= MAX_PET)
		return Parser::kArgError;


	if (!checkInt(TK, 2, &part) || part < 1)
	{
		QString partStr;
		checkString(TK, 2, &partStr);
		if (partStr.isEmpty())
			return Parser::kArgError;

		if (partStr.toLower() == "all" || partStr.toLower() == QString("全部"))
		{
			part = 100;
		}
		else
		{
			part = petEquipMap.value(partStr.toLower(), PET_EQUIPNONE);
			if (part == PET_EQUIPNONE)
				return Parser::kArgError;
		}
	}
	else
		--part;
	if (part < 100)
	{
		int spotIndex = injector.server->getItemEmptySpotIndex();
		if (spotIndex == -1)
			return Parser::kNoChange;

		injector.server->petitemswap(petIndex, part, spotIndex);
	}
	else
	{
		util::SafeVector<int> v;
		if (!injector.server->getItemEmptySpotIndexs(&v))
			return Parser::kNoChange;

		for (int i = 0; i < CHAR_EQUIPPLACENUM; ++i)
		{
			if (v.isEmpty())
				break;

			int itemIndex = v.takeFirst();

			injector.server->petitemswap(petIndex, i, itemIndex);
		}
	}

	return Parser::kNoChange;
}

int Interpreter::depositpet(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	int petIndex = 0;
	QString petName;
	if (!checkInt(TK, 1, &petIndex))
	{
		checkString(TK, 1, &petName);
		if (petName.isEmpty())
			return Parser::kArgError;
	}

	if (petIndex == 0 && petName.isEmpty())
		return Parser::kArgError;

	if (petIndex > 0)
		--petIndex;
	else
	{
		util::SafeVector<int> v;
		if (!injector.server->getPetIndexsByName(petName, &v))
			return Parser::kArgError;
		petIndex = v.first();
	}

	if (petIndex == -1)
		return Parser::kArgError;

	injector.server->IS_WAITFOR_DIALOG_FLAG = true;
	injector.server->depositPet(petIndex);
	waitfor(1000, [&injector]()->bool { return !injector.server->IS_WAITFOR_DIALOG_FLAG; });

	injector.server->IS_WAITFOR_DIALOG_FLAG = true;
	injector.server->press(BUTTON_YES);
	waitfor(1000, [&injector]()->bool { return !injector.server->IS_WAITFOR_DIALOG_FLAG; });

	injector.server->IS_WAITFOR_DIALOG_FLAG = true;
	injector.server->press(BUTTON_OK);
	waitfor(1000, [&injector]()->bool { return !injector.server->IS_WAITFOR_DIALOG_FLAG; });

	return Parser::kNoChange;
}

int Interpreter::deposititem(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	int min = 0, max = 14;
	if (!checkRange(TK, 1, &min, &max))
		return Parser::kArgError;

	min += CHAR_EQUIPPLACENUM;
	max += CHAR_EQUIPPLACENUM;


	QString itemName;
	checkString(TK, 2, &itemName);

	if (!itemName.isEmpty() && TK.value(2).type != TK_FUZZY)
	{
		QStringList itemNames = itemName.split(util::rexOR, Qt::SkipEmptyParts);
		if (itemNames.isEmpty())
			return Parser::kArgError;

		util::SafeVector<int> allv;
		for (const QString& name : itemNames)
		{
			util::SafeVector<int> v;
			if (!injector.server->getItemIndexsByName(name, "", &v))
				return Parser::kArgError;
			else
				allv.append(v);
		}

		std::sort(allv.begin(), allv.end());
		auto iter = std::unique(allv.begin(), allv.end());
		allv.erase(iter, allv.end());

		util::SafeVector<int> v;
		for (const int it : allv)
		{
			if (it < min || it > max)
				continue;

			injector.server->IS_WAITFOR_DIALOG_FLAG = true;
			injector.server->depositItem(it);
			waitfor(1000, [&injector]()->bool { return !injector.server->IS_WAITFOR_DIALOG_FLAG; });

			injector.server->IS_WAITFOR_DIALOG_FLAG = true;
			injector.server->press(1);
			waitfor(1000, [&injector]()->bool { return !injector.server->IS_WAITFOR_DIALOG_FLAG; });
		}

	}
	else
	{
		for (int i = CHAR_EQUIPPLACENUM; i < MAX_ITEM; ++i)
		{
			ITEM item = injector.server->pc.item[i];
			if (item.name.isEmpty() || item.useFlag == 0)
				continue;

			if (i < min || i > max)
				continue;

			injector.server->depositItem(i);
			injector.server->press(1);

		}
	}

	return Parser::kNoChange;
}

int Interpreter::withdrawpet(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	QString petName;
	checkString(TK, 1, &petName);
	if (petName.isEmpty())
		return Parser::kArgError;

	int level = 0;
	checkInt(TK, 2, &level);

	int maxHp = 0;
	checkInt(TK, 3, &maxHp);

	for (;;)
	{
		QPair<int, util::SafeVector<bankpet_t>> bankPetList = injector.server->currentBankPetList;
		int button = bankPetList.first;
		if (button == 0)
			break;

		util::SafeVector<bankpet_t> petList = bankPetList.second;
		int petIndex = 0;
		bool bret = false;
		for (const bankpet_t& it : petList)
		{
			if (!petName.startsWith(kFuzzyPrefix))
			{
				if (it.name == petName && it.maxHp == maxHp && it.level == level)
					bret = true;
				else if (it.name == petName && 0 == maxHp && it.level == level)
					bret = true;
				else if (it.name == petName && it.maxHp == maxHp && 0 == level)
					bret = true;
				else if (it.name == petName && 0 == maxHp && 0 == level)
					bret = true;
			}
			else
			{
				QString newName = petName.mid(1);
				if (it.name.contains(newName) && it.maxHp == maxHp && it.level == level)
					bret = true;
				else if (it.name.contains(newName) && 0 == maxHp && it.level == level)
					bret = true;
				else if (it.name.contains(newName) && it.maxHp == maxHp && 0 == level)
					bret = true;
				else if (it.name.contains(newName) && 0 == maxHp && 0 == level)
					bret = true;
			}

			if (bret)
				break;

			++petIndex;
		}

		if (bret)
		{
			injector.server->IS_WAITFOR_DIALOG_FLAG = true;
			injector.server->withdrawPet(petIndex);
			waitfor(1000, [&injector]()->bool { return !injector.server->IS_WAITFOR_DIALOG_FLAG; });

			injector.server->IS_WAITFOR_DIALOG_FLAG = true;
			injector.server->press(BUTTON_YES);
			waitfor(1000, [&injector]()->bool { return !injector.server->IS_WAITFOR_DIALOG_FLAG; });

			injector.server->IS_WAITFOR_DIALOG_FLAG = true;
			injector.server->press(BUTTON_OK);
			waitfor(1000, [&injector]()->bool { return !injector.server->IS_WAITFOR_DIALOG_FLAG; });
			break;
		}

		if (button & BUTTON_NEXT)
		{
			injector.server->IS_WAITFOR_BANK_FLAG = true;
			injector.server->press(BUTTON_NEXT);
			if (!waitfor(1000, [&injector]()->bool { return !injector.server->IS_WAITFOR_BANK_FLAG; }))
				break;
		}
		else
			break;
	}

	return Parser::kNoChange;
}

int Interpreter::withdrawitem(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	QString itemName;
	checkString(TK, 1, &itemName);
	if (itemName.isEmpty())
		return Parser::kArgError;

	QString memo;
	checkString(TK, 2, &memo);

	util::SafeVector<ITEM> bankItemList = injector.server->currentBankItemList;

	int itemIndex = 0;
	bool bret = false;
	for (const ITEM& it : bankItemList)
	{
		if (!itemName.startsWith(kFuzzyPrefix))
		{
			if (it.name == itemName && memo.isEmpty())
				bret = true;
			if (it.name == itemName && !memo.isEmpty() && it.memo.contains(memo))
				bret = true;
		}
		else
		{
			QString newName = itemName.mid(1);
			if (it.name.contains(newName) && memo.isEmpty())
				bret = true;
			else if (it.name.contains(newName) && it.memo.contains(memo))
				bret = true;
		}

		if (bret)
			break;

		++itemIndex;
	}

	if (bret)
	{
		injector.server->IS_WAITFOR_DIALOG_FLAG = true;
		injector.server->withdrawItem(itemIndex);
		waitfor(1000, [&injector]()->bool { return !injector.server->IS_WAITFOR_DIALOG_FLAG; });

		injector.server->IS_WAITFOR_DIALOG_FLAG = true;
		injector.server->press(1);
		waitfor(1000, [&injector]()->bool { return !injector.server->IS_WAITFOR_DIALOG_FLAG; });

	}

	return Parser::kNoChange;
}

int Interpreter::addpoint(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	QString pointName;
	checkString(TK, 1, &pointName);
	if (pointName.isEmpty())
		return Parser::kArgError;

	int max = 0;
	checkInt(TK, 2, &max);
	if (max <= 0)
		return Parser::kArgError;

	static const QHash<QString, int> hash = {
		{ u8"腕力", 0},
		{ u8"體力", 1},
		{ u8"耐力", 2},
		{ u8"速度", 3},

		{ u8"腕力", 0},
		{ u8"体力", 1},
		{ u8"耐力", 2},
		{ u8"速度", 3},

		{ u8"str", 0},
		{ u8"vit", 1},
		{ u8"tgh", 2},
		{ u8"dex", 3},
	};

	int point = hash.value(pointName.toLower(), -1);
	if (point == -1)
		return Parser::kArgError;

	injector.server->addPoint(point, max);

	return Parser::kNoChange;
}

int Interpreter::leftclick(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	QPoint p;
	checkInt(TK, 1, &p.rx());
	checkInt(TK, 2, &p.ry());

	injector.server->leftClick(p.x(), p.y());

	return Parser::kNoChange;
}

int Interpreter::rightclick(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	QPoint p;
	checkInt(TK, 1, &p.rx());
	checkInt(TK, 2, &p.ry());

	injector.server->rightClick(p.x(), p.y());

	return Parser::kNoChange;
}

int Interpreter::leftdoubleclick(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	QPoint p;
	checkInt(TK, 1, &p.rx());
	checkInt(TK, 2, &p.ry());

	injector.server->leftDoubleClick(p.x(), p.y());

	return Parser::kNoChange;
}

int Interpreter::mousedragto(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	QPoint pfrom;
	checkInt(TK, 1, &pfrom.rx());
	checkInt(TK, 2, &pfrom.ry());
	QPoint pto;
	checkInt(TK, 3, &pto.rx());
	checkInt(TK, 4, &pto.ry());

	injector.server->dragto(pfrom.x(), pfrom.y(), pto.x(), pto.y());

	return Parser::kNoChange;
}

int Interpreter::trade(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	checkBattleThenWait();

	QString name;
	checkString(TK, 1, &name);
	if (name.isEmpty())
		return Parser::kArgError;

	QString itemListStr;
	checkString(TK, 2, &itemListStr);

	QString petListStr;
	checkString(TK, 3, &petListStr);

	int gold = 0;
	if (!checkInt(TK, 4, &gold))
	{
		QString tmp;
		if (checkString(TK, 4, &tmp) && tmp == "all")
			gold = injector.server->pc.gold;
	}


	int timeout = DEFAULT_FUNCTION_TIMEOUT;
	checkInt(TK, 5, &timeout);

	mapunit_s unit;
	if (!injector.server->findUnit(name, util::OBJ_HUMAN, &unit))
		return Parser::kNoChange;

	QPoint dst;
	int dir = injector.server->mapAnalyzer->calcBestFollowPointByDstPoint(injector.server->nowFloor, injector.server->nowPoint, unit.p, &dst, true, unit.dir);
	if (dir == -1 || !findPath(dst, 1, 0, timeout))
		return Parser::kNoChange;

	injector.server->setPlayerFaceDirection(dir);

	if (!injector.server->tradeStart(name, timeout))
		return Parser::kNoChange;

	//bool ok = false;
	if (!itemListStr.isEmpty())
	{
		QStringList itemIndexList = itemListStr.split(util::rexOR, Qt::SkipEmptyParts);
		if (itemListStr.toLower() == "all")
		{
			for (int i = 1; i <= 15; ++i)
				itemIndexList.append(QString::number(i));
		}
		else if (itemListStr.count("-") == 1)
		{
			int min = 1;
			int max = 15;
			if (!checkRange(TK, 2, &min, &max))
				return Parser::kArgError;

			for (int i = min; i <= max; ++i)
				itemIndexList.append(QString::number(i));
		}

		util::SafeVector<int> itemIndexVec;
		for (const QString& itemIndex : itemIndexList)
		{
			bool bret = false;
			int index = itemIndex.toInt(&bret);
			--index;
			index += CHAR_EQUIPPLACENUM;
			if (bret && index >= CHAR_EQUIPPLACENUM && index < MAX_ITEM)
			{
				if (injector.server->pc.item[index].useFlag == 1)
					itemIndexVec.append(index);
			}
		}

		if (!itemIndexVec.isEmpty())
		{
			std::sort(itemIndexVec.begin(), itemIndexVec.end());
			auto it = std::unique(itemIndexVec.begin(), itemIndexVec.end());
			itemIndexVec.erase(it, itemIndexVec.end());
		}

		if (!itemIndexVec.isEmpty())
		{
			injector.server->tradeAppendItems(name, itemIndexVec);
			//ok = true;
		}
		else
			return Parser::kArgError;
	}

	if (!petListStr.isEmpty())
	{
		QStringList petIndexList = petListStr.split(util::rexOR, Qt::SkipEmptyParts);
		if (itemListStr.toLower() == "all")
		{
			for (int i = 1; i <= MAX_PET; ++i)
				petIndexList.append(QString::number(i));
		}
		else if (itemListStr.count("-") == 1)
		{
			int min = 1;
			int max = MAX_PET;
			if (!checkRange(TK, 2, &min, &max))
				return Parser::kArgError;

			for (int i = min; i <= max; ++i)
				petIndexList.append(QString::number(i));
		}

		util::SafeVector<int> petIndexVec;
		for (const QString& petIndex : petIndexList)
		{
			bool bret = false;
			int index = petIndex.toInt(&bret);
			--index;

			if (bret && index >= 0 && index < MAX_PET)
			{
				if (injector.server->pet[index].useFlag == 1)
					petIndexVec.append(index);
			}
		}

		std::sort(petIndexVec.begin(), petIndexVec.end());
		auto it = std::unique(petIndexVec.begin(), petIndexVec.end());
		petIndexVec.erase(it, petIndexVec.end());

		if (!petIndexVec.isEmpty())
		{
			injector.server->tradeAppendPets(name, petIndexVec);
			//ok = true;
		}
		else
			return Parser::kArgError;
	}

	if (gold > 0 && gold <= injector.server->pc.gold)
	{
		injector.server->tradeAppendGold(name, gold);
		//ok = true;
	}

	injector.server->tradeComfirm(name);

	waitfor(timeout, [&injector]()
		{
			return injector.server->pc.trade_confirm >= 3;
		});

	injector.server->tradeComplete(name);

	waitfor(timeout, [&injector]()
		{
			return !injector.server->IS_TRADING;
		});

	return Parser::kNoChange;
}

int Interpreter::mail(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	int addrIndex = 0;
	checkInt(TK, 1, &addrIndex);
	if (addrIndex <= 0 || addrIndex >= MAX_ADR_BOOK)
		return Parser::kArgError;
	--addrIndex;

	QString text;
	checkString(TK, 2, &text);
	if (text.isEmpty())
		return Parser::kArgError;

	injector.server->mail(addrIndex, text);

	return Parser::kNoChange;
}

//battle
int Interpreter::bh(int, const TokenMap& TK)//atk
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	if (!injector.server->getBattleFlag())
		return Parser::kNoChange;

	int index = 0;
	checkInt(TK, 1, &index);
	if (index <= 0)
		return Parser::kArgError;
	--index;

	injector.server->sendBattlePlayerAttackAct(index);

	return Parser::kNoChange;
}
int Interpreter::bj(int, const TokenMap& TK)//magic
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	if (!injector.server->getBattleFlag())
		return Parser::kNoChange;

	int magicIndex = 0;
	checkInt(TK, 1, &magicIndex);
	if (magicIndex <= 0)
		return Parser::kArgError;
	--magicIndex;

	int target = 0;
	checkInt(TK, 2, &target);
	if (target <= 0)
		return Parser::kArgError;
	--target;

	injector.server->sendBattlePlayerMagicAct(magicIndex, target);

	return Parser::kNoChange;
}
int Interpreter::bp(int, const TokenMap& TK)//skill
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	if (!injector.server->getBattleFlag())
		return Parser::kNoChange;

	int skillIndex = 0;
	checkInt(TK, 1, &skillIndex);
	if (skillIndex <= 0)
		return Parser::kArgError;

	int target = 0;
	checkInt(TK, 2, &target);
	if (target <= 0)
		return Parser::kArgError;

	injector.server->sendBattlePlayerJobSkillAct(skillIndex, target);

	return Parser::kNoChange;
}
int Interpreter::bs(int, const TokenMap& TK)//switch
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	if (!injector.server->getBattleFlag())
		return Parser::kNoChange;

	int index = 0;
	checkInt(TK, 1, &index);
	if (index <= 0)
		return Parser::kArgError;
	injector.server->sendBattlePlayerSwitchPetAct(index);

	return Parser::kNoChange;
}
int Interpreter::be(int, const TokenMap& TK)//escape
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	if (!injector.server->getBattleFlag())
		return Parser::kNoChange;

	injector.server->sendBattlePlayerEscapeAct();

	return Parser::kNoChange;
}
int Interpreter::bd(int, const TokenMap& TK)//defense
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	if (!injector.server->getBattleFlag())
		return Parser::kNoChange;

	injector.server->sendBattlePlayerDefenseAct();

	return Parser::kNoChange;
}
int Interpreter::bi(int, const TokenMap& TK)//item
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	if (!injector.server->getBattleFlag())
		return Parser::kNoChange;

	int index = 0;
	checkInt(TK, 1, &index);
	if (index <= 0)
		return Parser::kArgError;
	--index;

	int target = 0;
	checkInt(TK, 2, &target);
	if (target <= 0)
		return Parser::kArgError;
	--target;

	injector.server->sendBattlePlayerItemAct(index, target);

	return Parser::kNoChange;
}
int Interpreter::bt(int, const TokenMap& TK)//catch
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	if (!injector.server->getBattleFlag())
		return Parser::kNoChange;

	int index = 0;
	checkInt(TK, 1, &index);
	if (index <= 0)
		return Parser::kArgError;
	--index;

	injector.server->sendBattlePlayerCatchPetAct(index);

	return Parser::kNoChange;
}
int Interpreter::bn(int, const TokenMap& TK)//nothing
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	if (!injector.server->getBattleFlag())
		return Parser::kNoChange;

	injector.server->sendBattlePlayerDoNothing();

	return Parser::kNoChange;
}
int Interpreter::bw(int, const TokenMap& TK)//petskill
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	if (!injector.server->getBattleFlag())
		return Parser::kNoChange;

	int skillIndex = 0;
	checkInt(TK, 1, &skillIndex);
	if (skillIndex <= 0)
		return Parser::kArgError;
	--skillIndex;

	int target = 0;
	checkInt(TK, 2, &target);
	if (target <= 0)
		return Parser::kArgError;
	--target;

	injector.server->sendBattlePetSkillAct(skillIndex, target);

	return Parser::kNoChange;
}
int Interpreter::bwf(int, const TokenMap& TK)//pet nothing
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	if (!injector.server->getBattleFlag())
		return Parser::kNoChange;
	injector.server->sendBattlePetDoNothing();
	return Parser::kNoChange;
}

int Interpreter::bwait(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	if (!injector.server->getBattleFlag())
		return Parser::kNoChange;

	int timeout = DEFAULT_FUNCTION_TIMEOUT;
	checkInt(TK, 1, &timeout);
	injector.sendMessage(Injector::kEnableBattleDialog, false, NULL);
	bool bret = waitfor(timeout, [&injector]()
		{
			if (!injector.server->getBattleFlag())
				return true;
			int G = injector.server->getGameStatus();
			int W = injector.server->getWorldStatus();

			return W == 10 && G == 4;
		});
	if (injector.server->getBattleFlag())
		injector.sendMessage(Injector::kEnableBattleDialog, true, NULL);
	else
		bret = false;

	return checkJump(TK, 2, bret, FailedJump);
}

int Interpreter::bend(int, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance();

	if (injector.server.isNull())
		return Parser::kError;

	if (!injector.server->getBattleFlag())
		return Parser::kNoChange;

	int G = injector.server->getGameStatus();
	if (G >= 4)
	{
		++G;
		injector.server->setGameStatus(G);
	}
	return Parser::kNoChange;
}