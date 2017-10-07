/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <tools/resid.hxx>
#include <hintids.hxx>
#include <swtypes.hxx>
#include <txtatr.hxx>
#include <ndtxt.hxx>
#include <txttxmrk.hxx>
#include <tox.hxx>
#include <poolfmt.hrc>
#include <doc.hxx>
#include <docary.hxx>
#include <paratr.hxx>
#include <editeng/tstpitem.hxx>
#include <SwStyleNameMapper.hxx>
#include <hints.hxx>
#include <functional>
#include <calbck.hxx>

#include <boost/optional.hpp>

#include <algorithm>


using namespace std;


const sal_Unicode C_NUM_REPL      = '@';
const sal_Unicode C_END_PAGE_NUM   = '~';
const OUString S_PAGE_DELI(", ");


namespace
{

void lcl_FillAuthPattern(SwFormTokens &rAuthTokens, sal_uInt16 nTypeId)
{
    rAuthTokens.reserve(9); // Worst case: Start+Sep1+Auth+3*(Sep2+Auth)

    SwFormToken aStartToken( TOKEN_AUTHORITY );
    aStartToken.nAuthorityField = AUTH_FIELD_IDENTIFIER;
    rAuthTokens.push_back( aStartToken );
    SwFormToken aSeparatorToken( TOKEN_TEXT );
    aSeparatorToken.sText = ": ";
    rAuthTokens.push_back( aSeparatorToken );

    --nTypeId; // compensate +1 offset introduced by caller

    SwFormToken aTextToken( TOKEN_TEXT );
    aTextToken.sText = ", ";

    const ToxAuthorityField nVals[4] = {
        AUTH_FIELD_AUTHOR,
        AUTH_FIELD_TITLE,
        AUTH_FIELD_YEAR,
        nTypeId == AUTH_TYPE_WWW ? AUTH_FIELD_URL : AUTH_FIELD_END
    };

    for(size_t i = 0; i < SAL_N_ELEMENTS(nVals); ++i)
    {
        if(nVals[i] == AUTH_FIELD_END)
            break;
        if( i > 0 )
            rAuthTokens.push_back( aTextToken );

        // -> #i21237#
        SwFormToken aToken(TOKEN_AUTHORITY);

        aToken.nAuthorityField = nVals[i];
        rAuthTokens.push_back(aToken);
        // <- #i21237#
    }
}

}


/// pool default constructor
SwTOXMark::SwTOXMark()
    : SfxPoolItem( RES_TXTATR_TOXMARK )
    , SwModify( nullptr )
    ,
    m_pTextAttr( nullptr ), m_nLevel( 0 ),
    m_bAutoGenerated(false),
    m_bMainEntry(false)
{
}

SwTOXMark::SwTOXMark( const SwTOXType* pTyp )
    : SfxPoolItem( RES_TXTATR_TOXMARK )
    , SwModify( const_cast<SwTOXType*>(pTyp) )
    ,
    m_pTextAttr( nullptr ), m_nLevel( 0 ),
    m_bAutoGenerated(false),
    m_bMainEntry(false)
{
}

SwTOXMark::SwTOXMark( const SwTOXMark& rCopy )
    : SfxPoolItem( RES_TXTATR_TOXMARK )
    , SwModify(rCopy.GetRegisteredInNonConst())
    ,
    m_aPrimaryKey( rCopy.m_aPrimaryKey ), m_aSecondaryKey( rCopy.m_aSecondaryKey ),
    m_aTextReading( rCopy.m_aTextReading ),
    m_aPrimaryKeyReading( rCopy.m_aPrimaryKeyReading ),
    m_aSecondaryKeyReading( rCopy.m_aSecondaryKeyReading ),
    m_pTextAttr( nullptr ), m_nLevel( rCopy.m_nLevel ),
    m_bAutoGenerated( rCopy.m_bAutoGenerated),
    m_bMainEntry(rCopy.m_bMainEntry)
{
    // Copy AlternativString
    m_aAltText = rCopy.m_aAltText;
}

