/*
  Name:             Compiler
  Copyright:        Copyright (C) SIL International.
  Documentation:
  Description:
  Create Date:      20 Jun 2006

  Modified Date:    25 Oct 2016
  Authors:          mcdurdin
  Related Files:
  Dependencies:

  Bugs:
  Todo:
  Notes:
  History:          20 Jun 2006 - mcdurdin - Initial version
                    23 Aug 2006 - mcdurdin - Add VISUALKEYBOARD, KMW_RTL, KMW_HELPFILE, KMW_HELPTEXT, KMW_EMBEDJS system stores
                    14 Sep 2006 - mcdurdin - Support icons in version 7
                    28 Sep 2006 - mcdurdin - Added product validation
                    28 Sep 2006 - mcdurdin - Added test for version 7.0 icon support
                    06 Oct 2006 - mcdurdin - Fix buffer overflow in UTF8 conversion
                    04 Dec 2006 - mcdurdin - Fix readfile buffer bug
                    04 Jan 2007 - mcdurdin - Add notany support
                    22 Jan 2007 - mcdurdin - Add K_NPENTER reference
                    25 Jan 2007 - mcdurdin - Resize buffers to 4095
                    30 May 2007 - mcdurdin - I786 - Compiler crash if zero length string in keystroke any
                    23 Aug 2007 - mcdurdin - I1011 - Fix buffer clobbering for UTF8 conversion of large files
                    27 Mar 2008 - mcdurdin - I1358 - Support for multiple languages for office config
                    14 Jun 2008 - mcdurdin - Support documenting language id as a single WORD instead of by PRIMARY/SUB
                    14 Jun 2008 - mcdurdin - Support Windows languages list
                    28 Jul 2008 - mcdurdin - I1569 - line prefixes
                    22 Mar 2010 - mcdurdin - Compiler fixup - x64 support
                    25 May 2010 - mcdurdin - I1632 - Keyboard Options
                    24 Jun 2010 - mcdurdin - I2432 - Use local buffers so GetXString can be re-entrant (used by if())
                    26 Jul 2010 - mcdurdin - I2467 - 8.0 renumber
                    18 Mar 2011 - mcdurdin - I2646 - Compiler warning on invalid Ethnologue codes
                    18 Mar 2011 - mcdurdin - I2525 - Unterminated string can crash compiler
                    19 Jul 2011 - mcdurdin - I2993 - Named code constants cause a warning 0x208D to appear
                    26 Jun 2012 - mcdurdin - I3377 - KM9 - Update code references from 8.0 to 9.0
                    17 Aug 2012 - mcdurdin - I3431 - V9.0 - Spaces in values in if comparisons break the compiler parser
                    27 Aug 2012 - mcdurdin - I3439 - V9.0 - Refactor xstring support in C++ code
                    27 Aug 2012 - mcdurdin - I3437 - V9.0 - Add support for set(&layer) and layer()
                    27 Aug 2012 - mcdurdin - I3438 - V9.0 - Add support for custom virtual keys
                    27 Aug 2012 - mcdurdin - I3430 - V9.0 - Add support for if(&platform) and if(&baselayout) to compilers
                    27 Aug 2012 - mcdurdin - I3440 - V9.0 - Tidy up set statement delimiter recognition
                    24 Oct 2012 - mcdurdin - I3483 - V9.0 - Add support for compiling in the layout file
                    24 Oct 2012 - mcdurdin - I3481 - V9.0 - Eliminate unsafe calls in C++
                    24 Jan 2012 - mcdurdin - I3137 - If key part of VK rule is missing, compiler generates invalid file
                    06 Feb 2012 - mcdurdin - I3228 - kmcmpdll sometimes tries to write temp files to Program Files
                    03 Nov 2012 - mcdurdin - I3510 - V9.0 - Merge of I3228 - kmcmpdll sometimes tries to write temp files to Program Files
                    03 Nov 2012 - mcdurdin - I3511 - V9.0 - Merge of I3137 - If key part of VK rule is missing, compiler generates invalid file
                    13 Dec 2012 - mcdurdin - I3654 - V9.0 - Compiler appears to create unregistered keyboards even when registered
                    13 Dec 2012 - mcdurdin - I3641 - V9.0 - compiler dll buffer overrun bugs
                    13 Dec 2012 - mcdurdin - I3681 - V9.0 - KeymanWeb compiler should output formatted js when debug=1
                    13 Dec 2012 - mcdurdin - I3686 - V9.0 - AddStore attaches property flags to wrong store structure
                    19 Mar 2014 - mcdurdin - I4140 - V9.0 - Add keyboard version information to keyboards
                    04 Nov 2014 - mcdurdin - I4504 - V9.0 - Consolidate the compile action into single command
                    02 Jul 2015 - mcdurdin - I4784 - Compiler does not recognise baselayout, layer or platform at the start of a line
                    02 Jul 2015 - mcdurdin - I4785 - baselayout(), layer() and platform() produce incorrect compiled code
                    24 Aug 2015 - mcdurdin - I4865 - Add treat hints and warnings as errors into project
                    24 Aug 2015 - mcdurdin - I4866 - Add warn on deprecated features to project and compile
                    24 Aug 2015 - mcdurdin - I4867 - Add test for code after use warning to compile
                    06 Nov 2015 - mcdurdin - I4914 - kmcmpdll does not pick an index() statement that has an offset one past the key
                    23 Feb 2016 - mcdurdin - I4982 - Defined character constants cannot be referenced correctly in other stores
                    25 Oct 2016 - mcdurdin - I5135 - Remove product and licensing references from Developer projects
*/
#include "pch.h"

#include <kmcmplibapi.h>

#include "compfile.h"
#include <kmn_compiler_errors.h>
#include "../../../../common/windows/cpp/include/vkeys.h"
#include <cuchar>
#include "versioning.h"
#include "kmcmplib.h"
#include "DeprecationChecks.h"
#include "cp1252.h"
#include "virtualcharkeys.h"

// TODO: These three should be under common/cpp/include -- not windows specific
#include "../../../../common/windows/cpp/include/keymanversion.h"
#include "../../../../common/windows/cpp/include/crc32.h"
#include "../../../../common/windows/cpp/include/ConvertUTF.h"

#include "debugstore.h"
#include "NamedCodeConstants.h"

#include "xstring.h"

#include "Edition.h"

#include "CharToKeyConversion.h"
#include "CasedKeys.h"
#include <vector>
#include <xstring.h>
#include <codecvt>
#include <locale>
#include <string>
#include <iostream>
#include <sstream>

#include "UnreachableRules.h"
#include "CheckForDuplicates.h"
#include "km_u16.h"

/* These macros are adapted from winnt.h and legacy use only */
#define MAKELANGID(p, s)       ((((uint16_t)(s)) << 10) | (uint16_t)(p))
#define PRIMARYLANGID(lgid)    ((uint16_t)(lgid) & 0x3ff)
#define SUBLANGID(lgid)        ((uint16_t)(lgid) >> 10)

using namespace kmcmp;

  KMX_BOOL AWarnDeprecatedCode_GLOBAL_LIB;

namespace kmcmp{
  KMX_BOOL  FShouldAddCompilerVersion = TRUE;
  KMX_BOOL  FSaveDebug, FCompilerWarningsAsErrors;   // I4865   // I4866
  KMX_BOOL FMnemonicLayout = FALSE;
  KMX_BOOL FOldCharPosMatching = FALSE;
  int CompileTarget;
  int BeginLine[4];

  KMX_BOOL IsValidCallStore(PFILE_STORE fs);
  void CheckStoreUsage(PFILE_KEYBOARD fk, int storeIndex, KMX_BOOL fIsStore, KMX_BOOL fIsOption, KMX_BOOL fIsCall);
  KMX_DWORD UTF32ToUTF16(int n, int *n1, int *n2);
  KMX_DWORD CheckUTF16(int n);
  int cmpkeys(const void *key, const void *elem);
}

int xatoi(PKMX_WCHAR *p);
int atoiW(PKMX_WCHAR p);
bool isIntegerWstring(PKMX_WCHAR p);
void safe_wcsncpy(PKMX_WCHAR out, PKMX_WCHAR in, int cbMax);
int GetDeadKey(PFILE_KEYBOARD fk, PKMX_WCHAR p);

KMX_BOOL IsSameToken(PKMX_WCHAR *p, KMX_WCHAR const * token);
KMX_DWORD GetRHS(PFILE_KEYBOARD fk, PKMX_WCHAR p, PKMX_WCHAR buf, int bufsize, int offset, int IsUnicode);
PKMX_WCHAR GetDelimitedString(PKMX_WCHAR *p, KMX_WCHAR const * Delimiters, KMX_WORD Flags);
KMX_DWORD GetXString(PFILE_KEYBOARD fk, PKMX_WCHAR str, KMX_WCHAR const * token, PKMX_WCHAR output, int max, int offset, PKMX_WCHAR *newp, int isVKey, int isUnicode);
KMX_BOOL GetCompileTargetsFromTargetsStore(const KMX_WCHAR* store, int &targets);

int GetGroupNum(PFILE_KEYBOARD fk, PKMX_WCHAR p);

KMX_DWORD AddDebugStore(PFILE_KEYBOARD fk, KMX_WCHAR const * str);
KMX_DWORD ProcessKeyLine(PFILE_KEYBOARD fk, PKMX_WCHAR str, KMX_BOOL IsUnicode);
KMX_DWORD ProcessEthnologueStore(PKMX_WCHAR p); // I2646
KMX_DWORD ProcessHotKey(PKMX_WCHAR p, KMX_DWORD *hk);
KMX_DWORD ImportBitmapFile(PFILE_KEYBOARD fk, PKMX_WCHAR szName, PKMX_DWORD FileSize, PKMX_BYTE *Buf);

KMX_DWORD ExpandKp(PFILE_KEYBOARD fk, PFILE_KEY kpp, KMX_DWORD storeIndex);

int GetVKCode(PFILE_KEYBOARD fk, PKMX_WCHAR p); // I3438 // TODO: Consolidate GetDeadKey and GetVKCode?
KMX_BOOL ProcessSystemStore(PFILE_KEYBOARD fk, KMX_DWORD SystemID, PFILE_STORE sp);

KMX_DWORD process_if(PFILE_KEYBOARD fk, PKMX_WCHAR q, PKMX_WCHAR tstr, int *mx);
KMX_DWORD process_reset(PFILE_KEYBOARD fk, PKMX_WCHAR q, PKMX_WCHAR tstr, int *mx);
KMX_DWORD process_set(PFILE_KEYBOARD fk, PKMX_WCHAR q, PKMX_WCHAR tstr, int *mx);
KMX_DWORD process_save(PFILE_KEYBOARD fk, PKMX_WCHAR q, PKMX_WCHAR tstr, int *mx);
KMX_DWORD process_platform(PFILE_KEYBOARD fk, PKMX_WCHAR q, PKMX_WCHAR tstr, int *mx);  // I3430
KMX_DWORD process_baselayout(PFILE_KEYBOARD fk, PKMX_WCHAR q, PKMX_WCHAR tstr, int *mx); // I3430
KMX_DWORD process_set_synonym(KMX_DWORD dwSystemID, PFILE_KEYBOARD fk, PKMX_WCHAR q, PKMX_WCHAR tstr, int *mx); // I3437
KMX_DWORD process_expansion(PFILE_KEYBOARD fk, PKMX_WCHAR q, PKMX_WCHAR tstr, int *mx, int max);

KMX_BOOL IsValidKeyboardVersion(KMX_WCHAR *dpString);

bool resizeStoreArray(PFILE_KEYBOARD fk);
bool resizeKeyArray(PFILE_GROUP gp, int increment = 1);

const KMX_WCHAR * LineTokens[] = {
   u"SVNBHBGMNSCCLLCMLB",  u"store",  u"VERSION ",  u"NAME ",
   u"BITMAP ",  u"HOTKEY ",  u"begin",  u"group",  u"match",  u"nomatch",
   u"SHIFT FREES CAPS",  u"CAPS ON ONLY",  u"CAPS ALWAYS OFF",
   u"LANGUAGE ",  u"LAYOUT ",  u"COPYRIGHT ",  u"MESSAGE ",  u"LANGUAGENAME ",
   u"BITMAPS " };

#define SSN__PREFIX		u"&"

const KMX_WCHAR * StoreTokens[TSS__MAX + 2] = {
  u"",
  SSN__PREFIX u"BITMAP",
  SSN__PREFIX u"COPYRIGHT",
  SSN__PREFIX u"HOTKEY",
  SSN__PREFIX u"LANGUAGE",
  SSN__PREFIX u"LAYOUT",
  SSN__PREFIX u"MESSAGE",
  SSN__PREFIX u"NAME",
  SSN__PREFIX u"VERSION",
  SSN__PREFIX u"CAPSONONLY",
  SSN__PREFIX u"CAPSALWAYSOFF",
  SSN__PREFIX u"SHIFTFREESCAPS",
  SSN__PREFIX u"LANGUAGENAME",
  u"",
  u"",
  SSN__PREFIX u"ETHNOLOGUECODE",
  u"",
  SSN__PREFIX u"MNEMONICLAYOUT",
  SSN__PREFIX u"INCLUDECODES",
  SSN__PREFIX u"OLDCHARPOSMATCHING",
  u"",
  u"",
  u"",
  u"",
  SSN__PREFIX u"VISUALKEYBOARD",
  SSN__PREFIX u"KMW_RTL",
  SSN__PREFIX u"KMW_HELPFILE",
  SSN__PREFIX u"KMW_HELPTEXT",
  SSN__PREFIX u"KMW_EMBEDJS",
  SSN__PREFIX u"WINDOWSLANGUAGES",
  u"",
  SSN__PREFIX u"PLATFORM",    // read only  // I3430
  SSN__PREFIX u"BASELAYOUT",  // read only  // I3430
  SSN__PREFIX u"LAYER",       // read-write via set?  // I3430
  u"",                        // I3438
  SSN__PREFIX u"LAYOUTFILE",  // I3483
  SSN__PREFIX u"KEYBOARDVERSION",   // I4140
  SSN__PREFIX u"KMW_EMBEDCSS",
  SSN__PREFIX u"TARGETS",   // I4504
  SSN__PREFIX u"CASEDKEYS", // #2241
  SSN__PREFIX u"", // TSS_BEGIN_NEWCONTEXT
  SSN__PREFIX u"", // TSS_BEGIN_POSTKEYSTROKE
  SSN__PREFIX u"NEWLAYER",
  SSN__PREFIX u"OLDLAYER",

  // Keyman 17.0+
  SSN__PREFIX u"DISPLAYMAP",  // TSS_DISPLAYMAP (compile-time usage only)
  NULL
};

static_assert(sizeof(StoreTokens) / sizeof(StoreTokens[0]) == TSS__MAX + 2, "StoreTokens should have exactly TSS__MAX+2 elements");

enum LinePrefixType { lptNone, lptKeymanAndKeymanWeb, lptKeymanWebOnly, lptKeymanOnly, lptOther };

/* Compile target */

kmcmp_LoadFileProc kmcmp::loadfileproc = NULL;

int kmcmp::currentLine = 0;

kmcmp::NamedCodeConstants *kmcmp::CodeConstants = NULL;

PKMX_WCHAR strtowstr(PKMX_STR in)
{
  PKMX_WCHAR result;

  auto c = u16string_from_string(in);

  result = new KMX_WCHAR[c.length() + 1];
  u16cpy(result, c.c_str());
  return result;
}

PKMX_STR wstrtostr(PKMX_WCHAR in)
{
  PKMX_STR result;

  auto c = string_from_u16string(in);

  result = new KMX_CHAR[c.length() + 1];
  strcpy(result, c.c_str());
  return result;
}


KMX_BOOL ProcessBeginLine(PFILE_KEYBOARD fk, PKMX_WCHAR p) {
  KMX_WCHAR tstr[128];
  PKMX_WCHAR q, pp;
  int BeginMode;
  KMX_DWORD msg;

  pp = p;

  q = (PKMX_WCHAR) u16chr(p, '>');
  if (!q) {
    ReportCompilerMessage(KmnCompilerMessages::ERROR_NoTokensFound);
    return FALSE;
  }

  while (iswspace(*p)) p++;
  if (u16nicmp(p, u"unicode", 7) == 0) BeginMode = BEGIN_UNICODE;
  else if (u16nicmp(p, u"ansi", 4) == 0) BeginMode = BEGIN_ANSI;
  else if (u16nicmp(p, u"newContext", 10) == 0) BeginMode = BEGIN_NEWCONTEXT;
  else if (u16nicmp(p, u"postKeystroke", 13) == 0) BeginMode = BEGIN_POSTKEYSTROKE;
  else if (*p != '>') {
    ReportCompilerMessage(KmnCompilerMessages::ERROR_InvalidToken);
    return FALSE;
  }
  else BeginMode = BEGIN_ANSI;

  if(kmcmp::BeginLine[BeginMode] != -1) {
    ReportCompilerMessage(KmnCompilerMessages::ERROR_RepeatedBegin);
    return FALSE;
  }

  kmcmp::BeginLine[BeginMode] = kmcmp::currentLine;

  if ((msg = GetRHS(fk, p, tstr, 80, (int)(p - pp), FALSE)) != STATUS_Success) {
    ReportCompilerMessage(msg);
    return FALSE;
  }

  if (tstr[0] != UC_SENTINEL || tstr[1] != CODE_USE) {
    ReportCompilerMessage(KmnCompilerMessages::ERROR_InvalidBegin);
    return FALSE;
  }
  if (tstr[3] != 0) {
    ReportCompilerMessage(KmnCompilerMessages::ERROR_InvalidToken);
    return FALSE;
  }

  if (BeginMode == BEGIN_ANSI || BeginMode == BEGIN_UNICODE) {
    fk->StartGroup[BeginMode] = tstr[2] - 1;
    //mcd-03-01-2000: removed the secondary group idea; this was undocumented and
    //is not supported under Keyman 5.0: ugly!!
    //if(tstr[3] == UC_SENTINEL && tstr[4] == CODE_USE) fk->StartGroup[1] = tstr[5] - 1;

    if (kmcmp::FSaveDebug) {
      /* Record a system store for the line number of the begin statement */
      AddDebugStore(fk, BeginMode == BEGIN_UNICODE ? DEBUGSTORE_BEGIN u"Unicode" : DEBUGSTORE_BEGIN u"ANSI");
    }
  } else {
    PFILE_GROUP gp = &fk->dpGroupArray[tstr[2] - 1];
    if (!gp->fReadOnly) {
      ReportCompilerMessage(BeginMode == BEGIN_NEWCONTEXT ?
        KmnCompilerMessages::ERROR_NewContextGroupMustBeReadonly :
        KmnCompilerMessages::ERROR_PostKeystrokeGroupMustBeReadonly);
      return FALSE;
    }
    return AddStore(fk, BeginMode == BEGIN_NEWCONTEXT ? TSS_BEGIN_NEWCONTEXT : TSS_BEGIN_POSTKEYSTROKE, tstr, NULL);
  }

  return TRUE;
}

KMX_DWORD ValidateMatchNomatchOutput(PKMX_WCHAR p) {
  while (p && *p) {
    if (*p == UC_SENTINEL) {
      switch (*(p + 1)) {
      case CODE_CONTEXT:
      case CODE_CONTEXTEX:
      case CODE_INDEX:
        return KmnCompilerMessages::ERROR_ContextAndIndexInvalidInMatchNomatch;
      }
    }
    p = incxstr(p);
  }
  return STATUS_Success;
}

