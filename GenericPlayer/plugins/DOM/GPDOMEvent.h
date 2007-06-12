/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/* gdome.c
 *
 * Copyright (C) 1999 Raph Levien <raph@acm.org>
 * Copyright (C) 2000 Mathieu Lacage <mathieu@gnu.org>
 * CopyRight (C) 2001 Paolo Casarini <paolo@casarini.org>
 *
 * This file is generated automatically.  To make changes, edit
 * test/apigen/core.xml
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef GPDOMEvent_h
#define GPDOMEvent_h

/*Based on DOM events*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Based on DOM Event
 * http://www.w3.org/TR/DOM-Level-2-Events/events.html
 * http://www.w3.org/TR/2003/NOTE-DOM-Level-3-Events-20031107/events.html
 */

#define DOMEventDef "\
\
dom.DOMException(\
    dom.DOMException.value(uint))\
    dom.DOMException.impl(void*))\
\
dom.DocumentEvent.CreateEventFunc({dom.Document}\
    dom.DocumentEvent.CreateEventFunc.result(dom.Event*)\
    dom.DocumentEvent.CreateEventFunc.exception(dom.DOMException*)\
    dom.DocumentEvent.CreateEventFunc.eventType(uchar*))\
\
dom.DocumentEvent.CanDispatchFunc({dom.Document} \
    dom.DocumentEvent.CanDispatchFunc.result(dom.Event*)\
    dom.DocumentEvent.CreateEventFunc.type(uchar*))\
\
dom.EventTarget(\
    dom.EventTarget.type(dom.GPType*))\
    dom.EventTarget.impl(void*))\
\
dom.EventException(\
    dom.EventException.value(uint))\
    dom.EventException.impl(void*))\
\
dom.EventTarget.AddEventListenerFunc({dom.EventTarget}\
    dom.EventTarget.AddEventListenerFunc.type(uchar*)\
    dom.EventTarget.AddEventListenerFunc.listener(dom.EventListener*)\
    dom.EventTarget.AddEventListenerFunc.useCapture(bool))\
\
dom.EventTarget.RemoveEventListenerFunc({dom.EventTarget}\
    dom.EventTarget.RemoveEventListenerFunc.type(uchar*)\
    dom.EventTarget.RemoveEventListenerFunc.listener(dom.EventListener*)\
    dom.EventTarget.RemoveEventListenerFunc.useCapture(bool))\
\
dom.EventTarget.DispatchEventFunc({dom.EventTarget}\
    dom.EventTarget.DispatchEventFunc.result(bool*)\
    dom.EventTarget.DispatchEventFunc.exception(dom.EventException*)\
    dom.EventTarget.DispatchEventFunc.evt(dom.Event*))\
\
dom.Event(\
    dom.Event.gpType(gp.GPType*)\
    dom.Event.type(uchar*)\
    dom.Event.id(uint)\
    dom.Event.target(dom.EventTarget*)\
    dom.Event.currentTarget(dom.EventTarget*)\
    dom.Event.eventPhase(ushort)\
    dom.Event.bubbles(bool)\
    dom.Event.cancelable(bool)\
    dom.Event.timeStamp(dom.DOMTimeStamp)\
    dom.Event.namespaceURI(uchar*)\
    dom.Event.stopPropagation(bool)\
    dom.Event.defaultPrevented(bool))\
\
dom.Event.StopPropagationFunc({dom.Event})\
\
dom.Event.PreventDefaultFunc({dom.Event})\
\
dom.Event.IsCustomFunc({dom.Event}\
    dom.Event.IsCustomFunc.result(bool))\
\
dom.Event.IsDefaultPreventedFunc({dom.Event}\
    dom.Event.IsDefaultPreventedFunc.result(bool))\
\
dom.Event.StopImmediatePropagationFunc({dom.Event})\
\
dom.Event.InitEventFunc({dom.Event}\
    dom.Event.InitEventFunc.eventType(uchar*)\
    dom.Event.InitEventFunc.canBubble(bool)\
    dom.Event.InitEventFunc.cancelable(bool))\
