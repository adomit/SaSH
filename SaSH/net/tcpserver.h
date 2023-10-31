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

#pragma once
#include <QCoreApplication>
#include <QTimer>
#include <iostream>
#include <string>
#include <vector>
#include <WS2tcpip.h>

#include <threadplugin.h>
#include <indexer.h>
#include <util.h>
#include "lssproto.h"
#include "autil.h"
#include "map/mapanalyzer.h"

class Socket : public QTcpSocket
{
	Q_OBJECT

public:
	Socket(qintptr socketDescriptor, QObject* parent = nullptr);

	virtual ~Socket();

	QThread thread;

private slots:
	void onReadyRead();
	void onWrite(QByteArray ba, long long size);

private:

	long long index_ = -1;
	bool init = false;
	QFuture<void> netFuture_;
};

class Worker : public ThreadPlugin, public Lssproto
{
	Q_OBJECT

public:
	Worker(long long index, Socket* socket, QObject* parent);

	virtual ~Worker();

private:
	QHash<long long, QString> g_dirStrHash = {
		{ 0, QObject::tr("North") },
		{ 1, QObject::tr("ENorth") },
		{ 2, QObject::tr("East") },
		{ 3, QObject::tr("ESouth") },
		{ 4, QObject::tr("South") },
		{ 5, QObject::tr("WSouth") },
		{ 6, QObject::tr("West") },
		{ 7, QObject::tr("WNorth") },
	};


signals:
	void write(QByteArray ba, long long size = 0);
	void findPathFinished();

public slots:
	void processRead();

private:
	long long __fastcall dispatchMessage(const QByteArray& encoded);

	bool __fastcall handleCustomMessage(const QByteArray& data);

	Q_INVOKABLE void handleData(const QByteArray& data);

public://actions
	QString __fastcall battleStringFormat(const sa::battleobject_t& obj, QString formatStr);

	[[nodiscard]] long long __fastcall getWorldStatus();

	[[nodiscard]] long long __fastcall getGameStatus();

	[[nodiscard]] bool __fastcall checkWG(long long  w, long long g);

	[[nodiscard]] long long __fastcall getUnloginStatus();
	void __fastcall setWorldStatus(long long w);
	void __fastcall setGameStatus(long long g);

	bool __fastcall login(long long s);

	void __fastcall clientLogin(const QString& userName, const QString& password);
	void __fastcall playerLogin(long long index);

	QString __fastcall getBadStatusString(long long status);

	QString __fastcall getFieldString(long long field);

	void __fastcall unlockSecurityCode(const QString& code);

	void __fastcall clearNetBuffer();

	void __fastcall logOut();

	void __fastcall logBack();

	void __fastcall move(const QPoint& p, const QString& dir);

	void __fastcall move(const QPoint& p);

	void __fastcall announce(const QString& msg, long long color = 4);

	void __fastcall createCharacter(long long dataplacenum
		, const QString& charname
		, long long imgno
		, long long faceimgno
		, long long vit
		, long long str
		, long long tgh
		, long long dex
		, long long earth
		, long long water
		, long long fire
		, long long wind
		, long long hometown
		, bool forcecover);

	void __fastcall deleteCharacter(long long index, const QString securityCode, bool backtofirst = false);

	void __fastcall talk(const QString& text, long long color = 0, sa::TalkMode mode = sa::kTalkNormal);
	void __fastcall inputtext(const QString& text, long long dialogid = -1, long long npcid = -1);

	void __fastcall windowPacket(const QString& command, long long dialogid, long long npcid);

	void __fastcall EO();

	void __fastcall dropItem(long long index);
	void __fastcall dropItem(QVector<long long> index);

	void __fastcall useItem(long long itemIndex, long long target);


	void __fastcall swapItem(long long from, long long to);

	void __fastcall petitemswap(long long petIndex, long long from, long long to);

	void __fastcall useMagic(long long magicIndex, long long target);

	void __fastcall dropPet(long long petIndex);

	void __fastcall setSwitcher(long long flg, bool enable);

	void __fastcall setSwitcher(long long flg);

	void __fastcall press(sa::BUTTON_TYPE select, long long dialogid = -1, long long unitid = -1);
	void __fastcall press(long long row, long long dialogid = -1, long long unitid = -1);

	void __fastcall buy(long long index, long long amt, long long dialogid = -1, long long unitid = -1);
	void __fastcall sell(const QVector<long long>& indexs, long long dialogid = -1, long long unitid = -1);
	void __fastcall sell(long long index, long long dialogid = -1, long long unitid = -1);
	void __fastcall sell(const QString& name, const QString& memo = "", long long dialogid = -1, long long unitid = -1);
	void __fastcall learn(long long skillIndex, long long petIndex, long long spot, long long dialogid = -1, long long unitid = -1);