SwTOXMark::~SwTOXMark()
{
}

void SwTOXMark::RegisterToTOXType(SwTOXType& rType)
{
    rType.Add(this);
}

bool SwTOXMark::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return GetRegisteredIn() == static_cast<const SwTOXMark&>(rAttr).GetRegisteredIn();
}

SfxPoolItem* SwTOXMark::Clone( SfxItemPool* ) const
{
    return new SwTOXMark( *this );
}

void SwTOXMark::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew)
{
    NotifyClients(pOld, pNew);
    if (pOld && (RES_REMOVE_UNO_OBJECT == pOld->Which()))
    {   // invalidate cached uno object
        SetXTOXMark(css::uno::Reference<css::text::XDocumentIndexMark>(nullptr));
    }
}

void SwTOXMark::InvalidateTOXMark()
{
    SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT,
        &static_cast<SwModify&>(*this) ); // cast to base class!
    NotifyClients(&aMsgHint, &aMsgHint);
}

OUString SwTOXMark::GetText() const
{
    if( !m_aAltText.isEmpty() )
        return m_aAltText;

    if( m_pTextAttr && m_pTextAttr->GetpTextNd() )
    {
        const sal_Int32* pEndIdx = m_pTextAttr->GetEnd();
        OSL_ENSURE( pEndIdx, "TOXMark without mark!");
        if( pEndIdx )
        {
            const sal_Int32 nStt = m_pTextAttr->GetStart();
            return m_pTextAttr->GetpTextNd()->GetExpandText( nStt, *pEndIdx-nStt );
        }
    }

    return OUString();
}

void SwTOXMark::InsertTOXMarks( SwTOXMarks& aMarks, const SwTOXType& rType )
{
    SwIterator<SwTOXMark,SwTOXType> aIter(rType);
    SwTOXMark* pMark = aIter.First();
    while( pMark )
    {
        if(pMark->GetTextTOXMark())
            aMarks.push_back(pMark);
        pMark = aIter.Next();
    }
}

// Manage types of TOX
SwTOXType::SwTOXType( TOXTypes eTyp, const OUString& rName )
    : SwModify(nullptr),
    m_aName(rName),
    m_eType(eTyp)
{
}

SwTOXType::SwTOXType(const SwTOXType& rCopy)
    : SwModify( const_cast<SwModify*>(rCopy.GetRegisteredIn()) ),
    m_aName(rCopy.m_aName),
    m_eType(rCopy.m_eType)
{
}

// Edit forms
SwForm::SwForm( TOXTypes eTyp ) // #i21237#
    : m_eType( eTyp ), m_nFormMaxLevel( SwForm::GetFormMaxLevel( eTyp )),