\
dom.Event.InitEventNSFunc({dom.Event}\
    dom.Event.InitEventNSFunc.namespaceURI(uchar*)\
    dom.Event.InitEventNSFunc.eventType(uchar*)\
    dom.Event.InitEventNSFunc.canBubble(bool)\
    dom.Event.InitEventNSFunc.cancelable(bool))\
\
dom.UIEvent(\
  (dom.Event)\
  dom.UIEvent.view(dom.views.AbstractView*)\
  dom.UIEvent.detail(long))\
\
dom.UIEvent.InitUIEventFunc({dom.UIEvent}\
    dom.UIEvent.InitUIEventFunc.type(uchar*)\
    dom.UIEvent.InitUIEventFunc.canBubble(bool)\
    dom.UIEvent.InitUIEventFunc.cancelable(bool)\
    dom.UIEvent.InitUIEventFunc.view(dom.views.AbstractView*)\
    dom.UIEvent.InitUIEventFunc.detail(long))\
\
dom.MouseEvent(\
    (dom.UIEvent)\
    dom.MouseEvent.screenX(int)\
    dom.MouseEvent.screenY(int)\
    dom.MouseEvent.clientX(int)\
    dom.MouseEvent.clientY(int)\
    dom.MouseEvent.ctrlKey(bool)\
    dom.MouseEvent.shiftKey(bool)\
    dom.MouseEvent.altKey(bool)\
    dom.MouseEvent.metaKey(bool)\
    dom.MouseEvent.button(uint)\
    dom.MouseEvent.relatedTarget(dom.EventTarget*))\
\
dom.InitMouseEventFunc({dom.MouseEvent}\
    dom.InitMouseEventFunc.type(uchar*)\
    dom.InitMouseEventFunc.canBubble(bool)\
    dom.InitMouseEventFunc.cancelable(bool)\
    dom.InitMouseEventFunc.view(views.AbstractView*)\
    dom.InitMouseEventFunc.detail(int)\
    dom.InitMouseEventFunc.screenX(int)\
    dom.InitMouseEventFunc.screenY(int)\
    dom.InitMouseEventFunc.clientX(int)\
    dom.InitMouseEventFunc.clientY(int)\
    dom.InitMouseEventFunc.ctrlKey(bool)\
    dom.InitMouseEventFunc.shiftKey(bool)\
    dom.InitMouseEventFunc.altKey(bool)\
    dom.InitMouseEventFunc.metaKey(bool)\
    dom.InitMouseEventFunc.button(uint)\
    dom.InitMouseEventFunc.relatedTarget(dom.EventTarget*))\
dom.KeyboardEvent(\
    (dom.UIEvent)\
    dom.KeyboardEvent.keyIdentifier(uchar*)\
    dom.KeyboardEvent.keyLocation(uint)\
    dom.InitMouseEventFunc.ctrlKey(bool)\
    dom.InitMouseEventFunc.shiftKey(bool)\
    dom.InitMouseEventFunc.altKey(bool)\
    dom.InitMouseEventFunc.metaKey(bool))\
    dom.InitMouseEventFunc.keyCode(uint))\
\
dom.KeyboardEvent.GetModifierStateFunc({dom.KeyboardEvent}\
    dom.KeyboardEvent.getModifierStateFunc.result(bool)\
    dom.KeyboardEvent.getModifierStateFunc.keyIdentifier(uchar))\
\
dom.KeyboardEvent.InitKeyboardEventFunc({dom.KeyboardEvent}\
    dom.KeyboardEvent.InitKeyboardEventFunc.type(uchar*)\
    dom.KeyboardEvent.InitKeyboardEventFunc.canBubble(bool)\
    dom.KeyboardEvent.InitKeyboardEventFunc.cancelable(bool)\
    dom.KeyboardEvent.InitKeyboardEventFunc.view(dom.AbstractView*)\
    dom.KeyboardEvent.InitKeyboardEventFunc.keyIdentifier(uchar*)\
    dom.KeyboardEvent.InitKeyboardEventFunc.keyLocaton(uint)\
    dom.KeyboardEvent.InitKeyboardEventFunc.modifiersList(uchar*))\