KMX_BOOL ParseLine(PFILE_KEYBOARD fk, PKMX_WCHAR str) {
  PKMX_WCHAR p, q, pp;
  PFILE_GROUP gp;
  KMX_DWORD msg;
  int IsUnicode = TRUE; // For NOW!

  KMX_WCHAR sep[2] = u"\n";
  PKMX_WCHAR p_sep =sep;
  p = str;
  pp = str;

  switch (LineTokenType(&p))
  {
  case T_BLANK:
  case T_COMMENT:
    break;	// Ignore the line
  case T_VERSION:
  case T_STORE:
    break;	// The line has already been processed

  case T_BEGIN:
    // after a begin can be "Unicode" or "ANSI" or nothing (=ANSI)
    if (!ProcessBeginLine(fk, p)) {
      return FALSE;
    }
    break;

  case T_GROUP:
    if (fk->currentGroup == 0xFFFFFFFF) {
      fk->currentGroup = 0;
    } else {
      if (!ProcessGroupFinish(fk)) {
        return FALSE;		// finish off previous group first?
      }
      fk->currentGroup++;
    }
    break;

  case T_NAME:
    kmcmp::WarnDeprecatedHeader();   // I4866
    q = GetDelimitedString(&p, u"\"\"", 0);
    if (!q) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_InvalidName);
      return FALSE;
    }

    if (!AddStore(fk, TSS_NAME, q)) {
      return FALSE;
    }
    break;

  case T_COPYRIGHT:
    kmcmp::WarnDeprecatedHeader();   // I4866
    q = GetDelimitedString(&p, u"\"\"", 0);
    if (!q) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_InvalidCopyright);
      return FALSE;
    }

    if (!AddStore(fk, TSS_COPYRIGHT, q)) {
      return FALSE;
    }
    break;

  case T_MESSAGE:
    kmcmp::WarnDeprecatedHeader();   // I4866
    q = GetDelimitedString(&p, u"\"\"", 0);
    if (!q) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_InvalidMessage);
      return FALSE;
    }

    if (!AddStore(fk, TSS_MESSAGE, q)) {
      return FALSE;
    }
    break;

  case T_LANGUAGENAME:
    kmcmp::WarnDeprecatedHeader();   // I4866
    q = GetDelimitedString(&p, u"\"\"", 0);
    if (!q) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_InvalidLanguageName);
      return FALSE;
    }

    if (!AddStore(fk, TSS_LANGUAGENAME, q)) {
      return FALSE;
    }
    break;

  case T_LANGUAGE:
  {
    kmcmp::WarnDeprecatedHeader();   // I4866
    KMX_WCHAR *tokcontext = NULL;
    q = u16tok(p,  p_sep, &tokcontext);  // I3481

    if (!AddStore(fk, TSS_LANGUAGE, q)) {
      return FALSE;
    }
    break;
  }
  case T_LAYOUT:
  {
    kmcmp::WarnDeprecatedHeader();   // I4866
    KMX_WCHAR *tokcontext = NULL;
    q = u16tok(p, p_sep, &tokcontext);  // I3481
    if (!AddStore(fk, TSS_LAYOUT, q)) {
      return FALSE;
    }
    break;
  }
  case T_CAPSOFF:
    kmcmp::WarnDeprecatedHeader();   // I4866
    if (!AddStore(fk, TSS_CAPSALWAYSOFF, u"1")) {
      return FALSE;
    }
    break;

  case T_CAPSON:
    kmcmp::WarnDeprecatedHeader();   // I4866
    if (!AddStore(fk, TSS_CAPSONONLY, u"1")) {
      return FALSE;
    }
    break;

  case T_SHIFT:
    kmcmp::WarnDeprecatedHeader();   // I4866
    if (!AddStore(fk, TSS_SHIFTFREESCAPS, u"1")) {
      return FALSE;
    }
    break;

  case T_HOTKEY:
  {
    kmcmp::WarnDeprecatedHeader();   // I4866
    KMX_WCHAR *tokcontext = NULL;
    if ((q = u16tok(p,  p_sep, &tokcontext)) == NULL) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_CodeInvalidInThisSection);  // I3481
      return FALSE;
    }
    if (!AddStore(fk, TSS_HOTKEY, q)) {
      return FALSE;
    }
    break;
  }
  case T_BITMAP:
  {
    kmcmp::WarnDeprecatedHeader();   // I4866
    KMX_WCHAR *tokcontext = NULL;
    if ((q = u16tok(p,  p_sep, &tokcontext)) == NULL) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_InvalidBitmapLine);  // I3481
      return FALSE;
    }

    while (iswspace(*q)) q++;
    if (*q == '"') {
      p = q;
      q = GetDelimitedString(&p, u"\"\"", 0);
      if (!q) {
        ReportCompilerMessage(KmnCompilerMessages::ERROR_InvalidBitmapLine);
        return FALSE;
      }
    }

    if (!AddStore(fk, TSS_BITMAP, q)) {
      return FALSE;
    }
    break;
  }
  case T_BITMAPS:
  {
    kmcmp::WarnDeprecatedHeader();   // I4866
    KMX_WCHAR *tokcontext = NULL;
    ReportCompilerMessage(KmnCompilerMessages::WARN_BitmapNotUsed);

    if ((q = u16tok(p,  p_sep, &tokcontext)) == NULL) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_InvalidBitmapLine);  // I3481
      return FALSE;
    }

    if ((PKMX_WCHAR) u16chr(q, ',')) {
      *(PKMX_WCHAR) u16chr(q, ',') = 0;
    }
    if (!AddStore(fk, TSS_BITMAP, q)) {
      return FALSE;
    }

    break;
  }
  case T_KEYTOKEY:			// A rule
    if (fk->currentGroup == 0xFFFFFFFF) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_CodeInvalidInThisSection);
      return FALSE;
    }
    if ((msg = ProcessKeyLine(fk, p, IsUnicode)) != STATUS_Success) {
      ReportCompilerMessage(msg);
      return FALSE;
    }
    break;

  case T_MATCH:
    if (fk->currentGroup == 0xFFFFFFFF) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_CodeInvalidInThisSection);
      return FALSE;
    }
    {
      PKMX_WCHAR buf = new KMX_WCHAR[GLOBAL_BUFSIZE];
      if ((msg = GetRHS(fk, p, buf, GLOBAL_BUFSIZE - 1, (int)(p - pp), IsUnicode)) != STATUS_Success)
      {
        delete[] buf;
        ReportCompilerMessage(msg);
        return FALSE;
      }

      if ((msg = ValidateMatchNomatchOutput(buf)) != STATUS_Success) {
        delete[] buf;
        ReportCompilerMessage(msg);
        return FALSE;
      }

      gp = &fk->dpGroupArray[fk->currentGroup];

      gp->dpMatch = new KMX_WCHAR[u16len(buf) + 1];
      u16ncpy(gp->dpMatch, buf, u16len(buf) + 1);  // I3481

      delete[] buf;

      if (kmcmp::FSaveDebug)
      {
        KMX_WCHAR tstr[128];
        //swprintf(tstr, "%d", fk->currentGroup);
        /* Record a system store for the line number of the begin statement */
        //wcscpy(tstr, DEBUGSTORE_MATCH);
        u16sprintf(tstr, _countof(tstr), L"%ls%d ", DEBUGSTORE_MATCH_L, (int) fk->currentGroup);
        u16ncat(tstr, gp->szName, _countof(tstr));

        AddDebugStore(fk, tstr);
      }
    }
    break;

  case T_NOMATCH:
    if (fk->currentGroup == 0xFFFFFFFF) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_CodeInvalidInThisSection);
      return FALSE;
    }
    {
      PKMX_WCHAR buf = new KMX_WCHAR[GLOBAL_BUFSIZE];
      if ((msg = GetRHS(fk, p, buf, GLOBAL_BUFSIZE, (int)(p - pp), IsUnicode)) != STATUS_Success)
      {
        delete[] buf;
        ReportCompilerMessage(msg);
        return FALSE;
      }

      if ((msg = ValidateMatchNomatchOutput(buf)) != STATUS_Success) {
        delete[] buf;
        ReportCompilerMessage(msg);
        return FALSE;
      }

      gp = &fk->dpGroupArray[fk->currentGroup];

      gp->dpNoMatch = new KMX_WCHAR[u16len(buf) + 1];
      u16ncpy(gp->dpNoMatch, buf, u16len(buf) + 1);  // I3481

      delete[] buf;

      if (kmcmp::FSaveDebug)
      {
        KMX_WCHAR tstr[128];
        /* Record a system store for the line number of the begin statement */
        u16sprintf(tstr, _countof(tstr), L"%ls%d ", DEBUGSTORE_NOMATCH_L, (int) fk->currentGroup);
        u16ncat(tstr, gp->szName, _countof(tstr));
        AddDebugStore(fk, tstr);
      }
    }
    break;

  default:
    ReportCompilerMessage(KmnCompilerMessages::ERROR_InvalidToken);
    return FALSE;
  }

  return TRUE;
}

//**********************************************************************************************************************

KMX_BOOL ProcessGroupLine(PFILE_KEYBOARD fk, PKMX_WCHAR p)
{
  PFILE_GROUP gp;
  PKMX_WCHAR q;

  gp = new FILE_GROUP[fk->cxGroupArray + 1];
  if (!gp) {
    ReportCompilerMessage(KmnCompilerMessages::FATAL_CannotAllocateMemory);
    return FALSE;
  }

  if (fk->dpGroupArray)
  {
    memcpy(gp, fk->dpGroupArray, sizeof(FILE_GROUP) * fk->cxGroupArray);
    delete[] fk->dpGroupArray;
  }

  fk->dpGroupArray = gp;
  gp = &fk->dpGroupArray[fk->cxGroupArray];
  fk->cxGroupArray++;

  gp->dpKeyArray = NULL;
  gp->dpMatch = NULL;
  gp->dpNoMatch = NULL;
  gp->cxKeyArray = 0;

  q = GetDelimitedString(&p, u"()", GDS_CUTLEAD | GDS_CUTFOLL);
  if (!q) {
    ReportCompilerMessage(KmnCompilerMessages::ERROR_InvalidGroupLine);
    return FALSE;
  }

  gp->fUsingKeys = FALSE;
  gp->fReadOnly  = IsSameToken(&p, u"readonly");
  if (!gp->fReadOnly) {
    if (IsSameToken(&p, u"using") && IsSameToken(&p, u"keys"))
      gp->fUsingKeys = TRUE;
  }

  safe_wcsncpy(gp->szName, q, SZMAX_GROUPNAME);

  gp->Line = kmcmp::currentLine;

  if (kmcmp::FSaveDebug)
  {
    KMX_WCHAR tstr[128];
    /* Record a system store for the line number of the begin statement */
    u16sprintf(tstr, _countof(tstr), L"%ls%d ", DEBUGSTORE_GROUP_L, fk->cxGroupArray - 1);
    u16ncat(tstr, gp->szName, _countof(tstr));
    AddDebugStore(fk, tstr);
  }

  return CheckForDuplicateGroup(fk, gp);
}

int kmcmp::cmpkeys(const void *key, const void *elem)
{
  PFILE_KEY akey;
  PFILE_KEY  aelem;
  int l1, l2;
  KMX_WCHAR char_key, char_elem;
  akey = (PFILE_KEY)key;
  aelem = (PFILE_KEY)elem;
  char_key = kmcmp::VKToChar(akey->Key, akey->ShiftFlags);
  char_elem = kmcmp::VKToChar(aelem->Key, aelem->ShiftFlags);
  if (char_key == char_elem) //akey->Key == aelem->Key)
  {
    l1 = xstrlen(akey->dpContext); l2 = xstrlen(aelem->dpContext);
    if (l1 == l2)
    {
      if (akey->Line < aelem->Line) return -1;
      if (akey->Line > aelem->Line) return 1;
      if(akey->Key == aelem->Key) {
        if(akey->ShiftFlags == aelem->ShiftFlags) {
          return akey->LineStoreIndex - aelem->LineStoreIndex;
        }
        return akey->ShiftFlags - aelem->ShiftFlags;
      }
      return akey->Key - aelem->Key;
    }
    if (l1 < l2) return 1;
    if (l1 > l2) return -1;
    if(akey->Key == aelem->Key) {
      if(akey->ShiftFlags == aelem->ShiftFlags) {
        return akey->LineStoreIndex - aelem->LineStoreIndex;
      }
      return akey->ShiftFlags - aelem->ShiftFlags;
    }
    return akey->Key - aelem->Key;
  }
  return(char_key - char_elem); // akey->Key - aelem->Key);
}

KMX_BOOL ProcessGroupFinish(PFILE_KEYBOARD fk) {
  PFILE_GROUP gp;
  KMX_DWORD msg;

  if (fk->currentGroup == 0xFFFFFFFF) {
    // Just got to first group - so nothing to finish yet
    return TRUE;
  }

  gp = &fk->dpGroupArray[fk->currentGroup];

  // Finish off the previous group stuff!
  if ((msg = ExpandCapsRulesForGroup(fk, gp)) != STATUS_Success) {
    ReportCompilerMessage(msg);
    return FALSE;
  }
  qsort(gp->dpKeyArray, gp->cxKeyArray, sizeof(FILE_KEY), kmcmp::cmpkeys);

  VerifyUnreachableRules(gp);

  return TRUE;
}

/***************************************
* Store management
*/

KMX_BOOL ProcessStoreLine(PFILE_KEYBOARD fk, PKMX_WCHAR p) {
  PKMX_WCHAR q, pp;
  PFILE_STORE sp;
  KMX_DWORD msg;
  int i = 0;

  pp = p;

  if ((q = GetDelimitedString(&p, u"()", GDS_CUTLEAD | GDS_CUTFOLL)) == NULL) {
    ReportCompilerMessage(KmnCompilerMessages::ERROR_InvalidStoreLine);
    return FALSE;
  }

  if (*q == *SSN__PREFIX) {
    for (i = 0; StoreTokens[i]; i++) {
      if (!u16icmp(q, StoreTokens[i])) {  // I3481
        break;
      }
    }
    if (!StoreTokens[i]) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_InvalidSystemStore);
      return FALSE;
    }
  }

  if(!resizeStoreArray(fk)) {
    ReportCompilerMessage(KmnCompilerMessages::FATAL_CannotAllocateMemory);
    return FALSE;
  }
  sp = &fk->dpStoreArray[fk->cxStoreArray];

  sp->line = kmcmp::currentLine;
  sp->fIsOption = FALSE;
  sp->fIsReserved = FALSE;
  sp->fIsStore = FALSE;
  sp->fIsDebug = FALSE;
  sp->fIsCall = FALSE;

  safe_wcsncpy(sp->szName, q, SZMAX_STORENAME);
  {
    PKMX_WCHAR temp = new KMX_WCHAR[GLOBAL_BUFSIZE];

    if ((msg = GetXString(fk, p, u"c\n", temp, GLOBAL_BUFSIZE - 1, (int)(p - pp), &p, FALSE, TRUE)) != STATUS_Success) {
      delete[] temp;
      ReportCompilerMessage(msg);
      return FALSE;
    }

    sp->dwSystemID = i;
    sp->dpString = new KMX_WCHAR[u16len(temp) + 1];
    u16ncpy(sp->dpString, temp, u16len(temp) + 1);  // I3481

    delete[] temp;
  }

  if (xstrlen(sp->dpString) == 1 && *sp->dpString != UC_SENTINEL &&
    sp->dwSystemID == 0 && (fk->version >= VERSION_60 || fk->version == 0)) {
    // In this case, we want to change behaviour for older versioned keyboards so that
    // we don't mix up named character codes which weren't supported in 5.x
    if(!VerifyKeyboardVersion(fk, VERSION_60)) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_60FeatureOnly_NamedCodes);
      return FALSE;
    }
    // Add a single char store as a defined character constant
    if (Uni_IsSurrogate1(*sp->dpString)) {
      kmcmp::CodeConstants->AddCode(Uni_SurrogateToUTF32(sp->dpString[0], sp->dpString[1]), sp->szName, fk->cxStoreArray);
    } else {
      kmcmp::CodeConstants->AddCode(sp->dpString[0], sp->szName, fk->cxStoreArray);
    }
    kmcmp::CodeConstants->reindex(); // has to be done after every character add due to possible use in another store.   // I4982
  }

  fk->cxStoreArray++;	// increment now, because GetXString refers to stores

  if (i > 0) {
    if (!ProcessSystemStore(fk, i, sp)) {
      return FALSE;
    }
  }

  return CheckForDuplicateStore(fk, sp);
}

bool resizeStoreArray(PFILE_KEYBOARD fk) {
  if(fk->cxStoreArray % 100 == 0) {
    PFILE_STORE sp = new FILE_STORE[fk->cxStoreArray + 100];
    if (!sp) return false;

    if (fk->dpStoreArray)
    {
      memcpy(sp, fk->dpStoreArray, sizeof(FILE_STORE) * fk->cxStoreArray);
      delete[] fk->dpStoreArray;
    }

    fk->dpStoreArray = sp;
  }
  return true;
}

/**
 * reallocates the key array in increments of 100
 */
bool resizeKeyArray(PFILE_GROUP gp, int increment) {
  const int cxKeyArray = (int)gp->cxKeyArray;
  if((cxKeyArray + increment - 1) % 100 < increment) {
    PFILE_KEY kp = new FILE_KEY[((cxKeyArray + increment)/100 + 1) * 100];
    if (!kp) return false;
    if (gp->dpKeyArray)
    {
      memcpy(kp, gp->dpKeyArray, cxKeyArray * sizeof(FILE_KEY));
      delete[] gp->dpKeyArray;
    }

    gp->dpKeyArray = kp;
  }
  return true;
}

KMX_BOOL AddStore(PFILE_KEYBOARD fk, KMX_DWORD SystemID, const KMX_WCHAR * str, KMX_DWORD *dwStoreID) {
  PFILE_STORE sp;
  if(!resizeStoreArray(fk)) {
    ReportCompilerMessage(KmnCompilerMessages::FATAL_CannotAllocateMemory);
    return FALSE;
  }

  sp = &fk->dpStoreArray[fk->cxStoreArray];

  sp->line = kmcmp::currentLine;
  sp->fIsOption = FALSE;   // I3686
  sp->fIsReserved = (SystemID != TSS_NONE);
  sp->fIsStore = FALSE;
  sp->fIsDebug = FALSE;
  sp->fIsCall = FALSE;

  safe_wcsncpy(sp->szName, (PKMX_WCHAR) StoreTokens[SystemID], SZMAX_STORENAME);

  sp->dpString = new KMX_WCHAR[u16len(str) + 1];
  u16ncpy(sp->dpString, str, u16len(str) + 1);  // I3481

  sp->dwSystemID = SystemID;

  if (dwStoreID) *dwStoreID = fk->cxStoreArray;

  fk->cxStoreArray++;

  return ProcessSystemStore(fk, SystemID, sp);
}

KMX_DWORD AddDebugStore(PFILE_KEYBOARD fk, KMX_WCHAR const * str)
{
  PFILE_STORE sp;
  KMX_WCHAR tstr[16];
  u16sprintf(tstr, _countof(tstr), L"%d", kmcmp::currentLine);  // I3481

  if(!resizeStoreArray(fk)) {
    return KmnCompilerMessages::FATAL_CannotAllocateMemory;
  }
  sp = &fk->dpStoreArray[fk->cxStoreArray];

  safe_wcsncpy(sp->szName, (PKMX_WCHAR) str, SZMAX_STORENAME);

  sp->dpString = new KMX_WCHAR[u16len(tstr) + 1];
  u16ncpy(sp->dpString, tstr, u16len(tstr) + 1);  // I3481
  sp->line = 0;
  sp->fIsOption = FALSE;
  sp->fIsReserved = TRUE;
  sp->fIsStore = FALSE;
  sp->fIsDebug = TRUE;
  sp->fIsCall = FALSE;
  sp->dwSystemID = TSS_DEBUG_LINE;
  fk->cxStoreArray++;

  return STATUS_Success;
}