//  nFirstTabPos( lNumIndent ),
    m_bCommaSeparated(false)
{
    //bHasFirstTabPos =
    m_bIsRelTabPos = true;

    // The table of contents has a certain number of headlines + headings
    // The user has 10 levels + headings
    // Keyword has 3 levels + headings+ separator
    // Indexes of tables, object illustrations and authorities consist of a heading and one level

    sal_uInt16 nPoolId;
    switch( m_eType )
    {
    case TOX_INDEX:         nPoolId = STR_POOLCOLL_TOX_IDXH;    break;
    case TOX_USER:          nPoolId = STR_POOLCOLL_TOX_USERH;   break;
    case TOX_CONTENT:       nPoolId = STR_POOLCOLL_TOX_CNTNTH;  break;
    case TOX_ILLUSTRATIONS: nPoolId = STR_POOLCOLL_TOX_ILLUSH;  break;
    case TOX_OBJECTS      : nPoolId = STR_POOLCOLL_TOX_OBJECTH; break;
    case TOX_TABLES       : nPoolId = STR_POOLCOLL_TOX_TABLESH; break;
    case TOX_AUTHORITIES  : nPoolId = STR_POOLCOLL_TOX_AUTHORITIESH;    break;
    case TOX_CITATION  : nPoolId = STR_POOLCOLL_TOX_CITATION; break;
    default:
        OSL_ENSURE( false, "invalid TOXTyp");
        return ;
    }

    SwFormTokens aTokens;
    if (TOX_CONTENT == m_eType || TOX_ILLUSTRATIONS == m_eType )
    {
        SwFormToken aLinkStt (TOKEN_LINK_START);
        aLinkStt.sCharStyleName = SwResId(STR_POOLCHR_TOXJUMP);
        aTokens.push_back(aLinkStt);
    }

    if (TOX_CONTENT == m_eType)
    {
        aTokens.push_back(SwFormToken(TOKEN_ENTRY_NO));
        aTokens.push_back(SwFormToken(TOKEN_ENTRY_TEXT));
    }
    else
        aTokens.push_back(SwFormToken(TOKEN_ENTRY));

    if (TOX_AUTHORITIES != m_eType)
    {
        SwFormToken aToken(TOKEN_TAB_STOP);
        aToken.nTabStopPosition = 0;

        // #i36870# right aligned tab for all
        aToken.cTabFillChar = '.';
        aToken.eTabAlign = SvxTabAdjust::End;

        aTokens.push_back(aToken);
        aTokens.push_back(SwFormToken(TOKEN_PAGE_NUMS));
    }

    if (TOX_CONTENT == m_eType || TOX_ILLUSTRATIONS == m_eType)
        aTokens.push_back(SwFormToken(TOKEN_LINK_END));

    SetTemplate( 0, SwResId( nPoolId++ ));

    if(TOX_INDEX == m_eType)
    {
        for( sal_uInt16 i = 1; i < 5; ++i  )
        {
            if(1 == i)
            {
                SwFormTokens aTmpTokens;
                SwFormToken aTmpToken(TOKEN_ENTRY);
                aTmpTokens.push_back(aTmpToken);

                SetPattern( i, aTmpTokens );
                SetTemplate( i, SwResId( STR_POOLCOLL_TOX_IDXBREAK    ));
            }
            else
            {
                SetPattern( i, aTokens );
                SetTemplate( i, SwResId( STR_POOLCOLL_TOX_IDX1 + i - 2 ));
            }
        }
    }
    else
        for( sal_uInt16 i = 1; i < GetFormMax(); ++i, ++nPoolId )    // Number 0 is the title
        {
            if(TOX_AUTHORITIES == m_eType)
            {
                SwFormTokens aAuthTokens;
                lcl_FillAuthPattern(aAuthTokens, i);
                SetPattern(i, aAuthTokens);
            }
            else
                SetPattern( i, aTokens );

            if( TOX_CONTENT == m_eType && 6 == i )
                nPoolId = STR_POOLCOLL_TOX_CNTNT6;
            else if( TOX_USER == m_eType && 6 == i )
                nPoolId = STR_POOLCOLL_TOX_USER6;
            else if( TOX_AUTHORITIES == m_eType )
                nPoolId = STR_POOLCOLL_TOX_AUTHORITIES1;
            SetTemplate( i, SwResId( nPoolId ) );
        }
}

SwForm::SwForm(const SwForm& rForm)
    : m_eType( rForm.m_eType )
{
    *this = rForm;
}

SwForm& SwForm::operator=(const SwForm& rForm)
{
    m_eType = rForm.m_eType;
    m_nFormMaxLevel = rForm.m_nFormMaxLevel;
//  nFirstTabPos = rForm.nFirstTabPos;
//  bHasFirstTabPos = rForm.bHasFirstTabPos;
    m_bIsRelTabPos = rForm.m_bIsRelTabPos;
    m_bCommaSeparated = rForm.m_bCommaSeparated;
    for(sal_uInt16 i=0; i < m_nFormMaxLevel; ++i)
    {
        m_aPattern[i] = rForm.m_aPattern[i];
        m_aTemplate[i] = rForm.m_aTemplate[i];
    }
    return *this;
}

