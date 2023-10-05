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
#include <QScopedPointer>
#include <QRegularExpression>
#include <QObject>
#include <QWidget>
#include <QList>
#include <QVariant>
#include <QVariantMap>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QTabBar>
#include <QLocale>
#include <QCollator>
#include <QHash>
#include <type_traits>
#include "3rdparty/simplecrypt.h"
#include "model/treewidgetitem.h"

constexpr qint64 SASH_VERSION_MAJOR = 1;
constexpr qint64 SASH_VERSION_MINOR = 0;
constexpr qint64 SASH_VERSION_PATCH = 0;
constexpr qint64 SASH_MAX_THREAD = 65535;

typedef struct break_marker_s
{
	qint64 line = 0;
	qint64 count = 0;
	qint64 maker = 0;
	QString content = "\0";
} break_marker_t;

namespace mem
{
	bool read(HANDLE hProcess, quint64 desiredAccess, quint64 size, PVOID buffer);
	template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T> && !std::is_pointer_v<T>>>
	Q_REQUIRED_RESULT T read(HANDLE hProcess, quint64 desiredAccess);

	template char read<char>(HANDLE hProcess, quint64 desiredAccess);
	template short read<short>(HANDLE hProcess, quint64 desiredAccess);
	template int read<int>(HANDLE hProcess, quint64 desiredAccess);
	template float read<float>(HANDLE hProcess, quint64 desiredAccess);
	template long read<long>(HANDLE hProcess, quint64 desiredAccess);
	template long long read<long long>(HANDLE hProcess, quint64 desiredAccess);
	template unsigned char read<unsigned char>(HANDLE hProcess, quint64 desiredAccess);
	template unsigned short read<unsigned short>(HANDLE hProcess, quint64 desiredAccess);
	template unsigned int read<unsigned int>(HANDLE hProcess, quint64 desiredAccess);
	template unsigned long read<unsigned long>(HANDLE hProcess, quint64 desiredAccess);
	template unsigned long long read<unsigned long long>(HANDLE hProcess, quint64 desiredAccess);

	template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T> && !std::is_pointer_v<T>>>
	bool write(HANDLE hProcess, quint64 baseAddress, T data);
	template bool write<char>(HANDLE hProcess, quint64 baseAddress, char data);
	template bool write<short>(HANDLE hProcess, quint64 baseAddress, short data);
	template bool write<int>(HANDLE hProcess, quint64 baseAddress, int data);
	template bool write<float>(HANDLE hProcess, quint64 baseAddress, float data);
	template bool write<long>(HANDLE hProcess, quint64 baseAddress, long data);
	template bool write<long long>(HANDLE hProcess, quint64 baseAddress, long long data);
	template bool write<unsigned char>(HANDLE hProcess, quint64 baseAddress, unsigned char data);
	template bool write<unsigned short>(HANDLE hProcess, quint64 baseAddress, unsigned short data);
	template bool write<unsigned int>(HANDLE hProcess, quint64 baseAddress, unsigned int data);
	template bool write<unsigned long>(HANDLE hProcess, quint64 baseAddress, unsigned long data);
	template bool write<unsigned long long>(HANDLE hProcess, quint64 baseAddress, unsigned long long data);

	Q_REQUIRED_RESULT float readFloat(HANDLE hProcess, quint64 desiredAccess);
	Q_REQUIRED_RESULT qreal readDouble(HANDLE hProcess, quint64 desiredAccess);
	Q_REQUIRED_RESULT QString readString(HANDLE hProcess, quint64 desiredAccess, quint64 size, bool enableTrim = true, bool keepOriginal = false);
	bool write(HANDLE hProcess, quint64 baseAddress, PVOID buffer, quint64 dwSize);
	bool writeString(HANDLE hProcess, quint64 baseAddress, const QString& str);
	bool virtualFree(HANDLE hProcess, quint64 baseAddress);
	Q_REQUIRED_RESULT quint64 virtualAlloc(HANDLE hProcess, quint64 size);
	Q_REQUIRED_RESULT quint64 virtualAllocW(HANDLE hProcess, const QString& str);
	Q_REQUIRED_RESULT quint64 virtualAllocA(HANDLE hProcess, const QString& str);
	Q_REQUIRED_RESULT DWORD getRemoteModuleHandle(DWORD dwProcessId, const QString& moduleName);
	void freeUnuseMemory(HANDLE hProcess);

	typedef struct _IAT_EAT_INFO
	{
		char ModuleName[256] = {};
		char FuncName[64] = {};
		ULONG64 Address = 0;
		ULONG64 RecordAddr = 0;
		ULONG64 ModBase = 0;//just for export table
	} IAT_EAT_INFO, * PIAT_EAT_INFO;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && defined(_M_IX86)
	inline __declspec(naked) DWORD* getKernel32()
	{
		__asm
		{
			mov eax, fs: [0x30]
			mov eax, [eax + 0xC]
			mov eax, [eax + 0x1C]
			mov eax, [eax]
			mov eax, [eax]
			mov eax, [eax + 8]
			ret
		}
	}
#else
	inline DWORD* getKernel32()
	{
		return (DWORD*)GetModuleHandleW(L"kernelbase.dll");
	}
#endif

	DWORD getFunAddr(const DWORD* DllBase, const char* FunName);
	HMODULE getRemoteModuleHandleByProcessHandleW(HANDLE hProcess, const QString& szModuleName);
	long getProcessExportTable32(HANDLE hProcess, const QString& ModuleName, IAT_EAT_INFO tbinfo[], int tb_info_max);
	ULONG64 getProcAddressIn32BitProcess(HANDLE hProcess, const QString& ModuleName, const QString& FuncName);
	bool injectByWin7(qint64 index, DWORD dwProcessId, HANDLE hProcess, QString dllPath, HMODULE* phDllModule, quint64* phGameModule);
	bool injectBy64(qint64 index, HANDLE hProcess, QString dllPath, HMODULE* phDllModule, quint64* phGameModule);//兼容64位注入32位
	bool inject(qint64 index, HANDLE hProcess, QString dllPath, HMODULE* phDllModule, quint64* phGameModule);//32注入32
}

namespace util
{
	constexpr const char* SA_NAME = "sa_8001.exe";
	constexpr const char* DEFAULT_CODEPAGE = "UTF-8";
	constexpr const char* DEFAULT_GAME_CODEPAGE = "GBK";//gb18030/gb2312
	constexpr const char* SCRIPT_DEFAULT_SUFFIX = ".txt";
	constexpr const char* SCRIPT_LUA_SUFFIX_DEFAULT = ".lua";
	constexpr const char* SCRIPT_PRIVATE_SUFFIX_DEFAULT = ".sac";
	typedef enum
	{
		REASON_NO_ERROR,
		//Thread
		REASON_CLOSE_BEFORE_START,
		REASON_PID_INVALID,
		REASON_FAILED_TO_INIT,
		REASON_MEMBERSHIP_INVALID,
		REASON_REQUEST_STOP,
		REASON_TARGET_WINDOW_DISAPPEAR,
		REASON_FAILED_READ_DATA,

		//TCP
		REASON_TCP_PRECONNECTION_TIMEOUT,
		REASON_TCP_CONNECTION_TIMEOUT,
		REASON_TCP_CREATE_SOCKET_FAIL,
		REASON_TCP_IPV6_CONNECT_FAIL,
		REASON_TCP_IPV4_CONNECT_FAIL,
		REASON_TCP_EVENT_SELECT_FAIL,
		REASON_TCP_KEEPALIVE_FAIL,

		//CreateProcess
		REASON_DIRECTORY_EMPTY,
		REASON_ARGUMENT_EMPTY,
		REASON_IMAGE_PATH_NOFOUND,
		REASON_EXECUTABLE_FILE_PATH_EMPTY,
		REASON_FAILED_TO_START,

		//Injection
		REASON_ENUM_WINDOWS_FAIL,
		REASON_HOOK_DLL_SHA512_FAIL,
		REASON_GET_KERNEL32_FAIL,
		REASON_GET_KERNEL32_UNDOCUMENT_API_FAIL,
		REASON_OPEN_PROCESS_FAIL,
		REASON_VIRTUAL_ALLOC_FAIL,
		REASON_INJECT_LIBRARY_FAIL,
		REASON_CLOSE_MUTEX_FAIL,
		REASON_REMOTE_LIBRARY_INIT_FAIL,
	}REMOVE_THREAD_REASON, * LPREMOVE_THREAD_REASON;

	enum UserStatus
	{
		kLabelStatusNotUsed = 0,//未知
		kLabelStatusNotOpen,//未開啟石器
		kLabelStatusOpening,//開啟石器中
		kLabelStatusOpened,//已開啟石器
		kLabelStatusLogining,//登入
		kLabelStatusSignning,//簽入中
		kLabelStatusSelectServer,//選擇伺服器
		kLabelStatusSelectSubServer,//選擇分伺服器
		kLabelStatusGettingCharList,//取得人物中
		kLabelStatusSelectPosition,//選擇人物中
		kLabelStatusLoginSuccess,//登入成功
		kLabelStatusInNormal,//平時
		kLabelStatusInBattle,//戰鬥中
		kLabelStatusBusy,//忙碌中
		kLabelStatusTimeout,//連線逾時
		kLabelStatusLoginFailed,//登入失敗
		kLabelNoUserNameOrPassword,//無帳號密碼
		kLabelStatusDisconnected,//斷線
		kLabelStatusConnecting,//連線中
	};