	void __fastcall craft(sa::CraftType type, const QStringList& ingres);

	void __fastcall createRemoteDialog(unsigned long long type, unsigned long long button, const QString& text);

	void __fastcall mail(const QVariant& card, const QString& text, long long petIndex, const QString& itemName, const QString& itemMemo);

	void __fastcall warp();

	void __fastcall shopOk(long long n);
	void __fastcall saMenu(long long n);

	bool __fastcall addPoint(long long skillid, long long amt);

	void __fastcall pickItem(long long dir);

	void __fastcall dropGold(long long gold);

	void __fastcall depositGold(long long gold, bool isPublic);
	void __fastcall withdrawGold(long long gold, bool isPublic);

	void __fastcall depositPet(long long petIndex, long long dialogid = -1, long long unitid = -1);
	void __fastcall withdrawPet(long long petIndex, long long dialogid = -1, long long unitid = -1);

	void __fastcall depositItem(long long index, long long dialogid = -1, long long unitid = -1);
	void __fastcall withdrawItem(long long itemIndex, long long dialogid = -1, long long unitid = -1);

	bool __fastcall captchaOCR(QString* pmsg);

	void findPathAsync(const QPoint& pos);

	void __fastcall setAllPetState();
	void __fastcall setPetState(long long petIndex, sa::PetState state);
	void __fastcall setFightPet(long long petIndex);
	void __fastcall setRidePet(long long petIndex);
	void __fastcall setPetStateSub(long long petIndex, long long state);
	void __fastcall setPetStandby(long long petIndex, long long state);

	void __fastcall updateItemByMemory();
	void __fastcall updateDatasFromMemory();
	void __fastcall setUserDatas();

	void __fastcall doBattleWork(bool canDelay);
	bool asyncBattleAction(bool canDelay);

	void __fastcall downloadMap(long long floor = -1);
	void __fastcall downloadMap(long long x, long long y, long long floor = -1);

	bool __fastcall tradeStart(const QString& name, long long timeout);
	void __fastcall tradeComfirm(const QString& name);
	void __fastcall tradeCancel();
	void __fastcall tradeAppendItems(const QString& name, const QVector<long long>& itemIndexs);
	void __fastcall tradeAppendGold(const QString& name, long long gold);
	void __fastcall tradeAppendPets(const QString& name, const QVector<long long>& petIndex);
	void __fastcall tradeComplete(const QString& name);

	void __fastcall cleanChatHistory();
	[[nodiscard]] QString __fastcall getChatHistory(long long index);

	bool __fastcall findUnit(const QString& name, long long type, sa::mapunit_t* unit, const QString& freeName = "", long long modelid = -1);

	[[nodiscard]] QString __fastcall getGround();

	void __fastcall setTeamState(bool join);
	void __fastcall kickteam(long long n);

	long long __fastcall setCharFaceToPoint(const QPoint& pos);
	void __fastcall setCharFaceDirection(long long dir, bool noWindow = false);
	void __fastcall setCharFaceDirection(const QString& dirStr);

	[[nodiscard]] long long __fastcall getPartySize() const;
	[[nodiscard]] QStringList __fastcall getJoinableUnitList() const;
	[[nodiscard]] bool __fastcall getItemIndexsByName(const QString& name, const QString& memo, QVector<long long>* pv, long long from = 0, long long to = sa::MAX_ITEM);
	[[nodiscard]] long long __fastcall getItemIndexByName(const QString& name, bool isExact = true, const QString& memo = "", long long from = 0, long long to = sa::MAX_ITEM);
	[[nodiscard]] long long __fastcall getPetSkillIndexByName(long long& petIndex, const QString& name) const;
	[[nodiscard]] long long __fastcall getSkillIndexByName(const QString& name) const;
	[[nodiscard]] bool __fastcall getPetIndexsByName(const QString& name, QVector<long long>* pv) const;
	[[nodiscard]] long long __fastcall getMagicIndexByName(const QString& name, bool isExact = true) const;
	[[nodiscard]] long long __fastcall getItemEmptySpotIndex();
	bool __fastcall getItemEmptySpotIndexs(QVector<long long>* pv);
	void __fastcall clear();

	[[nodiscard]] bool __fastcall checkCharMp(long long cmpvalue, long long* target = nullptr, bool useequal = false);
	[[nodiscard]] bool __fastcall checkCharHp(long long cmpvalue, long long* target = nullptr, bool useequal = false);
	[[nodiscard]] bool __fastcall checkRideHp(long long cmpvalue, long long* target = nullptr, bool useequal = false);
	[[nodiscard]] bool __fastcall checkPetHp(long long cmpvalue, long long* target = nullptr, bool useequal = false);
	[[nodiscard]] bool __fastcall checkPartyHp(long long cmpvalue, long long* target);