sal_uInt16 SwForm::GetFormMaxLevel( TOXTypes eTOXType )
{
    switch( eTOXType )
    {
        case TOX_INDEX:
            return 5;
        case TOX_USER:
        case TOX_CONTENT:
            return MAXLEVEL + 1;
        case TOX_ILLUSTRATIONS:
        case TOX_OBJECTS:
        case TOX_TABLES:
            return 2;
        case TOX_BIBLIOGRAPHY:
        case TOX_CITATION:
        case TOX_AUTHORITIES:
            return AUTH_TYPE_END + 1;
    }
    return 0;
}

void SwForm::AdjustTabStops( SwDoc& rDoc ) // #i21237#
{
    const sal_uInt16 nFormMax = GetFormMax();
    for ( sal_uInt16 nLevel = 1; nLevel < nFormMax; ++nLevel )
    {
        SwTextFormatColl* pColl = rDoc.FindTextFormatCollByName( GetTemplate(nLevel) );
        if( pColl == nullptr )
        {
            // Paragraph Style for this level has not been created.
            // --> No need to propagate default values
            continue;
        }

        const SvxTabStopItem& rTabStops = pColl->GetTabStops(false);
        const sal_uInt16 nTabCount = rTabStops.Count();
        if (nTabCount != 0)
        {
            SwFormTokens aCurrentPattern = GetPattern(nLevel);
            SwFormTokens::iterator aIt = aCurrentPattern.begin();

            bool bChanged = false;
            for(sal_uInt16 nTab = 0; nTab < nTabCount; ++nTab)
            {
                const SvxTabStop& rTab = rTabStops[nTab];

                if ( rTab.GetAdjustment() == SvxTabAdjust::Default )
                    continue; // ignore the default tab stop

                aIt = find_if( aIt, aCurrentPattern.end(), SwFormTokenEqualToFormTokenType(TOKEN_TAB_STOP) );
                if ( aIt != aCurrentPattern.end() )
                {
                    bChanged = true;
                    aIt->nTabStopPosition = rTab.GetTabPos();
                    aIt->eTabAlign =
                        ( nTab == nTabCount - 1
                          && rTab.GetAdjustment() == SvxTabAdjust::Right )
                        ? SvxTabAdjust::End
                        : rTab.GetAdjustment();
                    aIt->cTabFillChar = rTab.GetFill();
                    ++aIt;
                }
                else
                    break; // no more tokens to replace
            }

            if ( bChanged )
                SetPattern( nLevel, aCurrentPattern );
        }
    }
}

OUString SwForm::GetFormEntry()       {return OUString("<E>");}
OUString SwForm::GetFormTab()         {return OUString("<T>");}
OUString SwForm::GetFormPageNums()    {return OUString("<#>");}
OUString SwForm::GetFormLinkStt()     {return OUString("<LS>");}
OUString SwForm::GetFormLinkEnd()     {return OUString("<LE>");}
OUString SwForm::GetFormEntryNum()    {return OUString("<E#>");}
OUString SwForm::GetFormEntryText()    {return OUString("<ET>");}
OUString SwForm::GetFormChapterMark() {return OUString("<C>");}
OUString SwForm::GetFormText()        {return OUString("<X>");}
OUString SwForm::GetFormAuth()        {return OUString("<A>");}

SwTOXBase::SwTOXBase(const SwTOXType* pTyp, const SwForm& rForm,
                     SwTOXElement nCreaType, const OUString& rTitle )
    : SwClient(const_cast<SwModify*>(static_cast<SwModify const *>(pTyp)))
    , m_aForm(rForm)
    , m_aTitle(rTitle)
    , m_eLanguage(::GetAppLanguage())
    , m_nCreateType(nCreaType)
    , m_nOLEOptions(SwTOOElements::NONE)
    , m_eCaptionDisplay(CAPTION_COMPLETE)
    , m_bProtected( true )
    , m_bFromChapter(false)
    , m_bFromObjectNames(false)
    , m_bLevelFromChapter(false)
    , maMSTOCExpression()
    , mbKeepExpression(true)
{
    m_aData.nOptions = SwTOIOptions::NONE;
}