	enum UserData
	{
		kUserDataNotUsed = 0x0,
		kUserItemNames = 0x1,
		kUserEnemyNames = 0x2,
		kUserPetNames = 0x4,
	};

	enum ComboBoxUpdateType
	{
		kComboBoxCharAction,
		kComboBoxPetAction,
		kComboBoxItem,
	};

	enum SelectTarget
	{
		kSelectSelf = 0x1,            // 己 (Self)
		kSelectPet = 0x2,             // 寵 (Pet)
		kSelectAllieAny = 0x4,        // 我任 (Any ally)
		kSelectAllieAll = 0x8,        // 我全 (All allies)
		kSelectEnemyAny = 0x10,       // 敵任 (Any enemy)
		kSelectEnemyAll = 0x20,       // 敵全 (All enemies)
		kSelectEnemyFront = 0x40,     // 敵前 (Front enemy)
		kSelectEnemyBack = 0x80,      // 敵後 (Back enemy)
		kSelectLeader = 0x100,        // 隊 (Leader)
		kSelectLeaderPet = 0x200,     // 隊寵 (Leader's pet)
		kSelectTeammate1 = 0x400,     // 隊1 (Teammate 1)
		kSelectTeammate1Pet = 0x800,  // 隊1寵 (Teammate 1's pet)
		kSelectTeammate2 = 0x1000,    // 隊2 (Teammate 2)
		kSelectTeammate2Pet = 0x2000, // 隊2寵 (Teammate 2's pet)
		kSelectTeammate3 = 0x4000,    // 隊3 (Teammate 3)
		kSelectTeammate3Pet = 0x8000, // 隊3寵 (Teammate 3's pet)
		kSelectTeammate4 = 0x10000,   // 隊4 (Teammate 4)
		kSelectTeammate4Pet = 0x20000,// 隊4寵 (Teammate 4's pet)
		kSelectEnemy1 = 0x40000,      // 敵1 (Enemy 1)
		kSelectEnemy2 = 0x80000,      // 敵1寵 (Enemy2)
		kSelectEnemy3 = 0x100000,     // 敵3 (Enemy 3)
		kSelectEnemy4 = 0x200000,     // 敵4 (Enemy 4)
		kSelectEnemy5 = 0x400000,     // 敵5 (Enemy 5)
		kSelectEnemy6 = 0x800000,     // 敵6 (Enemy 6)
		kSelectEnemy7 = 0x1000000,    // 敵7 (Enemy 7)
		kSelectEnemy8 = 0x2000000,    // 敵8 (Enemy 8)
		kSelectEnemy9 = 0x4000000,    // 敵9 (Enemy 9)
		kSelectEnemy10 = 0x8000000,   // 敵10 (Enemy 10)
	};

	enum UnLoginStatus
	{
		kStatusUnknown,
		kStatusDisappear,
		kStatusInputUser,
		kStatusSelectServer,
		kStatusSelectSubServer,
		kStatusSelectCharacter,
		kStatusLogined,
		kStatusDisconnect,
		kStatusConnecting,
		kStatusTimeout,
		kNoUserNameOrPassword,
		kStatusBusy,
		kStatusLoginFailed,
	};

	enum CraftType
	{
		kCraftFood,
		kCraftItem
	};

	enum UserSetting
	{
		kSettingNotUsed = 0,

		///////////////////

		kSettingMinValue,

		kServerValue,
		kSubServerValue,
		kPositionValue,
		kLockTimeValue,

		//afk->battle button
		kBattleCharRoundActionTargetValue,
		kBattleCharCrossActionTargetValue,
		kBattleCharNormalActionTargetValue,
		kBattlePetNormalActionTargetValue,

		kBattlePetRoundActionTargetValue,
		kBattlePetCrossActionTargetValue,

		//afk->battle combobox
		kBattleCharRoundActionRoundValue,
		kBattleCharRoundActionTypeValue,
		kBattleCharRoundActionEnemyValue,
		kBattleCharRoundActionLevelValue,

		kBattleCharCrossActionTypeValue,
		kBattleCharCrossActionRoundValue,

		kBattleCharNormalActionTypeValue,
		kBattleCharNormalActionEnemyValue,
		kBattleCharNormalActionLevelValue,

		kBattlePetRoundActionRoundValue,
		kBattlePetRoundActionTypeValue,
		kBattlePetRoundActionEnemyValue,
		kBattlePetRoundActionLevelValue,

		kBattlePetCrossActionTypeValue,
		kBattlePetCrossActionRoundValue,

		kBattlePetNormalActionTypeValue,
		kBattlePetNormalActionEnemyValue,
		kBattlePetNormalActionLevelValue,

		kBattlePetHealActionTypeValue,
		kBattlePetPurgActionTypeValue,
		kBattleCharPurgActionTypeValue,

		//afk->heal button
		kBattleMagicHealTargetValue,
		kBattleItemHealTargetValue,
		kBattleMagicReviveTargetValue,
		kBattleItemReviveTargetValue,
		kBattlePetHealTargetValue,
		kBattlePetPurgTargetValue,
		kBattleCharPurgTargetValue,

		//afk->heal combobox
		kBattleMagicHealMagicValue,
		kBattleMagicReviveMagicValue,
		kNormalMagicHealMagicValue,

		//afk->heal spinbox
		kBattleMagicHealCharValue,
		kBattleMagicHealPetValue,
		kBattleMagicHealAllieValue,

		kBattleItemHealCharValue,
		kBattleItemHealPetValue,
		kBattleItemHealAllieValue,

		kBattleItemHealMpValue,
		kBattleSkillMpValue,

		kNormalMagicHealCharValue,
		kNormalMagicHealPetValue,
		kNormalMagicHealAllieValue,

		kNormalItemHealCharValue,
		kNormalItemHealPetValue,
		kNormalItemHealAllieValue,

		kNormalItemHealMpValue,

		kBattlePetHealCharValue,
		kBattlePetHealPetValue,
		kBattlePetHealAllieValue,
		//afk->walk
		kAutoWalkDelayValue,
		kAutoWalkDistanceValue,
		kAutoWalkDirectionValue,

		//afk->catch
		kBattleCatchModeValue,
		kBattleCatchTargetLevelValue,
		kBattleCatchTargetMaxHpValue,
		kBattleCatchTargetMagicHpValue,
		kBattleCatchTargetItemHpValue,
		kBattleCatchCharMagicValue,
		kBattleCatchPetSkillValue,


		//afk->battle delay
		kBattleActionDelayValue,
		kBattleResendDelayValue,


		kDropPetStrValue,
		kDropPetDefValue,
		kDropPetAgiValue,
		kDropPetHpValue,
		kDropPetAggregateValue,

		//general
		kSpeedBoostValue,


		//other->group
		kAutoFunTypeValue,

		//lockpet
		kLockPetValue,
		kLockRideValue,

		//script
		kScriptSpeedValue,

		kSettingMaxValue,

		///////////////////

		kSettingMinEnable,

		kAutoLoginEnable,
		kAutoReconnectEnable,
		kLogOutEnable,
		kLogBackEnable,
		kEchoEnable,
		kHideCharacterEnable,
		kCloseEffectEnable,
		kOptimizeEnable,
		kHideWindowEnable,
		kMuteEnable,
		kAutoJoinEnable,
		kLockTimeEnable,
		kAutoFreeMemoryEnable,
		kFastWalkEnable,
		kPassWallEnable,
		kLockMoveEnable,
		kLockImageEnable,
		kAutoDropMeatEnable,
		kAutoDropEnable,
		kAutoStackEnable,
		kKNPCEnable,
		kAutoAnswerEnable,
		kAutoEatBeanEnable,
		kAutoWalkEnable,
		kFastAutoWalkEnable,
		kFastBattleEnable,
		kAutoBattleEnable,
		kAutoCatchEnable,
		kLockAttackEnable,
		kAutoEscapeEnable,
		kLockEscapeEnable,
		kBattleTimeExtendEnable,
		kFallDownEscapeEnable,
		kShowExpEnable,
		kWindowDockEnable,
		kBattleAutoSwitchEnable,
		kBattleAutoEOEnable,

		//switcher
		kSwitcherTeamEnable,
		kSwitcherPKEnable,
		kSwitcherCardEnable,
		kSwitcherTradeEnable,
		kSwitcherGroupEnable,
		kSwitcherFamilyEnable,
		kSwitcherJobEnable,
		kSwitcherWorldEnable,

		//afk->battle
		kBattleCrossActionCharEnable,
		kBattleCrossActionPetEnable,