PKMX_WCHAR pssBuf = NULL;

KMX_BOOL ProcessSystemStore(PFILE_KEYBOARD fk, KMX_DWORD SystemID, PFILE_STORE sp) {
  //WCHAR buf[GLOBAL_BUFSIZE];
  int i, j;
  KMX_DWORD msg;
  PKMX_WCHAR p, q;

  if (!pssBuf) pssBuf = new KMX_WCHAR[GLOBAL_BUFSIZE];
  PKMX_WCHAR buf = pssBuf;

  switch (SystemID)
  {
  case TSS_BASELAYOUT:
    break;

  case TSS_BITMAP:
    if ((msg = ImportBitmapFile(fk, sp->dpString, &fk->dwBitmapSize, &fk->lpBitmap)) != STATUS_Success) {
      ReportCompilerMessage(msg);
      return FALSE;
    }
    break;

  case TSS_CALLDEFINITION:
    break;

  case TSS_CALLDEFINITION_LOADFAILED:
    break;

  case TSS_CAPSALWAYSOFF:
    if (*sp->dpString == u'1') fk->dwFlags |= KF_CAPSALWAYSOFF;
    break;

  case TSS_CAPSONONLY:
    if (*sp->dpString == u'1') fk->dwFlags |= KF_CAPSONONLY;
    break;

  case TSS_COMPILEDVERSION:
    break;

  case TSS_COPYRIGHT:
    break;

  case TSS_CUSTOMKEYMANEDITION:
    break;

  case TSS_CUSTOMKEYMANEDITIONNAME:
    break;

  case TSS_DEBUG_LINE:
    break;

  case TSS_ETHNOLOGUECODE:
    if(!VerifyKeyboardVersion(fk, VERSION_60)) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_60FeatureOnly_EthnologueCode);
      return FALSE;
    }
    if ((msg = ProcessEthnologueStore(sp->dpString)) != STATUS_Success) {
      ReportCompilerMessage(msg);
      return FALSE;  // I2646
    }
    break;

  case TSS_HOTKEY:
    if ((msg = ProcessHotKey(sp->dpString, &fk->dwHotKey)) != STATUS_Success) {
      ReportCompilerMessage(msg);
      return FALSE;
    }
    u16sprintf(buf, GLOBAL_BUFSIZE, L"%d", (int)fk->dwHotKey);  // I3481
    delete[] sp->dpString;
    sp->dpString = new KMX_WCHAR[u16len(buf) + 1];
    u16ncpy(sp->dpString, buf, u16len(buf) + 1);  // I3481
    break;

  case TSS_INCLUDECODES:
    if(!VerifyKeyboardVersion(fk, VERSION_60)) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_60FeatureOnly_NamedCodes);
      return FALSE;
    }
    if (!kmcmp::CodeConstants->LoadFile(fk, sp->dpString)) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_CannotLoadIncludeFile);
      return FALSE;
    }
    kmcmp::CodeConstants->reindex();   // I4982
    break;

  case TSS_KEYMANCOPYRIGHT:
    break;

  case TSS_LANGUAGE:
  {
    KMX_WCHAR *context = NULL;
    KMX_WCHAR sep_c[3] = u", ";
    PKMX_WCHAR p_sep_c = sep_c;
    q = u16tok(sp->dpString, p_sep_c, &context);  // I3481
    if (!q) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_InvalidLanguageLine);
      return FALSE;
    }

    i = xatoi(&q);
    KMX_WCHAR sep_n[4] = u" c\n";
    PKMX_WCHAR p_sep_n = sep_n;
    q = u16tok(NULL, p_sep_n, &context);  // I3481
    if (!q)
    {
      if(!VerifyKeyboardVersion(fk, VERSION_70)) {
        ReportCompilerMessage(KmnCompilerMessages::ERROR_InvalidLanguageLine);
        return FALSE;
      }
      j = SUBLANGID(i);
      i = PRIMARYLANGID(i);
    }
    else
      j = xatoi(&q);

    if (i < 1 || j < 1 || i > 0x3FF || j > 0x3F) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_InvalidLanguageLine);
      return FALSE;
    }
    if (i >= 0x200 || j >= 0x20) {
      ReportCompilerMessage(KmnCompilerMessages::WARN_CustomLanguagesNotSupported);
    }

    fk->KeyboardID = (KMX_DWORD)MAKELANGID(i, j);

    u16sprintf(buf, GLOBAL_BUFSIZE, L"%x %x", i, j);  // I3481
    delete[] sp->dpString;
    sp->dpString = new KMX_WCHAR[u16len(buf) + 1];
    u16ncpy(sp->dpString, buf, u16len(buf) + 1);  // I3481

    break;
  }
  case TSS_LANGUAGENAME:
    break;

  case TSS_LAYER:
    break;

  case TSS_LAYOUT:
    if (fk->KeyboardID == 0) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_LayoutButNoLanguage);
      return FALSE;
    }

    q = sp->dpString;

    fk->KeyboardID |= (xatoi(&q) << 16L);
    break;

  case TSS_MESSAGE:
    break;

  case TSS_MNEMONIC:
    if(!VerifyKeyboardVersion(fk, VERSION_60)) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_60FeatureOnly_MnemonicLayout);
      return FALSE;
    };
    kmcmp::FMnemonicLayout = atoiW(sp->dpString) == 1;
    if (kmcmp::FMnemonicLayout && FindSystemStore(fk, TSS_CASEDKEYS) != NULL) {
      // The &CasedKeys system store is not supported for
      // mnemonic layouts
      ReportCompilerMessage(KmnCompilerMessages::ERROR_CasedKeysNotSupportedWithMnemonicLayout);
      return FALSE;
    }
    break;

  case TSS_NAME:
    break;

  case TSS_OLDCHARPOSMATCHING:
    if(!VerifyKeyboardVersion(fk, VERSION_60)) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_60FeatureOnly_OldCharPosMatching);
      return FALSE;
    }
    kmcmp::FOldCharPosMatching = atoiW(sp->dpString);
    break;

  case TSS_PLATFORM:
  case TSS_PLATFORM_MATCH:
  case TSS_PLATFORM_NOMATCH:
    // Never matched but included for completeness
    break;

  case TSS_SHIFTFREESCAPS:
    if (*sp->dpString == u'1') fk->dwFlags |= KF_SHIFTFREESCAPS;
    break;

  case TSS_VERSION:
    if ((fk->dwFlags & KF_AUTOMATICVERSION) == 0) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_VersionAlreadyIncluded);
      return FALSE;
    }
    p = sp->dpString;
    if (u16tof (p) < 5.0) {
      ReportCompilerMessage(KmnCompilerMessages::WARN_OldVersion);
    }

    if (u16ncmp(p, u"3.0", 3) == 0)       fk->version = VERSION_50;   //0x0a0b000n= a.bn
    else if (u16ncmp(p, u"3.1", 3) == 0)  fk->version = VERSION_50;   //all versions < 5.0
    else if (u16ncmp(p, u"3.2", 3) == 0)  fk->version = VERSION_50;   //we compile as if
    else if (u16ncmp(p, u"4.0", 3) == 0)  fk->version = VERSION_50;   //they are 5.0.100.0
    else if (u16ncmp(p, u"5.01", 4) == 0) fk->version = VERSION_501;
    else if (u16ncmp(p, u"5.0", 3) == 0)  fk->version = VERSION_50;
    else if (u16ncmp(p, u"6.0", 3) == 0)  fk->version = VERSION_60;
    else if (u16ncmp(p, u"7.0", 3) == 0)  fk->version = VERSION_70;
    else if (u16ncmp(p, u"8.0", 3) == 0)  fk->version = VERSION_80;
    else if (u16ncmp(p, u"9.0", 3) == 0)  fk->version = VERSION_90;
    else if (u16ncmp(p, u"10.0", 4) == 0)  fk->version = VERSION_100;
    else if (u16ncmp(p, u"14.0", 4) == 0)  fk->version = VERSION_140; // Adds support for #917 -- context() with notany() for KeymanWeb
    else if (u16ncmp(p, u"15.0", 4) == 0)  fk->version = VERSION_150; // Adds support for U_xxxx_yyyy #2858
    else if (u16ncmp(p, u"16.0", 4) == 0)  fk->version = VERSION_160; // KMXPlus
    else if (u16ncmp(p, u"17.0", 4) == 0)  fk->version = VERSION_170; // Flicks and gestures

    else {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_InvalidVersion);
      return FALSE;
    }

    if (fk->version < VERSION_60) kmcmp::FOldCharPosMatching = TRUE;

    fk->dwFlags &= ~KF_AUTOMATICVERSION;

    break;

  case TSS_VISUALKEYBOARD:
    if(!VerifyKeyboardVersion(fk, VERSION_70)) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_70FeatureOnly);
      return FALSE;
    }
    {
      // Store extra metadata for callers as we mutate this store during
      // compilation
      fk->extra->kvksFilename = string_from_u16string(sp->dpString);
      // Strip path from the store, leaving bare filename only
      p = sp->dpString;

      KMX_WCHAR *pp2 = (KMX_WCHAR*) u16rchr_slash((const PKMX_WCHAR) p);

      if (!pp2) {
        pp2 = p;
      } else {
        pp2++;
      }
      q = new KMX_WCHAR[u16len(pp2) + 1];
      u16ncpy(q, pp2, u16len(pp2) + 1);

      // Change compiled reference file extension to .kvk
      pp2 = ( km_core_cu *) u16chr(q, 0) - 5;
      if (pp2 > q && u16icmp(pp2, u".kvks") == 0) {
        pp2[4] = 0;
      }

      delete[] sp->dpString;
      sp->dpString = q;
    }
    break;
  case TSS_KMW_RTL:
  case TSS_KMW_HELPTEXT:
    if(!VerifyKeyboardVersion(fk, VERSION_70)) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_70FeatureOnly);
      return FALSE;
    }
    break;

  case TSS_KMW_HELPFILE:
  case TSS_KMW_EMBEDJS:
    if(!VerifyKeyboardVersion(fk, VERSION_70)) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_70FeatureOnly);
      return FALSE;
    }
    break;

  case TSS_KMW_EMBEDCSS:
    if(!VerifyKeyboardVersion(fk, VERSION_90)) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_90FeatureOnlyEmbedCSS);
      return FALSE;
    }
    break;

  case TSS_TARGETS:   // I4504
    if(!VerifyKeyboardVersion(fk, VERSION_90)) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_90FeatureOnlyTargets);
      return FALSE;
    }
    if(!GetCompileTargetsFromTargetsStore(sp->dpString, fk->extra->targets)) {
      return FALSE;
    }
    break;

  case TSS_WINDOWSLANGUAGES:
  {
    if(!VerifyKeyboardVersion(fk, VERSION_70)) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_70FeatureOnly);
      return FALSE;
    }
    KMX_WCHAR *context = NULL;
    size_t szQ = u16len(sp->dpString) * 6 + 1;  // I3481
    q = new KMX_WCHAR[szQ]; // guaranteed to be enough space for recoding
    *q = 0; KMX_WCHAR *r = q;
    KMX_WCHAR sep_s[4] = u" ";
    PKMX_WCHAR p_sep_s = sep_s;
    p = u16tok(sp->dpString, p_sep_s, &context);  // I3481
    while (p)
    {
      int n = xatoi(&p);

      j = SUBLANGID(n);
      i = PRIMARYLANGID(n);

      if (i < 1 || j < 1 || i > 0x3FF || j > 0x3F) {
        delete[] q;
        ReportCompilerMessage(KmnCompilerMessages::ERROR_InvalidLanguageLine);
        return FALSE;
      }

      u16sprintf(r, szQ - (size_t)(r - q), L"x%04.4x ", n);  // I3481
      p = u16tok(NULL, sep_s, &context);  // I3481
      r = (KMX_WCHAR*) u16chr(q, 0);  // I3481
    }
    delete[] sp->dpString;
    if (*q) {
      *((KMX_WCHAR*) u16chr(q, 0) - 1) = 0; // delete final space - safe because we control the formatting - ugly? scared?
    }
    sp->dpString = q;
    break;
  }
  case TSS_COMPARISON:
    if(!VerifyKeyboardVersion(fk, VERSION_80)) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_80FeatureOnly);
      return FALSE;
    }
    break;

  case TSS_VKDICTIONARY:  // I3438
    if(!VerifyKeyboardVersion(fk, VERSION_90)) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_90FeatureOnlyVirtualKeyDictionary);
      return FALSE;
    }
    break;

  case TSS_LAYOUTFILE:  // I3483
    if(!VerifyKeyboardVersion(fk, VERSION_90)) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_90FeatureOnlyLayoutFile);
      return FALSE;
    }
    // Used by KMW compiler
    break;

  case TSS_KEYBOARDVERSION:   // I4140
    if(!VerifyKeyboardVersion(fk, VERSION_90)) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_90FeatureOnlyKeyboardVersion);
      return FALSE;
    }
    if (!IsValidKeyboardVersion(sp->dpString)) {
      ReportCompilerMessage(KmnCompilerMessages::ERROR_KeyboardVersionFormatInvalid);
      return FALSE;
    }
    break;

  case TSS_CASEDKEYS:
    if ((msg = VerifyCasedKeys(sp)) != STATUS_Success) {
      ReportCompilerMessage(msg);
      return FALSE;
    }
    break;

  case TSS_BEGIN_NEWCONTEXT:
  case TSS_BEGIN_POSTKEYSTROKE:
    break;

  case TSS_NEWLAYER:
  case TSS_OLDLAYER:
    break;

  case TSS_DISPLAYMAP:
    // This store is allowed in older versions of Keyman, as it is a
    // compile-time only feature. Implemented only in kmc-kmn, not in
    // the legacy compilers.
    fk->extra->displayMapFilename = string_from_u16string(sp->dpString);
    break;

  default:
    ReportCompilerMessage(KmnCompilerMessages::ERROR_InvalidSystemStore);
    return FALSE;
  }
  return TRUE;
}

KMX_BOOL GetCompileTargetsFromTargetsStore(const KMX_WCHAR* store, int &targets) {
  // Compile to .kmx
  const std::vector<std::u16string> KMXKeymanTargets{
    u"windows", u"macosx", u"linux", u"desktop"
  };

  // Compile to .js
  const std::vector<std::u16string> KMWKeymanTargets{
    u"web", u"iphone", u"ipad", u"androidphone", u"androidtablet",
    u"mobile", u"tablet"
  };

  const std::u16string AnyTarget = u"any";

  targets = 0;
  auto p = new KMX_WCHAR[u16len(store)+1];
  u16cpy(p, store);
  KMX_WCHAR* ctx;
  auto token = u16tok(p, u" ", &ctx);
  while(token) {
    bool found = false;
    if(*token) {
      if(AnyTarget == token) {
        targets |= COMPILETARGETS_KMX | COMPILETARGETS_JS;
        found = true;
      }
      for(auto target: KMXKeymanTargets) {
        if(target == token) {
          targets |= COMPILETARGETS_KMX;
          found = true;
        }
      }
      for(auto target: KMWKeymanTargets) {
        if(target == token) {
          targets |= COMPILETARGETS_JS;
          found = true;
        }
      }

      if(!found) {
        ReportCompilerMessage(KmnCompilerMessages::ERROR_InvalidTarget, {
          /*target*/ string_from_u16string(token).c_str()
        });
        delete[] p;
        targets = 0;
        return FALSE;
      }
    }
    token = u16tok(nullptr, u" ", &ctx);
  }
  delete[] p;

  if(targets == 0) {
    ReportCompilerMessage(KmnCompilerMessages::ERROR_NoTargetsSpecified);
    return FALSE;
  }

  return TRUE;
}

KMX_BOOL IsValidKeyboardVersion(KMX_WCHAR *dpString) {   // I4140
  /**
    version format: /^\d+(\.\d+)*$/
    e.g. 9.0.3, 1.0, 1.2.3.4, 6.2.1.4.6.4, 11.22.3 are all ok;
    empty string is not permitted; whitespace is not permitted
  */

  do {
    if (!iswdigit(*dpString)) {
      return FALSE;
    }
    while (iswdigit(*dpString)) {
      dpString++;
    }
    if (*dpString == '.') {
      dpString++;
      if (!iswdigit(*dpString)) {
        return FALSE;
      }
    }
  } while (*dpString != 0);

  return TRUE;
}


KMX_BOOL kmcmp::AddCompilerVersionStore(PFILE_KEYBOARD fk) {
  if(!kmcmp::FShouldAddCompilerVersion) {
    return TRUE;
  }

  if (!AddStore(fk, TSS_COMPILEDVERSION, KEYMAN_VersionWin_W16)) {
    return FALSE;
  }

  return TRUE;
}

/****************************
* Rule lines
*/

KMX_DWORD CheckStatementOffsets(PFILE_KEYBOARD fk, PFILE_GROUP gp, PKMX_WCHAR context, PKMX_WCHAR output, PKMX_WCHAR key) {
  PKMX_WCHAR p, q;
  int i;
  for (p = output; *p; p = incxstr(p)) {
    if (*p == UC_SENTINEL) {
      if (*(p + 1) == CODE_INDEX) {
        int indexStore = *(p + 2) - 1;
        int contextOffset = *(p + 3);
        for (q = context, i = 1; *q && i < contextOffset; q = incxstr(q), i++);

        if (*q == 0) {
          if (!gp->fUsingKeys)
            // no key in the rule, so offset is past end of context
            return KmnCompilerMessages::ERROR_IndexDoesNotPointToAny;
          if (i < contextOffset) // I4914
            // offset is beyond the key
            return KmnCompilerMessages::ERROR_IndexDoesNotPointToAny;
          q = key;
        }

        // find the any
        if (*q != UC_SENTINEL || *(q + 1) != CODE_ANY)
          return KmnCompilerMessages::ERROR_IndexDoesNotPointToAny;

        int anyStore = *(q + 2) - 1;

        const int anyLength = xstrlen(fk->dpStoreArray[anyStore].dpString);
        const int indexLength = xstrlen(fk->dpStoreArray[indexStore].dpString);

        if (indexLength < anyLength) {
          ReportCompilerMessage(KmnCompilerMessages::WARN_IndexStoreShort);
        } else if(indexLength > anyLength) {
          ReportCompilerMessage(KmnCompilerMessages::HINT_IndexStoreLong);
        }
      } else if (*(p + 1) == CODE_CONTEXTEX) {
        int contextOffset = *(p + 2);
        if (contextOffset > xstrlen(context)) {
          return KmnCompilerMessages::ERROR_ContextExHasInvalidOffset;
        }

        // contextOffset <= 0 is tested in parse stage

        for (q = context, i = 1; *q && i < contextOffset; q = incxstr(q), i++);
        if(*q == UC_SENTINEL && (*(q + 1) == CODE_IFOPT || *(q + 1) == CODE_IFSYSTEMSTORE)) {
          return KmnCompilerMessages::ERROR_ContextExCannotReferenceIf;
        }
        if(*q == UC_SENTINEL && *(q + 1) == CODE_NUL) {
          return KmnCompilerMessages::ERROR_ContextExCannotReferenceNul;
        }

        // Due to a limitation in earlier versions of KeymanWeb, the minimum version
        // for context() referring to notany() is 14.0. See #917 for details.
        if (kmcmp::CompileTarget == CKF_KEYMANWEB) {
          if (*q == UC_SENTINEL && *(q + 1) == CODE_NOTANY) {
            if(!VerifyKeyboardVersion(fk, VERSION_140)) {
              return KmnCompilerMessages::ERROR_140FeatureOnlyContextAndNotAnyWeb;
            }
          }
        }
      }
    }
  }
  return STATUS_Success;
}