SwTOXBase::SwTOXBase( const SwTOXBase& rSource, SwDoc* pDoc )
    : SwClient( rSource.GetRegisteredInNonConst() )
    , mbKeepExpression(true)
{
    CopyTOXBase( pDoc, rSource );
}

void SwTOXBase::RegisterToTOXType( SwTOXType& rType )
{
    rType.Add( this );
}

void SwTOXBase::CopyTOXBase( SwDoc* pDoc, const SwTOXBase& rSource )
{
    maMSTOCExpression = rSource.maMSTOCExpression;
    SwTOXType* pType = const_cast<SwTOXType*>(rSource.GetTOXType());
    if( pDoc && !pDoc->GetTOXTypes().IsAlive(pType))
    {
        // type not in pDoc, so create it now
        const SwTOXTypes& rTypes = pDoc->GetTOXTypes();
        bool bFound = false;
        for( size_t n = rTypes.size(); n; )
        {
            const SwTOXType* pCmp = rTypes[ --n ];
            if( pCmp->GetType() == pType->GetType() &&
                pCmp->GetTypeName() == pType->GetTypeName() )
            {
                pType = const_cast<SwTOXType*>(pCmp);
                bFound = true;
                break;
            }
        }

        if( !bFound )
            pType = const_cast<SwTOXType*>(pDoc->InsertTOXType( *pType ));
    }
    pType->Add( this );

    m_nCreateType = rSource.m_nCreateType;
    m_aTitle      = rSource.m_aTitle;
    m_aForm       = rSource.m_aForm;
    m_aBookmarkName = rSource.m_aBookmarkName;
    m_aEntryTypeName = rSource.m_aEntryTypeName ;
    m_bProtected  = rSource.m_bProtected;
    m_bFromChapter = rSource.m_bFromChapter;
    m_bFromObjectNames = rSource.m_bFromObjectNames;
    m_sMainEntryCharStyle = rSource.m_sMainEntryCharStyle;
    m_sSequenceName = rSource.m_sSequenceName;
    m_eCaptionDisplay = rSource.m_eCaptionDisplay;
    m_nOLEOptions = rSource.m_nOLEOptions;
    m_eLanguage = rSource.m_eLanguage;
    m_sSortAlgorithm = rSource.m_sSortAlgorithm;
    m_bLevelFromChapter = rSource.m_bLevelFromChapter;

    for( sal_uInt16 i = 0; i < MAXLEVEL; ++i )
        m_aStyleNames[i] = rSource.m_aStyleNames[i];

    // it's the same data type!
    m_aData.nOptions =  rSource.m_aData.nOptions;

    if( !pDoc || pDoc->IsCopyIsMove() )
        m_aName = rSource.GetTOXName();
    else
        m_aName = pDoc->GetUniqueTOXBaseName( *pType, rSource.GetTOXName() );
}

// TOX specific functions
SwTOXBase::~SwTOXBase()
{
//    if( GetTOXType()->GetType() == TOX_USER  )
//        delete aData.pTemplateName;
}

void SwTOXBase::SetTitle(const OUString& rTitle)
    {   m_aTitle = rTitle; }

void SwTOXBase::SetBookmarkName(const OUString& bName)
{
     m_aBookmarkName = bName;
}

void SwTOXBase::SetEntryTypeName(const OUString& sName)
{
     m_aEntryTypeName = sName ;
}