		//afk->heal
		kBattleMagicHealEnable,
		kBattleItemHealEnable,
		kBattleItemHealMeatPriorityEnable,
		kBattleItemHealMpEnable,
		kBattleMagicReviveEnable,
		kBattleItemReviveEnable,
		kBattleSkillMpEnable,
		kBattlePetHealEnable,
		kBattlePetPurgEnable,
		kBattleCharPurgEnable,

		kNormalMagicHealEnable,
		kNormalItemHealEnable,
		kNormalItemHealMeatPriorityEnable,
		kNormalItemHealMpEnable,

		//afk->catch
		kBattleCatchTargetLevelEnable,
		kBattleCatchTargetMaxHpEnable,
		kBattleCatchCharMagicEnable,
		kBattleCatchCharItemEnable,
		kBattleCatchPetSkillEnable,

		kDropPetEnable,
		kDropPetStrEnable,
		kDropPetDefEnable,
		kDropPetAgiEnable,
		kDropPetHpEnable,
		kDropPetAggregateEnable,


		//lockpet
		kLockPetEnable,
		kLockRideEnable,

		kLockPetScheduleEnable,

		kBattleNoEscapeWhileLockPetEnable,

		kSettingMaxEnable,

		//////////////////

		kSettingMinString,

		//global
		kAutoDropItemString,
		kLockAttackString,
		kLockEscapeString,

		//afk->heal
		kBattleItemHealItemString,
		kBattleItemHealMpItemString,
		kBattleItemReviveItemString,

		kNormalItemHealItemString,
		kNormalItemHealMpItemString,

		//afk->catch
		kBattleCatchPetNameString,
		kBattleCatchCharItemString,
		kDropPetNameString,


		//other->group
		kAutoFunNameString,

		//other->lockpet
		kLockPetScheduleString,

		//other->other2
		kGameAccountString,
		kGamePasswordString,
		kGameSecurityCodeString,

		kMailWhiteListString,

		kEOCommandString,

		kBattleLogicsString,

		kSettingMaxString,

		kScriptDebugModeEnable,
	};

	enum ObjectType
	{
		OBJ_UNKNOWN,
		OBJ_ROAD,
		OBJ_UP,
		OBJ_DOWN,
		OBJ_JUMP,
		OBJ_WARP,
		OBJ_WALL,
		OBJ_ROCK,
		OBJ_ROCKEX,
		OBJ_BOUNDARY,
		OBJ_WATER,
		OBJ_EMPTY,
		OBJ_HUMAN,
		OBJ_NPC,
		OBJ_BUILDING,
		OBJ_ITEM,
		OBJ_PET,
		OBJ_GOLD,
		OBJ_GM,
		OBJ_MAX,
	};