	[[nodiscard]] bool __fastcall isPetSpotEmpty() const;
	[[nodiscard]] long long __fastcall checkJobDailyState(const QString& missionName, long long timeout);

	[[nodiscard]] bool __fastcall isDialogVisible();

	void __fastcall setCharFreeName(const QString& name);
	void __fastcall setPetFreeName(long long petIndex, const QString& name);

	[[nodiscard]] bool __fastcall getBattleFlag();
	[[nodiscard]] bool __fastcall getOnlineFlag() const;

	void __fastcall sortItem();

	[[nodiscard]] long long __fastcall getDir();
	[[nodiscard]] QPoint __fastcall getPoint();
	void __fastcall setPoint(const QPoint& pos);

	[[nodiscard]] long long __fastcall getFloor();
	void __fastcall setFloor(long long floor);

	[[nodiscard]] QString __fastcall getFloorName();
	//battle
	void __fastcall sendBattleCharAttackAct(long long target);
	void __fastcall sendBattleCharMagicAct(long long magicIndex, long long target);
	void __fastcall sendBattleCharJobSkillAct(long long skillIndex, long long target);
	void __fastcall sendBattleCharItemAct(long long itemIndex, long long target);
	void __fastcall sendBattleCharDefenseAct();
	void __fastcall sendBattleCharEscapeAct();
	void __fastcall sendBattleCharCatchPetAct(long long petIndex);
	void __fastcall sendBattleCharSwitchPetAct(long long petIndex);
	void __fastcall sendBattleCharDoNothing();
	void __fastcall sendBattlePetSkillAct(long long skillIndex, long long target);
	void __fastcall sendBattlePetDoNothing();
	void __fastcall setBattleEnd();

	void updateBattleTimeInfo();

	inline [[nodiscard]] sa::PC __fastcall getPC() const { /*QReadLocker locker(&charInfoLock_); */ return pc_.get(); }
	inline void __fastcall setPC(sa::PC pc) { pc_.set(pc); }

	inline [[nodiscard]] sa::MAGIC __fastcall getMagic(long long magicIndex) const { return magic_.value(magicIndex); }

	inline [[nodiscard]] sa::PROFESSION_SKILL __fastcall getSkill(long long skillIndex) const { /*QReadLocker locker(&charSkillInfoLock_); */ return profession_skill_.value(skillIndex); }
	inline [[nodiscard]] QHash<long long, sa::PROFESSION_SKILL> __fastcall getSkills() const { /*QReadLocker locker(&charSkillInfoLock_); */ return profession_skill_.toHash(); }

	inline [[nodiscard]] sa::PET __fastcall getPet(long long petIndex) const { QReadLocker locker(&petInfoLock_);  return pet_.value(petIndex); }
	inline [[nodiscard]] QHash<long long, sa::PET> __fastcall getPets() const { QReadLocker locker(&petInfoLock_);  return pet_.toHash(); }
	inline [[nodiscard]] long long __fastcall getPetSize() const
	{
		QReadLocker locker(&petInfoLock_);
		long long n = 0;
		QHash<long long, sa::PET> pets = pet_.toHash();
		for (const sa::PET& it : pets)
		{
			if (it.level > 0 && it.valid && it.maxHp > 0)
				++n;
		}
		return n;
	}

	inline [[nodiscard]] sa::ITEM __fastcall getItem(long long index) const { QReadLocker locker(&itemInfoLock_);  return item_.value(index); }
	inline [[nodiscard]] QHash<long long, sa::ITEM> __fastcall getItems() const { QReadLocker locker(&itemInfoLock_);  return item_.toHash(); }

	inline [[nodiscard]] sa::PET_SKILL __fastcall getPetSkill(long long petIndex, long long skillIndex) const { /*QReadLocker locker(&petSkillInfoLock_); */ return petSkill_.value(petIndex).value(skillIndex); }
	inline [[nodiscard]] QHash<long long, sa::PET_SKILL> __fastcall getPetSkills(long long petIndex) const { /*QReadLocker locker(&petSkillInfoLock_); */ return petSkill_.value(petIndex); }
	inline void __fastcall setPetSkills(long long petIndex, const QHash<long long, sa::PET_SKILL>& skills) { petSkill_.insert(petIndex, skills); }
	inline void __fastcall setPetSkill(long long petIndex, long long skillIndex, const sa::PET_SKILL& skill)
	{
		QHash<long long, sa::PET_SKILL> skills = petSkill_.value(petIndex);
		skills.insert(skillIndex, skill);
		petSkill_.insert(petIndex, skills);
	}