SwTOXBase & SwTOXBase::operator = (const SwTOXBase & rSource)
{
    m_aForm = rSource.m_aForm;
    m_aName = rSource.m_aName;
    m_aTitle = rSource.m_aTitle;
    m_aBookmarkName = rSource.m_aBookmarkName;
    m_aEntryTypeName = rSource.m_aEntryTypeName ;
    m_sMainEntryCharStyle = rSource.m_sMainEntryCharStyle;
    for(sal_uInt16 nLevel = 0; nLevel < MAXLEVEL; nLevel++)
        m_aStyleNames[nLevel] = rSource.m_aStyleNames[nLevel];
    m_sSequenceName = rSource.m_sSequenceName;
    m_eLanguage = rSource.m_eLanguage;
    m_sSortAlgorithm = rSource.m_sSortAlgorithm;
    m_aData = rSource.m_aData;
    m_nCreateType = rSource.m_nCreateType;
    m_nOLEOptions = rSource.m_nOLEOptions;
    m_eCaptionDisplay = rSource.m_eCaptionDisplay;
    m_bProtected = rSource.m_bProtected;
    m_bFromChapter = rSource.m_bFromChapter;
    m_bFromObjectNames = rSource.m_bFromObjectNames;
    m_bLevelFromChapter = rSource.m_bLevelFromChapter;

    if (rSource.GetAttrSet())
        SetAttrSet(*rSource.GetAttrSet());

    return *this;
}

OUString SwFormToken::GetString() const
{
    OUString sToken;

    switch( eTokenType )
    {
        case TOKEN_ENTRY_NO:
            sToken = SwForm::GetFormEntryNum();
        break;
        case TOKEN_ENTRY_TEXT:
            sToken = SwForm::GetFormEntryText();
        break;
        case TOKEN_ENTRY:
            sToken = SwForm::GetFormEntry();
        break;
        case TOKEN_TAB_STOP:
            sToken = SwForm::GetFormTab();
        break;
        case TOKEN_TEXT:
            // Return a Token only if Text is not empty!
            if( sText.isEmpty() )
            {
                return OUString();
            }
            sToken = SwForm::GetFormText();
        break;
        case TOKEN_PAGE_NUMS:
            sToken = SwForm::GetFormPageNums();
        break;
        case TOKEN_CHAPTER_INFO:
            sToken = SwForm::GetFormChapterMark();
        break;
        case TOKEN_LINK_START:
            sToken = SwForm::GetFormLinkStt();
        break;
        case TOKEN_LINK_END:
            sToken = SwForm::GetFormLinkEnd();
        break;
        case TOKEN_AUTHORITY:
        {
            sToken = SwForm::GetFormAuth();
        }
        break;
        case TOKEN_END:
        break;
    }

    OUString sData = " " + sCharStyleName + "," + OUString::number( nPoolId ) + ",";

    // TabStopPosition and TabAlign or ChapterInfoFormat
    switch (eTokenType)
    {
        case TOKEN_TAB_STOP:
            sData += OUString::number( nTabStopPosition ) + ","
                  +  OUString::number( static_cast< sal_Int32 >(eTabAlign) ) + ","
                  +  OUStringLiteral1(cTabFillChar) + ","
                  +  OUString::number( bWithTab ? 1 : 0 );
            break;
        case TOKEN_CHAPTER_INFO:
        case TOKEN_ENTRY_NO:
            // add also maximum permitted level
            sData += OUString::number( nChapterFormat ) + ","
                  +  OUString::number( nOutlineLevel );
            break;
        case TOKEN_TEXT:
            sData += OUStringLiteral1(TOX_STYLE_DELIMITER)
                  +  sText.replaceAll(OUStringLiteral1(TOX_STYLE_DELIMITER), "")
                  +  OUStringLiteral1(TOX_STYLE_DELIMITER);
            break;
        case TOKEN_AUTHORITY:
            if (nAuthorityField<10)
            {
                 sData = "0" + OUString::number( nAuthorityField ) + sData;
            }
            else
            {
                 sData = OUString::number( nAuthorityField ) + sData;
            }
            break;
        default:
            break;
    }

    return sToken.copy(0, sToken.getLength()-1) + sData + sToken.copy(sToken.getLength()-1);
}

// -> #i21237#