\
dom.KeyboardEvent.InitKeyboardEventNSFunc({dom.KeyboardEvent}\
    dom.KeyboardEvent.InitKeyboardEventNSFunc.namespaceURI(uchar*)\
    dom.KeyboardEvent.InitKeyboardEventNSFunc.type(uchar*)\
    dom.KeyboardEvent.InitKeyboardEventNSFunc.canBubble(bool)\
    dom.KeyboardEvent.InitKeyboardEventNSFunc.cancelable(bool)\
    dom.KeyboardEvent.InitKeyboardEventNSFunc.view(dom.AbstractView*)\
    dom.KeyboardEvent.InitKeyboardEventNSFunc.keyIdentifier(uchar*)\
    dom.KeyboardEvent.InitKeyboardEventNSFunc.keyLocaton(uint)\
    dom.KeyboardEvent.InitKeyboardEventNSFunc.modifiersList(uchar*))\
\
dom.CustomEvent(\
    (dom.Event))\ 
\
dom.CustomEvent.SetDispatchStateFunc({dom.CustomEvent}\
    dom.CustomEvent.SetDispatchStateFunc.target(dom.EventTarget*)\ 
    dom.CustomEvent.SetDispatchStateFunc.phase(short))\ 
\
dom.CustomEvent.IsPropagationStoppedFunc({dom.CustomEvent}\
    dom.CustomEvent.IsPropagationStoppedFunc.result(bool))\
dom.CustomEvent.IsImmediatePropagationStoppedFunc(\
    dom.CustomEvent.IsImmediatePropagationStoppedFunc.result(bool))\

#defind GP_UNSPECIFIED_EVENT_TYPE_ERR 0
#define GP_NOT_SUPPORTED_ERR 1

#define GP_CAPTURING_PHASE 1
#define GP_AT_TARGET 2
#define GP_BUBBLING_PHASE 3


#define    GP_KEY_LOCATION_STANDARD   0
#define    GP_KEY_LOCATION_LEFT       1
#define    GP_KEY_LOCATION_RIGHT      2
#define    GP_KEY_LOCATION_NUMPAD     3
#define    GP_KEY_LOCATION_TOP        4 /*Extension not standard*/
#define    GP_KEY_LOCATION_BOTTOM     5 /*Extension not standard*/
#define    GP_KEY_LOCATION_LEFT_SIDE  6 /*Extension not standard*/
#define    GP_KEY_LOCATION_RIGHT_SIDE 7 /*Extension not standard*/

#define GP_CREATE_UIEVENT "UIEvents"
#define GP_CREATE_HTMLEVENT "HTMLEvents"
#define GP_CREATE_MOUSEEVENT "MouseEvents"
#define GP_CREATE_KEYBOARDEVENT "KeyboardEvent"

#define GP_HTMLEVENT_LOAD "load"
#define GP_HTMLEVENT_UNLOAD "unload"
#define GP_HTMLEVENT_ABORT "abort"
#define GP_HTMLEVENT_ERROR "error"
#define GP_HTMLEVENT_SELECT "select"
#define GP_HTMLEVENT_CHANGE "change"
#define GP_HTMLEVENT_RESET "reset"
#define GP_HTMLEVENT_FOCUS "focus"
#define GP_HTMLEVENT_BLUR "blur"
#define GP_HTMLEVENT_RESIZE "resize"
#define GP_HTMLEVENT_SCROLL "scroll"

#define GP_UIEVENT_FOCUS_IN "DOMFocusIn"
#define GP_UIEVENT_FOCUS_OUT "DOMFocusOut"
#define GP_UIEVENT_ACTIVATE  "DOMActivate"

#define GP_MOUSEEVENT_CLICK  "click"
#define GP_MOUSEEVENT_DOWN  "mousedown"
#define GP_MOUSEEVENT_UP  "mouseup"
#define GP_MOUSEEVENT_OVER  "mouseover"
#define GP_MOUSEEVENT_MOVE  "mousemove"
#define GP_MOUSEEVENT_OUT  "mouseout"
#define GP_MOUSEEVENT_OUT  "mousescroll" /*Extension*/