	inline [[nodiscard]] sa::PARTY __fastcall getParty(long long partyIndex) const { /*QReadLocker locker(&teamInfoLock_); */ return party_.value(partyIndex); }
	inline [[nodiscard]] QHash<long long, sa::PARTY> __fastcall getParties() const { /*QReadLocker locker(&teamInfoLock_); */ return party_.toHash(); }

	inline [[nodiscard]] sa::ITEM __fastcall getPetEquip(long long petIndex, long long equipIndex) const {/* QReadLocker locker(&petEquipInfoLock_); */ return petItem_.value(petIndex).value(equipIndex); }
	inline [[nodiscard]] QHash<long long, sa::ITEM> __fastcall getPetEquips(long long petIndex) const { /*QReadLocker locker(&petEquipInfoLock_); */ return petItem_.value(petIndex); }

	inline [[nodiscard]] sa::ADDRESS_BOOK __fastcall getAddressBook(long long index) const { return addressBook_.value(index); }
	inline [[nodiscard]] QHash<long long, sa::ADDRESS_BOOK> __fastcall getAddressBooks() const { return addressBook_.toHash(); }

	inline [[nodiscard]] sa::battledata_t __fastcall getBattleData() const { return battleData.get(); }
	inline [[nodiscard]] sa::JOBDAILY __fastcall getJobDaily(long long index) const { return jobdaily_.value(index); }
	inline [[nodiscard]] QHash<long long, sa::JOBDAILY> __fastcall getJobDailys() const { return jobdaily_.toHash(); }
	inline [[nodiscard]] sa::CHARLISTTABLE __fastcall getCharListTable(long long index) const { return chartable_.value(index); }
	inline [[nodiscard]] sa::MAIL_HISTORY __fastcall getMailHistory(long long index) const { return mailHistory_.value(index); }

	[[nodiscard]] long long __fastcall findInjuriedAllie();

	void __fastcall refreshItemInfo();

	void __fastcall updateComboBoxList();

	void __fastcall setWindowTitle(QString formatStr);
private:
	void __fastcall setCharModelDir(long long dir);

	void __fastcall refreshItemInfo(long long index);

	void __fastcall setBattleFlag(bool enable);
	void __fastcall setOnlineFlag(bool enable);

	void __fastcall getCharMaxCarryingCapacity();
	inline [[nodiscard]] constexpr bool __fastcall isItemStackable(long long flg) { return ((flg >> 2) & 1); }
	QString __fastcall getAreaString(long long target);
	[[nodiscard]] bool __fastcall matchPetNameByIndex(long long index, const QString& name);
	[[nodiscard]] long long __fastcall getProfessionSkillIndexByName(const QString& names) const;

#pragma region BattleFunctions
	long long __fastcall playerDoBattleWork(const sa::battledata_t& bt);
	void __fastcall handleCharBattleLogics(const sa::battledata_t& bt);
	long long __fastcall petDoBattleWork(const sa::battledata_t& bt);
	void __fastcall handlePetBattleLogics(const sa::battledata_t& bt);

	bool __fastcall isCharMpEnoughForMagic(long long magicIndex) const;
	bool __fastcall isCharMpEnoughForSkill(long long magicIndex) const;
	bool __fastcall isCharHpEnoughForSkill(long long magicIndex) const;

	void __fastcall sortBattleUnit(QVector<sa::battleobject_t>& v) const;

	enum BattleMatchType
	{
		MatchNotUsed,
		MatchPos,
		MatchLevel,
		MatchHp,
		MatchMaxHp,
		MatchHPP,
		MatchModel,
		MatchName,
		MatchStatus
	};
	bool __fastcall matchBattleTarget(const QVector<sa::battleobject_t>& btobjs, BattleMatchType matchtype, long long firstMatchPos, QString op, QVariant cmpvar, long long* ppos);
	bool __fastcall conditionMatchTarget(QVector<sa::battleobject_t> btobjs, const QString& conditionStr, long long* ppos);

	[[nodiscard]] long long __fastcall getBattleSelectableEnemyTarget(const sa::battledata_t& bt) const;

	[[nodiscard]] long long __fastcall getBattleSelectableEnemyOneRowTarget(const sa::battledata_t& bt, bool front) const;

	[[nodiscard]] long long __fastcall getBattleSelectableAllieTarget(const sa::battledata_t& bt) const;

	[[nodiscard]] bool __fastcall matchBattleEnemyByName(const QString& name, bool isExact, const QVector<sa::battleobject_t>& src, QVector<sa::battleobject_t>* v) const;
	[[nodiscard]] bool __fastcall matchBattleEnemyByLevel(long long level, const QVector<sa::battleobject_t>& src, QVector<sa::battleobject_t>* v) const;
	[[nodiscard]] bool __fastcall matchBattleEnemyByMaxHp(long long  maxHp, const QVector<sa::battleobject_t>& src, QVector<sa::battleobject_t>* v) const;