	//用於將枚舉直轉換為字符串，提供給json當作key
	static const QHash<UserSetting, QString> user_setting_string_hash = {
		{ kSettingNotUsed, "SettingNotUsed" },

		{ kSettingMinValue, "SettingMinValue" },

		{ kServerValue, "ServerValue" },
		{ kSubServerValue, "SubServerValue" },
		{ kPositionValue, "PositionValue" },
		{ kLockTimeValue, "LockTimeValue" },
		{ kSpeedBoostValue, "SpeedBoostValue" },

		//afk->battle combobox
		{ kBattleCharRoundActionRoundValue, "BattleCharRoundActionRoundValue" },
		{ kBattleCharRoundActionTypeValue, "BattleCharRoundActionTypeValue" },
		{ kBattleCharRoundActionEnemyValue, "BattleCharRoundActionEnemyValue" },
		{ kBattleCharRoundActionLevelValue, "BattleCharRoundActionLevelValue" },

		{ kBattleCharCrossActionTypeValue, "BattleCharCrossActionTypeValue" },
		{ kBattleCharCrossActionRoundValue, "BattleCharCrossActionRoundValue" },

		{ kBattleCharNormalActionTypeValue, "BattleCharNormalActionTypeValue" },
		{ kBattleCharNormalActionEnemyValue, "BattleCharNormalActionEnemyValue" },
		{ kBattleCharNormalActionLevelValue, "BattleCharNormalActionLevelValue" },

		{ kBattlePetRoundActionRoundValue, "BattlePetRoundActionRoundValue" },
		{ kBattlePetRoundActionTypeValue, "BattlePetRoundActionTypeValue" },
		{ kBattlePetRoundActionEnemyValue, "BattlePetRoundActionEnemyValue" },
		{ kBattlePetRoundActionLevelValue, "BattlePetRoundActionLevelValue" },

		{ kBattlePetCrossActionTypeValue, "BattlePetCrossActionTypeValue" },
		{ kBattlePetCrossActionRoundValue, "BattlePetCrossActionRoundValue" },

		{ kBattlePetNormalActionTypeValue, "BattlePetNormalActionTypeValue" },
		{ kBattlePetNormalActionEnemyValue, "BattlePetNormalActionEnemyValue" },
		{ kBattlePetNormalActionLevelValue, "BattlePetNormalActionLevelValue" },

		//afk->battle buttom
		{ kBattleCharRoundActionTargetValue, "BattleCharRoundActionTargetValue" },
		{ kBattleCharCrossActionTargetValue, "BattleCharCrossActionTargetValue" },
		{ kBattleCharNormalActionTargetValue, "BattleCharNormalActionTargetValue" },

		{ kBattlePetRoundActionTargetValue, "BattlePetRoundActionTargetValue" },
		{ kBattlePetCrossActionTargetValue, "BattlePetCrossActionTargetValue" },
		{ kBattlePetNormalActionTargetValue, "BattlePetNormalActionTargetValue" },

		//afk->heal button
		{ kBattleMagicHealTargetValue, "BattleMagicHealTargetValue" },
		{ kBattleItemHealTargetValue, "BattleItemHealTargetValue" },
		{ kBattleMagicReviveTargetValue, "BattleMagicReviveTargetValue" },
		{ kBattleItemReviveTargetValue, "BattleItemReviveTargetValue" },

		//afk->heal combobox
		{ kBattleMagicHealMagicValue, "BattleMagicHealMagicValue" },
		{ kBattleMagicReviveMagicValue, "BattleMagicReviveMagicValue" },
		{ kNormalMagicHealMagicValue, "NormalMagicHealMagicValue" },
		{ kBattleSkillMpValue, "BattleSkillMpValue" },

		//afk->heal spinbox
		{ kBattleMagicHealCharValue, "MagicHealCharValue" },
		{ kBattleMagicHealPetValue, "MagicHealPetValue" },
		{ kBattleMagicHealAllieValue, "MagicHealAllieValue" },

		{ kBattleItemHealCharValue, "ItemHealCharValue" },
		{ kBattleItemHealPetValue, "ItemHealPetValue" },
		{ kBattleItemHealAllieValue, "ItemHealAllieValue" },

		{ kBattleItemHealMpValue, "ItemHealMpValue" },

		{ kNormalMagicHealCharValue, "MagicHealCharNormalValue" },
		{ kNormalMagicHealPetValue, "MagicHealPetNormalValue" },
		{ kNormalMagicHealAllieValue, "MagicHealAllieNormalValue" },

		{ kNormalItemHealCharValue, "ItemHealCharNormalValue" },
		{ kNormalItemHealPetValue, "ItemHealPetNormalValue" },
		{ kNormalItemHealAllieValue, "ItemHealAllieNormalValue" },

		{ kNormalItemHealMpValue, "ItemHealMpNormalValue" },

		//afk->walk
		{ kAutoWalkDelayValue, "AutoWalkDelayValue" },
		{ kAutoWalkDistanceValue, "AutoWalkDistanceValue" },
		{ kAutoWalkDirectionValue, "AutoWalkDirectionValue" },

		//afk->catch
		{ kBattleCatchModeValue, "BattleCatchModeValue" },
		{ kBattleCatchTargetLevelValue, "BattleCatchTargetLevelValue" },
		{ kBattleCatchTargetMaxHpValue, "BattleCatchTargetMaxHpValue" },
		{ kBattleCatchTargetMagicHpValue, "BattleCatchTargetMagicHpValue" },
		{ kBattleCatchTargetItemHpValue, "BattleCatchTargetItemHpValue" },
		{ kBattleCatchCharMagicValue, "BattleCatchCharMagicValue" },
		{ kBattleCatchPetSkillValue, "BattleCatchPetSkillValue" },

		//afk->battle delay
		{ kBattleActionDelayValue, "BattleActionDelayValue" },
		{ kBattleResendDelayValue, "BattleResendDelayValue" },

		{ kDropPetStrValue, "DropPetStrValue" },
		{ kDropPetDefValue, "DropPetDefValue" },
		{ kDropPetAgiValue, "DropPetAgiValue" },
		{ kDropPetHpValue, "DropPetHpValue" },
		{ kDropPetAggregateValue, "DropPetAggregateValue" },

		//other->group
		{ kAutoFunTypeValue, "AutoFunTypeValue" },

		//lockpet
		{ kLockPetValue, "LockPetValue" },
		{ kLockRideValue, "LockRideValue" },
		{ kLockPetScheduleEnable, "LockPetScheduleEnable" },

		//script
		{ kScriptSpeedValue, "ScriptSpeedValue" },

		{ kSettingMaxValue, "SettingMaxValue" },

		//check
		{ kSettingMinEnable, "SettingMinEnable" },
		{ kAutoLoginEnable, "AutoLoginEnable" },
		{ kAutoReconnectEnable, "AutoReconnectEnable" },
		{ kLogOutEnable, "LogOutEnable" },
		{ kLogBackEnable, "LogBackEnable" },
		{ kEchoEnable, "EchoEnable" },
		{ kHideCharacterEnable, "HideCharacterEnable" },
		{ kCloseEffectEnable, "CloseEffectEnable" },
		{ kOptimizeEnable, "OptimizeEnable" },
		{ kHideWindowEnable, "HideWindowEnable" },
		{ kMuteEnable, "MuteEnable" },
		{ kAutoJoinEnable, "AutoJoinEnable" },
		{ kLockTimeEnable, "LockTimeEnable" },
		{ kAutoFreeMemoryEnable, "AutoFreeMemoryEnable" },
		{ kFastWalkEnable, "FastWalkEnable" },
		{ kPassWallEnable, "PassWallEnable" },
		{ kLockMoveEnable, "LockMoveEnable" },
		{ kLockImageEnable, "LockImageEnable" },
		{ kAutoDropMeatEnable, "AutoDropMeatEnable" },
		{ kAutoDropEnable, "AutoDropEnable" },
		{ kAutoStackEnable, "AutoStackEnable" },
		{ kKNPCEnable, "KNPCEnable" },
		{ kAutoAnswerEnable, "AutoAnswerEnable" },
		{ kAutoEatBeanEnable, "AutoEatBeanEnable" },
		{ kAutoWalkEnable, "AutoWalkEnable" },
		{ kFastAutoWalkEnable, "FastAutoWalkEnable" },
		{ kFastBattleEnable, "FastBattleEnable" },
		{ kAutoBattleEnable, "AutoBattleEnable" },
		{ kAutoCatchEnable, "AutoCatchEnable" },
		{ kLockAttackEnable, "LockAttackEnable" },
		{ kAutoEscapeEnable, "AutoEscapeEnable" },
		{ kLockEscapeEnable, "LockEscapeEnable" },
		{ kBattleTimeExtendEnable, "BattleTimeExtendEnable" },
		{ kFallDownEscapeEnable, "FallDownEscapeEnable" },
		{ kShowExpEnable, "ShowExpEnable" },
		{ kWindowDockEnable, "WindowDockEnable" },
		{ kBattleAutoSwitchEnable, "BattleAutoSwitchEnable" },
		{ kBattleAutoEOEnable ,"BattleAutoEOEnable" },

		{ kSettingMaxEnable, "SettingMaxEnable" },

		//switcher
		{ kSwitcherTeamEnable, "SwitcherTeamEnable" },
		{ kSwitcherPKEnable, "SwitcherPKEnable" },
		{ kSwitcherCardEnable, "SwitcherCardEnable" },
		{ kSwitcherTradeEnable, "SwitcherTradeEnable" },
		{ kSwitcherGroupEnable, "SwitcherGroupEnable" },
		{ kSwitcherFamilyEnable, "SwitcherFamilyEnable" },
		{ kSwitcherJobEnable, "SwitcherJobEnable" },
		{ kSwitcherWorldEnable, "SwitcherWorldEnable" },
		//afk->battle
		{ kBattleCrossActionCharEnable, "CrossActionCharEnable" },
		{ kBattleCrossActionPetEnable, "CrossActionPetEnable" },

		//afk->heal
		{ kBattleMagicHealEnable, "BattleMagicHealEnable" },
		{ kBattleItemHealEnable, "BattleItemHealEnable" },
		{ kBattleItemHealMeatPriorityEnable, "BattleItemHealMeatPriorityEnable" },
		{ kBattleItemHealMpEnable, "BattleItemHealMpEnable" },
		{ kBattleMagicReviveEnable, "BattleMagicReviveEnable" },
		{ kBattleItemReviveEnable, "BattleItemReviveEnable" },
		{ kBattleSkillMpEnable, "BattleSkillMpEnable" },

		{ kNormalMagicHealEnable, "NormalMagicHealEnable" },
		{ kNormalItemHealEnable, "NormalItemHealEnable" },
		{ kNormalItemHealMeatPriorityEnable, "NormalItemHealMeatPriorityEnable" },
		{ kNormalItemHealMpEnable, "NormalItemHealMpEnable" },

		//afk->catch
		{ kBattleCatchTargetLevelEnable, "BattleCatchTargetLevelEnable" },
		{ kBattleCatchTargetMaxHpEnable, "BattleCatchTargetMaxHpEnable" },
		{ kBattleCatchCharMagicEnable, "BattleCatchCharMagicEnable" },
		{ kBattleCatchCharItemEnable, "BattleCatchCharItemEnable" },
		{ kBattleCatchPetSkillEnable, "BattleCatchPetSkillEnable" },

		{ kDropPetEnable, "DropPetEnable" },
		{ kDropPetStrEnable, "DropPetStrEnable" },
		{ kDropPetDefEnable, "DropPetDefEnable" },
		{ kDropPetAgiEnable, "DropPetAgiEnable" },
		{ kDropPetHpEnable, "DropPetHpEnable" },
		{ kDropPetAggregateEnable, "DropPetAggregateEnable" },

		//lockpet
		{ kLockPetEnable, "LockPetEnable" },
		{ kLockRideEnable, "LockRideEnable" },
		{ kBattleNoEscapeWhileLockPetEnable , "BattleNoEscapeWhileLockPetEnable" },


		//string
		{ kSettingMinString, "SettingMinString" },

		{ kAutoDropItemString, "AutoDropItemString" },
		{ kLockAttackString, "LockAttackString" },
		{ kLockEscapeString, "LockEscapeString" },

		//afk->heal
		{ kBattleItemHealItemString, "BattleItemHealItemString" },
		{ kBattleItemHealMpItemString, "BattleItemHealMpIteamString" },
		{ kBattleItemReviveItemString, "BattleItemReviveItemString" },
		{ kNormalItemHealItemString, "NormalItemHealItemString" },
		{ kNormalItemHealMpItemString, "NormalItemHealMpItemString" },

		//afk->catch
		{ kBattleCatchPetNameString, "BattleCatchPetNameString" },
		{ kBattleCatchCharItemString, "BattleCatchCharItemString" },
		{ kDropPetNameString, "DropPetNameString" },


		//other->group
		{ kAutoFunNameString, "AutoFunNameString" },

		//other->lockpet
		{ kLockPetScheduleString, "LockPetScheduleString" },

		{ kGameAccountString, "GameAccountString" },
		{ kGamePasswordString, "GamePasswordString" },
		{ kGameSecurityCodeString, "GameSecurityCodeString" },

		{ kMailWhiteListString , "MailWhiteListString" },

		{ kEOCommandString , "EOCommandString" },

		{ kSettingMaxString, "SettingMaxString" },

		{ kScriptDebugModeEnable, "ScriptDebugModeEnable" },
	};

	//8方位坐標補正
	static const QVector<QPoint> fix_point = {
		{0, -1},  //北0
		{1, -1},  //東北1
		{1, 0},	  //東2
		{1, 1},	  //東南3
		{0, 1},	  //南4
		{-1, 1},  //西南0
		{-1, 0},  //西1
		{-1, -1}, //西北2
	};

	Q_REQUIRED_RESULT QString applicationFilePath();

	Q_REQUIRED_RESULT QString applicationDirPath();

	Q_REQUIRED_RESULT QString applicationName();

	Q_REQUIRED_RESULT qint64 __vectorcall percent(qint64 value, qint64 total);

	template<typename T>
	inline Q_REQUIRED_RESULT QString toQString(T d, qint64 base = 10)
	{
		if constexpr (std::is_same_v<T, double>)
		{
			return QString::number(d, 'f', 5);
		}
		else if constexpr (std::is_same_v<T, float>)
		{
			return QString::number(d, 'f', 5);
		}
		else if constexpr (std::is_same_v<T, bool>)
		{
			return (d) ? QString("true") : QString("false");
		}
		else if constexpr (std::is_same_v<T, wchar_t*>)
		{
			return QString::fromWCharArray(d);
		}
		else if constexpr (std::is_same_v<T, const wchar_t*>)
		{
			return QString::fromWCharArray(d);
		}
		else if constexpr (std::is_same_v<T, char*>)
		{
			return QString::fromUtf8(d);
		}
		else if constexpr (std::is_same_v<T, const char*>)
		{
			return QString::fromUtf8(d);
		}
		else if constexpr (std::is_same_v<T, std::string>)
		{
			return QString::fromUtf8(d.c_str());
		}
		else if constexpr (std::is_same_v<T, qint64>)
		{
			return QString::number(d, base);
		}
		else if constexpr (std::is_same_v<T, quint64>)
		{
			return QString::number(d, base);
		}
		else if constexpr (std::is_same_v<T, int>)
		{
			return QString::number(d, base);
		}
		else if constexpr (std::is_same_v < T, QByteArray>)
		{
			return QString::fromUtf8(d);
		}
		else if constexpr (std::is_same_v < T, sol::object>)
		{
			if (d.is<std::string>())
				return QString::fromUtf8(d.as<std::string>().c_str());
			else
				return "";
		}
		else if constexpr (std::is_integral_v<T>)
		{
			return QString::number(d, base);
		}
		else
		{
			static_assert(false, "Unsupported type for toQString");
			return QString();
		}

	}