/**
 * Checks that the order of statements in the context matches the specification.
 *   Rule structure: [context] ['+' key] '>' output
 *   Context structure: [nul] [if()|baselayout()|platform()]+ [char|any|context()|deadkey()|dk()|index()|notany()|outs()]
 * Test that nul is first, then if(), baselayout(), platform() statements are before any other content.
 * Also verifies that virtual keys are not found in the context.
 */
void CheckContextStatementPositions(PKMX_WCHAR context) {
  KMX_BOOL hadContextChar = FALSE;
  for (PKMX_WCHAR p = context; *p; p = incxstr(p)) {
    if (*p == UC_SENTINEL) {
      switch (*(p + 1)) {
      case CODE_NUL:
        if (p > context) {
          ReportCompilerMessage(KmnCompilerMessages::WARN_NulNotFirstStatementInContext);
        }
        break;
      case CODE_IFOPT:
      case CODE_IFSYSTEMSTORE:
        if (hadContextChar) {
          ReportCompilerMessage(KmnCompilerMessages::WARN_IfShouldBeAtStartOfContext);
        }
        break;
      case CODE_EXTENDED:
        ReportCompilerMessage(KmnCompilerMessages::ERROR_VirtualKeyInContext);
        break;
      default:
        hadContextChar = TRUE;
      }
    }
    else {
      hadContextChar = TRUE;
    }
  }
}

/**
 *  Checks if a use() statement is followed by other content in the output of a rule
 */
KMX_DWORD CheckUseStatementsInOutput(PKMX_WCHAR output) {
  KMX_BOOL hasUse = FALSE;
  PKMX_WCHAR p;
  for (p = output; *p; p = incxstr(p)) {
    if (*p == UC_SENTINEL && *(p + 1) == CODE_USE) {
      hasUse = TRUE;
    } else if (hasUse) {
      ReportCompilerMessage(KmnCompilerMessages::WARN_UseNotLastStatementInRule);
      break;
    }
  }
  return STATUS_Success;
}

/**
 * Warn if output has virtual keys in it, which is not supported by Core at all,
 * but was unofficially supported, but never worked properly, in Keyman for
 * Windows for many years
 */
KMX_DWORD CheckVirtualKeysInOutput(PKMX_WCHAR output) {
  PKMX_WCHAR p;
  for (p = output; *p; p = incxstr(p)) {
    if (*p == UC_SENTINEL && *(p + 1) == CODE_EXTENDED) {
      ReportCompilerMessage(KmnCompilerMessages::WARN_VirtualKeyInOutput);
      break;
    }
  }
  return STATUS_Success;
}

const KMX_BOOL CODE__IS_TEXTUAL[] = {
  -1,     // undefined                0x00
  TRUE,   // CODE_ANY                 0x01
  TRUE,   // CODE_INDEX               0x02
  TRUE,   // CODE_CONTEXT             0x03
  FALSE,  // CODE_NUL                 0x04
  FALSE,  // CODE_USE                 0x05 // may trigger text effects but indirectly
  FALSE,  // CODE_RETURN              0x06
  FALSE,  // CODE_BEEP                0x07
  TRUE,   // CODE_DEADKEY             0x08
  -1,     // unused                   0x09
  TRUE,   // CODE_EXTENDED            0x0A
  -1,     // CODE_EXTENDEDEND         0x0B (unused)
  FALSE,  // CODE_SWITCH              0x0C
  -1,     // CODE_KEY                 0x0D (never used)
  FALSE,  // CODE_CLEARCONTEXT        0x0E
  FALSE,  // CODE_CALL                0x0F // may trigger text effects but indirectly
  -1,     // UC_SENTINEL_EXTENDEDEND  0x10 (not valid with UC_SENTINEL)
  TRUE,   // CODE_CONTEXTEX           0x11
  TRUE,   // CODE_NOTANY              0x12
  FALSE,  // CODE_SETOPT              0x13
  FALSE,  // CODE_IFOPT               0x14
  FALSE,  // CODE_SAVEOPT             0x15
  FALSE,  // CODE_RESETOPT            0x16
  FALSE,  // CODE_IFSYSTEMSTORE       0x17
  FALSE   // CODE_SETSYSTEMSTORE      0x18
};

static_assert(sizeof(CODE__IS_TEXTUAL) / sizeof(CODE__IS_TEXTUAL[0]) == (CODE_LASTCODE + 1), "Size of array CODE__EMITS_TEXT not correct");

KMX_BOOL IsTextualCode(KMX_WORD code) {
  assert(code >= CODE_ANY && code <= CODE_LASTCODE);
  if(!(code >= CODE_ANY && code <= CODE_LASTCODE)) {
    return FALSE;
  }
  return CODE__IS_TEXTUAL[code];
}

/**
 * Warn if output has text before or after a nul statement, which
 * doesn't make sense -- if nul is used, it indicates no output
 */
KMX_DWORD CheckNulUsageInOutput(PKMX_WCHAR output) {
  PKMX_WCHAR p;
  KMX_BOOL hasNul = FALSE, hasOther = FALSE;
  for (p = output; *p; p = incxstr(p)) {
    if (*p == UC_SENTINEL) {
      auto code = *(p+1);
      if(code == CODE_NUL) {
        hasNul = TRUE;
      } else if(IsTextualCode(code)) {
        hasOther = TRUE;
      }
    } else {
      hasOther = TRUE;
    }
  }

  if(hasNul && hasOther) {
    return KmnCompilerMessages::ERROR_TextBeforeOrAfterNulInOutput;
  }

  return STATUS_Success;
}

/**
 * Adds implicit `context` to start of output of rules for readonly groups
 */
KMX_DWORD InjectContextToReadonlyOutput(PKMX_WCHAR pklOut) {
  if (pklOut[0] != UC_SENTINEL || pklOut[1] != CODE_CONTEXT) {
    if (u16len(pklOut) > GLOBAL_BUFSIZE - 3) {
      return KmnCompilerMessages::FATAL_CannotAllocateMemory;
    }
    memmove(pklOut + 2, pklOut, (u16len(pklOut) + 1) * 2);
    pklOut[0] = UC_SENTINEL;
    pklOut[1] = CODE_CONTEXT;
  }
  return STATUS_Success;
}

/**
 * Verifies that a keyboard does not attempt to emit characters or
 * other changes to text store when processing a readonly group
 */
KMX_DWORD CheckOutputIsReadonly(const PFILE_KEYBOARD fk, const PKMX_WCHAR output) {  // I4867
  PKMX_WCHAR p;
  for (p = output; *p; p = incxstr(p)) {
    if (*p != UC_SENTINEL) {
      return KmnCompilerMessages::ERROR_OutputInReadonlyGroup;
    }
    switch (*(p + 1)) {
    case CODE_CALL:
      // We cannot be sure that the callee is going to be readonly
      // but we have to operate on a trust basis for call() in any
      // case, so we'll allow it.
      continue;
    case CODE_USE:
      // We only allow use() of other readonly groups
      {
        PFILE_GROUP targetGroup = &fk->dpGroupArray[*(p + 2) - 1];
        if (!targetGroup->fReadOnly) {
          return KmnCompilerMessages::ERROR_CannotUseReadWriteGroupFromReadonlyGroup;
        }
      }
      continue;
    case CODE_SETOPT:
    case CODE_RESETOPT:
    case CODE_SAVEOPT:
      // it is okay to set, reset or save keyboard options
      // although it's hard to see good use cases for this
      continue;
    case CODE_SETSYSTEMSTORE:
      // it is okay to set system stores; Engine or Core will
      // ignore set(&) that are not permissible in the given context
      continue;
    case CODE_CONTEXT:
      // We allow `context` but only as the very first statement in output
      if (p == output) {
        continue;
      }
      return KmnCompilerMessages::ERROR_OutputInReadonlyGroup;
    default:
      // Note: conceptually, CODE_NUL could be transformed to CODE_CONTEXT
      // if the context was also empty, but it is probably safest to avoid this,
      // given CODE_CONTEXT does what we need anyway
      return KmnCompilerMessages::ERROR_StatementNotPermittedInReadonlyGroup;
    }
  }
  return STATUS_Success;
}

KMX_DWORD ProcessKeyLineImpl(PFILE_KEYBOARD fk, PKMX_WCHAR str, KMX_BOOL IsUnicode, PKMX_WCHAR pklIn, PKMX_WCHAR pklKey, PKMX_WCHAR pklOut);

KMX_DWORD ProcessKeyLine(PFILE_KEYBOARD fk, PKMX_WCHAR str, KMX_BOOL IsUnicode)
{
  PKMX_WCHAR pklIn, pklKey, pklOut;

  pklIn  = new KMX_WCHAR[GLOBAL_BUFSIZE];    // I2432 - Allocate buffers each line -- slightly slower but safer than keeping a single buffer
  pklKey = new KMX_WCHAR[GLOBAL_BUFSIZE];
  pklOut = new KMX_WCHAR[GLOBAL_BUFSIZE];
  if (!pklIn || !pklKey || !pklOut)
    return KmnCompilerMessages::FATAL_CannotAllocateMemory; // forget about the little leak if pklKey or pklOut fail...

  KMX_DWORD result = ProcessKeyLineImpl(fk, str, IsUnicode, pklIn, pklKey, pklOut);

  delete[] pklIn;   // I2432 - Allocate buffers each line -- slightly slower but safer than keeping a single buffer
  delete[] pklKey;
  delete[] pklOut;

  return result;
}

KMX_DWORD ProcessKeyLineImpl(PFILE_KEYBOARD fk, PKMX_WCHAR str, KMX_BOOL IsUnicode, PKMX_WCHAR pklIn, PKMX_WCHAR pklKey, PKMX_WCHAR pklOut) {
  PKMX_WCHAR p, pp;
  KMX_DWORD msg;
  PFILE_GROUP gp;
  PFILE_KEY kp;

  gp = &fk->dpGroupArray[fk->currentGroup];

  pp = str;

  if (gp->fUsingKeys) {
    if ((msg = GetXString(fk, str, u"+", pklIn, GLOBAL_BUFSIZE - 1, (int)(str - pp), &p, TRUE, IsUnicode)) != STATUS_Success) return msg;

    str = p + 1;
    if ((msg = GetXString(fk, str, u">", pklKey, GLOBAL_BUFSIZE - 1, (int)(str - pp), &p, TRUE, IsUnicode)) != STATUS_Success) return msg;

    if (pklKey[0] == 0) return KmnCompilerMessages::ERROR_ZeroLengthString;

    if(Uni_IsSurrogate1(pklKey[0])) {
      // #11643: non-BMP characters do not makes sense for key codes
      return KmnCompilerMessages::ERROR_NonBMPCharactersNotSupportedInKeySection;
    }

    if (xstrlen(pklKey) > 1) {
      ReportCompilerMessage(KmnCompilerMessages::WARN_KeyBadLength);
    }
  } else {
    if ((msg = GetXString(fk, str, u">", pklIn, GLOBAL_BUFSIZE - 1, (int)(str - pp), &p, TRUE, IsUnicode)) != STATUS_Success) return msg;
    if (pklIn[0] == 0) return KmnCompilerMessages::ERROR_ZeroLengthString;
  }

  str = p + 1;
  if ((msg = GetXString(fk, str, u"c\n", pklOut, GLOBAL_BUFSIZE - 1, (int)(str - pp), &p, TRUE, IsUnicode)) != STATUS_Success) return msg;

  if (pklOut[0] == 0) return KmnCompilerMessages::ERROR_ZeroLengthString;

  CheckContextStatementPositions(pklIn);

  // Test index and context offsets in context
  if ((msg = CheckStatementOffsets(fk, gp, pklIn, pklOut, pklKey)) != STATUS_Success) return msg;

  // Test that use() statements are not followed by other content
  if ((msg = CheckUseStatementsInOutput(pklOut)) != STATUS_Success) {
    return msg;   // I4867
  }

  // Warn if virtual keys are used in the output, as they are unsupported by Core
  if ((msg = CheckVirtualKeysInOutput(pklOut)) != STATUS_Success) {
    return msg;
  }

  if ((msg = CheckNulUsageInOutput(pklOut)) != STATUS_Success) {
    return msg;
  }

  if (gp->fReadOnly) {
    // Ensure no output is made from the rule, and that
    // use() statements meet required readonly semantics
    if ((msg = CheckOutputIsReadonly(fk, pklOut)) != STATUS_Success) {
      return msg;
    }

    // Inject `context` to start of output if group is readonly
    // to keep the output internally consistent
    if ((msg = InjectContextToReadonlyOutput(pklOut)) != STATUS_Success) {
      return msg;
    }
  }

  if(!resizeKeyArray(gp)) {
    return KmnCompilerMessages::FATAL_CannotAllocateMemory;
  }

  kp = &gp->dpKeyArray[gp->cxKeyArray];

  gp->cxKeyArray++;

  kp->dpOutput = new KMX_WCHAR[u16len(pklOut) + 1];
  u16ncpy(kp->dpOutput, pklOut, u16len(pklOut) + 1);  // I3481

  kp->dpContext = new KMX_WCHAR[u16len(pklIn) + 1];
  u16ncpy(kp->dpContext, pklIn, u16len(pklIn) + 1);  // I3481

  kp->Line = kmcmp::currentLine;
  kp->LineStoreIndex = 0;

  // Finished if we are not using keys

  if (!gp->fUsingKeys)
  {
    kp->Key = 0;
    kp->ShiftFlags = 0;
    return STATUS_Success;
  }

  // Expand each rule out into multiple rules - much faster processing at the key hit time

  if (*pklKey == 0) return KmnCompilerMessages::ERROR_ZeroLengthString;

  if (*pklKey == UC_SENTINEL)
    switch (*(pklKey + 1))
    {
    case CODE_ANY:
      kp->ShiftFlags = 0;
      if ((msg = ExpandKp(fk, kp, *(pklKey + 2) - 1)) != STATUS_Success) return msg;
      break;

    case CODE_EXTENDED:
      kp->Key = *(pklKey + 3);
      kp->ShiftFlags = *(pklKey + 2);
      break;

    default:
      return KmnCompilerMessages::ERROR_InvalidCodeInKeyPartOfRule;
    }
  else
  {
    kp->ShiftFlags = 0;
    kp->Key = *pklKey;
  }

  return STATUS_Success;
}



KMX_DWORD ExpandKp_ReplaceIndex(PFILE_KEYBOARD fk, PFILE_KEY k, KMX_DWORD keyIndex, int nAnyIndex)
{
  /* Replace each index(xx,keyIndex) in k->dpOutput with appropriate char as based on nAnyIndex */
  PFILE_STORE s;
  int i;
  PKMX_WCHAR pIndex, pStore;

  for (pIndex = k->dpOutput; *pIndex; pIndex = incxstr(pIndex))
  {
    if (*pIndex == UC_SENTINEL && *(pIndex + 1) == CODE_INDEX && *(pIndex + 3) == keyIndex)
    {
      s = &fk->dpStoreArray[*(pIndex + 2) - 1];
      for (i = 0, pStore = s->dpString; i < nAnyIndex; i++, pStore = incxstr(pStore));
      PKMX_WCHAR qStore = incxstr(pStore);

      int w = (int)(qStore - pStore);
      if (w > 4)
      {
        *pIndex = UC_SENTINEL;
        *(pIndex + 1) = CODE_BEEP;
        memmove(pIndex + 2, pIndex + 4, u16len(pIndex + 3) * 2);
      }
      else
      {
        memcpy(pIndex, pStore, w * 2);
        if (w < 4) memmove(pIndex + w, pIndex + 4, u16len(pIndex + 3) * 2);
      }
    }
  }

  return STATUS_Success;
}


KMX_DWORD ExpandKp(PFILE_KEYBOARD fk, PFILE_KEY kpp, KMX_DWORD storeIndex)
{
  PFILE_KEY k;
  PKMX_WCHAR pn;
  KMX_DWORD nchrs, n;
  int keyIndex;

  PFILE_STORE sp = &fk->dpStoreArray[storeIndex];
  PFILE_GROUP gp = &fk->dpGroupArray[fk->currentGroup];

  PKMX_WCHAR dpContext = kpp->dpContext;
  PKMX_WCHAR dpOutput = kpp->dpOutput;

  nchrs = xstrlen(sp->dpString);
  pn = sp->dpString;
  keyIndex = xstrlen(dpContext) + 1;

  /*
   Now we change them to plain characters in the output in multiple rules,
   and set the keystroke to the appropriate character in the store.
  */

  int offset = (int)(kpp - gp->dpKeyArray);

  if (!resizeKeyArray(gp, nchrs)) {
    return KmnCompilerMessages::FATAL_CannotAllocateMemory;
  }

  kpp = &gp->dpKeyArray[offset];
  gp->cxKeyArray += nchrs - 1;

  for (k = kpp, n = 0, pn = sp->dpString; *pn; pn = incxstr(pn), k++, n++)
  {
    k->dpContext = new KMX_WCHAR[u16len(dpContext) + 1];
    k->dpOutput = new KMX_WCHAR[u16len(dpOutput) + 1];

    u16ncpy(k->dpContext, dpContext, u16len(dpContext) + 1);	// copy the context.  // I3481
    u16ncpy(k->dpOutput, dpOutput, u16len(dpOutput) + 1);		// copy the output.

    if (*pn == UC_SENTINEL)
    {
      switch (*(pn + 1))
      {
      case CODE_EXTENDED:
        k->Key = *(pn + 3);		// set the key to store offset.
        k->ShiftFlags = *(pn + 2);
        break;
      default:
        return KmnCompilerMessages::ERROR_CodeInvalidInKeyStore;
      }
    } else if(Uni_IsSurrogate1(*pn)) {
      // #11643: non-BMP characters do not makes sense for key codes
      return KmnCompilerMessages::ERROR_NonBMPCharactersNotSupportedInKeySection;
    } else {
      k->Key = *pn;				// set the key to store offset.
      k->ShiftFlags = 0;
    }
    k->Line = kpp->Line;
    k->LineStoreIndex = n;
    ExpandKp_ReplaceIndex(fk, k, keyIndex, n);
  }

  delete[] dpContext;
  delete[] dpOutput;

  return STATUS_Success;
}

