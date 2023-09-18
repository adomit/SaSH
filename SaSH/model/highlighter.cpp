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
#include <highlighter.h>

Highlighter::Highlighter(QObject* parent)
	: QsciLexerLua(parent)
	, m_font("YaHei Consolas Hybrid", 10, 570/*QFont::DemiBold*/, false)
{
}

const char* Highlighter::keywords(int set) const
{
	//switch (set)
	//{
	//case 1://粉色
	//{
	//	//lua key word
	//	return "goto call function end pause exit label jmp return back break for continue "
	//		"if ifmap ifplayer ifpet ifpetex ifitem ifteam ifitemfull ifdaily ifbattle ifpos ifonline ifnormal "
	//		"waitdlg waitsay waititem waitmap waitteam "
	//		"while repeat until do in then else elseif ";
	//}
	//case 2://QsciLexerLua::BasicFunctions//黃色
	//{
	//	return "ocr dlg "
	//		"print sleep timer msg logout logback eo button say input menu "
	//		"talk cls set saveset loadset "
	//		"chpet chplayername chpetname chmap "
	//		"usemagic doffpet buy sell useitem doffitem swapitem pickup put "
	//		"get putpet getpet make cook uequip requip "
	//		"wequip pequip puequip skup join leave kick move "
	//		"walkpos w dir findpath movetonpc lclick rclick ldbclick dragto warp "
	//		"learn trade run dostring sellpet mail reg "
	//		"regex rex rexg upper lower half toint tostr todb replace find full "
	//		"bh bj bp bs be bd bi bn bw bwf bwait bend "
	//		"dofile createch delch doffstone send ";
	//}
	//case 3://QsciLexerLua::StringTableMathsFunction//草綠
	//{
	//	return "string table ";
	//}
	//case 4://QsciLexerLua::CoroutinesIOSystemFacilities//青綠
	//{
	//	return "";
	//}
	//case 5://KeywordSet5//深藍色
	//{
	//	return "local var delete releaseall rnd true false any format "
	//		"int double bool not and or nil ";
	//}
	//case 6://KeywordSet6//淺藍色
	//{
	//	return "out player pet magic skill petskill equip petequip map dialog chat point battle char ";
	//}
	//case 7://KeywordSet7//土橘色
	//{
	//	return "! ";
	//}
	//case 8://KeywordSet8//紫色
	//{
	//	return "_GAME_ _WORLD_ vret _IFEXPR _IFRESULT _LUARESULT _LUAEXPR";
	//}
	//case 9:
	//	return "";

	//default:
	//	break;
	//}

	return QsciLexerLua::keywords(set);
}

// Returns the foreground colour of the text for a style.
QColor Highlighter::defaultColor(int style) const
{
	//switch (style)
	//{
	//	//綠色
	//case Comment:
	//case LineComment:
	//	return QColor(106, 153, 85);

	//	//草綠
	//case Number:
	//	return QColor(181, 206, 168);

	//	//粉色
	//case Keyword:
	//	return QColor(197, 134, 192);

	//	//土黃色
	//case BasicFunctions:
	//	return QColor(220, 220, 170);

	//	//草綠
	//case CoroutinesIOSystemFacilities:
	//	return QColor(181, 206, 168);

	//	//土橘色
	//case Label:
	//case String:
	//case Character:
	//case LiteralString:
	//case Preprocessor:
	//	return QColor(206, 145, 120);

	//case KeywordSet7:
	//case Operator:
	//	return QColor(206, 145, 0);

	//	//灰白色
	//case Default:
	//case Identifier:
	//	return QColor(212, 212, 212);

	//	//乳藍色

	//	//return QColor(77, 177, 252);

	//	//青綠
	//case StringTableMathsFunctions:
	//case UnclosedString:
	//	return QColor(78, 201, 176);

	//	//深藍色
	//case KeywordSet5:
	//	return QColor(86, 156, 214);

	//	//淺藍色
	//case KeywordSet6:
	//	return QColor(156, 220, 254);

	//	//紫色
	//case KeywordSet8:
	//	return QColor(190, 183, 255);

	//case NewlineArrow:
	//	return QColor(80, 80, 255);

	//default:
	//	break;
	//}

	return QsciLexerLua::defaultColor(style);
}

// Returns the font of the text for a style.
QFont Highlighter::defaultFont(int n) const
{
	//return m_font;
	return QsciLexerLua::defaultFont(n);
}

// Return the set of character sequences that can separate auto-completion
// words.
QStringList Highlighter::autoCompletionWordSeparators() const
{
	return QsciLexerLua::autoCompletionWordSeparators();
}

// Returns the background colour of the text for a style.
QColor Highlighter::defaultPaper(int style) const
{
	//return QColor(30, 30, 30);
	return QsciLexerLua::defaultPaper(style);
}

// Default implementation to return the set of fill up characters that can end
// auto-completion.
const char* Highlighter::autoCompletionFillups() const
{
	return QsciLexerLua::autoCompletionFillups();
}

// Return the list of characters that can end a block.
const char* Highlighter::blockEnd(int* style) const
{
	if (style)
		*style = Operator;

	return QsciLexerLua::blockEnd(style);
}

const char* Highlighter::blockStartKeyword(int* style) const
{
	if (style)
		*style = Keyword;

	return QsciLexerLua::blockStartKeyword(style);
}

// Return the list of characters that can start a block.
const char* Highlighter::blockStart(int* style) const
{
	if (style)
		*style = Operator;

	return QsciLexerLua::blockStart(style);
}

// Return the string of characters that comprise a word.
const char* Highlighter::wordCharacters() const
{
	//return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_#";
	return QsciLexerLua::wordCharacters();
}

// Returns the end-of-line fill for a style.
bool Highlighter::defaultEolFill(int style) const
{
	//switch (style)
	//{
	//case InactiveUnclosedString:
	//case InactiveVerbatimString:
	//case InactiveRegex:
	//case TripleQuotedVerbatimString:
	//case InactiveTripleQuotedVerbatimString:
	//case HashQuotedString:
	//case InactiveHashQuotedString:
	//case Comment:
	//case UnclosedString:
	//	return true;
	//}

	return QsciLexerLua::defaultEolFill(style);
}