	inline Q_REQUIRED_RESULT QString toUnicode(const char* str, bool ext = true)
	{
		QTextCodec* codec = QTextCodec::codecForName(util::DEFAULT_GAME_CODEPAGE);//QTextCodec::codecForMib(2025);//取GB2312解碼器
		QString qstr = codec->toUnicode(str);//先以GB2312解碼轉成UNICODE
		UINT ACP = ::GetACP();
		if (950 == ACP && ext)
		{
#ifndef _DEBUG
			// 繁體系統要轉繁體否則遊戲視窗標題會亂碼(一堆問號字)
			std::wstring wstr = qstr.toStdWString();
			qint64 size = lstrlenW(wstr.c_str());
			QScopedArrayPointer <wchar_t> wbuf(new wchar_t[size + 1]());
			//繁體字碼表映射
			LCMapStringEx(LOCALE_NAME_SYSTEM_DEFAULT, LCMAP_TRADITIONAL_CHINESE, wstr.c_str(), size, wbuf.data(), size, NULL, NULL, NULL);
			qstr = util::toQString(wbuf.data());
#endif
		}

		return qstr;
	}

	inline Q_REQUIRED_RESULT std::string fromUnicode(const QString& str, bool keppOrigin = false)
	{
		QString qstr = str;
		std::wstring wstr = qstr.toStdWString();
		UINT ACP = ::GetACP();
		if (950 == ACP && !keppOrigin)
		{
			// 繁體系統要轉回簡體體否則遊戲視窗會亂碼
			qint64 size = lstrlenW(wstr.c_str());
			QScopedArrayPointer <wchar_t> wbuf(new wchar_t[size + 1]());
			LCMapStringEx(LOCALE_NAME_SYSTEM_DEFAULT, LCMAP_SIMPLIFIED_CHINESE, wstr.c_str(), size, wbuf.data(), size, NULL, NULL, NULL);
			qstr = util::toQString(wbuf.data());
		}
		QTextCodec* codec = QTextCodec::codecForName(util::DEFAULT_GAME_CODEPAGE);//QTextCodec::codecForMib(2025);//QTextCodec::codecForName("gb2312");
		QByteArray bytes = codec->fromUnicode(qstr);

		std::string s = bytes.data();

		return s;
	}

	inline void setTab(QTabWidget* pTab)
	{
		QString styleSheet = R"(
			QTabWidget{
				background-color:rgb(249, 249, 249);
				background-clip:rgb(31, 31, 31);
				background-origin:rgb(31, 31, 31);
			}

			QTabWidget::pane{
				top:0px;
				border-top:2px solid #7160E8;
				border-bottom:1px solid rgb(61,61,61);
				border-right:1px solid rgb(61,61,61);
				border-left:1px solid rgb(61,61,61);
			}

			QTabWidget::tab-bar
			{
				alignment: left;
				top:0px;
				left:5px;
				border:none;
			}

			QTabBar::tab:first{
				/*color:rgb(178,178,178);*/
				background:rgb(249, 249, 249);

				padding-left:0px;
				padding-right:0px;
				/*width:100px;*/
				height:24px;
				margin-left:0px;
				margin-right:0px;

				border-top:2px solid rgb(31, 31, 31);
			}

			QTabBar::tab:middle{
				/*color:rgb(178,178,178);*/
				background:rgb(249, 249, 249);
				padding-left:0px;
				padding-right:0px;
				/*width:100px;*/
				height:24px;
				margin-left:0px;
				margin-right:0px;

				border-top:2px solid rgb(31, 31, 31);

			}

			QTabBar::tab:last{
				/*color:rgb(178,178,178);*/
				background:rgb(249, 249, 249);

				padding-left:0px;
				padding-right:0px;
				/*width:100px;*/
				height:24px;
				margin-left:0px;
				margin-right:0px;

				border-top:2px solid rgb(31, 31, 31);
			}

			QTabBar::tab:selected{
				background:rgb(50, 130, 246);
				border-top:2px solid rgb(50, 130, 246);

			}

			QTabBar::tab:hover{
				background:rgb(50, 130, 246);
				border-top:2px solid rgb(50, 130, 246);
				padding-left:0px;
				padding-right:0px;
				/*width:100px;*/
				height:24px;
				margin-left:0px;
				margin-right:0px;
			}
		)";
		//pTab->setStyleSheet(styleSheet);
		pTab->setAttribute(Qt::WA_StyledBackground);
		QTabBar* pTabBar = pTab->tabBar();

		pTabBar->setDocumentMode(true);
		pTabBar->setExpanding(true);
	}

	bool createFileDialog(const QString& name, QString* retstring, QWidget* parent);

	bool customStringCompare(const QString& str1, const QString& str2);

	QFileInfoList loadAllFileLists(TreeWidgetItem* root, const QString& path, QStringList* list = nullptr,
		const QString& fileIcon = ":/image/icon_txt.png", const QString& folderIcon = ":/image/icon_directory.png");
	QFileInfoList loadAllFileLists(TreeWidgetItem* root, const QString& path, const QString& suffix, const QString& icon, QStringList* list = nullptr
		, const QString& folderIcon = ":/image/icon_directory.png");

	void searchFiles(const QString& dir, const QString& fileNamePart, const QString& suffixWithDot, QList<QString>* result);

	bool enumAllFiles(const QString dir, const QString suffix, QVector<QPair<QString, QString>>* result);

	QString findFileFromName(const QString& fileName, const QString& dirpath = util::applicationDirPath());

	template<typename T>
	QList<T*> findWidgets(QWidget* widget)
	{
		QList<T*> widgets;

		if (widget)
		{
			// 檢查 widget 是否為指定類型的控件，如果是，則添加到結果列表中
			T* typedWidget = qobject_cast<T*>(widget);
			if (typedWidget)
			{
				widgets.append(typedWidget);
			}

			// 遍歷 widget 的子控件並遞歸查找
			QList<QWidget*> childWidgets = widget->findChildren<QWidget*>();
			for (QWidget* childWidget : childWidgets)
			{
				// 遞歸調用 findWidgets 函數並將結果合並到 widgets 列表中
				QList<T*> childResult = findWidgets<T>(childWidget);
				widgets += childResult;
			}
		}

		return widgets;
	}

	QString formatMilliseconds(qint64 milliseconds);

	QString formatSeconds(qint64 seconds);

	bool writeFireWallOverXP(const LPCTSTR& ruleName, const LPCTSTR& appPath, bool NoopIfExist);

	bool monitorThreadResourceUsage(quint64 threadId, FILETIME& preidleTime, FILETIME& prekernelTime, FILETIME& preuserTime, double* pCpuUsage, double* pMemUsage, double* pMaxMemUsage);

	QFont getFont();

	void asyncRunBat(const QString& path, QString data);

	bool readFileFilter(const QString& fileName, QString& content, bool* pisPrivate);

	bool readFile(const QString& fileName, QString* pcontent, bool* isPrivate = nullptr);

	bool writeFile(const QString& fileName, const QString& content);

	void sortWindows(const QVector<HWND>& windowList, bool alignLeft);