/**
   Returns the type of a token.

   @param sToken     the string representation of the token
   @param pTokenLen  return parameter the length of the head of the token

   If pTokenLen is non-NULL the length of the token's head is
   written to *pTokenLen

   @return the type of the token
*/
static FormTokenType lcl_GetTokenType(const OUString & sToken,
                                      sal_Int32 *const pTokenLen)
{
    static struct
    {
        OUString sNm;
        sal_uInt16 nOffset;
        FormTokenType eToken;
    } const aTokenArr[] = {
        { SwForm::GetFormTab(),         1, TOKEN_TAB_STOP },
        { SwForm::GetFormPageNums(),    1, TOKEN_PAGE_NUMS },
        { SwForm::GetFormLinkStt(),     1, TOKEN_LINK_START },
        { SwForm::GetFormLinkEnd(),     1, TOKEN_LINK_END },
        { SwForm::GetFormEntryNum(),    1, TOKEN_ENTRY_NO },
        { SwForm::GetFormEntryText(),    1, TOKEN_ENTRY_TEXT },
        { SwForm::GetFormChapterMark(), 1, TOKEN_CHAPTER_INFO },
        { SwForm::GetFormText(),        1, TOKEN_TEXT },
        { SwForm::GetFormEntry(),       1, TOKEN_ENTRY },
        { SwForm::GetFormAuth(),        3, TOKEN_AUTHORITY }
    };

    for(const auto & i : aTokenArr)
    {
        const sal_Int32 nLen(i.sNm.getLength());
        if( sToken.startsWith( i.sNm.copy(0, nLen - i.nOffset) ))
        {
            if (pTokenLen)
                *pTokenLen = nLen;
            return i.eToken;
        }
    }

    SAL_WARN("sw.core", "SwFormTokensHelper: invalid token");
    return TOKEN_END;
}

/**
   Returns the string of a token.

   @param sPattern    the whole pattern
   @param nStt        starting position of the token

   @return   the string representation of the token
*/
static OUString
lcl_SearchNextToken(const OUString & sPattern, sal_Int32 const nStt)
{
    sal_Int32 nEnd = sPattern.indexOf( '>', nStt );
    if (nEnd >= 0)
    {
        // apparently the TOX_STYLE_DELIMITER act as a bracketing for
        // TOKEN_TEXT tokens so that the user can have '>' inside the text...
        const sal_Int32 nTextSeparatorFirst = sPattern.indexOf( TOX_STYLE_DELIMITER, nStt );
        if (    nTextSeparatorFirst >= 0
            &&  nTextSeparatorFirst + 1 < sPattern.getLength()
            &&  nTextSeparatorFirst < nEnd)
        {
            const sal_Int32 nTextSeparatorSecond = sPattern.indexOf( TOX_STYLE_DELIMITER,
                                                                     nTextSeparatorFirst + 1 );
            // Since nEnd>=0 we don't need to check if nTextSeparatorSecond<0!
            if( nEnd < nTextSeparatorSecond )
                nEnd = sPattern.indexOf( '>', nTextSeparatorSecond );
            // FIXME: No check to verify that nEnd is still >=0?
            assert(nEnd >= 0);
        }

        ++nEnd;

        return sPattern.copy( nStt, nEnd - nStt );
    }

    return OUString();
}

/**
   Builds a token from its string representation.

   @sPattern          the whole pattern
   @nCurPatternPos    starting position of the token

   @return the token
 */
