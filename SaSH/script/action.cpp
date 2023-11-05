﻿/*
				GNU GENERAL PUBLIC LICENSE
				   Version 2, June 1991
COPYRIGHT (C) Bestkakkoii 2023 All Rights Reserved.
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/

#include "stdafx.h"
#include "interpreter.h"
#include "util.h"
#include "injector.h"
#include "map/mapdevice.h"

//action
long long Interpreter::useitem(long long currentIndex, long long currentLine, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance(currentIndex);

	if (injector.worker.isNull())
		return Parser::kServerNotReady;

	checkOnlineThenWait();
	checkBattleThenWait();

	QHash<QString, long long> hash = {
		{ "自己", 0},
		{ "戰寵", injector.worker->getCharacter().battlePetNo},
		{ "騎寵", injector.worker->getCharacter().ridePetNo},
		{ "隊長", 6},

		{ "自己", 0},
		{ "战宠", injector.worker->getCharacter().battlePetNo},
		{ "骑宠", injector.worker->getCharacter().ridePetNo},
		{ "队长", 6},

		{ "self", 0},
		{ "battlepet", injector.worker->getCharacter().battlePetNo},
		{ "ride", injector.worker->getCharacter().ridePetNo},
		{ "leader", 6},
	};

	for (long long i = 0; i < sa::MAX_PET; ++i)
	{
		hash.insert("寵物" + util::toQString(i + 1), i + 1);
		hash.insert("宠物" + util::toQString(i + 1), i + 1);
		hash.insert("pet" + util::toQString(i + 1), i + 1);
	}

	for (long long i = 1; i < sa::MAX_TEAM; ++i)
	{
		hash.insert("隊員" + util::toQString(i), i + 1 + sa::MAX_PET);
		hash.insert("队员" + util::toQString(i), i + 1 + sa::MAX_PET);
		hash.insert("teammate" + util::toQString(i), i + 1 + sa::MAX_PET);
	}

	injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.reset();

	QString itemName;
	QString itemMemo;
	QStringList itemNames;
	QStringList itemMemos;

	long long target = 0;
	long long totalUse = 1;

	long long min = 0, max = static_cast<long long>(sa::MAX_ITEM - sa::CHAR_EQUIPSLOT_COUNT - 1);
	if (!checkRange(TK, 1, &min, &max))
	{
		checkString(TK, 1, &itemName);
		checkString(TK, 2, &itemMemo);
		if (itemName.isEmpty() && itemMemo.isEmpty())
			return Parser::kArgError + 1ll;

		itemNames = itemName.split(util::rexOR);
		itemMemos = itemMemo.split(util::rexOR);

		target = -2;
		checkInteger(TK, 3, &target);
		if (target == -2)
		{
			QString targetTypeName;
			checkString(TK, 3, &targetTypeName);
			if (targetTypeName.isEmpty())
				target = 0;
			else
			{


				if (!hash.contains(targetTypeName))
					return Parser::kArgError + 3ll;

				target = hash.value(targetTypeName, -1);
				if (target < 0)
					return Parser::kArgError + 3ll;
			}
		}

		checkInteger(TK, 4, &totalUse);
		if (totalUse <= 0)
			return Parser::kArgError + 4ll;
	}
	else
	{
		min += sa::CHAR_EQUIPSLOT_COUNT;
		max += sa::CHAR_EQUIPSLOT_COUNT;

		target = -2;
		checkInteger(TK, 2, &target);
		if (target == -2)
		{
			QString targetTypeName;
			checkString(TK, 2, &targetTypeName);
			if (targetTypeName.isEmpty())
				target = 0;
			else
			{
				if (!hash.contains(targetTypeName))
					return Parser::kArgError + 2ll;

				target = hash.value(targetTypeName, -1);
				if (target < 0)
					return Parser::kArgError + 2ll;
			}
		}

		checkString(TK, 3, &itemName);
		checkString(TK, 4, &itemMemo);
		if (itemName.isEmpty() && itemMemo.isEmpty())
			return Parser::kArgError + 3ll;

		itemNames = itemName.split(util::rexOR);
		itemMemos = itemMemo.split(util::rexOR);

	}

	if (target > 100 || target == -1)
	{
		if (itemNames.size() >= itemMemos.size())
		{
			for (const QString& name : itemNames)
			{
				QString memo;
				if (itemMemos.size() > 0)
					memo = itemMemos.takeFirst();

				QVector<long long> v;
				if (!injector.worker->getItemIndexsByName(name, memo, &v, sa::CHAR_EQUIPSLOT_COUNT))
					continue;

				totalUse = target - 100;

				bool ok = false;
				QHash<long long, sa::item_t> items = injector.worker->getItems();
				for (const long long& it : v)
				{
					sa::item_t item = items.value(it);
					long long size = item.stack;
					for (long long i = 0; i < size; ++i)
					{
						injector.worker->useItem(it, 0);
						if (target != -1)
						{
							--totalUse;
							if (totalUse <= 0)
							{
								ok = true;
								break;
							}
						}
					}

					if (ok)
						break;
				}
			}
		}
		else
		{
			for (const QString& memo : itemMemos)
			{
				QString name;
				if (itemNames.size() > 0)
					name = itemNames.takeFirst();

				QVector<long long> v;
				if (!injector.worker->getItemIndexsByName(name, memo, &v, sa::CHAR_EQUIPSLOT_COUNT))
					continue;

				totalUse = target - 100;

				bool ok = false;
				QHash<long long, sa::item_t> items = injector.worker->getItems();
				for (const long long& it : v)
				{
					sa::item_t item = items.value(it);
					long long size = item.stack;
					for (long long i = 0; i < size; ++i)
					{
						injector.worker->useItem(it, 0);
						if (target != -1)
						{
							--totalUse;
							if (totalUse <= 0)
							{
								ok = true;
								break;
							}
						}
					}

					if (ok)
						break;
				}
			}
		}


		return Parser::kNoChange;
	}

	if (itemNames.size() >= itemMemos.size())
	{
		for (const QString& name : itemNames)
		{
			QString memo;
			if (itemMemos.size() > 0)
				memo = itemMemos.takeFirst();

			QVector<long long> v;
			if (!injector.worker->getItemIndexsByName(name, memo, &v, sa::CHAR_EQUIPSLOT_COUNT))
				continue;

			if (totalUse == 1)
			{
				long long itemIndex = v.front();
				injector.worker->useItem(itemIndex, target);
				break;
			}

			long long n = totalUse;
			for (;;)
			{
				if (totalUse != -1 && n <= 0)
					break;

				if (v.size() == 0)
					break;

				long long itemIndex = v.takeFirst();
				injector.worker->useItem(itemIndex, target);
				--n;
			}
		}
	}
	else
	{
		for (const QString& memo : itemMemos)
		{
			QString name;
			if (itemNames.size() > 0)
				name = itemNames.takeFirst();

			QVector<long long> v;
			if (!injector.worker->getItemIndexsByName(itemName, memo, &v, sa::CHAR_EQUIPSLOT_COUNT))
				continue;

			if (totalUse == 1)
			{
				long long itemIndex = v.front();
				injector.worker->useItem(itemIndex, target);
				injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.inc();
				break;
			}

			long long n = totalUse;
			for (;;)
			{
				if (totalUse != -1 && n <= 0)
					break;

				if (v.size() == 0)
					break;

				long long itemIndex = v.takeFirst();
				injector.worker->useItem(itemIndex, target);
				injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.inc();
				--n;
			}
		}
	}

	waitfor(500, [&injector]()->bool { return injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.get() <= 0; });
	injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.reset();
	return Parser::kNoChange;
}

long long Interpreter::dropitem(long long currentIndex, long long currentLine, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance(currentIndex);

	if (injector.worker.isNull())
		return Parser::kServerNotReady;

	checkOnlineThenWait();
	checkBattleThenWait();

	QString tempName;
	QString memo;
	long long itemIndex = -1;
	checkString(TK, 1, &tempName);
	checkInteger(TK, 1, &itemIndex);

	checkString(TK, 2, &memo);
	if (tempName.isEmpty() && memo.isEmpty() && itemIndex == -1)
	{
		injector.worker->dropItem(-1);
		return Parser::kNoChange;
	}

	if (tempName == kFuzzyPrefix)
	{
		for (long long i = sa::CHAR_EQUIPSLOT_COUNT; i < sa::MAX_ITEM; ++i)
			injector.worker->dropItem(i);
	}

	if (tempName.isEmpty() && memo.isEmpty()
		&& ((itemIndex >= 1 && itemIndex <= (sa::MAX_ITEM - sa::CHAR_EQUIPSLOT_COUNT))
			|| (itemIndex >= 101 && itemIndex <= static_cast<long long>(sa::CHAR_EQUIPSLOT_COUNT + 100))))
	{
		if (itemIndex < 100)
		{
			--itemIndex;
			itemIndex += sa::CHAR_EQUIPSLOT_COUNT;
		}
		else
			itemIndex -= 100;

		injector.worker->dropItem(itemIndex);
		return Parser::kNoChange;
	}

	injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.reset();

	//指定丟棄白名單，位於白名單的物品不丟棄
	if (tempName == QString("非"))
	{
		long long min = 0, max = static_cast<long long>(sa::MAX_ITEM - sa::CHAR_EQUIPSLOT_COUNT - 1);
		if (!checkRange(TK, 2, &min, &max))
			return Parser::kArgError + 2ll;

		min += sa::CHAR_EQUIPSLOT_COUNT;
		max += sa::CHAR_EQUIPSLOT_COUNT;

		QString itemNames;
		checkString(TK, 3, &itemNames);
		if (itemNames.isEmpty())
			return Parser::kArgError + 3ll;

		QVector<long long> indexs;
		if (injector.worker->getItemIndexsByName(itemNames, "", &indexs, min, max))
		{
			//排除掉所有包含在indexs的
			for (long long i = min; i <= max; ++i)
			{
				if (!indexs.contains(i))
				{
					injector.worker->dropItem(i);
				}
			}
		}
	}
	else
	{
		long long onlyOne = 0;
		bool bOnlyOne = false;
		checkInteger(TK, 1, &onlyOne);
		if (onlyOne == 1)
			bOnlyOne = true;

		QString itemNames = tempName;

		QStringList itemNameList = itemNames.split(util::rexOR, Qt::SkipEmptyParts);
		if (itemNameList.isEmpty())
			return Parser::kArgError + 1ll;

		//long long size = itemNameList.size();
		for (const QString& name : itemNameList)
		{
			QVector<long long> indexs;
			bool ok = false;
			if (injector.worker->getItemIndexsByName(name, memo, &indexs))
			{
				for (const long long it : indexs)
				{
					injector.worker->dropItem(it);
					if (bOnlyOne)
					{
						ok = true;
						break;
					}
				}
			}

			if (ok)
				break;
		}

	}

	waitfor(500, [&injector]()->bool { return injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.get() <= 0; });
	injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.reset();
	return Parser::kNoChange;
}

safe::Hash<long long, QHash<long long, sa::item_t>> recordedEquip_;
long long Interpreter::recordequip(long long currentIndex, long long currentLine, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance(currentIndex);

	if (injector.worker.isNull())
		return Parser::kServerNotReady;

	checkOnlineThenWait();

	injector.worker->updateItemByMemory();
	QHash<long long, sa::item_t> items = injector.worker->getItems();
	QHash<long long, sa::item_t> recordedItems = recordedEquip_.value(currentIndex);
	for (long long i = 0; i < sa::CHAR_EQUIPSLOT_COUNT; ++i)
	{
		injector.worker->announce(QObject::tr("record equip:[%1]%2").arg(i + 1).arg(items.value(i).name));
		recordedItems.insert(i, items.value(i));
	}
	recordedEquip_.insert(currentIndex, recordedItems);
	return Parser::kNoChange;
}

long long Interpreter::wearequip(long long currentIndex, long long currentLine, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance(currentIndex);

	if (injector.worker.isNull())
		return Parser::kServerNotReady;

	checkOnlineThenWait();
	checkBattleThenWait();

	if (!injector.worker->getOnlineFlag())
		return Parser::kNoChange;

	injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.reset();

	QHash<long long, sa::item_t> items = injector.worker->getItems();
	for (long long i = 0; i < sa::CHAR_EQUIPSLOT_COUNT; ++i)
	{
		sa::item_t item = items.value(i);
		sa::item_t recordedItem = recordedEquip_.value(currentIndex).value(i);
		if (!recordedItem.valid || recordedItem.name.isEmpty())
			continue;

		if (item.name == recordedItem.name && item.memo == recordedItem.memo)
			continue;

		long long itemIndex = injector.worker->getItemIndexByName(recordedItem.name, true, "", sa::CHAR_EQUIPSLOT_COUNT);
		if (itemIndex == -1)
			continue;

		injector.worker->useItem(itemIndex, 0);
	}

	waitfor(500, [&injector]()->bool { return injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.get() <= 0; });
	injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.reset();
	return Parser::kNoChange;
}

long long Interpreter::unwearequip(long long currentIndex, long long currentLine, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance(currentIndex);

	if (injector.worker.isNull())
		return Parser::kServerNotReady;

	checkOnlineThenWait();
	checkBattleThenWait();

	long long part = sa::CHAR_EQUIPNONE;

	if (!checkInteger(TK, 1, &part) || part < 1)
	{
		QString partStr;
		checkString(TK, 1, &partStr);
		if (partStr.isEmpty())
			return Parser::kArgError + 1ll;

		if (partStr.toLower() == "all" || partStr.toLower() == QString("全部"))
		{
			part = 100;
		}
		else
		{
			part = sa::equipMap.value(partStr.toLower(), sa::CHAR_EQUIPNONE);
			if (part == sa::CHAR_EQUIPNONE)
				return Parser::kArgError + 1ll;
		}
	}
	else
		--part;

	injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.reset();

	if (part < 100)
	{
		long long spotIndex = injector.worker->getItemEmptySpotIndex();
		if (spotIndex == -1)
			return Parser::kNoChange;

		injector.worker->swapItem(part, spotIndex);
	}
	else
	{
		QVector<long long> v;
		if (!injector.worker->getItemEmptySpotIndexs(&v))
			return Parser::kNoChange;

		for (long long i = 0; i < sa::CHAR_EQUIPSLOT_COUNT; ++i)
		{
			if (v.isEmpty())
				break;

			long long itemIndex = v.takeFirst();

			injector.worker->swapItem(i, itemIndex);
		}
	}

	waitfor(500, [&injector]()->bool { return injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.get() <= 0; });
	injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.reset();
	return Parser::kNoChange;
}

long long Interpreter::petequip(long long currentIndex, long long currentLine, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance(currentIndex);

	if (injector.worker.isNull())
		return Parser::kServerNotReady;

	checkOnlineThenWait();
	checkBattleThenWait();

	long long petIndex = -1;
	if (!checkInteger(TK, 1, &petIndex))
		return Parser::kArgError + 1ll;

	--petIndex;

	if (petIndex < 0 || petIndex >= sa::MAX_PET)
		return Parser::kArgError + 1ll;

	QString itemName;
	checkString(TK, 2, &itemName);

	QStringList itemNames = itemName.split(util::rexOR);

	QString itemMemo;
	checkString(TK, 3, &itemMemo);

	if (itemName.isEmpty() && itemMemo.isEmpty())
		return Parser::kArgError + 1ll;

	QStringList itemMemos = itemMemo.split(util::rexOR);

	QStringList itemMemos2 = itemMemos;
	for (const QString& name : itemNames)
	{
		QString memo;
		if (!itemMemos2.isEmpty())
			memo = itemMemos2.takeFirst();

		long long itemIndex = injector.worker->getItemIndexByName(name, true, memo, sa::CHAR_EQUIPSLOT_COUNT);
		if (itemIndex != -1)
			injector.worker->petitemswap(petIndex, itemIndex, -1);
	}

	QStringList itemNames2 = itemNames;
	for (const QString& memo : itemMemos)
	{
		QString name;
		if (!itemNames2.isEmpty())
			name = itemNames2.takeFirst();

		long long itemIndex = injector.worker->getItemIndexByName(name, true, memo, sa::CHAR_EQUIPSLOT_COUNT);
		if (itemIndex != -1)
			injector.worker->petitemswap(petIndex, itemIndex, -1);
	}

	return Parser::kNoChange;
}

long long Interpreter::petunequip(long long currentIndex, long long currentLine, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance(currentIndex);

	if (injector.worker.isNull())
		return Parser::kServerNotReady;

	checkOnlineThenWait();
	checkBattleThenWait();

	long long part = sa::CHAR_EQUIPNONE;

	long long petIndex = -1;
	if (!checkInteger(TK, 1, &petIndex))
		return Parser::kArgError + 1ll;

	--petIndex;

	if (petIndex < 0 || petIndex >= sa::MAX_PET)
		return Parser::kArgError + 1ll;

	QVector<long long> partIndexs;
	if (!checkInteger(TK, 2, &part) || part < 1)
	{
		QString prePartStr;
		checkString(TK, 2, &prePartStr);
		if (prePartStr.isEmpty())
			return Parser::kArgError + 2ll;

		QStringList partStrs = prePartStr.split(util::rexOR, Qt::SkipEmptyParts);

		if (prePartStr.toLower() == "all" || prePartStr.toLower() == QString("全部"))
		{
			part = 100;
		}
		else
		{
			for (const QString& partStr : partStrs)
			{
				part = sa::petEquipMap.value(partStr.toLower(), sa::PET_EQUIPNONE);
				if (part != sa::PET_EQUIPNONE)
					partIndexs.append(part);
			}
		}
	}
	else
	{
		--part;
		partIndexs.append(part);
	}

	injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.reset();

	if (!partIndexs.isEmpty())
	{
		QVector<long long> spots;
		if (!injector.worker->getItemEmptySpotIndexs(&spots))
			return Parser::kNoChange;

		for (const long long& index : partIndexs)
		{
			if (spots.isEmpty())
				break;

			injector.worker->petitemswap(petIndex, index, spots.takeFirst());
		}
	}
	else if (part == 100)
	{
		QVector<long long> spots;
		if (!injector.worker->getItemEmptySpotIndexs(&spots))
			return Parser::kNoChange;

		for (long long index = 0; index < sa::MAX_PET_ITEM; ++index)
		{
			if (spots.isEmpty())
				break;

			injector.worker->petitemswap(petIndex, index, spots.takeFirst());
		}
	}

	waitfor(500, [&injector]()->bool { return injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.get() <= 0; });
	injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.reset();
	return Parser::kNoChange;
}

long long Interpreter::depositpet(long long currentIndex, long long currentLine, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance(currentIndex);

	if (injector.worker.isNull())
		return Parser::kServerNotReady;

	checkOnlineThenWait();
	checkBattleThenWait();

	long long petIndex = 0;
	QString petName;
	if (!checkInteger(TK, 1, &petIndex))
	{
		checkString(TK, 1, &petName);
		if (petName.isEmpty())
			return Parser::kArgError + 1ll;
	}

	if (petIndex == 0 && petName.isEmpty())
		return Parser::kArgError + 1ll;

	if (petIndex > 0)
		--petIndex;
	else
	{
		QVector<long long> v;
		if (!injector.worker->getPetIndexsByName(petName, &v))
			return Parser::kArgError + 1ll;
		petIndex = v.first();
	}

	if (petIndex == -1)
		return Parser::kArgError + 1ll;

	injector.worker->IS_WAITFOR_DIALOG_FLAG.on();
	injector.worker->depositPet(petIndex);
	waitfor(1000, [&injector]()->bool { return !injector.worker->IS_WAITFOR_DIALOG_FLAG.get(); });

	injector.worker->IS_WAITFOR_DIALOG_FLAG.on();
	injector.worker->press(sa::kButtonYes);
	waitfor(1000, [&injector]()->bool { return !injector.worker->IS_WAITFOR_DIALOG_FLAG.get(); });

	injector.worker->IS_WAITFOR_DIALOG_FLAG.on();
	injector.worker->press(sa::kButtonOk);
	waitfor(1000, [&injector]()->bool { return !injector.worker->IS_WAITFOR_DIALOG_FLAG.get(); });

	injector.worker->IS_WAITFOR_DIALOG_FLAG.off();
	return Parser::kNoChange;
}

long long Interpreter::deposititem(long long currentIndex, long long currentLine, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance(currentIndex);

	if (injector.worker.isNull())
		return Parser::kServerNotReady;

	checkOnlineThenWait();
	checkBattleThenWait();

	long long min = 0, max = static_cast<long long>(sa::MAX_ITEM - sa::CHAR_EQUIPSLOT_COUNT - 1);
	checkRange(TK, 1, &min, &max);

	min += sa::CHAR_EQUIPSLOT_COUNT;
	max += sa::CHAR_EQUIPSLOT_COUNT;

	QString itemName;
	checkString(TK, 2, &itemName);

	injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.reset();

	if (!itemName.isEmpty())
	{
		QStringList itemNames = itemName.split(util::rexOR, Qt::SkipEmptyParts);
		if (itemNames.isEmpty())
			return Parser::kArgError + 2ll;

		QVector<long long> allv;
		for (const QString& name : itemNames)
		{
			QVector<long long> v;
			if (injector.worker->getItemIndexsByName(name, "", &v, sa::CHAR_EQUIPSLOT_COUNT))
				allv.append(v);
		}

		std::sort(allv.begin(), allv.end());
		auto iter = std::unique(allv.begin(), allv.end());
		allv.erase(iter, allv.end());

		QVector<long long> v;
		for (const long long i : allv)
		{
			if (i < min || i > max)
				continue;

			injector.worker->depositItem(i);
		}
	}
	else
	{
		injector.worker->updateItemByMemory();
		QHash<long long, sa::item_t> items = injector.worker->getItems();
		for (long long i = sa::CHAR_EQUIPSLOT_COUNT; i < sa::MAX_ITEM; ++i)
		{
			sa::item_t item = items.value(i);
			if (item.name.isEmpty() || !item.valid)
				continue;

			if (i < min || i > max)
				continue;

			injector.worker->depositItem(i);
			injector.worker->press(1);
		}
	}

	waitfor(500, [&injector]()->bool { return injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.get() <= 0; });
	injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.reset();
	return Parser::kNoChange;
}

long long Interpreter::withdrawpet(long long currentIndex, long long currentLine, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance(currentIndex);

	if (injector.worker.isNull())
		return Parser::kServerNotReady;

	checkOnlineThenWait();
	checkBattleThenWait();

	QString petName;
	checkString(TK, 1, &petName);
	if (petName.isEmpty())
		return Parser::kArgError + 1ll;

	long long level = 0;
	checkInteger(TK, 2, &level);

	long long maxHp = 0;
	checkInteger(TK, 3, &maxHp);

	for (;;)
	{
		QPair<long long, QVector<sa::bank_pet_t>> bankPetList = injector.worker->currentBankPetList;
		long long button = bankPetList.first;
		if (button == 0)
			break;

		QVector<sa::bank_pet_t> petList = bankPetList.second;
		long long petIndex = 0;
		bool bret = false;
		for (const sa::bank_pet_t& it : petList)
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
			injector.worker->IS_WAITFOR_DIALOG_FLAG.on();
			injector.worker->withdrawPet(petIndex);
			waitfor(1000, [&injector]()->bool { return !injector.worker->IS_WAITFOR_DIALOG_FLAG.get(); });

			injector.worker->IS_WAITFOR_DIALOG_FLAG.on();
			injector.worker->press(sa::kButtonYes);
			waitfor(1000, [&injector]()->bool { return !injector.worker->IS_WAITFOR_DIALOG_FLAG.get(); });

			injector.worker->IS_WAITFOR_DIALOG_FLAG.on();
			injector.worker->press(sa::kButtonOk);
			waitfor(1000, [&injector]()->bool { return !injector.worker->IS_WAITFOR_DIALOG_FLAG.get(); });

			injector.worker->IS_WAITFOR_DIALOG_FLAG.off();
			break;
		}

		if (util::checkAND(button, sa::kButtonNext))
		{
			injector.worker->IS_WAITFOR_BANK_FLAG.on();
			injector.worker->press(sa::kButtonNext);
			if (!waitfor(1000, [&injector]()->bool { return !injector.worker->IS_WAITFOR_BANK_FLAG.get(); }))
			{
				injector.worker->IS_WAITFOR_BANK_FLAG.off();
				break;
			}
			injector.worker->IS_WAITFOR_BANK_FLAG.off();
		}
		else
			break;
	}


	return Parser::kNoChange;
}

long long Interpreter::withdrawitem(long long currentIndex, long long currentLine, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance(currentIndex);

	if (injector.worker.isNull())
		return Parser::kServerNotReady;

	checkOnlineThenWait();
	checkBattleThenWait();

	QString itemNames;
	checkString(TK, 1, &itemNames);
	QStringList itemList = itemNames.split(util::rexOR, Qt::SkipEmptyParts);
	long long itemListSize = itemList.size();

	QString memos;
	checkString(TK, 2, &memos);
	QStringList memoList = memos.split(util::rexOR, Qt::SkipEmptyParts);
	long long memoListSize = memoList.size();

	bool isAll = false;
	long long nisall = 0;
	checkInteger(TK, 3, &nisall);
	if (nisall > 1)
		isAll = true;

	if (itemListSize == 0 && memoListSize == 0)
		return Parser::kArgError + 1ll;

	long long max = 0;
	// max 以大於0且最小的為主 否則以不為0的為主
	if (itemListSize > 0)
		max = itemListSize;
	if (memoListSize > 0 && (max == 0 || memoListSize < max))
		max = memoListSize;

	QVector<sa::item_t> bankItemList = injector.worker->currentBankItemList.toVector();

	injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.reset();

	for (long long i = 0; i < max; ++i)
	{
		QString name = "";
		QString memo = "";
		if (!itemList.isEmpty())
			name = itemList.value(i);
		if (!memoList.isEmpty())
			memo = memoList.value(i);

		long long itemIndex = 0;
		bool bret = false;
		for (const sa::item_t& it : bankItemList)
		{
			if (!name.isEmpty())
			{
				if (name.startsWith(kFuzzyPrefix))
				{
					QString newName = name.mid(1);
					if (it.name.contains(newName) && memo.isEmpty())
						bret = true;
					else if (it.name.contains(newName) && it.memo.contains(memo))
						bret = true;
				}
				else
				{
					if (it.name == name && memo.isEmpty())
						bret = true;
					if (it.name == name && !memo.isEmpty() && it.memo.contains(memo))
						bret = true;

				}
			}
			else if (!memo.isEmpty() && it.memo.contains(memo))
			{
				bret = true;
			}

			if (bret)
			{
				bankItemList.remove(itemIndex);
				if (!isAll)
					break;
			}

			++itemIndex;
		}

		if (bret)
		{
			//injector.worker->IS_WAITFOR_DIALOG_FLAG.on();
			injector.worker->withdrawItem(itemIndex);
			//waitfor(1000, [&injector]()->bool { return !injector.worker->IS_WAITFOR_DIALOG_FLAG.get() });
			//injector.worker->IS_WAITFOR_DIALOG_FLAG.on();
			//injector.worker->press(1);
			//waitfor(1000, [&injector]()->bool { return !injector.worker->IS_WAITFOR_DIALOG_FLAG.get() });
		}
	}

	waitfor(500, [&injector]()->bool { return injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.get() <= 0; });
	injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.reset();
	return Parser::kNoChange;
}

long long Interpreter::trade(long long currentIndex, long long currentLine, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance(currentIndex);

	if (injector.worker.isNull())
		return Parser::kServerNotReady;

	checkOnlineThenWait();
	checkBattleThenWait();

	QString name;
	checkString(TK, 1, &name);
	if (name.isEmpty())
		return Parser::kArgError + 1ll;

	QString itemListStr;
	checkString(TK, 2, &itemListStr);

	QString petListStr;
	checkString(TK, 3, &petListStr);

	long long gold = 0;
	if (!checkInteger(TK, 4, &gold))
	{
		QString tmp;
		if (checkString(TK, 4, &tmp) && tmp == "all")
			gold = injector.worker->getCharacter().gold;
	}


	long long timeout = DEFAULT_FUNCTION_TIMEOUT;
	checkInteger(TK, 5, &timeout);

	sa::map_unit_t unit;
	if (!injector.worker->findUnit(name, sa::kObjectHuman, &unit))
		return Parser::kNoChange;

	QPoint dst;
	AStarDevice astar;
	long long dir = injector.worker->mapDevice.calcBestFollowPointByDstPoint(
		currentIndex, &astar, injector.worker->getFloor(), injector.worker->getPoint(), unit.p, &dst, true, unit.dir);
	if (dir == -1)
		return Parser::kNoChange;

	injector.worker->setCharFaceDirection(dir);

	if (!injector.worker->tradeStart(name, timeout))
		return Parser::kNoChange;

	//bool ok = false;
	if (!itemListStr.isEmpty())
	{
		QStringList itemIndexList = itemListStr.split(util::rexOR, Qt::SkipEmptyParts);
		if (itemListStr.toLower() == "all")
		{
			for (long long i = 1; i <= 15; ++i)
				itemIndexList.append(util::toQString(i));
		}
		else if (itemListStr.count("-") == 1)
		{
			long long min = 1;
			long long max = static_cast<long long>(sa::MAX_ITEM - sa::CHAR_EQUIPSLOT_COUNT);
			if (!checkRange(TK, 2, &min, &max))
				return Parser::kArgError + 2ll;

			for (long long i = min; i <= max; ++i)
				itemIndexList.append(util::toQString(i));
		}

		QVector<long long> itemIndexVec;
		for (const QString& itemIndex : itemIndexList)
		{
			bool bret = false;
			long long index = itemIndex.toLongLong(&bret);
			--index;
			index += sa::CHAR_EQUIPSLOT_COUNT;
			QHash<long long, sa::item_t> items = injector.worker->getItems();
			if (bret && index >= sa::CHAR_EQUIPSLOT_COUNT && index < sa::MAX_ITEM)
			{
				if (items.value(index).valid)
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
			injector.worker->tradeAppendItems(name, itemIndexVec);
			//ok = true;
		}
		else
			return Parser::kArgError + 2ll;
	}

	if (!petListStr.isEmpty())
	{
		QStringList petIndexList = petListStr.split(util::rexOR, Qt::SkipEmptyParts);
		if (itemListStr.toLower() == "all")
		{
			for (long long i = 1; i <= sa::MAX_PET; ++i)
				petIndexList.append(util::toQString(i));
		}
		else if (itemListStr.count("-") == 1)
		{
			long long min = 1;
			long long max = sa::MAX_PET;
			if (!checkRange(TK, 2, &min, &max))
				return Parser::kArgError + 2ll;

			for (long long i = min; i <= max; ++i)
				petIndexList.append(util::toQString(i));
		}

		QVector<long long> petIndexVec;
		for (const QString& petIndex : petIndexList)
		{
			bool bret = false;
			long long index = petIndex.toLongLong(&bret);
			--index;

			if (bret && index >= 0 && index < sa::MAX_PET)
			{
				sa::pet_t pet = injector.worker->getPet(index);
				if (pet.valid)
					petIndexVec.append(index);
			}
		}

		std::sort(petIndexVec.begin(), petIndexVec.end());
		auto it = std::unique(petIndexVec.begin(), petIndexVec.end());
		petIndexVec.erase(it, petIndexVec.end());

		if (!petIndexVec.isEmpty())
		{
			injector.worker->tradeAppendPets(name, petIndexVec);
			//ok = true;
		}
		else
			return Parser::kArgError + 3ll;
	}

	if (gold > 0 && gold <= injector.worker->getCharacter().gold)
	{
		injector.worker->tradeAppendGold(name, gold);
		//ok = true;
	}

	injector.worker->tradeComfirm(name);

	waitfor(timeout, [&injector]()
		{
			return injector.worker->getCharacter().trade_confirm >= 3;
		});

	injector.worker->tradeComplete(name);

	waitfor(timeout, [&injector]()
		{
			return !injector.worker->IS_TRADING.get();
		});

	return Parser::kNoChange;
}

long long Interpreter::usemagic(long long currentIndex, long long currentLine, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance(currentIndex);

	if (injector.worker.isNull())
		return Parser::kServerNotReady;

	checkOnlineThenWait();
	checkBattleThenWait();

	QString magicName;
	checkString(TK, 1, &magicName);
	if (magicName.isEmpty())
		return Parser::kArgError + 1ll;

	long long target = -1;
	checkInteger(TK, 2, &target);
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
			QHash<QString, long long> hash = {
				{ "自己", 0},
				{ "戰寵", injector.worker->getCharacter().battlePetNo},
				{ "騎寵", injector.worker->getCharacter().ridePetNo},
				{ "隊長", 6},

				{ "自己", 0},
				{ "战宠", injector.worker->getCharacter().battlePetNo},
				{ "骑宠", injector.worker->getCharacter().ridePetNo},
				{ "队长", 6},

				{ "self", 0},
				{ "battlepet", injector.worker->getCharacter().battlePetNo},
				{ "ride", injector.worker->getCharacter().ridePetNo},
				{ "leader", 6},
			};

			for (long long i = 0; i < sa::MAX_PET; ++i)
			{
				hash.insert("寵物" + util::toQString(i + 1), i + 1);
				hash.insert("宠物" + util::toQString(i + 1), i + 1);
				hash.insert("pet" + util::toQString(i + 1), i + 1);
			}

			for (long long i = 1; i < sa::MAX_TEAM; ++i)
			{
				hash.insert("隊員" + util::toQString(i), i + 1 + sa::MAX_PET);
				hash.insert("队员" + util::toQString(i), i + 1 + sa::MAX_PET);
				hash.insert("teammate" + util::toQString(i), i + 1 + sa::MAX_PET);
			}

			if (!hash.contains(targetTypeName))
				return Parser::kArgError + 2ll;

			target = hash.value(targetTypeName, -1);
			if (target < 0)
				return Parser::kArgError + 2ll;
		}
	}

	long long magicIndex = injector.worker->getMagicIndexByName(magicName);
	if (magicIndex < 0)
		return Parser::kArgError + 1ll;

	injector.worker->useMagic(magicIndex, target);

	return Parser::kNoChange;
}

long long Interpreter::mail(long long currentIndex, long long currentLine, const TokenMap& TK)
{
	Injector& injector = Injector::getInstance(currentIndex);

	if (injector.worker.isNull())
		return Parser::kServerNotReady;

	checkOnlineThenWait();
	checkBattleThenWait();

	QVariant card;
	QString name;
	long long addrIndex = -1;
	if (!checkInteger(TK, 1, &addrIndex))
	{
		if (!checkString(TK, 1, &name))
			return Parser::kArgError + 1ll;
		if (name.isEmpty())
			return Parser::kArgError + 1ll;

		card = name;
	}
	else
	{
		if (addrIndex <= 0 || addrIndex >= sa::MAX_ADDRESS_BOOK)
			return Parser::kArgError + 1ll;
		--addrIndex;

		card = addrIndex;
	}

	QString text;
	checkString(TK, 2, &text);

	long long petIndex = -1;
	checkInteger(TK, 3, &petIndex);
	if (petIndex != -1)
	{
		--petIndex;
		if (petIndex < 0 || petIndex >= sa::MAX_PET)
			return Parser::kArgError + 3ll;
	}

	QString itemName = "";
	checkString(TK, 4, &itemName);

	QString itemMemo = "";
	checkString(TK, 5, &itemMemo);

	if (petIndex != -1 && itemMemo.isEmpty() && !itemName.isEmpty())
		return Parser::kArgError + 4ll;

	injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.reset();

	injector.worker->mail(card, text, petIndex, itemName, itemMemo);

	waitfor(500, [&injector]()->bool { return injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.get() <= 0; });
	injector.worker->IS_WAITOFR_ITEM_CHANGE_PACKET.reset();
	return Parser::kNoChange;
}