#pragma region swap_row
	inline void SwapRow(QTableWidget* p, QListWidget* p2, qint64 selectRow, qint64 targetRow)
	{

		if (p)
		{
			QStringList selectRowLine, targetRowLine;
			qint64 count = p->columnCount();
			for (qint64 i = 0; i < count; ++i)
			{
				selectRowLine.append(p->item(selectRow, i)->text());
				targetRowLine.append(p->item(targetRow, i)->text());
				if (!p->item(selectRow, i))
					p->setItem(selectRow, i, q_check_ptr(new QTableWidgetItem(targetRowLine.value(i))));
				else
					p->item(selectRow, i)->setText(targetRowLine.value(i));

				if (!p->item(targetRow, i))
					p->setItem(targetRow, i, q_check_ptr(new QTableWidgetItem(selectRowLine.value(i))));
				else
					p->item(targetRow, i)->setText(selectRowLine.value(i));
			}
		}
		else if (p2)
		{
			if (p2->count() == 0) return;
			if (selectRow == targetRow || targetRow < 0 || selectRow < 0) return;
			QString selectRowStr = p2->item(selectRow)->text();
			QString targetRowStr = p2->item(targetRow)->text();
			if (selectRow > targetRow)
			{
				p2->takeItem(selectRow);
				p2->takeItem(targetRow);
				p2->insertItem(targetRow, selectRowStr);
				p2->insertItem(selectRow, targetRowStr);
			}
			else
			{
				p2->takeItem(targetRow);
				p2->takeItem(selectRow);
				p2->insertItem(selectRow, targetRowStr);
				p2->insertItem(targetRow, selectRowStr);
			}
		}
	}

	inline void SwapRowUp(QTableWidget* p)
	{
		if (p->rowCount() <= 0)
			return; //至少有一行
		const QList<QTableWidgetItem*> list = p->selectedItems();
		if (list.size() <= 0)
			return; //有選中
		qint64 t = list.value(0)->row();
		if (t - 1 < 0)
			return; //不是第一行

		qint64 selectRow = t;	 //當前行
		qint64 targetRow = t - 1; //目標行

		SwapRow(p, nullptr, selectRow, targetRow);

		QModelIndex cur = p->model()->index(targetRow, 0);
		p->setCurrentIndex(cur);
	}

	inline void SwapRowDown(QTableWidget* p)
	{
		if (p->rowCount() <= 0)
			return; //至少有一行
		const QList<QTableWidgetItem*> list = p->selectedItems();
		if (list.size() <= 0)
			return; //有選中
		qint64 t = list.value(0)->row();
		if (t + 1 > static_cast<qint64>(p->rowCount()) - 1)
			return; //不是最後一行

		qint64 selectRow = t;	 //當前行
		qint64 targetRow = t + 1; //目標行

		SwapRow(p, nullptr, selectRow, targetRow);

		QModelIndex cur = p->model()->index(targetRow, 0);
		p->setCurrentIndex(cur);
	}

	inline void SwapRowUp(QListWidget* p)
	{
		if (p->count() <= 0)
			return;
		qint64 t = p->currentIndex().row(); // ui->tableWidget->rowCount();
		if (t < 0)
			return;
		if (t - 1 < 0)
			return;

		qint64 selectRow = t;
		qint64 targetRow = t - 1;

		SwapRow(nullptr, p, selectRow, targetRow);

		QModelIndex cur = p->model()->index(targetRow, 0);
		p->setCurrentIndex(cur);
	}

	inline void SwapRowDown(QListWidget* p)
	{
		if (p->count() <= 0)
			return;
		qint64 t = p->currentIndex().row();
		if (t < 0)
			return;
		if (t + 1 > p->count() - 1)
			return;

		qint64 selectRow = t;
		qint64 targetRow = t + 1;

		SwapRow(nullptr, p, selectRow, targetRow);

		QModelIndex cur = p->model()->index(targetRow, 0);
		p->setCurrentIndex(cur);
	}