/*The Accept (Commit) key.*/
#define GPK_ACCEPT "Accept"

/*The Again key.*/
#define GPK_AGAIN "Again"

/*The All Candidates key.*/
#define GPK_ALL_CANDIDATES "AllCandidates"

/*The Alphanumeric key.*/
#define GPK_ALPHANUMERIC "Alphanumeric"

/*The Alt (Menu) key.*/
#define GPK_ALT "Alt"

/*The Alt-Graph key.*/
#define GPK_ALT_GRAPH "AltGraph"

/*The Application key.*/
#define GPK_ALT_APPS "Apps"

/*The ATTN key.*/
#define GPK_ALT_ATTN "Attn"

/*The Browser Back key.*/
#define GPK_ALT_BROWSER_BACK "BrowserBack"

/*The Browser Favorites key.*/
#define GPK_ALT_BROWSER_FAVORITES "BrowserFavorites"

/*The Browser Forward key.*/
#define GPK_ALT_BROWSER_FORWARD "BrowserForward"

/*The Browser Home key.*/
#define GPK_ALT_BROWSER_HOME "BrowserHome"

/*The Browser Refresh key.*/
#define GPK_ALT_BROWSER_REFRESH "BrowserRefresh"

/*The Browser Search key.*/
#define GPK_ALT_BROWSER_SEARCH "BrowserSearch"

/*The Browser Stop key.*/
#define GPK_ALT_BROWSER_STOP "BrowserStop"

/*The Caps Lock (Capital) key.*/
#define GPK_ALT_CAPSLOCK "CapsLock"

/*The Clear key.*/
#define GPK_ALT_CLEAR "Clear"

/*The Code Input key.*/
#define GPK_ALT_CODE_INPUT "CodeInput"

/*The Compose key.*/
#define GPK_ALT_COMPOSE "Compose"

/*The Control (Ctrl) key.*/
#define GPK_ALT_CONTROL "Control"

/*The Crsel key.*/
#define GPK_ALT_CRSEL "Crsel"

/*The Convert key.*/
#define GPK_ALT_CONVERT "Convert"

/*The Copy key.*/
#define GPK_ALT_COPY "Copy"

/*The Cut key.*/
#define GPK_CUT "Cut"

/*The Down Arrow key.*/
#define GPK_DOWN "Down"

/*The End key.*/
#define GPK_END "End"

/*The Enter key.
  Note: This key identifier is also used for the Return (Macintosh
  numpad) key.
*/
#define GPK_ENTER "Enter"

/*The Erase EOF key.*/
#define GPK_ERASE_EOF "EraseEof"

/*The Execute key.*/
#define GPK_EXECUTE "Execute"

/*The Exsel key.*/
#define GPK_EXSEL "Exsel"

/*The F1 key.*/
#define GPK_F1 "F1"

/*The F2 key.*/
#define GPK_F2 "F2"

/*The F3 key.*/
#define GPK_F3 "F3"

/*The F4 key.*/
#define GPK_F4 "F4"

/*The F5 key.*/
#define GPK_F5 "F5"

/*The F6 key.*/
#define GPK_F6 "F6"

/*The F7 key.*/
#define GPK_F7 "F7"

/*The F8 key.*/
#define GPK_F8 "F8"

/*The F9 key.*/
#define GPK_F9 "F9"

/*The F10 key.*/
#define GPK_F10 "F10"

/*The F11 key.*/
#define GPK_F11 "F11"

/*The F12 key.*/
#define GPK_F12 "F12"

/*The F13 key.*/
#define GPK_F13 "F13"

/*The F14 key.*/
#define GPK_F14 "F14"

/*The F15 key.*/
#define GPK_F15 "F15"

/*The F16 key.*/
#define GPK_F16 "F16"

/*The F17 key.*/
#define GPK_F17 "F17"

/*The F18 key.*/
#define GPK_F18 "F18"

/*The F19 key.*/
#define GPK_F19 "F19"

/*The F20 key.*/
#define GPK_F20 "F20"

/*The F21 key.*/
#define GPK_F21 "F21"

/*The F22 key.*/
#define GPK_F22 "F22"

/*The F23 key.*/
#define GPK_F23 "F23"