	[[nodiscard]] long long __fastcall getGetPetSkillIndexByName(long long etIndex, const QString& name) const;

	bool __fastcall fixCharTargetByMagicIndex(long long magicIndex, long long oldtarget, long long* target) const;
	bool __fastcall fixCharTargetBySkillIndex(long long magicIndex, long long oldtarget, long long* target) const;
	bool __fastcall fixCharTargetByItemIndex(long long itemIndex, long long oldtarget, long long* target) const;
	bool __fastcall fixPetTargetBySkillIndex(long long skillIndex, long long oldtarget, long long* target) const;
	void __fastcall updateCurrentSideRange(sa::battledata_t* bt);
	bool __fastcall checkFlagState(long long pos);

	inline void __fastcall setBattleData(const sa::battledata_t& data) { battleData.set(data); }

	//自動鎖寵
	void checkAutoLockPet();

	//自動加點
	void checkAutoAbility();

	//檢查並自動吃肉、或丟肉
	void checkAutoDropMeat();

	//自動吃經驗加乘道具
	void __fastcall checkAutoEatBoostExpItem();

	//自動丟棄道具
	void __fastcall checkAutoDropItems();

	//自動補血、氣
	void checkAutoHeal();

	//自動丟寵
	void checkAutoDropPet();
#pragma endregion

#pragma region SAClientOriginal
	//StoneAge Client Original Functions

	void __fastcall swapItemLocal(long long from, long long to);

	void __fastcall realTimeToSATime(sa::LSTIME* lstime);

#pragma endregion

private: //lockers
	mutable QReadWriteLock petInfoLock_;
	//mutable QReadWriteLock petSkillInfoLock_;
	//mutable QReadWriteLock charInfoLock_;
	//mutable QReadWriteLock charSkillInfoLock_;
	//mutable QReadWriteLock charMagicInfoLock_;
	mutable QReadWriteLock itemInfoLock_;
	//mutable QReadWriteLock petEquipInfoLock_;
	//mutable QReadWriteLock teamInfoLock_;
	mutable QMutex moveLock_;

private:
	safe::Flag IS_BATTLE_FLAG = false;//是否在戰鬥中
	safe::Flag IS_ONLINE_FLAG = false;//是否在線上

	util::Timer eoTTLTimer;//伺服器響應時間(MS)
	util::Timer connectingTimer;//登入連接時間(MS)
	safe::Flag petEnableEscapeForTemp = false;//寵物臨時設置逃跑模式(觸發調用DoNothing)
	safe::Integer tempCatchPetTargetIndex = -1;//臨時捕捉寵物目標索引

	safe::Data<sa::battledata_t> battleData; //戰鬥數據

	safe::Flag IS_WAITFOT_SKUP_RECV = false; //等待接收SKUP封包
	QFuture<void> skupFuture; //自動加點線程管理器

	safe::Flag IS_LOCKATTACK_ESCAPE_DISABLE = false;//鎖定攻擊不逃跑 (轉指定攻擊)

	safe::Flag battleBackupThreadFlag = false;//戰鬥動作備用線程標誌

	safe::Data<sa::PC> pc_ = {};

	safe::Hash<long long, sa::PARTY> party_ = {};
	safe::Hash<long long, sa::ITEM> item_ = {};
	safe::Hash<long long, QHash<long long, sa::ITEM>> petItem_ = {};
	safe::Hash<long long, sa::PET> pet_ = {};
	safe::Hash<long long, sa::MAGIC> magic_ = {};
	safe::Hash<long long, sa::ADDRESS_BOOK> addressBook_ = {};
	safe::Hash<long long, sa::JOBDAILY> jobdaily_ = {};
	safe::Hash<long long, sa::CHARLISTTABLE> chartable_ = {};
	safe::Hash<long long, sa::MAIL_HISTORY> mailHistory_ = {};
	safe::Hash<long long, sa::PROFESSION_SKILL> profession_skill_ = {};
	safe::Hash<long long, QHash<long long, sa::PET_SKILL>> petSkill_ = {};

	QHash<QString, bool> itemStackFlagHash = {};

	safe::Vector<bool> battlePetDisableList_ = {};

	QFuture<void> battleBackupFuture_; //戰鬥動作備用線程管理器
	QFuture<void> autoLockPet_; //自動鎖寵線程管理器
	QFuture<void> dropMeatFuture_; //自動丟肉線程管理器
	QFuture<void> autoHealFuture_; //自動補血線程管理器
	QFuture<void> autoAbilityFuture_; //自動加點線程管理器
	QFuture<void> autoDropPetFuture_; //自動丟寵線程管理器