#pragma endregion

	//基於Qt QHash 的線程安全Hash容器
	template <typename K, typename V>
	class SafeHash
	{
	public:
		SafeHash() = default;

		inline SafeHash(std::initializer_list<std::pair<K, V> > list)
		{
			QWriteLocker locker(&lock);
			hash = list;
		}

		bool isEmpty() const
		{
			QReadLocker locker(&lock);
			return hash.isEmpty();
		}

		//copy
		SafeHash(const QHash<K, V>& other)
		{
			QWriteLocker locker(&lock);
			hash = other;
		}

		//copy assign
		SafeHash(const SafeHash<K, V>& other)
		{
			QWriteLocker locker(&lock);
			hash = other.hash;
		}

		//move
		SafeHash(QHash<K, V>&& other)
		{
			QWriteLocker locker(&lock);
			hash = other;
		}

		//move assign
		SafeHash(SafeHash<K, V>&& other) noexcept
		{
			QWriteLocker locker(&lock);
			hash = other.hash;
		}

		SafeHash operator=(const SafeHash& other)
		{
			QWriteLocker locker(&lock);
			hash = other.hash;
			return *this;
		}

		//operator=
		SafeHash operator=(const QHash <K, V>& other)
		{
			QWriteLocker locker(&lock);
			hash = other;
			return *this;
		}

		inline void insert(const K& key, const V& value)
		{
			QWriteLocker locker(&lock);
			hash.insert(key, value);
		}
		inline void remove(const K& key)
		{
			QWriteLocker locker(&lock);
			hash.remove(key);
		}
		inline bool contains(const K& key) const
		{
			QReadLocker locker(&lock);
			return hash.contains(key);
		}
		inline V value(const K& key) const
		{
			QReadLocker locker(&lock);
			return hash.value(key);
		}
		inline V value(const K& key, const V& defaultValue) const
		{
			QReadLocker locker(&lock);
			return hash.value(key, defaultValue);
		}

		inline qint64 size() const
		{
			QReadLocker locker(&lock);
			return hash.size();
		}
		inline QList <K> keys() const
		{
			QReadLocker locker(&lock);
			return hash.keys();
		}

		inline QList <V> values() const
		{
			QReadLocker locker(&lock);
			return hash.values();
		}
		//take
		inline V take(const K& key)
		{
			QWriteLocker locker(&lock);
			return hash.take(key);
		}

		inline void clear()
		{
			QWriteLocker locker(&lock);
			hash.clear();
		}

		inline K key(const V& value) const
		{
			QReadLocker locker(&lock);
			return hash.key(value);
		}

		inline K key(const V& value, const K& defaultValue) const
		{
			QReadLocker locker(&lock);
			return hash.key(value, defaultValue);
		}

		inline typename QHash<K, V>::iterator begin()
		{
			QReadLocker locker(&lock);
			return hash.begin();
		}

		inline typename QHash<K, V>::const_iterator begin() const
		{
			QReadLocker locker(&lock);
			return hash.begin();
		}

		//const
		inline typename QHash<K, V>::const_iterator cbegin() const
		{
			QReadLocker locker(&lock);
			return hash.constBegin();
		}

		inline typename QHash<K, V>::iterator end()
		{
			QReadLocker locker(&lock);
			return hash.end();
		}

		//const
		inline typename QHash<K, V>::const_iterator cend() const
		{
			QReadLocker locker(&lock);
			return hash.constEnd();
		}

		inline typename QHash<K, V>::const_iterator end() const
		{
			QReadLocker locker(&lock);
			return hash.end();
		}

		//erase
		inline  typename QHash<K, V>::iterator erase(typename QHash<K, V>::iterator it)
		{
			QWriteLocker locker(&lock);
			return hash.erase(it);
		}

		//find
		inline typename QHash<K, V>::iterator find(const K& key)
		{
			QReadLocker locker(&lock);
			return hash.find(key);
		}

		QHash <K, V> toHash() const
		{
			QReadLocker locker(&lock);
			return hash;
		}

		void lockForRead() const
		{
			lock.lockForRead();
		}

		void lockForWrite() const
		{
			lock.lockForWrite();
		}

		void unlock() const
		{
			lock.unlock();
		}

	private:
		QHash<K, V> hash;
		mutable QReadWriteLock lock;
	};

	template <typename V>
	class SafeQueue
	{
	public:
		explicit SafeQueue(qint64 maxSize)
			: maxSize_(maxSize)
		{
		}
		virtual ~SafeQueue() = default;

		void clear()
		{
			QWriteLocker locker(&lock_);
			queue_.clear();
		}

		QVector<V> values() const
		{
			QReadLocker locker(&lock_);
			return queue_.toVector();

		}

		qint64 size() const
		{
			QReadLocker locker(&lock_);
			return queue_.size();
		}

		bool isEmpty() const
		{
			QReadLocker locker(&lock_);
			return queue_.isEmpty();
		}

		void enqueue(const V& value)
		{
			QWriteLocker locker(&lock_);
			if (queue_.size() >= maxSize_)
			{
				queue_.dequeue();
			}
			queue_.enqueue(value);
		}

		bool dequeue(V* pvalue)
		{
			QWriteLocker locker(&lock_);
			if (queue_.isEmpty())
			{
				return false;
			}
			if (pvalue)
				*pvalue = queue_.dequeue();
			return true;
		}

		V front() const
		{
			QReadLocker locker(&lock_);
			return queue_.head();
		}

		void setMaxSize(qint64 maxSize)
		{
			QWriteLocker locker(&lock_);
			queue_.setMaxSize(maxSize);
		}

		//=
		SafeQueue& operator=(const SafeQueue& other)
		{
			QWriteLocker locker(&lock_);
			if (this != &other)
			{
				queue_ = other.queue_;
			}
			return *this;
		}

	private:
		QQueue<V> queue_;
		qint64 maxSize_;
		mutable QReadWriteLock lock_;
	};;

	template <typename T>
	class SafeData
	{
	public:
		SafeData() = default;
		virtual ~SafeData() = default;

		T get() const
		{
			QReadLocker locker(&lock_);
			return data_;
		}

		T data() const
		{
			QReadLocker locker(&lock_);
			return data_;
		}

		void set(const T& data)
		{
			QWriteLocker locker(&lock_);
			data_ = data;
		}

		SafeData<T>& operator=(const T& data)
		{
			set(data);
			return *this;
		}

		operator T() const
		{
			return get();
		}

		//==
		bool operator==(const T& data) const
		{
			return (get() == data);
		}

		//!=
		bool operator!=(const T& data) const
		{
			return (get() != data);
		}

	private:
		T data_;
		mutable QReadWriteLock lock_;
	};;

	template <typename T>
	class SafeVector
	{
	public:
		SafeVector() = default;
		explicit SafeVector(qint64 size) : data_(size)
		{
		}

		explicit SafeVector(const QVector<T>& other) : data_(other)
		{
		}

		explicit SafeVector(QVector<T>&& other) : data_(other)
		{
		}

		explicit SafeVector(std::initializer_list<T> args) : data_(args)
		{
		}

		SafeVector<T>& operator=(const QVector<T>& other)
		{
			QWriteLocker locker(&lock_);
			data_ = other;
			return *this;
		}

		explicit SafeVector(const SafeVector<T>& other) : data_(other.data_)
		{
		}

		SafeVector(SafeVector<T>&& other) noexcept : data_(other.data_)
		{
		}

		SafeVector<T>& operator=(SafeVector<T>&& other) noexcept
		{
			QWriteLocker locker(&lock_);
			data_ = other.data_;
			return *this;
		}

		T* operator->()
		{
			QWriteLocker locker(&lock_);
			return data_.data();
		}

		explicit SafeVector(const std::vector<T>& other) : data_(other.begin(), other.end())
		{
		}

		T& operator[](qint64 i)
		{
			QWriteLocker locker(&lock_);
			if (i < 0 || i >= data_.size())
			{
				return defaultValue_;
			}
			return data_[i];
		}

		const T& operator[](qint64 i) const
		{
			QReadLocker locker(&lock_);
			if (i < 0 || i >= data_.size())
			{
				return defaultValue_;
			}
			return data_[i];
		}

		const T at(qint64 i) const
		{
			QReadLocker locker(&lock_);
			if (i < 0 || i >= data_.size())
			{
				return defaultValue_;
			}
			return data_.at(i);
		}

		bool contains(const T& value) const
		{
			QReadLocker locker(&lock_);
			return data_.contains(value);
		}

		bool operator==(const QVector<T>& other) const
		{
			QReadLocker locker(&lock_);
			return (data_ == other);
		}

		void clear()
		{
			QWriteLocker locker(&lock_);
			data_.clear();
		}

		void append(const T& value)
		{
			QWriteLocker locker(&lock_);
			data_.append(value);
		}

		void append(const SafeVector<T>& other)
		{
			QWriteLocker locker(&lock_);
			data_.append(other.data_);
		}

		void append(const QVector<T>& other)
		{
			QWriteLocker locker(&lock_);
			data_.append(other);
		}

		void append(const std::vector<T>& other)
		{
			QWriteLocker locker(&lock_);
			data_.append(other.begin(), other.end());
		}

		void append(const std::initializer_list<T>& args)
		{
			QWriteLocker locker(&lock_);
			data_.append(args);
		}

		void push_back(const T& value)
		{
			QWriteLocker locker(&lock_);
			data_.push_back(value);
		}

		typename QVector<T>::iterator begin()
		{
			QWriteLocker locker(&lock_);
			return data_.begin();
		}

		typename QVector<T>::iterator end()
		{
			QWriteLocker locker(&lock_);
			return data_.end();
		}

		typename QVector<T>::const_iterator cbegin() const
		{
			QReadLocker locker(&lock_);
			return data_.cbegin();
		}

		typename QVector<T>::const_iterator cend() const
		{
			QReadLocker locker(&lock_);
			return data_.cend();
		}

		void resize(qint64 size)
		{
			QWriteLocker locker(&lock_);
			data_.resize(size);
		}

		qint64 size() const
		{
			QReadLocker locker(&lock_);
			return data_.size();
		}

		bool isEmpty() const
		{
			QReadLocker locker(&lock_);
			return data_.isEmpty();
		}

		QVector<T> toVector() const
		{
			QReadLocker locker(&lock_);
			return data_;
		}

		T takeFirst()
		{
			QWriteLocker locker(&lock_);
			return data_.takeFirst();
		}

		T first() const
		{
			QReadLocker locker(&lock_);
			return data_.first();
		}

		T front() const
		{
			QReadLocker locker(&lock_);
			return data_.front();
		}

		void pop_front()
		{
			QWriteLocker locker(&lock_);
			data_.pop_front();
		}

		void erase(typename QVector<T>::iterator position)
		{
			QWriteLocker locker(&lock_);
			data_.erase(position);
		}

		void erase(typename QVector<T>::iterator first, typename QVector<T>::iterator last)
		{
			QWriteLocker locker(&lock_);
			data_.erase(first, last);
		}

		T value(qint64 i) const
		{
			QReadLocker locker(&lock_);
			return data_.value(i);
		}

		virtual ~SafeVector() = default;

	private:
		QVector<T> data_;
		T defaultValue_;
		mutable QReadWriteLock lock_;
	};

	struct MapData
	{
		qint64 floor = 0;
		qint64 x = 0;
		qint64 y = 0;
		QString name = "";
	};

	//Json配置讀寫
	class Config
	{
	public:
		explicit Config(const QString& fileName);
		virtual ~Config();

		bool open();
		void sync();

		void removeSec(const QString sec);

		void write(const QString& key, const QVariant& value);
		void write(const QString& sec, const QString& key, const QVariant& value);
		void write(const QString& sec, const QString& key, const QString& sub, const QVariant& value);

		void WriteHash(const QString& sec, const QString& key, QMap<QString, QPair<bool, QString>>& hash);
		QMap<QString, QPair<bool, QString>> EnumString(const QString& sec, const QString& key) const;

		template <typename T>
		T read(const QString& sec, const QString& key, const QString& sub) const
		{
			if (!cache_.contains(sec))
			{
				return T();
			}

			QJsonObject json = cache_.value(sec).toJsonObject();
			if (!json.contains(key))
			{
				return T();
			}

			QJsonObject subjson = json.value(key).toObject();
			if (!subjson.contains(sub))
			{
				return T();
			}
			return subjson[sub].toVariant().value<T>();
		}

		template <typename T>
		T read(const QString& sec, const QString& key) const
		{
			//read json value from cache_
			if (cache_.contains(sec))
			{
				QJsonObject json = cache_.value(sec).toJsonObject();
				if (json.contains(key))
				{
					return json.value(key).toVariant().value<T>();
				}
			}
			return T();
		}

		template <typename T>
		T read(const QString& key) const
		{
			if (cache_.contains(key))
			{
				return cache_.value(key).value<T>();
			}

			return T();
		}

		template <typename T>
		QList<T> readArray(const QString& sec, const QString& key, const QString& sub) const
		{
			QList<T> result;
			QVariant variant;

			if (cache_.contains(sec))
			{
				QJsonObject json = cache_.value(sec).toJsonObject();

				if (json.contains(key))
				{
					QJsonObject subJson = json.value(key).toObject();

					if (subJson.contains(sub))
					{
						QJsonArray jsonArray = subJson.value(sub).toArray();

						for (const QJsonValue& value : jsonArray)
						{
							variant = value.toVariant();
							result.append(variant.value<T>());
						}
					}
				}
			}

			return result;
		}

		template <typename T>
		void writeArray(const QString& sec, const QString& key, const QList<T>& values)
		{
			if (!cache_.contains(sec))
			{
				cache_.insert(sec, QJsonObject());
			}

			QVariantList variantList;
			for (const T& value : values)
			{
				variantList.append(value);
			}

			QJsonObject json = cache_.value(sec).toJsonObject();
			json.insert(key, QJsonArray::fromVariantList(variantList));
			cache_.insert(sec, json);

			if (!hasChanged_)
				hasChanged_ = true;
		}

		template <typename T>
		void writeArray(const QString& sec, const QString& key, const QString& sub, const QList<T>& values)
		{
			QJsonObject json;

			if (cache_.contains(sec))
			{
				json = cache_.value(sec).toJsonObject();
			}

			QJsonArray jsonArray;

			for (const T& value : values)
			{
				jsonArray.append(value);
			}

			QJsonObject subJson;

			if (json.contains(key))
			{
				subJson = json.value(key).toObject();
			}

			subJson.insert(sub, jsonArray);
			json.insert(key, subJson);
			cache_.insert(sec, json);

			if (!hasChanged_)
				hasChanged_ = true;
		}

		void writeMapData(const QString& sec, const util::MapData& data);

		QList<util::MapData> readMapData(const QString& key) const;

	private:
		QJsonDocument document_;

		QFile file_ = {};

		QString fileName_ = "\0";

		QVariantMap cache_ = {};

		bool isVaild = false;

		bool hasChanged_ = false;
	};

	class ScopedFileLocker
	{
	public:
		explicit ScopedFileLocker(const QString& fileName)
			:lock_(fileName)
		{
			isFileLocked = lock_.tryLock();
		}

		virtual ~ScopedFileLocker()
		{
			if (isFileLocked)
			{
				lock_.unlock();
			}
		}

		bool isLocked() const
		{
			return isFileLocked;
		}

	private:
		QLockFile lock_;
		bool isFileLocked = false;
	};

	class ScopedLocker
	{
	public:
		explicit ScopedLocker(QMutex* lock)
			:lock_(*lock)
		{
			isLocked_ = lock_.tryLock();
		}

		virtual ~ScopedLocker()
		{
			if (isLocked_)
			{
				lock_.unlock();
			}
		}

		bool isLocked() const
		{
			return isLocked_;
		}



	private:
		QMutex& lock_;
		bool isLocked_ = false;

	};

	//簡易字符串加解密 主要用於將一些二進制數據轉換為可視字符串方便保存json
	class Crypt
	{
	public:
		Crypt()
			:crypto_(new SimpleCrypt(0x7f5f6c3ee5bai64))
		{

		}

		virtual ~Crypt() = default;

		bool isBase64(const QString data) const
		{
			//正則式 + 長度判斷 + 字符集判斷 + 字符串判斷 是否為 Base64
			QRegularExpression rx("^[A-Za-z0-9+/]+={0,2}$");
			return rx.match(data).hasMatch() && data.length() % 4 == 0 && data.contains(QRegularExpression("[^A-Za-z0-9+/=]")) == false;
		}

		/* 加密數據 */
		QString encryptToString(const QString& plaintext)
		{
			QString  ciphertext = crypto_->encryptToString(plaintext);
			//QString str = QEncode(ciphertext);
			return ciphertext;

		}

		QString encryptFromByteArray(QByteArray plaintext)
		{
			QString  ciphertext = crypto_->encryptToString(plaintext);
			//QString str = QEncode(ciphertext);
			return ciphertext;
		}

		/* 解密數據 */
		QString decryptToString(const QString& plaintext)
		{
			//QString str = QDecode(plaintext);
			QString ciphertext = crypto_->decryptToString(plaintext);
			return ciphertext;
		}

		QString decryptFromByteArray(QByteArray plaintext)
		{
			//QString str = QDecode(plaintext);
			QString ciphertext = crypto_->decryptToString(plaintext);
			return ciphertext;
		}

		QByteArray decryptToByteArray(const QString& plaintext)
		{
			//QString str = QDecode(plaintext);
			QByteArray ciphertext = crypto_->decryptToByteArray(plaintext);
			return ciphertext;
		}

	private:
		QScopedPointer<SimpleCrypt> crypto_;

	};

	//介面排版或配置管理 主用於保存窗口狀態
	class FormSettingManager
	{
	public:
		explicit FormSettingManager(QWidget* widget) { widget_ = widget; }
		explicit FormSettingManager(QMainWindow* widget) { mainwindow_ = widget; }

		void loadSettings();
		void saveSettings();

	private:
		QWidget* widget_ = nullptr;
		QMainWindow* mainwindow_ = nullptr;
	};

	//遠程虛擬內存申請、釋放、或申請+寫入字符串(ANSI | UNICODE)
	class VirtualMemory
	{
	public:
		enum VirtualEncode
		{
			kAnsi,
			kUnicode,
		};

		VirtualMemory() = default;

		VirtualMemory(HANDLE h, qint64 size, bool autoclear)
			: autoclear(autoclear)
			, hProcess(h)
		{

			lpAddress = mem::virtualAlloc(h, size);
		}

		VirtualMemory(HANDLE h, const QString& str, VirtualEncode use_unicode, bool autoclear)
			: autoclear(autoclear)
		{

			lpAddress = (use_unicode) == VirtualMemory::kUnicode ? (mem::virtualAllocW(h, str)) : (mem::virtualAllocA(h, str));
			hProcess = h;
		}

		inline operator quint64() const
		{
			return this->lpAddress;
		}

		VirtualMemory& operator=(qint64 other)
		{
			this->lpAddress = other;
			return *this;
		}

		VirtualMemory* operator&()
		{
			return this;
		}

		// copy constructor
		VirtualMemory(const VirtualMemory& other)
			: autoclear(other.autoclear)
			, lpAddress(other.lpAddress)
		{
		}
		//copy assignment
		VirtualMemory& operator=(const VirtualMemory& other)
		{
			this->lpAddress = other.lpAddress;
			this->autoclear = other.autoclear;
			this->hProcess = other.hProcess;
			return *this;
		}
		// move constructor
		VirtualMemory(VirtualMemory&& other) noexcept
			: autoclear(other.autoclear)
			, lpAddress(other.lpAddress)
			, hProcess(other.hProcess)

		{
			other.lpAddress = 0;
		}
		// move assignment
		VirtualMemory& operator=(VirtualMemory&& other) noexcept
		{
			this->lpAddress = other.lpAddress;
			this->autoclear = other.autoclear;
			this->hProcess = other.hProcess;
			other.lpAddress = 0;
			return *this;
		}

		friend constexpr inline bool operator==(const VirtualMemory& p1, const VirtualMemory& p2)
		{
			return p1.lpAddress == p2.lpAddress;
		}
		friend constexpr inline bool operator!=(const VirtualMemory& p1, const VirtualMemory& p2)
		{
			return p1.lpAddress != p2.lpAddress;
		}

		virtual ~VirtualMemory()
		{
			do
			{
				if ((autoclear) && (this->lpAddress) && (hProcess))
				{

					mem::write<unsigned char>(hProcess, this->lpAddress, 0);
					mem::virtualFree(hProcess, this->lpAddress);
					this->lpAddress = NULL;
				}
			} while (false);
		}

		Q_REQUIRED_RESULT inline bool isNull() const
		{
			return !lpAddress;
		}

		Q_REQUIRED_RESULT inline bool isData(BYTE* data, qint64 size) const
		{
			QScopedArrayPointer <BYTE> _data(data);
			mem::read(hProcess, lpAddress, size, _data.data());
			return memcmp(data, _data.data(), size) == 0;
		}

		inline void clear()
		{

			if ((this->lpAddress))
			{
				mem::virtualFree(hProcess, (this->lpAddress));
				this->lpAddress = NULL;
			}
		}

		Q_REQUIRED_RESULT inline bool isValid()const
		{
			return (this->lpAddress) != NULL;
		}

	private:
		bool autoclear = false;
		quint64 lpAddress = NULL;
		HANDLE hProcess = NULL;
	};

	//智能文件句柄類
	class ScopedFile : public QFile
	{
	public:
		ScopedFile(const QString& filename, OpenMode ioFlags)
			: QFile(filename)
		{
			QFile::open(ioFlags);
		}

		virtual ~ScopedFile()
		{
			if (m_maps.size())
			{
				for (uchar* map : m_maps)
				{
					QFile::unmap(map);
				}
				m_maps.clear();
			}
			if (QFile::isOpen())
				QFile::close();
		}

		template <typename T>
		bool __fastcall  mmap(T*& p, qint64 offset, qint64 size, QFile::MemoryMapFlags flags = QFileDevice::MapPrivateOption)//QFile::NoOptions
		{
			uchar* uc = QFile::map(offset, size, flags);
			if (uc)
			{
				m_maps.insert(uc);
				p = reinterpret_cast<T*>(uc);
			}
			return uc != nullptr;
		}

		QFile& __fastcall  file() { return *this; }

	private:
		QSet<uchar*> m_maps;
	};

	class TextStream : public QTextStream
	{
	public:
		explicit TextStream(FILE* file)
			: QTextStream(file)
		{
			init();
		}

		explicit TextStream(QIODevice* device)
			: QTextStream(device)
		{
			init();
	}

	private:
		void init()
		{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			setCodec(util::DEFAULT_CODEPAGE);
#else
			setEncoding(QStringConverter::Utf8);
#endif
			setGenerateByteOrderMark(true);
		}
};

	//用於掛機訊息紀錄
	typedef struct tagAfkRecorder
	{
		qint64 levelrecord = 0;
		qint64 leveldifference = 0;
		qint64 exprecord = 0;
		qint64 expdifference = 0;
		qint64 goldearn = 0;
		qint64 deadthcount = 0;
		qint64 reprecord = 0;
		qint64 repearn = 0;
		bool deadthcountflag = false;
	}AfkRecorder;
	//#pragma warning(push)
	//#pragma warning(disable:304)
	//	Q_DECLARE_METATYPE(VirtualMemory);
	//	Q_DECLARE_TYPEINFO(VirtualMemory, Q_MOVABLE_TYPE);
	//#pragma warning(pop)

	static const QRegularExpression rexSpace(R"([ ]+)");
	static const QRegularExpression rexOR(R"(\s*\|\s*)");
	static const QRegularExpression rexComma(R"(\s*,\s*)");
	static const QRegularExpression rexSemicolon(R"(\s*;\s*)");
	static const QRegularExpression rexColon(R"(\s*:\s*)");
	static const QRegularExpression rexDec(R"(\s*-\s*)");
	//(\('[\w\p{Han}]+'\))
	static const QRegularExpression rexQuoteWithParentheses(R"(\('?([\w\W\S\s\p{Han}]+)'?\))");
	//\[(\d+)\]
	static const QRegularExpression rexSquareBrackets(R"(\[(\d+)\])");
	//([\w\p{Han}]+)\[(\d+)\]

}