/*The F24 key.*/
#define GPK_F24 "F24"

/*The Final Mode (Final) key used on some asian keyboards.
#define GPK_FINAL_MODE "FinalMode"

/*The Find key.*/
#define GPK_FIND "Find"

/*The Full-Width Characters key.*/
#define GPK_FULL_WIDTH "FullWidth"

/*The Half-Width Characters key.*/
#define GPK_HALF_WIDTH "HalfWidth"

/*The Hangul (Korean characters) Mode key.*/
#define GPK_HANGUL_MODE "HangulMode"

/*The Hanja (Korean characters) Mode key.*/
#define GPK_HANJA_MODE "HanjaMode"

/*The Help key.*/
#define GPK_HELP "Help"

/*The Hiragana (Japanese Kana characters) key.*/
#define GPK_HIRAGANA "Hiragana"

/*The Home key.*/
#define GPK_HOME "Home"

/*The Insert (Ins) key.*/
#define GPK_INSERT "Insert"

/*The Japanese-Hiragana key.*/
#define GPK_JAPANESE_HIRAGANA "JapaneseHiragana"

/*The Japanese-Katakana key.*/
#define GPK_JAPANESE_KATAKANA "JapaneseKatakana"

/*The Japanese-Romaji key.*/
#define GPK_JAPANESE_ROMAJI "JapaneseRomaji"

/*The Junja Mode key.*/
#define GPK_JUNJA_MODE "JunjaMode"

/*The Kana Mode (Kana Lock) key.*/
#define GPK_KANA_MODE "KanaMode"

/*The Kanji (Japanese name for ideographic characters of Chinese
    origin) Mode key.*/
#define GPK_KANJI_MODE  "KanjiMode"

/*The Katakana (Japanese Kana characters) key.*/
#define GPK_KATAKANA  "Katakana"

/*The Start Application One key.*/
#define GPK_LAUNCH_APPLICATION1 "LaunchApplication1"

/*The Start Application Two key.*/
#define GPK_LAUNCH_APPLICATION2 "LaunchApplication2"

/*The Start Mail key.*/
#define GPK_LAUNCH_MAIL "LaunchMail"

/*The Left Arrow key.*/
#define GPK_LEFT "Left"

/*The Meta key.*/
#define GPK_META  "Meta"

/*The Media Next Track key.*/
#define GPK_MEDIA_NEXT_TRACK " "MediaNextTrack"

/*The Media Play Pause key.*/
#define GPK_MEDIA_PLAY_PAUSE "MediaPlayPause"

/*The Media Previous Track key.*/
#define GPK_MEDIA_PREVIOUS_TRACK  "MediaPreviousTrack"

/*The Media Stok key.*/
#define GPK_MEDIA_STOP "MediaStop"

/*The Mode Change key.*/
#define GPK_MEDIA_MODE_CHANGE "ModeChange"

/*The Nonconvert (Don't Convert) key.*/
#define GPK_NONCONVERT "Nonconvert"

/*The Num Lock key.*/
#define GPK_NUMLOCK "NumLock"

/*The Page Down (Next) key.*/
#define GPK_PAGE_DOWN "PageDown"

/*The Page Up key.*/
#define GPK_PAGE_UP "PageUp"

/*The Paste key.*/
#define GPK_PASTE "Paste"

/*The Pause key.*/
#define GPK_PAUSE "Pause"

/*The Play key.*/
#define GPK_PLAY "Play"

/*The Previous Candidate function key.*/
#define GPK_PREVIOUS_CANDIDATE "PreviousCandidate"

/*The Print Screen (PrintScrn, SnapShot) key.*/
#define GPK_PRINT_SCREEN "PrintScreen"

/*The Process key.*/
#define GPK_PROCESS "Process"

/*The Props key.*/
#define GPK_PROPS "Props"

/*The Right Arrow key.*/
#define GPK_RIGHT "Right"

/*The Roman Characters function key.*/
#define GPK_ROMAN_CHARACTERS "RomanCharacters"

/*The Scroll Lock key.*/
#define GPK_SCROLL "Scroll"