/**
 * When called with a pointer to a wide-character C-string string, the open and close delimiters, and
 * optional flags, returns a pointer to the section of the KMX_WCHAR string identified by the delimiters.
 * The supplied string will be terminated by a null where the close delimiter was.  The pointer to the supplied
 * string is adjusted to point either to the null where the close delimiter was, or if there are trailing
 * whitespaces after the close delimiter, to the last of these.  Whitespaces before the open delimiter
 * are always skipped.  If the flag contains GDS_CUTLEAD, whitespaces after the open delimiter are skipped;
 * if the flag contains GDS_CUTFOLL, whitespace immediately before the close delimiter is skipped by setting
 * the first such character to null.
 *
 * @param p a pointer to a wide-character C-string
 *
 * @param Delimiters a pointer to a two-character wide-character C-string containing the open and close
 * delimiters
 *
 * @param Flags include GDS_CUTLEAD and/or GDS_CUTFOLL to cut leading and/or following whitespace from
 * the delimited string
 *
 * @return a pointer to the section of the wide-character C-string identified by the delimiters, or NULL if
 * the delimiters cannot be found
*/
PKMX_WCHAR GetDelimitedString(PKMX_WCHAR *p, KMX_WCHAR const * Delimiters, KMX_WORD Flags)
{
  PKMX_WCHAR q, r;
  KMX_WCHAR dOpen, dClose;

  dOpen = *Delimiters; dClose = *(Delimiters + 1);

  q = *p;
  while (iswspace(*q)) q++;            //***QUERY

  if (*q != dOpen) return NULL;

  q++;

  r = (PKMX_WCHAR) u16chr(q, dClose);			        // Find closing delimiter
  if (!r) return NULL;

  if (Flags & GDS_CUTLEAD)
    while (iswspace(*q)) q++;	        // cut off leading spaces

  if (Flags & GDS_CUTFOLL)
    if (!iswspace(*(r - 1))) *r = 0; // delete close delimiter
    else
    {
      r--;							// Cut off following spaces
      while (iswspace(*r) && r > q) r--;
      if (!iswspace(*r)) r++;

      *r = 0; // delete first following space

      r = (PKMX_WCHAR) u16chr((r + 1), dClose);

      *r = 0; // delete close delimiter
    }
  else *r = 0; // delete close delimiter

  r++; while (iswspace(*r)) r++;	        // Ignore spaces after the close
  if (*r == 0) r--;					    // Safety for terminating strings.

  *p = r;								    // Update pointer position

  return q;							// Return delimited string
}

#ifndef __iswcsym
#define __iswcsym(_c) (iswalnum(_c) || ((_c)=='_'))
#endif

LinePrefixType GetLinePrefixType(PKMX_WCHAR *p)
{
  PKMX_WCHAR s = *p;

  while (iswspace(*s)) s++;

  PKMX_WCHAR q = s;

  if (*s != '$') return lptNone;

  /* I1569 - fix named constants at the start of the line */
  s++;
  while (__iswcsym(*s)) s++;
  if (*s != ':') return lptNone;

  if (u16nicmp(q, u"$keyman:", 8) == 0)
  {
    *p += 8;
    return lptKeymanAndKeymanWeb;
  }
  if (u16nicmp(q, u"$keymanweb:", 11) == 0)
  {
    *p += 11;
    return lptKeymanWebOnly;
  }
  if (u16nicmp(q, u"$keymanonly:", 12) == 0)
  {
    *p += 12;
    return lptKeymanOnly;
  }

  return lptOther;
}

int LineTokenType(PKMX_WCHAR *str)
{
  int i;
  size_t l;
  PKMX_WCHAR p = *str;

  LinePrefixType lpt = GetLinePrefixType(&p);
  if (lpt == lptOther) return T_BLANK;

  /* Test KeymanWeb, Keyman and KeymanOnly prefixes */
  if (kmcmp::CompileTarget == CKF_KEYMAN && lpt == lptKeymanWebOnly) return T_BLANK;
  if (kmcmp::CompileTarget == CKF_KEYMANWEB && lpt == lptKeymanOnly) return T_BLANK;

  while (iswspace(*p)) p++;

  if (u16chr(LineTokens[0], towupper(*p)))
    for (i = 0; i <= T_W_END - T_W_START; i++)
    {
      l = u16len(LineTokens[i + 1]);
      if (u16nicmp(p, LineTokens[i + 1], l) == 0)
      {
        p += l; while (iswspace(*p)) p++; *str = p;
        return i + T_W_START;
      }
    }

  switch (towupper(*p))
  {
  case 'C':
    if (iswspace(*(p + 1))) return T_COMMENT;
    break;
  case 0:
    return T_BLANK;
  default:
    if (u16chr(u"\"aAbBlLpPnN[OoxXdD0123456789\'+UuiI$", *p))   // I4784
    {
      *str = p;
      return T_KEYTOKEY;
    }
  }
  return T_UNKNOWN;
}

KMX_WCHAR const * DeadKeyChars =
u"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_";

KMX_BOOL StrValidChrs(PKMX_WCHAR q, KMX_WCHAR const * chrs)
{
  for (; *q; q++)
    if (!u16chr(chrs, *q)) return FALSE;
  return TRUE;
}

KMX_DWORD GetXStringImpl(PKMX_WCHAR tstr, PFILE_KEYBOARD fk, PKMX_WCHAR str, KMX_WCHAR const * token,
  PKMX_WCHAR output, int max, int offset, PKMX_WCHAR *newp, int isUnicode
);

KMX_DWORD GetXString(PFILE_KEYBOARD fk, PKMX_WCHAR str, KMX_WCHAR const * token,
  PKMX_WCHAR output, int max, int offset, PKMX_WCHAR *newp, int /*isVKey*/, int isUnicode
) {
  PKMX_WCHAR tstr = new KMX_WCHAR[max];    // I2432 - Allocate buffers each line -- slightly slower but safer than keeping a single buffer - GetXString is re-entrant with if()
  KMX_DWORD err = GetXStringImpl(tstr, fk, str, token, output, max, offset, newp, isUnicode);
  delete[] tstr;
  return err;
}

KMX_DWORD GetXStringImpl(PKMX_WCHAR tstr, PFILE_KEYBOARD fk, PKMX_WCHAR str, KMX_WCHAR const * token,
  PKMX_WCHAR output, int max, int offset, PKMX_WCHAR *newp, int isUnicode
) {
  KMX_DWORD err;
  PKMX_WCHAR p = str, q, r;
  int type, mx = 0, n, n1, n2, tokenFound = FALSE, z, j;
  KMX_DWORD i;
  KMX_WCHAR c;

  *tstr = 0;

  *output = 0;

  p = str;
  do
  {
    if (mx >= max) {
      // This is an error condition, we want the compiler
      // to crash if we reach this
      return KmnCompilerMessages::FATAL_BufferOverflow;
    }

    tokenFound = FALSE;
    while (iswspace(*p) && !u16chr(token, *p)) p++;
    if (!*p) break;

    ErrChr = (int)(p - str) + offset + 1;

    /*
    char *tokenTypes[] = {
      "clearcontext", "deadkey", "context", "return", "switch",
      "index", "outs", "beep", "nul", "use", "any", "fix", "dk", "k_", "x", "d", "c",
      "[", "]" };
    */

    switch (towupper(*p))
    {
    case 'X':
    case 'D':  type = 0; break;		// xFF, d130: chars, deadkey(n)
    case '\"': type = 1; break;		// "xxxx": chars
    case '\'': type = 2; break;		// 'xxxx': chars
    case 'A':  type = 3; break;		// any(s)
    case 'B':  type = 4; break;		// beep, baselayout (synonym for if(&baselayout))  // I3430
    case 'I':  type = 5; break;		// index(s,n), if
    case 'O':  type = 6; break;		// outs(s)
    case 'C':  type = 7; break;		// context, comments, clearcontext, call(s)
    case 'N':  type = 8; break;		// nul, notany
    case 'U':  type = 9; break;		// use(g)
    case 'R':  type = 10; break;	// return, reset
    case '[':  type = 11; break;	// start of vkey section
    //case ']':  type = 12; break;	// end of vkey section
    //case 'K':  type = 13; break;	// virtual key name or "key"
    case 'S':  type = 14; break;	// switch, set, save
    case 'F':  type = 15; break;	// fix (synonym for clearcontext)
    case '$':  type = 16; break;	// named code constants
    case 'P':  type = 17; break;  // platform (synonym for if(&platform))  // I3430
    case 'L':  type = 18; break;  // layer (synonym for set(&layer))  // I3437
    case '.':  type = 19; break;  // .. allows us to specify ranges -- either vkeys or characters
    default:
      if (iswdigit(*p)) type = 0;	// octal number
      else type = 99;				// error!
    }
    if (u16chr(token, *p)) tokenFound = TRUE;

    switch (type)
    {
    case 99:
      if (tokenFound) break;
      // TODO: report an error about the missing token details --> "token: %c",(int)*p);
      return KmnCompilerMessages::ERROR_InvalidToken;
    case 0:
      if (u16nicmp(p, u"deadkey", z = 7) == 0 ||
        u16nicmp(p, u"dk", z = 2) == 0)
      {
        p += z;
        q = GetDelimitedString(&p, u"()", GDS_CUTLEAD | GDS_CUTFOLL);
        if (!q || !*q) return KmnCompilerMessages::ERROR_InvalidDeadkey;

        tstr[mx++] = UC_SENTINEL;
        tstr[mx++] = CODE_DEADKEY;
        if (!StrValidChrs(q, DeadKeyChars)) return KmnCompilerMessages::ERROR_InvalidDeadkey;
        tstr[mx++] = GetDeadKey(fk, q); //atoiW(q); 7-5-01: named deadkeys
        tstr[mx] = 0;
      }
      else
      {
        n = xatoi(&p);
        if (*p != '\0' && !iswspace(*p)) return KmnCompilerMessages::ERROR_InvalidValue;
        if ((err = kmcmp::UTF32ToUTF16(n, &n1, &n2)) != STATUS_Success) return err;
        tstr[mx++] = n1;
        if (n2 >= 0) tstr[mx++] = n2;
        tstr[mx] = 0;
      }
      continue;

    case 1:
      q = (PKMX_WCHAR) u16chr(p + 1, '\"');
      if (!q) return KmnCompilerMessages::ERROR_UnterminatedString;
      if ((int)(q - p) - 1 + mx > max) return KmnCompilerMessages::ERROR_ExtendedStringTooLong;
      u16ncat(tstr,  p + 1, (int)(q - p) - 1);  // I3481
      mx += (int)(q - p) - 1;
      tstr[mx] = 0;
      p = q + 1;
      continue;
    case 2:
      q = (PKMX_WCHAR) u16chr(p + 1, '\'');
      if (!q) return KmnCompilerMessages::ERROR_UnterminatedString;
      if ((int)(q - p) - 1 + mx > max) return KmnCompilerMessages::ERROR_ExtendedStringTooLong;
      u16ncat(tstr,  p + 1, (int)(q - p) - 1);  // I3481
      mx += (int)(q - p) - 1;
      tstr[mx] = 0;
      p = q + 1;
      continue;
    case 3:
      if (u16nicmp(p, u"any", 3) != 0) return KmnCompilerMessages::ERROR_InvalidToken;
      p += 3;
      q = GetDelimitedString(&p, u"()", GDS_CUTLEAD | GDS_CUTFOLL);
      if (!q || !*q) return KmnCompilerMessages::ERROR_InvalidAny;

      for (i = 0; i < fk->cxStoreArray; i++)
      {
        if (u16icmp(q, fk->dpStoreArray[i].szName) == 0) break;
      }
      if (i == fk->cxStoreArray) return KmnCompilerMessages::ERROR_StoreDoesNotExist;

      if (!*fk->dpStoreArray[i].dpString) return KmnCompilerMessages::ERROR_ZeroLengthString;
      kmcmp::CheckStoreUsage(fk, i, TRUE, FALSE, FALSE);

      tstr[mx++] = UC_SENTINEL;
      tstr[mx++] = CODE_ANY;
      tstr[mx++] = (KMX_WCHAR)i + 1;	// store to index + 1, avoids End-of-string
      tstr[mx] = 0;
      continue;
    case 4:
      if (u16nicmp(p, u"beep", 4) == 0)
      {
        p += 4;
        tstr[mx++] = UC_SENTINEL;
        tstr[mx++] = CODE_BEEP;
        tstr[mx] = 0;
      }
      else if (u16nicmp(p, u"baselayout", 10) == 0)  // I3430
      {
        if(!VerifyKeyboardVersion(fk, VERSION_90)) {
          return KmnCompilerMessages::ERROR_90FeatureOnly_IfSystemStores;
        }
        p += 10;
        q = GetDelimitedString(&p, u"()", GDS_CUTLEAD | GDS_CUTFOLL);
        if (!q || !*q) return KmnCompilerMessages::ERROR_InvalidToken;
        err = process_baselayout(fk, q, tstr, &mx);
        if (err != STATUS_Success) return err;
      }
      else
        return KmnCompilerMessages::ERROR_InvalidToken;

      continue;
    case 5:
      if (u16nicmp(p, u"if", 2) == 0)
      {
        if(!VerifyKeyboardVersion(fk, VERSION_80)) {
          return KmnCompilerMessages::ERROR_80FeatureOnly;
        }
        p += 2;
        q = GetDelimitedString(&p, u"()", GDS_CUTLEAD | GDS_CUTFOLL);
        if (!q || !*q) return KmnCompilerMessages::ERROR_InvalidIf;

        err = process_if(fk, q, tstr, &mx);
        if (err != STATUS_Success) return err;
      }
      else
      {
        if (u16nicmp(p, u"index", 5) != 0) return KmnCompilerMessages::ERROR_InvalidToken;
        p += 5;
        q = GetDelimitedString(&p, u"()", GDS_CUTLEAD | GDS_CUTFOLL);

        if (!q || !*q) return KmnCompilerMessages::ERROR_InvalidIndex;

        {
          KMX_WCHAR *context = NULL;
          r = u16tok(q, ',', &context);
          if (!r) return KmnCompilerMessages::ERROR_InvalidIndex;
          r = u16trim(r);

          for (i = 0; i < fk->cxStoreArray; i++)
          {
            if (u16icmp(r, fk->dpStoreArray[i].szName) == 0) break;
          }
          if (i == fk->cxStoreArray) return KmnCompilerMessages::ERROR_StoreDoesNotExist;

          kmcmp::CheckStoreUsage(fk, i, TRUE, FALSE, FALSE);

          r = context;
          if (!r || !*r ) return KmnCompilerMessages::ERROR_InvalidIndex;
          r = u16trim(r);
          if (!isIntegerWstring(r) || atoiW(r) < 1) return KmnCompilerMessages::ERROR_InvalidIndex;
        }
        tstr[mx++] = UC_SENTINEL;
        tstr[mx++] = CODE_INDEX;
        tstr[mx++] = (KMX_WCHAR)i + 1;	    // avoid EOS for stores
        tstr[mx++] = atoiW(r);	// character offset of original any.

        tstr[mx] = 0;
      }
      continue;
    case 6:
      if (u16nicmp(p, u"outs", 4) != 0) return KmnCompilerMessages::ERROR_InvalidToken;
      p += 4;
      q = GetDelimitedString(&p, u"()", GDS_CUTLEAD | GDS_CUTFOLL);
      if (!q || !*q) return KmnCompilerMessages::ERROR_InvalidOuts;

      for (i = 0; i < fk->cxStoreArray; i++)
      {
        if (u16icmp(q, fk->dpStoreArray[i].szName) == 0) break;
      }
      if (i == fk->cxStoreArray) return KmnCompilerMessages::ERROR_StoreDoesNotExist;

      kmcmp::CheckStoreUsage(fk, i, TRUE, FALSE, FALSE);

      for (q = fk->dpStoreArray[i].dpString; *q; q++)
      {
        tstr[mx++] = *q;
        if (mx >= max - 1) {
          return KmnCompilerMessages::ERROR_OutsTooLong;
        }
      }
      tstr[mx] = 0;
      continue;
    case 7:
      if (iswspace(*(p + 1))) break;		// is a comment -- pre-stripped - so why this test?
      if (u16nicmp(p, u"context", 7) == 0)
      {
        p += 7;

        q = GetDelimitedString(&p, u"()", GDS_CUTLEAD | GDS_CUTFOLL);
        if (q && *q)
        {
          if(!VerifyKeyboardVersion(fk, VERSION_60)) {
            return KmnCompilerMessages::ERROR_60FeatureOnly_Contextn;
          }
          int n1b;
          n1b = atoiW(q);
          if (n1b < 1 || n1b >= 0xF000) {
            return KmnCompilerMessages::ERROR_ContextExHasInvalidOffset;
          }
          tstr[mx++] = UC_SENTINEL;
          tstr[mx++] = CODE_CONTEXTEX;
          tstr[mx++] = n1b;
          tstr[mx] = 0;
        }
        else
        {
          tstr[mx++] = UC_SENTINEL;
          tstr[mx++] = CODE_CONTEXT;
          tstr[mx] = 0;
        }
      }
      else if (u16nicmp(p, u"clearcontext", 12) == 0)
      {
        p += 12;
        tstr[mx++] = UC_SENTINEL;
        tstr[mx++] = CODE_CLEARCONTEXT;
        tstr[mx] = 0;
      }
      else if (u16nicmp(p, u"call", 4) == 0)
      {
        if(!VerifyKeyboardVersion(fk, VERSION_501)) {
          return KmnCompilerMessages::ERROR_501FeatureOnly_Call;
        }
        p += 4;
        q = GetDelimitedString(&p, u"()", GDS_CUTLEAD | GDS_CUTFOLL);
        if (!q || !*q) return KmnCompilerMessages::ERROR_InvalidCall;

        for (i = 0; i < fk->cxStoreArray; i++)
        {
          if (u16icmp(q, fk->dpStoreArray[i].szName) == 0) break;
        }
        if (i == fk->cxStoreArray) return KmnCompilerMessages::ERROR_StoreDoesNotExist;

        if (!kmcmp::IsValidCallStore(&fk->dpStoreArray[i])) return KmnCompilerMessages::ERROR_InvalidCall;
        kmcmp::CheckStoreUsage(fk, i, FALSE, FALSE, TRUE);

        tstr[mx++] = UC_SENTINEL;
        tstr[mx++] = CODE_CALL;
        tstr[mx++] = (KMX_WCHAR)i + 1;
        tstr[mx] = 0;

        fk->dpStoreArray[i].dwSystemID = TSS_CALLDEFINITION;
      }
      else
        return KmnCompilerMessages::ERROR_InvalidToken;
      continue;
    case 8:
      if (u16nicmp(p, u"notany", 6) == 0)
      {
        if(!VerifyKeyboardVersion(fk, VERSION_70)) {
          return KmnCompilerMessages::ERROR_70FeatureOnly;
        }
        p += 6;
        q = GetDelimitedString(&p, u"()", GDS_CUTLEAD | GDS_CUTFOLL);
        if (!q || !*q) return KmnCompilerMessages::ERROR_InvalidAny;

        for (i = 0; i < fk->cxStoreArray; i++)
        {
          if (u16icmp(q, fk->dpStoreArray[i].szName) == 0) break;
        }
        if (i == fk->cxStoreArray) return KmnCompilerMessages::ERROR_StoreDoesNotExist;
        kmcmp::CheckStoreUsage(fk, i, TRUE, FALSE, FALSE);
        tstr[mx++] = UC_SENTINEL;
        tstr[mx++] = CODE_NOTANY;
        tstr[mx++] = (KMX_WCHAR)i + 1;	// store to index + 1, avoids End-of-string
        tstr[mx] = 0;
        continue;
      }
      if (u16nicmp(p, u"nul", 3) != 0) return KmnCompilerMessages::ERROR_InvalidToken;

      p += 3;
      tstr[mx++] = UC_SENTINEL;
      tstr[mx++] = CODE_NUL;
      tstr[mx] = 0;
      continue;
    case 9:
      if (u16nicmp(p, u"use", 3) != 0)
      {
        if (*(p + 1) == '+')
        {
          n = xatoi(&p);
          if (*p != '\0' && !iswspace(*p)) return KmnCompilerMessages::ERROR_InvalidValue;
          if ((err = kmcmp::UTF32ToUTF16(n, &n1, &n2)) != STATUS_Success) return err;
          tstr[mx++] = n1;
          if (n2 >= 0) tstr[mx++] = n2;
          tstr[mx] = 0;
          if (!isUnicode) {
            ReportCompilerMessage(KmnCompilerMessages::WARN_UnicodeInANSIGroup);
          }
          continue;
        }
        return KmnCompilerMessages::ERROR_InvalidToken;
      }
      p += 3;

      q = GetDelimitedString(&p, u"()", GDS_CUTLEAD | GDS_CUTFOLL);
      if (!q || !*q) return KmnCompilerMessages::ERROR_InvalidUse;
      tstr[mx++] = UC_SENTINEL;
      tstr[mx++] = CODE_USE;
      tstr[mx] = GetGroupNum(fk, q);
      if (tstr[mx] == 0) return KmnCompilerMessages::ERROR_GroupDoesNotExist;
      tstr[++mx] = 0;
      continue;
    case 10:
      if (u16nicmp(p, u"reset", 5) == 0)
      {
        if(!VerifyKeyboardVersion(fk, VERSION_80)) {
          return KmnCompilerMessages::ERROR_80FeatureOnly;
        }
        p += 5;
        q = GetDelimitedString(&p, u"()", GDS_CUTLEAD | GDS_CUTFOLL);
        if (!q || !*q) return KmnCompilerMessages::ERROR_InvalidReset;

        err = process_reset(fk, q, tstr, &mx);
        if (err != STATUS_Success) return err;
      }
      else
      {
        if (u16nicmp(p, u"return", 6) != 0) return KmnCompilerMessages::ERROR_InvalidToken;

        p += 6;
        tstr[mx++] = UC_SENTINEL;
        tstr[mx++] = CODE_RETURN;
        tstr[mx] = 0;
        u16ncpy(output, tstr, max);  // I3481
        output[max - 1] = 0;
        return 0;
      }
      continue;
    case 11:
      {
        int sFlag = ISVIRTUALKEY /* 0 */;
        KMX_BOOL finished = FALSE;
        KMX_BOOL wsRequired = FALSE;

        p++;

        //printf("--EXTENDEDSTRING--\n");

        do
        {
          if (wsRequired && !iswspace(*p))
            return KmnCompilerMessages::ERROR_InvalidToken; // #12307

          while (iswspace(*p)) p++;

          switch (towupper(*p))
          {
          case 'N':
            if (u16nicmp(p, u"NCAPS", 5) == 0)
              sFlag |= NOTCAPITALFLAG, wsRequired = TRUE, p += 5;
            else finished = TRUE;
            break;
          case 'L':
            if (u16nicmp(p, u"LALT", 4) == 0)
              sFlag |= LALTFLAG, wsRequired = TRUE, p += 4;
            else if (u16nicmp(p, u"LCTRL", 5) == 0)
              sFlag |= LCTRLFLAG, wsRequired = TRUE, p += 5;
            else finished = TRUE;
            break;
          case 'R':
            if (u16nicmp(p, u"RALT", 4) == 0)
              sFlag |= RALTFLAG, wsRequired = TRUE, p += 4;
            else if (u16nicmp(p, u"RCTRL", 5) == 0)
              sFlag |= RCTRLFLAG, wsRequired = TRUE, p += 5;
            else finished = TRUE;
            break;
          case 'A':
            if (u16nicmp(p, u"ALT", 3) == 0)
              sFlag |= K_ALTFLAG, wsRequired = TRUE, p += 3;
            else finished = TRUE;
            break;
          case 'C':
            if (u16nicmp(p, u"CTRL", 4) == 0)
              sFlag |= K_CTRLFLAG, wsRequired = TRUE, p += 4;
            else if (u16nicmp(p, u"CAPS", 4) == 0)
              sFlag |= CAPITALFLAG, wsRequired = TRUE, p += 4;
            else finished = TRUE;
            break;
          case 'S':
            if (u16nicmp(p, u"SHIFT", 5) == 0)
              sFlag |= K_SHIFTFLAG, wsRequired = TRUE, p += 5;
            else finished = TRUE;
            break;
          default:
            finished = TRUE;
            break;
          }
        } while (!finished);

        if ((sFlag & (LCTRLFLAG | LALTFLAG)) && (sFlag & (RCTRLFLAG | RALTFLAG))) {
          ReportCompilerMessage(KmnCompilerMessages::WARN_MixingLeftAndRightModifiers);
        }

        // If we use chiral modifiers, or we use state keys, and we target web in
        // the keyboard, and we don't manually specify a keyboard version, bump
        // the minimum version to 10.0. This makes an assumption that if we are
        // using these features in a keyboard and it has no version specified,
        // that we want to use the features in the web target platform, even if
        // there are platform() rules excluding this possibility. In that (rare)
        // situation, the keyboard developer should simply specify the &version to
        // be 9.0 or whatever to avoid this behaviour.
        if (sFlag & (LCTRLFLAG | LALTFLAG | RCTRLFLAG | RALTFLAG | CAPITALFLAG | NOTCAPITALFLAG | NUMLOCKFLAG | NOTNUMLOCKFLAG | SCROLLFLAG | NOTSCROLLFLAG) &&
          kmcmp::CompileTarget == CKF_KEYMANWEB &&
          fk->dwFlags & KF_AUTOMATICVERSION) {
          if(!VerifyKeyboardVersion(fk, VERSION_100)) {
            return STATUS_Success;
          }
        }
        //printf("sFlag: %x\n", sFlag);

        tstr[mx++] = UC_SENTINEL;
        tstr[mx++] = CODE_EXTENDED;
        tstr[mx++] = sFlag;

        q = p;

        if (*q == ']') {
          return KmnCompilerMessages::ERROR_InvalidToken; // I3137 - key portion of VK is missing e.g. "[CTRL ALT]", this generates invalid kmx file that can crash Keyman or compiler later on   // I3511
        }

        if (*q == '\'' || *q == '"') {
          if(!VerifyKeyboardVersion(fk, VERSION_60)) {
            return KmnCompilerMessages::ERROR_60FeatureOnly_VirtualCharKey;
          }

          if (!kmcmp::FMnemonicLayout) {
            ReportCompilerMessage(KmnCompilerMessages::WARN_VirtualCharKeyWithPositionalLayout);
          }

          KMX_WCHAR chQuote = *q;

          q++; // skip quote
          if (*q == chQuote || *q == '\n' || *q == 0)
            return KmnCompilerMessages::ERROR_InvalidToken;

          tstr[mx - 1] |= VIRTUALCHARKEY;
          tstr[mx++] = *q;

          q++; // skip key
          if (*q != chQuote)
            return KmnCompilerMessages::ERROR_InvalidToken;
          q++; // skip quote
        } else {
          for (j = 0; !iswspace(*q) && *q != ']' && *q != 0; q++, j++);

          if (*q == 0)
            return KmnCompilerMessages::ERROR_InvalidToken;

          KMX_WCHAR vkname[SZMAX_VKDICTIONARYNAME];  // I3438

          if (j >= SZMAX_VKDICTIONARYNAME)
            return KmnCompilerMessages::ERROR_InvalidToken;

          u16ncpy(vkname, p, j);  // I3481
          vkname[j] = 0;

          if (u16icmp(vkname, u"K_NPENTER") == 0)
            i = 5;  // I649 - K_NPENTER hack
          else {
            for (i = 0; i <= VK__MAX; i++) {
              if (u16icmp(vkname, VKeyNames[i]) == 0 || u16icmp(vkname, VKeyISO9995Names[i]) == 0)
                break;
            }
          }

          if (i == VK__MAX + 1) {
            if(!VerifyKeyboardVersion(fk, VERSION_90)) {
              return KmnCompilerMessages::ERROR_InvalidToken;
            }

            i = GetVKCode(fk, vkname);  // I3438
            if (i == 0)
              return KmnCompilerMessages::ERROR_InvalidToken;
          }

          tstr[mx++] = (int)i;

          if (kmcmp::FMnemonicLayout && (i <= VK__MAX) && VKeyMayBeVCKey[i]) {
            ReportCompilerMessage(KmnCompilerMessages::WARN_VirtualKeyWithMnemonicLayout);  // I3438
          }
        }

        while (iswspace(*q)) q++;

        if (*q != ']')
          return KmnCompilerMessages::ERROR_InvalidToken;

        tstr[mx++] = UC_SENTINEL_EXTENDEDEND;
        tstr[mx] = 0;
        //printf("--EXTENDEDEND--\n");

        p = q + 1;
      }

      continue;
    case 14:
      if (u16nicmp(p, u"set", 3) == 0)
      {
        if(!VerifyKeyboardVersion(fk, VERSION_80)) {
          return KmnCompilerMessages::ERROR_80FeatureOnly;
        }
        p += 3;
        q = GetDelimitedString(&p, u"()", GDS_CUTLEAD | GDS_CUTFOLL);
        if (!q || !*q) return KmnCompilerMessages::ERROR_InvalidSet;

        err = process_set(fk, q, tstr, &mx);
        if (err != STATUS_Success) return err;
      }
      else if (u16nicmp(p, u"save", 4) == 0)
      {
        if(!VerifyKeyboardVersion(fk, VERSION_80)) {
          return KmnCompilerMessages::ERROR_80FeatureOnly;
        }
        p += 4;
        q = GetDelimitedString(&p, u"()", GDS_CUTLEAD | GDS_CUTFOLL);
        if (!q || !*q) return KmnCompilerMessages::ERROR_InvalidSave;

        err = process_save(fk, q, tstr, &mx);
        if (err != STATUS_Success) return err;
      }
      else
      {
        if (u16nicmp(p, u"switch", 6) != 0) return KmnCompilerMessages::ERROR_InvalidToken;
        p += 6;
        q = GetDelimitedString(&p, u"()", GDS_CUTLEAD | GDS_CUTFOLL);
        if (!q || !*q) return KmnCompilerMessages::ERROR_InvalidSwitch;
        tstr[mx++] = UC_SENTINEL;
        tstr[mx++] = CODE_SWITCH;
        tstr[mx++] = atoiW(q);
        tstr[mx] = 0;
      }
      continue;
    case 15:
      if (u16nicmp(p, u"fix", 3) == 0)
      {
        p += 3;
        tstr[mx++] = UC_SENTINEL;
        tstr[mx++] = CODE_CLEARCONTEXT;
        tstr[mx] = 0;
      }
      else
        return KmnCompilerMessages::ERROR_InvalidToken;
      continue;
    case 16:
      if(!VerifyKeyboardVersion(fk, VERSION_60)) {
        return KmnCompilerMessages::ERROR_60FeatureOnly_NamedCodes;
      }
      q = p + 1;
      while (*q && !iswspace(*q)) q++;
      c = *q; *q = 0;
      n = kmcmp::CodeConstants->GetCode(p + 1, &i);
      *q = c;
      if (n == 0) return KmnCompilerMessages::ERROR_InvalidNamedCode;
      if (i < 0xFFFFFFFFL) kmcmp::CheckStoreUsage(fk, i, TRUE, FALSE, FALSE);   // I2993
      if (n > 0xFFFF)
      {
        tstr[mx++] = Uni_UTF32ToSurrogate1(n);
        tstr[mx++] = Uni_UTF32ToSurrogate2(n);
      }
      else
        tstr[mx++] = n;
      tstr[mx] = 0;
      p = q;
      continue;
    case 17:
      if (u16nicmp(p, u"platform", 8) != 0) return KmnCompilerMessages::ERROR_InvalidToken;  // I3430
      if(!VerifyKeyboardVersion(fk, VERSION_90)) {
        return KmnCompilerMessages::ERROR_90FeatureOnly_IfSystemStores;
      }
      p += 8;
      q = GetDelimitedString(&p, u"()", GDS_CUTLEAD | GDS_CUTFOLL);
      if (!q || !*q) return KmnCompilerMessages::ERROR_InvalidToken;
      err = process_platform(fk, q, tstr, &mx);
      if (err != STATUS_Success) return err;
      continue;
    case 18:  // I3437
      if (u16nicmp(p, u"layer", 5) != 0) return KmnCompilerMessages::ERROR_InvalidToken;
      if(!VerifyKeyboardVersion(fk, VERSION_90)) {
        return KmnCompilerMessages::ERROR_90FeatureOnly_SetSystemStores;
      }
      p += 5;
      q = GetDelimitedString(&p, u"()", GDS_CUTLEAD | GDS_CUTFOLL);
      if (!q || !*q) return KmnCompilerMessages::ERROR_InvalidToken;
      err = process_set_synonym(TSS_LAYER, fk, q, tstr, &mx);
      if (err != STATUS_Success) return err;
      continue;
    case 19:  // #2241
      if (*(p + 1) != '.') return KmnCompilerMessages::ERROR_InvalidToken;
      p += 2;
      err = process_expansion(fk, p, tstr, &mx, max);
      if (err != STATUS_Success) return err;
      continue;
    default:
      return KmnCompilerMessages::ERROR_InvalidToken;
    }
    if (tokenFound)
    {
      *newp = p;
      u16ncpy(output,  tstr, max);  // I3481
      output[max - 1] = 0;
      ErrChr = 0;
      return STATUS_Success;
    }
  } while (*p);

  if (!*token)
  {
    *newp = p;
    u16ncpy(output, tstr, max);  // I3481
    output[max - 1] = 0;
    ErrChr = 0;
    return STATUS_Success;
  }

  return KmnCompilerMessages::ERROR_NoTokensFound;
}