	//client original 目前很多都是沒用處的
#pragma region ClientOriginal
	QString lastSecretChatName = "";//最後一次收到密語的發送方名稱

	//遊戲內當前時間相關
	sa::LSTIME saTimeStruct = {};
	safe::Integer serverTime = 0LL;
	safe::Integer firstServerTime = 0LL;

	//交易相關
	long long opp_showindex = 0;
	QString opp_sockfd;
	QString opp_name;
	QString opp_goldmount;
	long long showindex[7] = {};
	long long tradeWndDropGoldGet = 0;
	QString opp_itemgraph;
	QString opp_itemname;
	QString opp_itemeffect;
	QString opp_itemindex;
	QString opp_itemdamage;
	QString trade_kind;
	QString trade_command;
	long long tradeStatus = 0;
	long long tradePetIndex = -1;
	sa::PET tradePet[2] = {};
	sa::showitem opp_item[sa::MAX_MAXHAVEITEM];	//交易道具阵列增为15个
	sa::showpet opp_pet[sa::MAX_PET];
	QStringList myitem_tradeList;
	QStringList mypet_tradeList = { "P|-1", "P|-1", "P|-1" , "P|-1", "P|-1" };
	long long mygoldtrade = 0;

	//郵件相關
	long long mailHistoryWndSelectNo = 0;
	long long mailHistoryWndPageNo = 0;
#pragma endregion

public:
	safe::Queue<QByteArray> readQueue_; //接收來自客戶端的數據隊列

	safe::Integer nowFloor_; //當前地圖編號
	safe::Data<QString> nowFloorName_; //當前地圖名稱
	safe::Data<QPoint> nowPoint_; //當前人物座標

	//戰鬥相關
	safe::Flag battleCharAlreadyActed = true; //戰鬥人物已經動作
	safe::Flag battlePetAlreadyActed = true; //戰鬥寵物已經動作
	safe::Integer battleCharCurrentPos = 0; //戰鬥人物當前位置
	safe::Integer battleBpFlag = 0; //戰鬥BP標誌
	safe::Integer battleField = 0; //戰鬥場地
	safe::Flag battleCharEscapeFlag = 0; //戰鬥人物退戰標誌
	safe::Integer battleCharCurrentMp = 0; //戰鬥人物當前MP
	safe::Integer battleCurrentAnimeFlag = 0; //戰鬥當前動畫標誌

	//custom
	safe::Flag IS_TRADING = false;
	safe::Flag IS_DISCONNECTED = false;

	safe::Flag IS_WAITFOR_JOBDAILY_FLAG = false;
	safe::Flag IS_WAITFOR_BANK_FLAG = false;
	safe::Flag IS_WAITFOR_DIALOG_FLAG = false;
	safe::Flag IS_WAITFOR_CUSTOM_DIALOG_FLAG = false;
	safe::Integer IS_WAITOFR_ITEM_CHANGE_PACKET = false;

	safe::Flag isBattleDialogReady = false;
	safe::Flag isEOTTLSend = false;
	safe::Integer lastEOTime = 0;

	util::Timer loginTimer;
	util::Timer battleDurationTimer;
	util::Timer normalDurationTimer;
	util::Timer oneRoundDurationTimer;

	safe::Integer battleCurrentRound = 0;
	safe::Integer battle_total_time = 0;
	safe::Integer battle_total = 0;
	safe::Integer battle_one_round_time = 0;

	safe::Integer saCurrentGameTime = 0;//遊戲時間 LSTIME_SECTION

	MapAnalyzer mapAnalyzer;

	safe::Data<sa::currencydata_t> currencyData = {};
	safe::Data<sa::customdialog_t> customDialog = {};

	safe::Hash<long long, sa::mapunit_t> mapUnitHash;
	safe::Hash<QPoint, sa::mapunit_t> npcUnitPointHash;
	safe::Queue<QPair<long long, QString>> chatQueue;

	QPair<long long, QVector<sa::bankpet_t>> currentBankPetList;
	safe::Vector<sa::ITEM> currentBankItemList;

	util::Timer repTimer;
	util::AfkRecorder recorder[1 + sa::MAX_PET] = {};

	safe::Data<sa::dialog_t> currentDialog = {};

	//用於緩存要發送到UI的數據(開啟子窗口初始化並加載當前最新數據時使用)
	safe::Hash<long long, QVariant> playerInfoColContents;
	safe::Hash<long long, QVariant> itemInfoRowContents;
	safe::Hash<long long, QVariant> equipInfoRowContents;
	safe::Data<QStringList> enemyNameListCache;
	safe::Data<QString> timeLabelContents;
	safe::Data<QString> labelCharAction;
	safe::Data<QString> labelPetAction;

private:
	enum BufferControl
	{
		BUFF_NONE,
		BUFF_NEED_TO_CLEAN,
		BUFF_HAS_NEXT,
		BUFF_ABOUT_TO_END,
		BUFF_INVALID,
	};