static boost::optional<SwFormToken>
lcl_BuildToken(const OUString & sPattern, sal_Int32 & nCurPatternPos)
{
    OUString sToken( lcl_SearchNextToken(sPattern, nCurPatternPos) );
    nCurPatternPos += sToken.getLength();
    sal_Int32 nTokenLen = 0;
    FormTokenType const eTokenType = lcl_GetTokenType(sToken, &nTokenLen);
    if (TOKEN_END == eTokenType) // invalid input? skip it
    {
        nCurPatternPos = sPattern.getLength();
        return boost::optional<SwFormToken>();
    }

    // at this point sPattern contains the
    // character style name, the PoolId, tab stop position, tab stop alignment, chapter info format
    // the form is: CharStyleName, PoolId[, TabStopPosition|ChapterInfoFormat[, TabStopAlignment[, TabFillChar]]]
    // in text tokens the form differs from the others: CharStyleName, PoolId[,\0xffinserted text\0xff]
    SwFormToken eRet( eTokenType );
    const OUString sAuthFieldEnum = sToken.copy( 2, 2 );
    sToken = sToken.copy( nTokenLen, sToken.getLength() - nTokenLen - 1);

    eRet.sCharStyleName = sToken.getToken( 0, ',');
    OUString sTmp( sToken.getToken( 1, ',' ));
    if( !sTmp.isEmpty() )
        eRet.nPoolId = static_cast<sal_uInt16>(sTmp.toInt32());

    switch( eTokenType )
    {
//i53420
    case TOKEN_CHAPTER_INFO:
//i53420
    case TOKEN_ENTRY_NO:
        sTmp = sToken.getToken( 2, ',' );
        if( !sTmp.isEmpty() )
            eRet.nChapterFormat = static_cast<sal_uInt16>(sTmp.toInt32());
        sTmp = sToken.getToken( 3, ',' );
        if( !sTmp.isEmpty() )
            eRet.nOutlineLevel = static_cast<sal_uInt16>(sTmp.toInt32()); //the maximum outline level to examine
        break;

    case TOKEN_TEXT:
        {
            const sal_Int32 nStartText = sToken.indexOf( TOX_STYLE_DELIMITER );
            if( nStartText>=0 && nStartText+1<sToken.getLength())
            {
                const sal_Int32 nEndText = sToken.indexOf( TOX_STYLE_DELIMITER,
                                                           nStartText + 1);
                if( nEndText>=0 )
                {
                    eRet.sText = sToken.copy( nStartText + 1,
                                                nEndText - nStartText - 1);
                }
            }
        }
        break;

    case TOKEN_TAB_STOP:
        sTmp = sToken.getToken( 2, ',' );
        if( !sTmp.isEmpty() )
            eRet.nTabStopPosition = sTmp.toInt32();

        sTmp = sToken.getToken( 3, ',' );
        if( !sTmp.isEmpty() )
            eRet.eTabAlign = static_cast<SvxTabAdjust>(sTmp.toInt32());

        sTmp = sToken.getToken( 4, ',' );
        if( !sTmp.isEmpty() )
            eRet.cTabFillChar = sTmp[0];

        sTmp = sToken.getToken( 5, ',' );
        if( !sTmp.isEmpty() )
            eRet.bWithTab = 0 != sTmp.toInt32();
        break;

    case TOKEN_AUTHORITY:
        eRet.nAuthorityField = static_cast<sal_uInt16>(sAuthFieldEnum.toInt32());
        break;
    default: break;
    }
    return eRet;
}

SwFormTokensHelper::SwFormTokensHelper(const OUString & rPattern)
{
    sal_Int32 nCurPatternPos = 0;

    while (nCurPatternPos < rPattern.getLength())
    {
        boost::optional<SwFormToken> const oToken(
                lcl_BuildToken(rPattern, nCurPatternPos));
        if (oToken)
            m_Tokens.push_back(oToken.get());
    }
}

// <- #i21237#

void SwForm::SetPattern(sal_uInt16 nLevel, const SwFormTokens& rTokens)
{
    OSL_ENSURE(nLevel < GetFormMax(), "Index >= FORM_MAX");
    m_aPattern[nLevel] = rTokens;
}

void SwForm::SetPattern(sal_uInt16 nLevel, const OUString & rStr)
{
    OSL_ENSURE(nLevel < GetFormMax(), "Index >= FORM_MAX");

    SwFormTokensHelper aHelper(rStr);
    m_aPattern[nLevel] = aHelper.GetTokens();
}

const SwFormTokens& SwForm::GetPattern(sal_uInt16 nLevel) const
{
    OSL_ENSURE(nLevel < GetFormMax(), "Index >= FORM_MAX");
    return m_aPattern[nLevel];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