/*The Select key.*/
#define GPK_SELECT "Select"

/*The Select Media key.*/
#define GPK_SELECT_MEDIA "SelectMedia"

/*The Shift key.*/
#define GPK_SHIFT "Shift"

/*The Stop key.*/
#define GPK_STOP "Stop"

/*The Up Arrow key.*/
#define GPK_UP "Up"

/*The Undo key.*/
#define GPK_UNDO "Undo"

/*The Volume Down key.*/
#define GPK_VOLUMEDOWN "VolumeDown"

/*The Volume Mute key.*/
#define GPK_VOLUMEMUTE "VolumeMute"

/*The Volume Up key.*/
#define GPK_VOLUMEUP "VolumeUp"

/*The Windows Logo key.*/
#define GPK_WIN "Win"

/*The Zoom key.*/
#define GPK_ZOOM "Zoom"

/*The Backspace (Back) key.*/
#define GPK_BACKSPACE "U+0008"
/*The Horizontal Tabulation (Tab) key.*/
#define GPK_TAB "U+0009"

/*The Cancel key.*/
#define GPK_CANCEL "U+0018"

/*The Escape (Esc) key.*/
#define GPK_ESCAPE "U+001B"

/*The Space (Spacebar) key.*/
#define GPK_SPACE "U+0020"

/*The Exclamation Mark (Factorial, Bang) key (!).*\
#define GPK_EXCLAMATION "U+0021"

/*The Quotation Mark (Quote Double) key (").*\
#define GPK_DOUBLE_QUOTE "U+0022"

/*The Number Sign (Pound Sign, Hash, Crosshatch, Octothorpe) key
    (#).*\
#define GPK_HASH "U+0023"

/*The Dollar Sign (milreis, escudo) key ($).*\
#define GPK_DOLLAR "U+0024"

/*The Ampersand key (&).*\
#define GPK_AMPERSAND "U+0026"
/*The Apostrophe (Apostrophe-Quote, APL Quote) key (').*\
#define GPK_QUOTE "U+0027"

/*The Left Parenthesis (Opening Parenthesis) key (().*\
#define GPK_LEFT_PAREN "U+0028"

/*The Right Parenthesis (Closing Parenthesis) key ()).*\
#define GPK_RIGHT_PAREN "U+0029"

/*The Asterix (Star) key (*).*\
#define GPK_STAR "U+002A"

/*The Plus Sign (Plus) key (+).*\
#define GPK_PLUS "U+002B"

/*The Comma (decimal separator) sign key (,).*\
#define GPK_COMMA "U+002C"

/*The Hyphen-minus (hyphen or minus sign) key (-).*\
#define GPK_MINUS "U+002D"

/*The Full Stop (period, dot, decimal point) key (.).*\
#define GPK_PERIOD "U+002E"

/*The Solidus (slash, virgule, shilling) key (/).*\
#define GPK_SLASH "U+002F"

/*The Digit Zero key (0).*\
#define GPK_ZERO "U+0030"

/*The Digit One key (1).*\
#define GPK_1 "U+0031"

/*The Digit Two key (2).*\
#define GPK_2 "U+0032"

/*The Digit Three key (3).*\
#define GPK_3 "U+0033"

/*The Digit Four key (4).*\
#define GPK_4 "U+0034"

/*The Digit Five key (5).*\
#define GPK_5 "U+0035"

/*The Digit Six key (6).*\
#define GPK_6 "U+0036"

/*The Digit Seven key (7).*\
#define GPK_7 "U+0037"

/*The Digit Eight key (8).*\
#define GPK_8 "U+0038"

/*The Digit Nine key (9).*\
#define GPK_9 "U+0039"

/*The Colon key (:).*\
#define GPK_COLON "U+003A"

/*The Semicolon key (;).*\
#define GPK_SEMICOLON "U+003B"

/*The Less-Than Sign key (<).*\
#define GPK_LESS_THAN "U+003C"

/*The Equals Sign key (=).*\
#define GPK_EQUALS "U+003D"

/*The Greater-Than Sign key (>).*\
#define GPK_GREATER_THAN "U+003E"

/*The Question Mark key (?).*\
#define GPK_QUESTION_MARK "U+003F"

/*The Commercial At (@) key.*/
#define GPK_AT "U+0040"