KMX_DWORD process_if_synonym(KMX_DWORD dwSystemID, PFILE_KEYBOARD fk, PKMX_WCHAR q, PKMX_WCHAR tstr, int *mx);  // I3430

KMX_DWORD process_baselayout(PFILE_KEYBOARD fk, PKMX_WCHAR q, PKMX_WCHAR tstr, int *mx)  // I3430
{
  /* baselayout(<XString+outs>) */
  return process_if_synonym(TSS_BASELAYOUT, fk, q, tstr, mx);
}

KMX_DWORD process_platform(PFILE_KEYBOARD fk, PKMX_WCHAR q, PKMX_WCHAR tstr, int *mx)  // I3430
{
  /* platform(<XString+outs>) */
  return process_if_synonym(TSS_PLATFORM, fk, q, tstr, mx);
}

KMX_DWORD process_if_synonym(KMX_DWORD dwSystemID, PFILE_KEYBOARD fk, PKMX_WCHAR q, PKMX_WCHAR tstr, int *mx)  // I3430
{
  PKMX_WCHAR temp = new KMX_WCHAR[GLOBAL_BUFSIZE];

  KMX_DWORD msg;

  PKMX_WCHAR r;

  if ((msg = GetXString(fk, q, u"", temp, GLOBAL_BUFSIZE - 1, 0, &r, FALSE, TRUE)) != STATUS_Success)
  {
    delete[] temp;
    return msg;
  }

  KMX_DWORD dwStoreID;

  if (!AddStore(fk, TSS_COMPARISON, temp, &dwStoreID)) {
    delete[] temp;
    // TODO: redundant error
    return KmnCompilerMessages::ERROR_InvalidIf;
  }

  delete[] temp;

  tstr[(*mx)++] = UC_SENTINEL;
  tstr[(*mx)++] = (KMX_WCHAR)CODE_IFSYSTEMSTORE;
  tstr[(*mx)++] = (KMX_WCHAR)(dwSystemID + 1);   // I4785
  tstr[(*mx)++] = 2;
  tstr[(*mx)++] = (KMX_WCHAR)(dwStoreID + 1);
  tstr[(*mx)] = 0;

  return STATUS_Success;
}

KMX_DWORD process_if(PFILE_KEYBOARD fk, PKMX_WCHAR q, PKMX_WCHAR tstr, int *mx)  // I3431
{
  /* if(<store> <'='|'!='> <XString+outs>) */
  KMX_DWORD i, code;
  KMX_DWORD nnot = FALSE;
  PKMX_WCHAR r = q, s = q;
  while (*s && *s != u' ' && *s != u'!' && *s !=u'=') s++;
  r = s;
  while (*s == u' ') s++;
  if (*s == u'!')
  {
    s++;
    nnot = TRUE;
  }

  if (*s != '=') return KmnCompilerMessages::ERROR_InvalidIf;
  s++;
  while (*s == ' ') s++;
  *r = 0;
  r = q;

  if (r[0] == '&')
  {
    if(!VerifyKeyboardVersion( fk, VERSION_90)) {
      return KmnCompilerMessages::ERROR_90FeatureOnly_IfSystemStores;
    }
    for (i = 0; StoreTokens[i]; i++)
    {
      if (u16icmp(r, StoreTokens[i]) == 0) break;
    }
    if (!StoreTokens[i]) return KmnCompilerMessages::ERROR_IfSystemStore_NotFound;
    code = CODE_IFSYSTEMSTORE;
  }
  else
  {
    code = CODE_IFOPT;

    for (i = 0; i < fk->cxStoreArray; i++)
    {
      if (u16icmp(r, fk->dpStoreArray[i].szName) == 0) break;
    }
    if (i == fk->cxStoreArray) return KmnCompilerMessages::ERROR_StoreDoesNotExist;
    kmcmp::CheckStoreUsage(fk, i, FALSE, TRUE, FALSE);
  }

  PKMX_WCHAR temp = new KMX_WCHAR[GLOBAL_BUFSIZE];

  KMX_DWORD msg;

  if ((msg = GetXString(fk, s, u"", temp, GLOBAL_BUFSIZE - 1, 0, &r, FALSE, TRUE)) != STATUS_Success)
  {
    delete[] temp;
    return msg;
  }

  KMX_DWORD dwStoreID;

  if (!AddStore(fk, TSS_COMPARISON, temp, &dwStoreID)) {
    delete[] temp;
    // TODO: redundant error
    return KmnCompilerMessages::ERROR_InvalidIf;
  }

  delete[] temp;

  tstr[(*mx)++] = UC_SENTINEL;
  tstr[(*mx)++] = (KMX_WCHAR)code;
  tstr[(*mx)++] = (KMX_WCHAR)(i + 1);
  tstr[(*mx)++] = nnot ? 1 : 2;
  tstr[(*mx)++] = (KMX_WCHAR)(dwStoreID + 1);
  tstr[(*mx)] = 0;

  return STATUS_Success;
}

KMX_DWORD process_reset(PFILE_KEYBOARD fk, PKMX_WCHAR q, PKMX_WCHAR tstr, int *mx)
{
  /* reset(<store>) */
  KMX_DWORD i;
  for (i = 0; i < fk->cxStoreArray; i++)
  {
    if (u16icmp(q, fk->dpStoreArray[i].szName) == 0) break;
  }
  if (i == fk->cxStoreArray) return KmnCompilerMessages::ERROR_StoreDoesNotExist;
  kmcmp::CheckStoreUsage(fk, i, FALSE, TRUE, FALSE);

  tstr[(*mx)++] = UC_SENTINEL;
  tstr[(*mx)++] = CODE_RESETOPT;
  tstr[(*mx)++] = (KMX_WCHAR)(i + 1);
  tstr[(*mx)] = 0;

  return STATUS_Success;
}