	char net_data[NETDATASIZE] = {};
	char net_resultdata[SBUFSIZE] = {};
	QByteArrayList dataList_ = {};
	QByteArray net_readbuf_;
	QByteArray net_raw_;

private://lssproto
	long long __fastcall appendReadBuf(const QByteArray& data);
	bool __fastcall splitLinesFromReadBuf(QByteArrayList& list);
	long long __fastcall a62toi(const QString& a) const;
	long long __fastcall getStringToken(const QString& src, const QString& delim, long long count, QString& out) const;
	long long __fastcall getIntegerToken(const QString& src, const QString& delim, long long count) const;
	long long __fastcall getInteger62Token(const QString& src, const QString& delim, long long count) const;
	void __fastcall makeStringFromEscaped(QString& src) const;

private://lssproto_recv
#pragma region Lssproto_Recv
	virtual void __fastcall lssproto_XYD_recv(const QPoint& pos, long long dir) override;//戰鬥結束後的大地圖座標
	virtual void __fastcall lssproto_EV_recv(long long dialogid, long long result) override;
	virtual void __fastcall lssproto_EN_recv(long long result, long long field) override;
	virtual void __fastcall lssproto_RS_recv(char* data) override;
	virtual void __fastcall lssproto_RD_recv(char* data) override;
	virtual void __fastcall lssproto_B_recv(char* command) override;
	virtual void __fastcall lssproto_I_recv(char* data) override;
	virtual void __fastcall lssproto_SI_recv(long long fromindex, long long toindex) override;
	virtual void __fastcall lssproto_MSG_recv(long long aindex, char* text, long long color) override;	//收到普通郵件或寵物郵件
	virtual void __fastcall lssproto_PME_recv(long long unitid, long long graphicsno, const QPoint& pos, long long dir, long long flg, long long no, char* cdata) override;
	virtual void __fastcall lssproto_AB_recv(char* data) override;
	virtual void __fastcall lssproto_ABI_recv(long long num, char* data) override;
	virtual void __fastcall lssproto_TK_recv(long long index, char* message, long long color) override;
	virtual void __fastcall lssproto_MC_recv(long long fl, long long x1, long long y1, long long x2, long long y2, long long tilesum, long long objsum, long long eventsum, char* data) override;
	virtual void __fastcall lssproto_M_recv(long long fl, long long x1, long long y1, long long x2, long long y2, char* data) override;
	virtual void __fastcall lssproto_C_recv(char* data) override;
	virtual void __fastcall lssproto_CA_recv(char* data) override;
	virtual void __fastcall lssproto_CD_recv(char* data) override;
	virtual void __fastcall lssproto_R_recv(char* data) override;
	virtual void __fastcall lssproto_S_recv(char* data) override;
	virtual void __fastcall lssproto_D_recv(long long category, long long dx, long long dy, char* data) override;
	virtual void __fastcall lssproto_FS_recv(long long flg) override;
	virtual void __fastcall lssproto_HL_recv(long long flg) override;//戰鬥中是否要Help
	virtual void __fastcall lssproto_PR_recv(long long request, long long result) override;
	virtual void __fastcall lssproto_KS_recv(long long petarray, long long result) override;//指定那一只寵物出場戰鬥
#ifdef _STANDBYPET
	virtual void __fastcall lssproto_SPET_recv(long long standbypet, long long result) override;
#endif
	virtual void __fastcall lssproto_PS_recv(long long result, long long havepetindex, long long havepetskill, long long toindex) override;	//寵物合成
	virtual void __fastcall lssproto_SKUP_recv(long long point) override;//取得可加的屬性點數
	virtual void __fastcall lssproto_WN_recv(long long windowtype, long long buttontype, long long dialogid, long long unitid, char* data) override;
	virtual void __fastcall lssproto_EF_recv(long long effect, long long level, char* option) override;
	virtual void __fastcall lssproto_SE_recv(const QPoint& pos, long long senumber, long long sw) override;
	virtual void __fastcall lssproto_ClientLogin_recv(char* result) override;
	virtual void __fastcall lssproto_CreateNewChar_recv(char* result, char* data) override;
	virtual void __fastcall lssproto_CharDelete_recv(char* result, char* data) override;
	virtual void __fastcall lssproto_CharLogin_recv(char* result, char* data) override;
	virtual void __fastcall lssproto_CharList_recv(char* result, char* data) override;
	virtual void __fastcall lssproto_CharLogout_recv(char* result, char* data) override;
	virtual void __fastcall lssproto_ProcGet_recv(char* data) override;
	virtual void __fastcall lssproto_CharNumGet_recv(long long logincount, long long player) override;
	virtual void __fastcall lssproto_Echo_recv(char* test) override;
	virtual void __fastcall lssproto_NU_recv(long long AddCount) override;
	virtual void __fastcall lssproto_WO_recv(long long effect) override;//取得轉生的特效
	virtual void __fastcall lssproto_TD_recv(char* data) override;
	virtual void __fastcall lssproto_FM_recv(char* data) override;
#ifdef _ITEM_CRACKER
	virtual void __fastcall lssproto_IC_recv(const QPoint& pos) override;
#endif
#ifdef _MAGIC_NOCAST//沈默
	virtual void __fastcall lssproto_NC_recv(long long flg) override;
#endif
#ifdef _CHECK_GAMESPEED
	virtual void __fastcall lssproto_CS_recv(long long deltimes) override;
#endif
#ifdef _PETS_SELECTCON
	virtual void __fastcall lssproto_PETST_recv(long long petarray, long long result) override;
#endif
#ifdef _CHATROOMPROTOCOL			// (不可開) 聊天室頻道
	virtual void __fastcall lssproto_CHATROOM_recv(char* data) override;
#endif
#ifdef _NEWREQUESTPROTOCOL			// (不可開) 新增Protocol要求細項
	virtual void __fastcall lssproto_RESIST_recv(char* data) override;
#endif
#ifdef _ALCHEPLUS
	virtual void __fastcall lssproto_ALCHEPLUS_recv(char* data) override;
#endif

#ifdef _OUTOFBATTLESKILL			// (不可開) Syu ADD 非戰鬥時技能Protocol
	virtual void __fastcall lssproto_BATTLESKILL_recv(char* data) override;
#endif
	virtual void __fastcall lssproto_CHAREFFECT_recv(char* data) override;

#ifdef _STREET_VENDOR
	virtual void __fastcall lssproto_STREET_VENDOR_recv(char* data) override;	// 擺攤功能
#endif

#ifdef _JOBDAILY
	virtual void __fastcall lssproto_JOBDAILY_recv(char* data) override;
#endif

#ifdef _FAMILYBADGE_
	virtual void __fastcall lssproto_FamilyBadge_recv(char* data) override;
#endif

#ifdef _TEACHER_SYSTEM
	virtual void __fastcall lssproto_TEACHER_SYSTEM_recv(char* data) override;
#endif