/*The Latin Capital Letter A key (A).*\
#define GPK_A "U+0041"

/*The Latin Capital Letter B key (B).*\
#define GPK_B "U+0042"

/*The Latin Capital Letter C key (C).*\
#define GPK_C "U+0043"

/*The Latin Capital Letter D key (D).*\
#define GPK_D "U+0044"

/*The Latin Capital Letter E key (E).*\
#define GPK_E "U+0045"

/*The Latin Capital Letter F key (F).*\
#define GPK_F "U+0046"

/*The Latin Capital Letter G key (G).*\
#define GPK_G "U+0047"

/*The Latin Capital Letter H key (H).*\
#define GPK_H "U+0048"

/*The Latin Capital Letter I key (I).*\
#define GPK_I "U+0049"

/*The Latin Capital Letter J key (J).*\
#define GPK_J "U+004A"

/*The Latin Capital Letter K key (K).*\
#define GPK_K "U+004B"

/*The Latin Capital Letter L key (L).*\
#define GPK_L "U+004C"

/*The Latin Capital Letter M key (M).*\
#define GPK_M "U+004D"

/*The Latin Capital Letter N key (N).*\
#define GPK_N "U+004E"

/*The Latin Capital Letter O key (O).*\
#define GPK_O "U+004F"

/*The Latin Capital Letter P key (P).*\
#define GPK_P "U+0050"

/*The Latin Capital Letter Q key (Q).*\
#define GPK_Q "U+0051"

/*The Latin Capital Letter R key (R).*\
#define GPK_R "U+0052"

/*The Latin Capital Letter S key (S).*\
#define GPK_S "U+0053"

/*The Latin Capital Letter T key (T).*\
#define GPK_T "U+0054"

/*The Latin Capital Letter U key (U).*\
#define GPK_U "U+0055"

/*The Latin Capital Letter V key (V).*\
#define GPK_V "U+0056"

/*The Latin Capital Letter W key (W).*\
#define GPK_W "U+0057"

/*The Latin Capital Letter X key (X).*\
#define GPK_X "U+0058"

/*The Latin Capital Letter Y key (Y).*\
#define GPK_Y "U+0059"

/*The Latin Capital Letter Z key (Z).*\
#define GPK_Z "U+005A"

/*The Left Square Bracket (Opening Square Bracket) key ([).*\
#define GPK_LEFT_BRACKET "U+005B"

/*The Reverse Solidus (Backslash) key (\).*\
#define GPK_BACKLSASH "U+005C"

/*The Right Square Bracket (Closing Square Bracket) key (]).*\
#define GPK_RIGHT_BRACKET "U+005D"

/*The Circumflex Accent key (^).*\
#define GPK_CIRCUMFLEX "U+005E"

/*The Low Sign (Spacing Underscore, Underscore) key (_).*\
#define GPK_UNDERSCORE "U+005F"

/*The Grave Accent (Back Quote) key (`).*\
#define GPK_BACK_QOUTE "U+0060"

/*The Left Curly Bracket (Opening Curly Bracket, Opening Brace,
    Brace Left) key ({).*\
#define GPK_LEFT_BRACE "U+007B"

/*The Vertical Line (Vertical Bar, Pipe) key (|).*\
#define GPK_PIPE "U+007C"

/*The Right Curly Bracket (Closing Curly Bracket, Closing Brace,
    Brace Right) key (}).*\
#define GPK_RIGHT_BRACKET "U+007D"

/*The Delete (Del) Key.
#define GPK_DELETE "U+007F"

/*The Inverted Exclamation Mark key (¡).*\
#define GPK_INVERTED_EXCLAMATION "U+00A1"

/*The Combining Grave Accent (Greek Varia, Dead Grave) key.*/
#define GPK_DEAD_GRAVE "U+0300"

/*The Combining Acute Accent (Stress Mark, Greek Oxia, Tonos,
    Dead Eacute) key.*/
#define GPK_DEAD_EACUTE "U+0301"