KMX_DWORD process_expansion(PFILE_KEYBOARD fk, PKMX_WCHAR q, PKMX_WCHAR tstr, int *mx, int max) {
  KMX_BOOL isVKey = FALSE;

  KMX_WORD BaseKey=0, BaseShiftFlags=0;
  KMX_DWORD BaseChar=0;

  if (*mx == 0) {
    return KmnCompilerMessages::ERROR_ExpansionMustFollowCharacterOrVKey;
  }
  PKMX_WCHAR p = &tstr[*mx];
  p = decxstr(p, tstr);
  if (*p == UC_SENTINEL) {
    if (*(p + 1) != CODE_EXTENDED) {
      return KmnCompilerMessages::ERROR_ExpansionMustFollowCharacterOrVKey;
    }
    isVKey = TRUE;
    BaseKey = *(p + 3);
    BaseShiftFlags = *(p + 2);
  }
  else {
    BaseChar = Uni_UTF16ToUTF32(p);
  }

  // Look ahead at next element
  KMX_WCHAR temp[GLOBAL_BUFSIZE];
  PKMX_WCHAR r = NULL;

  KMX_DWORD msg;

  if ((msg = GetXString(fk, q, u"", temp, (KMX_DWORD)_countof(temp) - 1, 0, &r, FALSE, TRUE)) != STATUS_Success)
  {
    return msg;
  }

  KMX_WORD HighKey, HighShiftFlags;
  KMX_DWORD HighChar;

  switch(temp[0]) {
  case 0:
    return isVKey ? KmnCompilerMessages::ERROR_VKeyExpansionMustBeFollowedByVKey : KmnCompilerMessages::ERROR_CharacterExpansionMustBeFollowedByCharacter;
  case UC_SENTINEL:
    // Verify that range is valid virtual key range
    if(!isVKey) {
      return KmnCompilerMessages::ERROR_CharacterExpansionMustBeFollowedByCharacter;
    }
    if (temp[1] != CODE_EXTENDED) {
      return KmnCompilerMessages::ERROR_VKeyExpansionMustBeFollowedByVKey;
    }
    HighKey = temp[3], HighShiftFlags = temp[2];
    if (HighShiftFlags != BaseShiftFlags) {
      return KmnCompilerMessages::ERROR_VKeyExpansionMustUseConsistentShift;
    }
    if (HighKey <= BaseKey) {
      return KmnCompilerMessages::ERROR_ExpansionMustBePositive;
    }
    // Verify space in buffer
    if (*mx + (HighKey - BaseKey) * 5 + 1 >= max) {
      return KmnCompilerMessages::ERROR_VirtualKeyExpansionTooLong;
    }
    // Inject an expansion.
    for (BaseKey++; BaseKey < HighKey; BaseKey++) {
      // < HighKey because caller will add HighKey to output
      tstr[(*mx)++] = UC_SENTINEL;
      tstr[(*mx)++] = CODE_EXTENDED;
      tstr[(*mx)++] = BaseShiftFlags;
      tstr[(*mx)++] = BaseKey;
      tstr[(*mx)++] = UC_SENTINEL_EXTENDEDEND;
    }
    tstr[*mx] = 0;
    break;
  default:
    // Verify that range is a valid character range
    if (isVKey) {
      return KmnCompilerMessages::ERROR_VKeyExpansionMustBeFollowedByVKey;
    }

    HighChar = Uni_UTF16ToUTF32(temp);
    if (HighChar <= BaseChar) {
      return KmnCompilerMessages::ERROR_ExpansionMustBePositive;
    }
    // Inject an expansion.
    for (BaseChar++; BaseChar < HighChar; BaseChar++) {
      // < HighChar because caller will add HighChar to output
      if (Uni_IsSMP(BaseChar)) {
        // We'll test on each char to avoid complex calculations crossing SMP boundary
        if (*mx + 3 >= max) {
          return KmnCompilerMessages::ERROR_CharacterRangeTooLong;
        }
        tstr[(*mx)++] = (KMX_WCHAR) Uni_UTF32ToSurrogate1(BaseChar);
        tstr[(*mx)++] = (KMX_WCHAR) Uni_UTF32ToSurrogate2(BaseChar);
      }
      else {
        if (*mx + 2 >= max) {
          return KmnCompilerMessages::ERROR_CharacterRangeTooLong;
        }
        tstr[(*mx)++] = (KMX_WCHAR) BaseChar;
      }
    }
    tstr[*mx] = 0;
  }

  return STATUS_Success;
}

KMX_DWORD process_set_synonym(KMX_DWORD dwSystemID, PFILE_KEYBOARD fk, PKMX_WCHAR q, PKMX_WCHAR tstr, int *mx)  // I3437
{
  /* set(<store> <'='> <XString+outs>), layer */
  PKMX_WCHAR temp = new KMX_WCHAR[GLOBAL_BUFSIZE], r;
  KMX_DWORD msg;

  if ((msg = GetXString(fk, q, u"", temp, GLOBAL_BUFSIZE - 1, 0, &r, FALSE, TRUE)) != STATUS_Success)
  {
    delete[] temp;
    return msg;
  }

  KMX_DWORD dwStoreID;

  if(!AddStore(fk, TSS_COMPARISON, temp, &dwStoreID)) {
    delete[] temp;
    // TODO: redundant error
    return KmnCompilerMessages::ERROR_InvalidSet;
  }

  delete[] temp;

  tstr[(*mx)++] = UC_SENTINEL;
  tstr[(*mx)++] = (KMX_WCHAR)CODE_SETSYSTEMSTORE;
  tstr[(*mx)++] = (KMX_WCHAR)(dwSystemID + 1);
  tstr[(*mx)++] = (KMX_WCHAR)(dwStoreID + 1);
  tstr[(*mx)] = 0;
  return STATUS_Success;
}

KMX_DWORD process_set(PFILE_KEYBOARD fk, PKMX_WCHAR q, PKMX_WCHAR tstr, int *mx)
{
  /* set(<store> <'='> <XString+outs> */
  PKMX_WCHAR r = q, s = q;  // I3440
  while (*s && *s != u' ' && *s != u'=') s++;
  r = s;
  while (*s == u' ') s++;
  if (*s != '=') return KmnCompilerMessages::ERROR_InvalidSet;
  s++;
  while (*s == ' ') s++;
  *r = 0;
  r = q;

  KMX_DWORD i, code;

  if (r[0] == '&')
  {
    if(!VerifyKeyboardVersion(fk, VERSION_90)) {
      return KmnCompilerMessages::ERROR_90FeatureOnly_SetSystemStores;  // I3437
    }
    for (i = 0; StoreTokens[i]; i++)
    {
      if (u16icmp(r, StoreTokens[i]) == 0) break;
    }
    if (!StoreTokens[i]) return KmnCompilerMessages::ERROR_SetSystemStore_NotFound;
    code = CODE_SETSYSTEMSTORE;
  }
  else
  {
    KMX_WCHAR *context = NULL;
    KMX_WCHAR sep_eq[3] = u" =";
    PKMX_WCHAR r2 = u16tok(q,  sep_eq, &context);  // I3481

    for (i = 0; i < fk->cxStoreArray; i++)
    {
      if (u16icmp(r2, fk->dpStoreArray[i].szName) == 0) break;
    }
    if (i == fk->cxStoreArray) return KmnCompilerMessages::ERROR_StoreDoesNotExist;
    kmcmp::CheckStoreUsage(fk, i, FALSE, TRUE, FALSE);
    code = CODE_SETOPT;
  }

  PKMX_WCHAR temp = new KMX_WCHAR[GLOBAL_BUFSIZE];

  KMX_DWORD msg;

  //r = wcstok(NULL, L" =");

  if ((msg = GetXString(fk, s, u"", temp, GLOBAL_BUFSIZE - 1, 0, &r, FALSE, TRUE)) != STATUS_Success)
  {
    delete[] temp;
    return msg;
  }

  KMX_DWORD dwStoreID;

  if(!AddStore(fk, TSS_COMPARISON, temp, &dwStoreID)) {
    delete[] temp;
    // TODO: redundant error
    return KmnCompilerMessages::ERROR_InvalidSet;
  }

  delete[] temp;

  tstr[(*mx)++] = UC_SENTINEL;
  tstr[(*mx)++] = (KMX_WCHAR)code;
  tstr[(*mx)++] = (KMX_WCHAR)(i + 1);
  tstr[(*mx)++] = (KMX_WCHAR)(dwStoreID + 1);
  tstr[(*mx)] = 0;
  return STATUS_Success;
}

KMX_DWORD process_save(PFILE_KEYBOARD fk, PKMX_WCHAR q, PKMX_WCHAR tstr, int *mx)
{
  /* save(<store>) */
  KMX_DWORD i;
  for (i = 0; i < fk->cxStoreArray; i++)
  {
    if (u16icmp(q, fk->dpStoreArray[i].szName) == 0) break;
  }
  if (i == fk->cxStoreArray) return KmnCompilerMessages::ERROR_StoreDoesNotExist;
  kmcmp::CheckStoreUsage(fk, i, FALSE, TRUE, FALSE);

  tstr[(*mx)++] = UC_SENTINEL;
  tstr[(*mx)++] = CODE_SAVEOPT;
  tstr[(*mx)++] = (KMX_WCHAR)(i + 1);
  tstr[(*mx)] = 0;
  return STATUS_Success;
}

int xatoi(PKMX_WCHAR *p)
{
  PKMX_WCHAR endptr;
  int n;

  switch (towupper(**p))
  {
  case 'U':
    (*p)++;
    if (**p != '+') return 0;
    (*p)++;
    n = (int)u16tol(*p, &endptr, 16);
    *p = endptr;
    break;
  case 'X':
    (*p)++;
    n = (int)u16tol(*p, &endptr, 16);
    *p = endptr;
    break;
  case 'D':
    (*p)++;
    n = (int)u16tol(*p, &endptr, 10);
    *p = endptr;
    break;
  default:
    n = (int)u16tol(*p, &endptr, 8);
    *p = endptr;
    break;
  }
  return n;
}

int GetGroupNum(PFILE_KEYBOARD fk, PKMX_WCHAR p)
{
  PFILE_GROUP gp;
  KMX_DWORD i;

  for (i = 0, gp = fk->dpGroupArray; i < fk->cxGroupArray; gp++, i++)
  {
    if (u16icmp(gp->szName, p) == 0) return i + 1;
  }
  return 0;
}


KMX_DWORD ProcessEthnologueStore(PKMX_WCHAR p) // I2646
{
  KMX_DWORD res = STATUS_Success;
  PKMX_WCHAR q = NULL;
  while (*p)
  {
    while (u16chr(u" ,;", *p))
    {
      if (*p != ' ') res = KmnCompilerMessages::WARN_PunctuationInEthnologueCode;
      p++;
    }
    if (q == p) return KmnCompilerMessages::ERROR_InvalidEthnologueCode;
    if (*p)
    {
      for (int i = 0; i < 3; i++)
      {
        if (!iswalpha(*p)) return KmnCompilerMessages::ERROR_InvalidEthnologueCode;
        p++;
      }
    }
    q = p;
  }
  return res;
}

#define K_HOTKEYSHIFTFLAGS (K_SHIFTFLAG | K_CTRLFLAG | K_ALTFLAG | ISVIRTUALKEY)

KMX_DWORD ProcessHotKey(PKMX_WCHAR p, KMX_DWORD *hk)
{
  PKMX_WCHAR q, r;
  KMX_DWORD sFlag;
  int j, i;

  *hk = 0;

  if (*p == UC_SENTINEL && *(p + 1) == CODE_EXTENDED) {
    KMX_WORD Key = *(p + 3);
    KMX_WORD ShiftFlags = *(p + 2);

    // Convert virtual key to hotkey (different bitflags)

    if (ShiftFlags & ~K_HOTKEYSHIFTFLAGS) {
      ReportCompilerMessage(KmnCompilerMessages::WARN_HotkeyHasInvalidModifier);
    }

    if (ShiftFlags & K_SHIFTFLAG) *hk |= HK_SHIFT;
    if (ShiftFlags & K_CTRLFLAG) *hk |= HK_CTRL;
    if (ShiftFlags & K_ALTFLAG) *hk |= HK_ALT;

    *hk |= Key;

    return STATUS_Success;
  }

  q = (PKMX_WCHAR) u16chr(p, '[');
  if (q)
  {
    q++;
    sFlag = 0;

    do
    {
      while (iswspace(*q)) q++;

      if (u16nicmp(q, u"ALT", 3) == 0) sFlag |= HK_ALT, q += 3;
      else if (u16nicmp(q, u"CTRL", 4) == 0) sFlag |= HK_CTRL, q += 4;
      else if (u16nicmp(q, u"SHIFT", 5) == 0) sFlag |= HK_SHIFT, q += 5;
      else if (towupper(*q) != 'K') return KmnCompilerMessages::ERROR_InvalidToken;
    } while (towupper(*q) != 'K');

    r = (PKMX_WCHAR) u16chr(q, ']');
    if (r)
    {
      r--;
      while (iswspace(*r) && r > q) r--;
      r++;
    }
    else return KmnCompilerMessages::ERROR_NoTokensFound;

    j = (int)(r - q);

    for (i = 0; i <= VK__MAX; i++)  // I3438
      if (j == (int) u16len(VKeyNames[i]) && u16nicmp(q, VKeyNames[i], j) == 0) break;

    if (i == VK__MAX + 1) return KmnCompilerMessages::ERROR_InvalidToken;  // I3438

    *hk = i | sFlag;

    return STATUS_Success;
  }

  q = GetDelimitedString(&p, u"\"\"", GDS_CUTLEAD | GDS_CUTFOLL);
  if (q)
  {
    if (u16chr(q, '^')) *hk |= HK_CTRL;
    if (u16chr(q, '+')) *hk |= HK_SHIFT;
    if (u16chr(q, '%')) *hk |= HK_ALT;
    q = (PKMX_WCHAR) u16chr(q, 0) - 1;
    *hk |= *q;
    return STATUS_Success;
  }

  return KmnCompilerMessages::ERROR_CodeInvalidInThisSection;
}


void SetChecksum(PKMX_BYTE buf, PKMX_DWORD CheckSum, KMX_DWORD sz)
{
  BuildCRCTable();
  *CheckSum = (KMX_DWORD)CalculateBufferCRC(buf, sz);
}


void kmcmp::CheckStoreUsage(PFILE_KEYBOARD fk, int storeIndex, KMX_BOOL fIsStore, KMX_BOOL fIsOption, KMX_BOOL fIsCall) {
  PFILE_STORE sp = &fk->dpStoreArray[storeIndex];
  if (fIsStore && !sp->fIsStore) {
    if (sp->fIsDebug || sp->fIsOption || sp->fIsReserved || sp->fIsCall) {
      ReportCompilerMessage(KmnCompilerMessages::WARN_StoreAlreadyUsedAsOptionOrCall);
    }
    sp->fIsStore = TRUE;
  } else if (fIsOption && !sp->fIsOption) {
    if (sp->fIsDebug || sp->fIsStore || sp->fIsReserved || sp->fIsCall) {
      ReportCompilerMessage(KmnCompilerMessages::WARN_StoreAlreadyUsedAsStoreOrCall);
    }
    sp->fIsOption = TRUE;
  } else if (fIsCall && !sp->fIsCall) {
    if (sp->fIsDebug || sp->fIsStore || sp->fIsReserved || sp->fIsOption) {
      ReportCompilerMessage(KmnCompilerMessages::WARN_StoreAlreadyUsedAsStoreOrOption);
    }
    sp->fIsCall = TRUE;
  }
}

KMX_DWORD WriteCompiledKeyboard(PFILE_KEYBOARD fk, KMX_BYTE**data, size_t& dataSize)
{
  PFILE_GROUP fgp;
  PFILE_STORE fsp;
  PFILE_KEY fkp;

  PCOMP_KEYBOARD ck;
  PCOMP_GROUP gp;
  PCOMP_STORE sp;
  PCOMP_KEY kp;
  PKMX_BYTE buf;
  size_t offset;
  size_t size;
  KMX_DWORD i, j;

  *data = nullptr;
  dataSize = 0;

  // Calculate how much memory to allocate

  size = sizeof(COMP_KEYBOARD) +
    fk->cxGroupArray * sizeof(COMP_GROUP) +
    fk->cxStoreArray * sizeof(COMP_STORE) +
    /*wcslen(fk->szName)*2 + 2 +
    wcslen(fk->szCopyright)*2 + 2 +
    wcslen(fk->szLanguageName)*2 + 2 +
    wcslen(fk->szMessage)*2 + 2 +*/
    fk->dwBitmapSize;

  for (i = 0, fgp = fk->dpGroupArray; i < fk->cxGroupArray; i++, fgp++)
  {
    if (kmcmp::FSaveDebug) size += u16len(fgp->szName) * 2 + 2;
    size += fgp->cxKeyArray * sizeof(COMP_KEY);
    for (j = 0, fkp = fgp->dpKeyArray; j < fgp->cxKeyArray; j++, fkp++)
    {
      size += u16len(fkp->dpOutput) * 2 + 2;
      size += u16len(fkp->dpContext) * 2 + 2;
    }

    if (fgp->dpMatch) size += u16len(fgp->dpMatch) * 2 + 2;
    if (fgp->dpNoMatch) size += u16len(fgp->dpNoMatch) * 2 + 2;
  }

  for (i = 0; i < fk->cxStoreArray; i++)
  {
    size += u16len(fk->dpStoreArray[i].dpString) * 2 + 2;
    if (kmcmp::FSaveDebug || fk->dpStoreArray[i].fIsOption) size += u16len(fk->dpStoreArray[i].szName) * 2 + 2;
  }

  buf = new KMX_BYTE[size];
  if (!buf) return KmnCompilerMessages::FATAL_CannotAllocateMemory;
  memset(buf, 0, size);

  ck = (PCOMP_KEYBOARD)buf;

  ck->dwIdentifier = FILEID_COMPILED;
  ck->dwFileVersion = fk->version;
  ck->dwCheckSum = 0;		// do checksum afterwards.
  ck->KeyboardID = fk->KeyboardID;
  ck->IsRegistered = TRUE;    // I5135
  ck->cxStoreArray = fk->cxStoreArray;
  ck->cxGroupArray = fk->cxGroupArray;
  ck->StartGroup[0] = fk->StartGroup[0];
  ck->StartGroup[1] = fk->StartGroup[1];
  ck->dwHotKey = fk->dwHotKey;

  ck->dwFlags = fk->dwFlags;

  offset = sizeof(COMP_KEYBOARD);

  /*ck->dpLanguageName = offset;
  wcscpy((PWSTR)(buf + offset), fk->szLanguageName);
  offset += wcslen(fk->szLanguageName)*2 + 2;

  ck->dpName = offset;
  wcscpy((PWSTR)(buf + offset), fk->szName);
  offset += wcslen(fk->szName)*2 + 2;

  ck->dpCopyright = offset;
  wcscpy((PWSTR)(buf + offset), fk->szCopyright);
  offset += wcslen(fk->szCopyright)*2 + 2;

  ck->dpMessage = offset;
  wcscpy((PWSTR)(buf + offset), fk->szMessage);
  offset += wcslen(fk->szMessage)*2 + 2;*/

  ck->dpStoreArray = (KMX_DWORD)offset;
  sp = (PCOMP_STORE)(buf + offset);
  fsp = fk->dpStoreArray;
  offset += sizeof(COMP_STORE) * ck->cxStoreArray;
  for (i = 0; i < ck->cxStoreArray; i++, sp++, fsp++)
  {
    sp->dwSystemID = fsp->dwSystemID;
    sp->dpString = (KMX_DWORD)offset;
    u16ncpy((PKMX_WCHAR)(buf + offset), fsp->dpString, (size - offset) / sizeof(KMX_WCHAR));  // I3481   // I3641
    offset += u16len(fsp->dpString) * 2 + 2;

    if (kmcmp::FSaveDebug || fsp->fIsOption)
    {
      sp->dpName = (KMX_DWORD)offset;
      u16ncpy((PKMX_WCHAR)(buf + offset), fsp->szName, (size - offset) / sizeof(KMX_WCHAR));  // I3481   // I3641
      offset += u16len(fsp->szName) * 2 + 2;
    }
    else sp->dpName = 0;
  }

  ck->dpGroupArray = (KMX_DWORD)offset;
  gp = (PCOMP_GROUP)(buf + offset);
  fgp = fk->dpGroupArray;

  offset += sizeof(COMP_GROUP) * ck->cxGroupArray;

  for (i = 0; i < ck->cxGroupArray; i++, gp++, fgp++)
  {
    gp->cxKeyArray = fgp->cxKeyArray;
    gp->fUsingKeys = fgp->fUsingKeys;

    gp->dpMatch = gp->dpNoMatch = 0;

    if (fgp->dpMatch)
    {
      gp->dpMatch = (KMX_DWORD)offset;
     u16ncpy((PKMX_WCHAR)(buf + offset), fgp->dpMatch, (size - offset) / sizeof(KMX_WCHAR));  // I3481   // I3641
      offset += u16len(fgp->dpMatch) * 2 + 2;
    }
    if (fgp->dpNoMatch)
    {
      gp->dpNoMatch = (KMX_DWORD)offset;
      u16ncpy((PKMX_WCHAR)(buf + offset), fgp->dpNoMatch, (size - offset) / sizeof(KMX_WCHAR));  // I3481   // I3641
      offset += u16len(fgp->dpNoMatch) * 2 + 2;
    }

    if (kmcmp::FSaveDebug)
    {
      gp->dpName = (KMX_DWORD)offset;
      u16ncpy((PKMX_WCHAR)(buf + offset), fgp->szName, (size - offset) / sizeof(KMX_WCHAR));  // I3481   // I3641
      offset += u16len(fgp->szName) * 2 + 2;
    }
    else gp->dpName = 0;

    gp->dpKeyArray = (KMX_DWORD)offset;
    kp = (PCOMP_KEY )(buf + offset);
    fkp = fgp->dpKeyArray;
    offset += gp->cxKeyArray * sizeof(COMP_KEY);
    for (j = 0; j < gp->cxKeyArray; j++, kp++, fkp++)
    {
      kp->_reserved = 0;
      kp->Key = fkp->Key;
      if (kmcmp::FSaveDebug) kp->Line = fkp->Line; else kp->Line = 0;
      kp->ShiftFlags = fkp->ShiftFlags;
      kp->dpOutput = (KMX_DWORD)offset;
      u16ncpy((PKMX_WCHAR)(buf + offset), fkp->dpOutput, (size - offset) / sizeof(KMX_WCHAR));  // I3481   // I3641
      offset += u16len(fkp->dpOutput) * 2 + 2;
      kp->dpContext = (KMX_DWORD)offset;
      u16ncpy((PKMX_WCHAR)(buf + offset), fkp->dpContext, (size - offset) / sizeof(KMX_WCHAR));  // I3481   // I3641
      offset += u16len(fkp->dpContext) * 2 + 2;
    }
  }

  ck->dwBitmapSize = fk->dwBitmapSize;
  ck->dpBitmapOffset = (KMX_DWORD)offset;
  memcpy(buf + offset, fk->lpBitmap, fk->dwBitmapSize);
  offset += fk->dwBitmapSize;

  if (offset != size) {
    delete[] buf;
    return KmnCompilerMessages::FATAL_SomewhereIGotItWrong;
  }

  SetChecksum(buf, &ck->dwCheckSum, (KMX_DWORD)size);

  *data = buf;
  dataSize = size;

  return STATUS_Success;
}