	virtual void __fastcall lssproto_S2_recv(char* data) override;

#ifdef _ITEM_FIREWORK
	virtual void __fastcall lssproto_Firework_recv(long long nCharaindex, long long nType, long long nActionNum) override;	// 煙火功能
#endif
#ifdef _THEATER
	virtual void __fastcall lssproto_TheaterData_recv(char* pData) override;
#endif
#ifdef _MOVE_SCREEN
	virtual void __fastcall lssproto_MoveScreen_recv(BOOL bMoveScreenMode, long long iXY) override;	// client 移動熒幕
#endif
#ifdef _NPC_MAGICCARD
	virtual void __fastcall lssproto_MagiccardAction_recv(char* data) override;	//魔法牌功能
	virtual void __fastcall lssproto_MagiccardDamage_recv(long long position, long long damage, long long offsetx, long long offsety) override;
#endif
#ifdef  _NPC_DANCE
	virtual void __fastcall lssproto_DancemanOption_recv(long long option) override;	//動一動狀態
#endif
#ifdef _ANNOUNCEMENT_
	virtual void __fastcall lssproto_DENGON_recv(char* data, long long colors, long long nums) override;
#endif
#ifdef _HUNDRED_KILL
	virtual void __fastcall lssproto_hundredkill_recv(long long flag) override;
#endif
#ifdef _PK2007
	virtual void __fastcall lssproto_pkList_recv(long long count, char* data) override;
#endif
#ifdef _PETBLESS_
	virtual void __fastcall lssproto_petbless_send(long long petpos, long long type) override;
#endif
#ifdef _PET_SKINS
	virtual void __fastcall lssproto_PetSkins_recv(char* data) override;
#endif

	//////////////////////////////////

	virtual void __fastcall lssproto_CustomWN_recv(const QString& data) override;
	virtual void __fastcall lssproto_CustomTK_recv(const QString& data) override;
#pragma endregion
};

class Server : public QTcpServer
{
	Q_OBJECT
public:
	explicit Server(QObject* parent = nullptr);

	virtual ~Server();

	bool start(QObject* parent);

	void clear();

protected:
	virtual void incomingConnection(qintptr socketDescriptor) override;

private:
	QList<Socket*> clientSockets_;
};