/*The Combining Circumflex Accent (Hat, Dead Circumflex) key.*/
#define GPK_DEAD_CIRCUMFLEX "U+0302"

/*The Combining Tilde (Dead Tilde) key.*/
#define GPK_DEAD_TILDE "U+0303"

/*The Combining Macron (Long, Dead Macron) key.*/
#define GPK_DEAD_MACRON "U+0304"

/*The Combining Breve (Short, Dead Breve) key.*/
#define GPK_DEAD_BREVE "U+0306"

/*The Combining Dot Above (Derivative, Dead Above Dot) key.*/
#define GPK_DEAD_ABOVE_DOT "U+0307"

/*The Combining Diaeresis (Double Dot Abode, Umlaut, Greek
    Dialytika, Double Derivative, Dead Diaeresis) key.*/
#define GPK_DEAD_UMLAUT "U+0308"

/*The Combining Ring Above (Dead Above Ring) key.*/
#define GPK_DEAD_ABOVE_RING "U+030A"

/*The Combining Double Acute Accent (Dead Doubleacute) key.*/
#define GPK_DEAD_DOUBLE_ACUTE "U+030B"

/*The Combining Caron (Hacek, V Above, Dead Caron) key.*/
#define GPK_DEAD_CARON "U+030C"

/*The Combining Cedilla (Dead Cedilla) key.*/
#define GPK_DEAD_CEDILLA "U+0327"

/*The Combining Ogonek (Nasal Hook, Dead Ogonek) key.*/
#define GPK_DEAD_OGONEK "U+0328"

/*The Combining Greek Ypogegrammeni (Greek Non-Spacing Iota Below,
    Iota Subscript, Dead Iota) key.*/
#define GPK_DEAD_IOTA "U+0345"

/*The Euro Currency Sign key (€).*\
#define GPK_EURO "U+20AC"

/*The Combining Katakana-Hiragana Voiced Sound Mark (Dead Voiced
    Sound) key.*/
#define GPK_DEAD_VOICED_SOUND "U+3099"

/*The Combining Katakana-Hiragana Semi-Voiced Sound Mark (Dead
    Semivoiced Sound) key.*/
#define GPK_DEAD_SEMI_VOICED_SOUND "U+309A"

typdef unsigned long long GPDOMTimeStamp;

typedef GPDOMEvent {
    GPType* gpType;
    char* type;
    unsigned int id;
    GPEventTarget* target;
    GPEventTarget* currentTarget;
    unsigned short eventPhase uint;
    bool bubbles;
    bool cancelable;
    GPDOMTimeStamp timeStamp;
    char* namespaceURI;
    bool stopPropagation;
    bool defaultPrevented;
} GPDOMEvent;

typedef {
  GPDOMEvent super;
  GPViewsAbstractView* view;
  long detail;
} GPDOMUIEvent;

typedef struct {
    GPDOMUIEvent super; 
    int screenX;
    int screenY;
    int clientX;
    int clientY;
    bool ctrlKey;
    bool shiftKey;
    bool altKey;
    bool metaKey;
    unsigned int button;
    GPDOMEventTarget* relatedTarget;
} GPDOMMouseEvent; 

typedef struct {
    char* namespaceURI;
    char* type;
    unsigned int id;
    bool bubbles;
    bool cancelable;
    bool stopPropagation;
    bool defaultPrevented;
    GPDOMMouseEvent* event;
    GPViewsAbstractView* view;
    long detail;
    int screenX;
    int screenY;
    int clientX;
    int clientY;
    bool ctrlKey;
    bool shiftKey;
    bool altKey;
    bool metaKey;
    unsigned int button;
    GPDOMEventTarget* relatedTarget;
} GPDOMInitMouseEventFunc;

void GPDOM_InitMouseEventFunc(GPDOMMouseEvent* event,) {
    static GPType* func;
    GPDOMInitMouseEventFunc data = {event:event};
    if(!func)
        func = GPType_get("gp.GPWindow.Plugin.GPWindowFunc");
    func->function(func,&data);
}

#ifdef __cplusplus
}  /* end extern "C" */
#endif 

#endif /*GPDOMEvent_h*/