KMX_DWORD ReadLine(KMX_BYTE* infile, int sz, int& offset, PKMX_WCHAR wstr, KMX_BOOL PreProcess)
{
  KMX_DWORD len;
  PKMX_WCHAR p;
  KMX_BOOL LineCarry = FALSE, InComment = FALSE;
  KMX_DWORD n;
  KMX_WCHAR currentQuotes = 0;
  KMX_WCHAR str[LINESIZE + 3];

  if(offset >= sz)  {
    return STATUS_EndOfFile;
  }

  len = offset + LINESIZE*2 > sz ? sz-offset : LINESIZE*2;
  memcpy(str, infile+offset, len);
  offset += len;
  len /= 2;
  str[len] = 0;

  if(offset == sz) {
    // \r\n is still added here even though Linux doesn`t use \r.
    // This is to ensure to still have a working windows-only-version
    u16ncat(str, u"\r\n", _countof(str));  // I3481     // Always a "\r\n" to the EOF, avoids funny bugs
  }

  if (len == 0) {
    return STATUS_EndOfFile;
  }


  for (p = str, n = 0; n < len; n++, p++)
  {
    if (currentQuotes != 0)
    {
      if (*p == L'\n')
      {
        *p = 0;  // I2525
        u16ncpy(wstr, str, LINESIZE);  // I3481
        return (PreProcess ? STATUS_Success : KmnCompilerMessages::ERROR_UnterminatedString);
      }
      if (*p == currentQuotes) currentQuotes = 0;
      continue;
    }
    if (InComment) {
      if (*p == L'\n') break;
      *p = L' ';
      continue;
    }
    if(*p == L'\\') {
      LineCarry = TRUE;
      *p = L' ';
      continue;
    }
    if (LineCarry)
    {
      switch (*p)
      {
      case L' ':
      case L'\t':
      case L'\r':
        *p = L' ';
        continue;
      case L'\n':
        kmcmp::currentLine++;
        LineCarry = FALSE;
        *p = L' ';
        continue;
      }
      *p = 0; // I2525
      u16ncpy(wstr, str, LINESIZE);  // I3481
      return (PreProcess ? STATUS_Success : KmnCompilerMessages::ERROR_InvalidLineContinuation);
    }

    if (*p == L'\n') break;
    switch (*p)
    {
    case L'c':
    case L'C':
      if ((p == str || iswspace(*(p - 1))) && iswspace(*(p + 1))) {
        InComment = TRUE;
        *p = L' ';
      }
      continue;
    case L'\r':
    case L'\t':
      *p = L' ';
      continue;
    case L'\'':
    case L'\"':
      currentQuotes = *p;
      continue;
    }
  }

  if (n == len)
  {
    str[LINESIZE - 1] = 0;  // I2525
    u16ncpy(wstr, str, LINESIZE);  // I3481
    if (len == LINESIZE)
      return (PreProcess ? STATUS_Success : KmnCompilerMessages::ERROR_LineTooLong);
  }

  kmcmp::currentLine++;

  offset -= (int)(len * 2 - (int)(p - str) * 2 - 2);
  if(offset >= sz) {
    // If we've appended a \n, we can go past EOF
    offset = sz;
  }

  p--;
  while (p >= str && iswspace(*p)) p--;
  p++;
  *p++ = L'\n';
  *p = 0;
  // trim spaces now, why not?
  u16ncpy(wstr, str, LINESIZE);  // I3481

  return STATUS_Success;
}

KMX_DWORD GetRHS(PFILE_KEYBOARD fk, PKMX_WCHAR p, PKMX_WCHAR buf, int bufsize, int offset, int IsUnicode)
{
  PKMX_WCHAR q;
  p = (const PKMX_WCHAR) u16chr(p, '>');

  if (!p) return KmnCompilerMessages::ERROR_NoTokensFound;

  p++;

  return GetXString(fk, p, u"c\n", buf, bufsize, offset, &q, TRUE, IsUnicode);
}

void safe_wcsncpy(PKMX_WCHAR out, PKMX_WCHAR in, int cbMax)
{
  u16ncpy(out, in, cbMax - 1);  // I3481
  out[cbMax - 1] = 0;
}

KMX_BOOL IsSameToken(PKMX_WCHAR *p, KMX_WCHAR const * token)
{
  PKMX_WCHAR q;
  q = *p;
  while (iswspace(*q)) q++;
  if (u16nicmp(q, token, u16len(token)) == 0)
  {
    q += u16len(token);
    while (iswspace(*q)) q++;
    *p = q;
    return TRUE;
  }
  return FALSE;
}

static bool endsWith(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

KMX_DWORD ImportBitmapFile(PFILE_KEYBOARD fk, PKMX_WCHAR szName, PKMX_DWORD FileSize, PKMX_BYTE *Buf)
{
  auto szNameUtf8 = string_from_u16string(szName);

  std::vector<uint8_t> bufvec = loadfileproc(szNameUtf8, fk->extra->kmnFilename);
  if(!bufvec.size()) {
    // Append .bmp and try again
    if(endsWith(szNameUtf8, ".bmp")) {
      return KmnCompilerMessages::ERROR_CannotReadBitmapFile;
    }
    szNameUtf8.append(".bmp");
    bufvec = loadfileproc(szNameUtf8, fk->extra->kmnFilename);
  }

  if(bufvec.size() < 2) {
    // Zero-byte file is invalid; 2 byte file is too, but we only really care
    // about the prolog at this point so we don't overrun our buffer
    return KmnCompilerMessages::ERROR_CannotReadBitmapFile;
  }

  *FileSize = static_cast<KMX_DWORD>(bufvec.size());
  *Buf = new KMX_BYTE[*FileSize];
  std::copy(bufvec.begin(), bufvec.end(), *Buf);

  /* Test for version 7.0 icon support */
  if (*((PKMX_CHAR)*Buf) != 'B' && *(((PKMX_CHAR)*Buf) + 1) != 'M') {
    if(!VerifyKeyboardVersion(fk, VERSION_70)) {
      return KmnCompilerMessages::ERROR_70FeatureOnly;
    }
  }

  return STATUS_Success;
}

int atoiW(PKMX_WCHAR p)
{
  PKMX_STR q = wstrtostr(p);
  int i = atoi(q);
  delete[] q;
  return i;
}

/**
 * Checks if a wide-character C-string represents an integer.
 * It does not strip whitespace, and depends on the action of atoi()
 * to determine if the C-string is an integer.
 *
 * @param p a pointer to a wide-character C-string
 *
 * @return true if p represents an integer, false otherwise
*/
bool isIntegerWstring(PKMX_WCHAR p) {
  if (!p || !*p)
    return false;
  PKMX_STR q = wstrtostr(p);
  std::ostringstream os;
  os << atoi(q);
  int cmp = strcmp(q, os.str().c_str());
  delete[] q;
  return cmp == 0 ? true : false;
}

KMX_DWORD kmcmp::CheckUTF16(int n)
{
  const int res[] = {
    0xFDD0, 0xFDD1, 0xFDD2, 0xFDD3, 0xFDD4, 0xFDD5, 0xFDD6, 0xFDD7,
    0xFDD8, 0xFDD9, 0xFDDA, 0xFDDB, 0xFDDC, 0xFDDD, 0xFDDE, 0xFDDF,
    0xFDE0, 0xFDE1, 0xFDE2, 0xFDE3, 0xFDE4, 0xFDE5, 0xFDE6, 0xFDE7,
    0xFDE8, 0xFDE9, 0xFDEA, 0xFDEB, 0xFDEC, 0xFDED, 0xFDEE, 0xFDEF,
    0xFFFF, 0xFFFE, 0 };

  if (n == 0) return KmnCompilerMessages::ERROR_ReservedCharacter;
  for (int i = 0; res[i] > 0; i++)
    if (n == res[i])
    {
      ReportCompilerMessage(KmnCompilerMessages::WARN_ReservedCharacter);
      break;
    }
  return STATUS_Success;
}

KMX_DWORD kmcmp::UTF32ToUTF16(int n, int *n1, int *n2)
{
  *n2 = -1;
  if (n <= 0xFFFF)
  {
    *n1 = n;
    if (n >= 0xD800 && n <= 0xDFFF) {
      ReportCompilerMessage(KmnCompilerMessages::WARN_UnicodeSurrogateUsed);
    }
    return kmcmp::CheckUTF16(*n1);
  }

  if ((n & 0xFFFF) == 0xFFFF || (n & 0xFFFF) == 0xFFFE) {
    ReportCompilerMessage(KmnCompilerMessages::WARN_ReservedCharacter);
  }
  if (n < 0 || n > 0x10FFFF) {
    return KmnCompilerMessages::ERROR_InvalidCharacter;
  }
  n = n - 0x10000;
  *n1 = (n / 0x400) + 0xD800;
  *n2 = (n % 0x400) + 0xDC00;
  KMX_DWORD msg;
  if ((msg = kmcmp::CheckUTF16(*n1)) != STATUS_Success) return msg;
  return kmcmp::CheckUTF16(*n2);
}

KMX_BOOL BuildVKDictionary(PFILE_KEYBOARD fk) {
  KMX_DWORD i;
  size_t len = 0;
  if (fk->cxVKDictionary == 0) return TRUE;
  for (i = 0; i < fk->cxVKDictionary; i++)
  {
    len += u16len(fk->dpVKDictionary[i].szName) + 1;
  }
  PKMX_WCHAR storeval = new KMX_WCHAR[len], p = storeval;
  for (i = 0; i < fk->cxVKDictionary; i++)
  {
    u16ncpy(p, fk->dpVKDictionary[i].szName, len - (size_t)(p - storeval));  // I3481
    p = (PKMX_WCHAR) u16chr(p, 0);
    *p = ' ';
    p++;
  }

  p--;
  *p = 0;

  KMX_DWORD dwStoreID;
  if(!AddStore(fk, TSS_VKDICTIONARY, storeval, &dwStoreID)) {
    delete[] storeval;
    return FALSE;
  }
  delete[] storeval;
  return TRUE;
}

int GetVKCode(PFILE_KEYBOARD fk, PKMX_WCHAR p)
{
  KMX_DWORD i;

  for (i = 0; i < fk->cxVKDictionary; i++)
    if (u16icmp(fk->dpVKDictionary[i].szName, p) == 0)
      return i + VK__MAX + 1;  // 256

  if (fk->cxVKDictionary % 10 == 0)
  {
    PFILE_VKDICTIONARY pvk = new FILE_VKDICTIONARY[fk->cxVKDictionary + 10];
    memcpy(pvk, fk->dpVKDictionary, fk->cxVKDictionary * sizeof(FILE_VKDICTIONARY));
    delete[] fk->dpVKDictionary;
    fk->dpVKDictionary = pvk;
  }
  u16ncpy(fk->dpVKDictionary[fk->cxVKDictionary].szName, p, _countof(fk->dpVKDictionary[fk->cxVKDictionary].szName) );  // I3481
  fk->dpVKDictionary[fk->cxVKDictionary].szName[SZMAX_VKDICTIONARYNAME - 1] = 0;

  fk->cxVKDictionary++;
  return fk->cxVKDictionary + VK__MAX; // 256-1
}

int GetDeadKey(PFILE_KEYBOARD fk, PKMX_WCHAR p)
{
  KMX_DWORD i;

  for (i = 0; i < fk->cxDeadKeyArray; i++)
    if (u16icmp(fk->dpDeadKeyArray[i].szName, p) == 0)
      return i + 1;

  if (fk->cxDeadKeyArray % 10 == 0)
  {
    PFILE_DEADKEY dk = new FILE_DEADKEY[fk->cxDeadKeyArray + 10];
    memcpy(dk, fk->dpDeadKeyArray, fk->cxDeadKeyArray * sizeof(FILE_DEADKEY));
    delete[] fk->dpDeadKeyArray;
    fk->dpDeadKeyArray = dk;
  }
  u16ncpy(fk->dpDeadKeyArray[fk->cxDeadKeyArray].szName,p, _countof(fk->dpDeadKeyArray[fk->cxDeadKeyArray].szName));  // I3481
  fk->dpDeadKeyArray[fk->cxDeadKeyArray].szName[SZMAX_DEADKEYNAME - 1] = 0;

  fk->cxDeadKeyArray++;
  return fk->cxDeadKeyArray;
}

void kmcmp::RecordDeadkeyNames(PFILE_KEYBOARD fk)
{
  KMX_WCHAR buf[SZMAX_DEADKEYNAME + 16];
  KMX_DWORD i;
  for (i = 0; i < fk->cxDeadKeyArray; i++)
  {
    u16sprintf(buf, _countof(buf), L"%ls%d ", DEBUGSTORE_DEADKEY_L, (int)i);
    u16ncat(buf, fk->dpDeadKeyArray[i].szName, _countof(buf));

    AddDebugStore(fk, buf);
  }
}

KMX_BOOL kmcmp::IsValidCallStore(PFILE_STORE fs)
{
  int i;
  PKMX_WCHAR p;
  for (i = 0, p = fs->dpString; *p; p++)
    if (*p == ':') i++;
    else if (!((*p >= 'a' && *p <= 'z') ||
      (*p >= 'A' && *p <= 'Z') ||
      (*p >= '0' && *p <= '9') ||
      *p == '.' ||
      *p == '_'))
      return FALSE;

  return i == 1;
}

///////////////////

bool hasPreamble(std::u16string result) {
  return result.size() > 0 && result[0] == 0xFEFF;
}

bool isValidUtf8(KMX_BYTE* str, int sz) {
  int i = 0;
  while (i < sz) {
    int remaining = sz - i;
    if (str[i] <= 0x7F) {
      // ASCII
      i++;
    } else if ((str[i] & 0xE0) == 0xC0) {
      // 2-byte sequence
      if (remaining < 2 ||
          (str[i + 1] & 0xC0) != 0x80 ||
          str[i] == 0xC0 || // C0 and C1 are illegal values
          str[i] == 0xC1) {
        return false;
      }
      i += 2;
    } else if ((str[i] & 0xF0) == 0xE0) {
      // 3-byte sequence
      if (remaining < 3 ||
          (str[i+1] & 0xC0) != 0x80 ||
          (str[i+2] & 0xC0) != 0x80) {
        return false;
      }
      if (str[i] == 0xE0 && (str[i + 1] & 0xE0) == 0x80) {
        return false;
      }
      if (str[i] == 0xED && (str[i + 1] & 0xE0) == 0xA0) {
        return false;
      }
      i += 3;
    } else if ((str[i] & 0xF8) == 0xF0) {
      // 4-byte sequence
      if (remaining < 4 ||
          (str[i+1] & 0xC0) != 0x80 ||
          (str[i+2] & 0xC0) != 0x80 ||
          (str[i+3] & 0xC0) != 0x80) {
        return false;
      }
      if (str[i] == 0xF0 && (str[i + 1] & 0xF0) == 0x80) {
        return false;
      }
      if (str[i] > 0xF4 || (str[i] == 0xF4 && str[i + 1] > 0x8F)) {
        return false;
      }
      i += 4;
    } else {
      return false;
    }
  }
  return true;
}

bool UTF16TempFromUTF8(KMX_BYTE* infile, int sz, KMX_BYTE** tempfile, int *sz16) {
  if(sz == 0) {
    return FALSE;
  }

  std::u16string result;
  if (isValidUtf8(infile, sz)) {
    std::string infileStr(reinterpret_cast<const char*>(infile), sz);
    result = u16string_from_string(infileStr);
  } else {
    ReportCompilerMessage(KmnCompilerMessages::HINT_NonUnicodeFile);
    result.resize(sz);
    for(int i = 0; i < sz; i++) {
      result[i] = CP1252_UNICODE[infile[i]];
    }
  }

  if(hasPreamble(result)) {
    *sz16 = static_cast<int>(result.size()) * 2 - 2;
    *tempfile = new KMX_BYTE[*sz16];
    memcpy(*tempfile, result.c_str() + 1, *sz16);
  } else {
    *sz16 = static_cast<int>(result.size()) * 2;
    *tempfile = new KMX_BYTE[*sz16];
    memcpy(*tempfile, result.c_str(), *sz16);
  }

  return TRUE;
}

PFILE_STORE FindSystemStore(PFILE_KEYBOARD fk, KMX_DWORD dwSystemID)
{
  assert(fk != NULL);
  assert(dwSystemID != 0);

  PFILE_STORE sp = fk->dpStoreArray;
  for (KMX_DWORD i = 0; i < fk->cxStoreArray; i++, sp++) {
    if (sp->dwSystemID == dwSystemID) {
      return sp;
    }
  }
  return NULL;
}
