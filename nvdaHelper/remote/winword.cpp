/*
This file is a part of the NVDA project.
URL: http://www.nvda-project.org/
Copyright 2006-2010 NVDA contributers.
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2.0, as published by
    the Free Software Foundation.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
This license can be found at:
http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
*/

#define WIN32_LEAN_AND_MEAN 

#include <sstream>
#include <comdef.h>
#include <windows.h>
#include <oleacc.h>
#include <common/xml.h>
#include <common/log.h>
#include "nvdaHelperRemote.h"
#include "nvdaInProcUtils.h"
#include "nvdaInProcUtils.h"
#include "winword.h"

using namespace std;

#define wdDISPID_DOCUMENT_RANGE 2000
#define wdDISPID_WINDOW_DOCUMENT 2
#define wdDISPID_WINDOW_APPLICATION 1000
#define wdDISPID_WINDOW_SELECTION 4
#define wdDISPID_APPLICATION_SCREENUPDATING 26
#define wdDISPID_SELECTION_RANGE 400
#define wdDISPID_SELECTION_SETRANGE 100
#define wdDISPID_SELECTION_STARTISACTIVE 404
#define wdDISPID_RANGE_STORYTYPE 7
#define wdDISPID_RANGE_MOVEEND 111
#define wdDISPID_RANGE_COLLAPSE 101
#define wdDISPID_RANGE_TEXT 0
#define wdDISPID_RANGE_EXPAND 129
#define wdDISPID_RANGE_SELECT 65535
#define wdDISPID_RANGE_SETRANGE 100
#define wdDISPID_RANGE_START 3
#define wdDISPID_RANGE_END 4
#define wdDISPID_RANGE_INFORMATION 313
#define wdDISPID_RANGE_STYLE 151
#define wdDISPID_RANGE_LANGUAGEID 153
#define wdDISPID_STYLE_NAMELOCAL 0
#define wdDISPID_RANGE_SPELLINGERRORS 316
#define wdDISPID_SPELLINGERRORS_COUNT 1
#define wdDISPID_RANGE_FONT 5
#define wdDISPID_FONT_COLOR 159
#define wdDISPID_FONT_BOLD 130
#define wdDISPID_FONT_ITALIC 131
#define wdDISPID_FONT_UNDERLINE 140
#define wdDISPID_FONT_NAME 142
#define wdDISPID_FONT_SIZE 141
#define wdDISPID_FONT_SUBSCRIPT 138
#define wdDISPID_FONT_SUPERSCRIPT 139
#define wdDISPID_RANGE_PARAGRAPHFORMAT 1102
#define wdDISPID_PARAGRAPHFORMAT_ALIGNMENT 101
#define wdDISPID_RANGE_LISTFORMAT 68
#define wdDISPID_LISTFORMAT_LISTSTRING 75
#define wdDISPID_RANGE_PARAGRAPHS 59
#define wdDISPID_PARAGRAPHS_ITEM 0
#define wdDISPID_PARAGRAPH_RANGE 0
#define wdDISPID_PARAGRAPH_OUTLINELEVEL 202
#define wdDISPID_RANGE_FOOTNOTES 54
#define wdDISPID_FOOTNOTES_ITEM 0
#define wdDISPID_FOOTNOTES_COUNT 2
#define wdDISPID_FOOTNOTE_INDEX 6
#define wdDISPID_RANGE_ENDNOTES 55
#define wdDISPID_ENDNOTES_ITEM 0
#define wdDISPID_ENDNOTES_COUNT 2
#define wdDISPID_ENDNOTE_INDEX 6
#define wdDISPID_RANGE_INLINESHAPES 319
#define wdDISPID_INLINESHAPES_COUNT 1
#define wdDISPID_INLINESHAPES_ITEM 0 
#define wdDISPID_INLINESHAPE_TYPE 6
#define wdDISPID_INLINESHAPE_ALTERNATIVETEXT 131
#define wdDISPID_RANGE_HYPERLINKS 156
#define wdDISPID_HYPERLINKS_COUNT 1
#define wdDISPID_RANGE_COMMENTS 56
#define wdDISPID_COMMENTS_COUNT 2
#define wdDISPID_RANGE_TABLES 50
#define wdDISPID_TABLES_ITEM 0
#define wdDISPID_TABLE_NESTINGLEVEL 108
#define wdDISPID_TABLE_RANGE 0
#define wdDISPID_RANGE_CELLS 57
#define wdDISPID_CELLS_ITEM 0
#define wdDISPID_CELL_RANGE 0
#define wdDISPID_TABLE_COLUMNS 100
#define wdDISPID_COLUMNS_COUNT 2
#define wdDISPID_TABLE_ROWS 101
#define wdDISPID_ROWS_COUNT 2

#define wdCommentsStory 4

#define wdCharacter 1
#define wdWord 2
#define wdLine 5
#define wdCharacterFormatting 13

#define wdCollapseEnd 0
#define wdCollapseStart 1

#define wdActiveEndAdjustedPageNumber 1
#define wdFirstCharacterLineNumber 10
#define wdWithInTable 12
#define wdStartOfRangeRowNumber 13
#define wdMaximumNumberOfRows 15
#define wdStartOfRangeColumnNumber 16
#define wdMaximumNumberOfColumns 18

#define wdAlignParagraphLeft 0
#define wdAlignParagraphCenter 1
#define wdAlignParagraphRight 2
#define wdAlignParagraphJustify 3
#define wdLanguageNone 0  //&H0
#define wdNoProofing 1024  //&H400
#define wdLanguageUnknown 9999999

#define formatConfig_reportFontName 1
#define formatConfig_reportFontSize 2
#define formatConfig_reportFontAttributes 4
#define formatConfig_reportColor 8
#define formatConfig_reportAlignment 16
#define formatConfig_reportStyle 32
#define formatConfig_reportSpellingErrors 64
#define formatConfig_reportPage 128
#define formatConfig_reportLineNumber 256
#define formatConfig_reportTables 512
#define formatConfig_reportLists 1024
#define formatConfig_reportLinks 2048
#define formatConfig_reportComments 4096
#define formatConfig_reportHeadings 8192
#define formatConfig_reportLanguage 16384

#define formatConfig_fontFlags (formatConfig_reportFontName|formatConfig_reportFontSize|formatConfig_reportFontAttributes|formatConfig_reportColor)
#define formatConfig_initialFormatFlags (formatConfig_reportPage|formatConfig_reportLineNumber|formatConfig_reportTables|formatConfig_reportHeadings)

UINT wm_winword_expandToLine=0;
typedef struct {
	int offset;
	int lineStart;
	int lineEnd;
} winword_expandToLine_args;
void winword_expandToLine_helper(HWND hwnd, winword_expandToLine_args* args) {
	//Fetch all needed objects
	IDispatchPtr pDispatchWindow=NULL;
	if(AccessibleObjectFromWindow(hwnd,OBJID_NATIVEOM,IID_IDispatch,(void**)&pDispatchWindow)!=S_OK||!pDispatchWindow) {
		LOG_DEBUGWARNING(L"AccessibleObjectFromWindow failed");
		return;
	}
	IDispatchPtr pDispatchApplication=NULL;
	if(_com_dispatch_raw_propget(pDispatchWindow,wdDISPID_WINDOW_APPLICATION,VT_DISPATCH,&pDispatchApplication)!=S_OK||!pDispatchApplication) {
		LOG_DEBUGWARNING(L"window.application failed");
		return;
	}
	IDispatchPtr pDispatchSelection=NULL;
	if(_com_dispatch_raw_propget(pDispatchWindow,wdDISPID_WINDOW_SELECTION,VT_DISPATCH,&pDispatchSelection)!=S_OK||!pDispatchSelection) {
		LOG_DEBUGWARNING(L"application.selection failed");
		return;
	}
	BOOL startWasActive=false;
	if(_com_dispatch_raw_propget(pDispatchSelection,wdDISPID_SELECTION_STARTISACTIVE,VT_BOOL,&startWasActive)!=S_OK) {
		LOG_DEBUGWARNING(L"selection.StartIsActive failed");
	}
	IDispatch* pDispatchOldSelRange=NULL;
	if(_com_dispatch_raw_propget(pDispatchSelection,wdDISPID_SELECTION_RANGE,VT_DISPATCH,&pDispatchOldSelRange)!=S_OK||!pDispatchOldSelRange) {
		LOG_DEBUGWARNING(L"selection.range failed");
		return;
	}
	//Disable screen updating as we will be moving the selection temporarily
	_com_dispatch_raw_propput(pDispatchApplication,wdDISPID_APPLICATION_SCREENUPDATING,VT_BOOL,false);
	//Move the selection to the given range
	_com_dispatch_raw_method(pDispatchSelection,wdDISPID_SELECTION_SETRANGE,DISPATCH_METHOD,VT_EMPTY,NULL,L"\x0003\x0003",args->offset,args->offset);
	//Expand the selection to the line
	_com_dispatch_raw_method(pDispatchSelection,wdDISPID_RANGE_EXPAND,DISPATCH_METHOD,VT_EMPTY,NULL,L"\x0003",wdLine);
	//Collect the start and end offsets of the selection
	_com_dispatch_raw_propget(pDispatchSelection,wdDISPID_RANGE_START,VT_I4,&(args->lineStart));
	_com_dispatch_raw_propget(pDispatchSelection,wdDISPID_RANGE_END,VT_I4,&(args->lineEnd));
	//Move the selection back to its original location
	_com_dispatch_raw_method(pDispatchOldSelRange,wdDISPID_RANGE_SELECT,DISPATCH_METHOD,VT_EMPTY,NULL,NULL);
	//Restore the old selection direction
	_com_dispatch_raw_propput(pDispatchSelection,wdDISPID_SELECTION_STARTISACTIVE,VT_BOOL,startWasActive);
	//Reenable screen updating
	_com_dispatch_raw_propput(pDispatchApplication,wdDISPID_APPLICATION_SCREENUPDATING,VT_BOOL,true);
}

int generateHeadingXML(IDispatch* pDispatchRange, wostringstream& XMLStream) {
	IDispatchPtr pDispatchParagraphs=NULL;
	IDispatchPtr pDispatchParagraph=NULL;
	int level=0;
	if(_com_dispatch_raw_propget(pDispatchRange,wdDISPID_RANGE_PARAGRAPHS,VT_DISPATCH,&pDispatchParagraphs)!=S_OK||!pDispatchParagraphs) {
		return 0;
	}
	if(_com_dispatch_raw_method(pDispatchParagraphs,wdDISPID_PARAGRAPHS_ITEM,DISPATCH_METHOD,VT_DISPATCH,&pDispatchParagraph,L"\x0003",1)!=S_OK||!pDispatchParagraph) {
		return 0;
	}
	if(_com_dispatch_raw_propget(pDispatchParagraph,wdDISPID_PARAGRAPH_OUTLINELEVEL,VT_I4,&level)!=S_OK||level<=0||level>=7) {
		return 0;
	}
	XMLStream<<L"<control role=\"heading\" level=\""<<level<<L"\">";
	return 1;
}

int getHyperlinkCount(IDispatch* pDispatchRange) {
	IDispatchPtr pDispatchHyperlinks=NULL;
	int count=0;
	if(_com_dispatch_raw_propget(pDispatchRange,wdDISPID_RANGE_HYPERLINKS,VT_DISPATCH,&pDispatchHyperlinks)!=S_OK||!pDispatchHyperlinks) {
		return 0;
	}
	if(_com_dispatch_raw_propget(pDispatchHyperlinks,wdDISPID_HYPERLINKS_COUNT,VT_I4,&count)!=S_OK||count<=0) {
		return 0;
	}
	return count;
}

int getCommentCount(IDispatch* pDispatchRange) {
	IDispatchPtr pDispatchComments=NULL;
	int count=0;
	if(_com_dispatch_raw_propget(pDispatchRange,wdDISPID_RANGE_COMMENTS,VT_DISPATCH,&pDispatchComments)!=S_OK||!pDispatchComments) {
		return 0;
	}
	if(_com_dispatch_raw_propget(pDispatchComments,wdDISPID_COMMENTS_COUNT,VT_I4,&count)!=S_OK||count<=0) {
		return 0;
	}
	return count;
}

bool fetchTableInfo(IDispatch* pDispatchTable, int* rowCount, int* columnCount, int* nestingLevel) {
	IDispatchPtr pDispatchRows=NULL;
	IDispatchPtr pDispatchColumns=NULL;
	if(_com_dispatch_raw_propget(pDispatchTable,wdDISPID_TABLE_ROWS,VT_DISPATCH,&pDispatchRows)==S_OK&&pDispatchRows) {
		_com_dispatch_raw_propget(pDispatchRows,wdDISPID_ROWS_COUNT,VT_I4,rowCount);
	}
	if(_com_dispatch_raw_propget(pDispatchTable,wdDISPID_TABLE_COLUMNS,VT_DISPATCH,&pDispatchColumns)==S_OK&&pDispatchColumns) {
		_com_dispatch_raw_propget(pDispatchColumns,wdDISPID_COLUMNS_COUNT,VT_I4,columnCount);
	}
	_com_dispatch_raw_propget(pDispatchTable,wdDISPID_TABLE_NESTINGLEVEL,VT_I4,nestingLevel);
	return true;
}

int generateTableXML(IDispatch* pDispatchRange, int startOffset, int endOffset, wostringstream& XMLStream) {
	int numTags=0;
	int iVal=0;
	IDispatchPtr pDispatchTables=NULL;
	IDispatchPtr pDispatchTable=NULL;
	int rowCount=0;
	int columnCount=0;
	int nestingLevel=0;
	if(
		_com_dispatch_raw_propget(pDispatchRange,wdDISPID_RANGE_TABLES,VT_DISPATCH,&pDispatchTables)!=S_OK||!pDispatchTables\
		||_com_dispatch_raw_method(pDispatchTables,wdDISPID_TABLES_ITEM,DISPATCH_METHOD,VT_DISPATCH,&pDispatchTable,L"\x0003",1)!=S_OK||!pDispatchTable\
		||!fetchTableInfo(pDispatchTable,&rowCount,&columnCount,&nestingLevel)\
	) {
		return 0;
	}
	numTags+=1;
	XMLStream<<L"<control role=\"table\" table-id=\"1\" table-rowcount=\""<<rowCount<<L"\" table-columncount=\""<<columnCount<<L"\" level=\""<<nestingLevel<<L"\" ";
	IDispatchPtr pDispatchTableRange=NULL;
	if(_com_dispatch_raw_propget(pDispatchTable,wdDISPID_TABLE_RANGE,VT_DISPATCH,&pDispatchTableRange)==S_OK&&pDispatchTableRange) {
		if(_com_dispatch_raw_propget(pDispatchTableRange,wdDISPID_RANGE_START,VT_I4,&iVal)==S_OK&&iVal>=startOffset) {
			XMLStream<<L"_startOfNode=\"1\" ";
		}
		if(_com_dispatch_raw_propget(pDispatchTableRange,wdDISPID_RANGE_END,VT_I4,&iVal)==S_OK&&iVal<=endOffset) {
			XMLStream<<L"_endOfNode=\"1\" ";
		}
	}
	numTags+=1;
	XMLStream<<L"><control role=\"tableCell\" table-id=\"1\" ";
	if(_com_dispatch_raw_method(pDispatchRange,wdDISPID_RANGE_INFORMATION,DISPATCH_PROPERTYGET,VT_I4,&iVal,L"\x0003",wdStartOfRangeRowNumber)==S_OK) {
		XMLStream<<L"table-rownumber=\""<<iVal<<L"\" ";
	}
	if(_com_dispatch_raw_method(pDispatchRange,wdDISPID_RANGE_INFORMATION,DISPATCH_PROPERTYGET,VT_I4,&iVal,L"\x0003",wdStartOfRangeColumnNumber)==S_OK) {
		XMLStream<<L"table-columnnumber=\""<<iVal<<L"\" ";
	}
	IDispatchPtr pDispatchCells=NULL;
	IDispatchPtr pDispatchCell=NULL;
	IDispatchPtr pDispatchCellRange=NULL;
	if(
		_com_dispatch_raw_propget(pDispatchRange,wdDISPID_RANGE_CELLS,VT_DISPATCH,&pDispatchCells)==S_OK&&pDispatchCells\
		&&_com_dispatch_raw_method(pDispatchCells,wdDISPID_CELLS_ITEM,DISPATCH_METHOD,VT_DISPATCH,&pDispatchCell,L"\x0003",1)==S_OK&&pDispatchCell\
		&&_com_dispatch_raw_propget(pDispatchCell,wdDISPID_CELL_RANGE,VT_DISPATCH,&pDispatchCellRange)==S_OK&&pDispatchCellRange\
	) {
		if(_com_dispatch_raw_propget(pDispatchCellRange,wdDISPID_RANGE_START,VT_I4,&iVal)==S_OK&&iVal>=startOffset) {
			XMLStream<<L"_startOfNode=\"1\" ";
		}
		if(_com_dispatch_raw_propget(pDispatchCellRange,wdDISPID_RANGE_END,VT_I4,&iVal)==S_OK&&iVal<=endOffset) {
			XMLStream<<L"_endOfNode=\"1\" ";
		}
	}
	XMLStream<<L">";
	return numTags;
}

void generateXMLAttribsForFormatting(IDispatch* pDispatchRange, int startOffset, int endOffset, int formatConfig, wostringstream& formatAttribsStream) {
	int iVal=0;
	if((formatConfig&formatConfig_reportPage)&&(_com_dispatch_raw_method(pDispatchRange,wdDISPID_RANGE_INFORMATION,DISPATCH_PROPERTYGET,VT_I4,&iVal,L"\x0003",wdActiveEndAdjustedPageNumber)==S_OK)&&iVal>0) {
		formatAttribsStream<<L"page-number=\""<<iVal<<L"\" ";
	}
	if((formatConfig&formatConfig_reportLineNumber)&&(_com_dispatch_raw_method(pDispatchRange,wdDISPID_RANGE_INFORMATION,DISPATCH_PROPERTYGET,VT_I4,&iVal,L"\x0003",wdFirstCharacterLineNumber)==S_OK)) {
		formatAttribsStream<<L"line-number=\""<<iVal<<L"\" ";
	}
	if(formatConfig&formatConfig_reportAlignment) {
		IDispatchPtr pDispatchParagraphFormat=NULL;
		if(_com_dispatch_raw_propget(pDispatchRange,wdDISPID_RANGE_PARAGRAPHFORMAT,VT_DISPATCH,&pDispatchParagraphFormat)==S_OK&&pDispatchParagraphFormat) {
			if(_com_dispatch_raw_propget(pDispatchParagraphFormat,wdDISPID_PARAGRAPHFORMAT_ALIGNMENT,VT_I4,&iVal)==S_OK) {
				switch(iVal) {
					case wdAlignParagraphLeft:
					formatAttribsStream<<L"text-align=\"left\" ";
					break;
					case wdAlignParagraphCenter:
					formatAttribsStream<<L"text-align=\"center\" ";
					break;
					case wdAlignParagraphRight:
					formatAttribsStream<<L"text-align=\"right\" ";
					break;
					case wdAlignParagraphJustify:
					formatAttribsStream<<L"text-align=\"justified\" ";
					break;
				}
			}
		}
	}
	if(formatConfig&formatConfig_reportLists) {
		IDispatchPtr pDispatchListFormat=NULL;
		if(_com_dispatch_raw_propget(pDispatchRange,wdDISPID_RANGE_LISTFORMAT,VT_DISPATCH,&pDispatchListFormat)==S_OK&&pDispatchListFormat) {
			BSTR listString=NULL;
			if(_com_dispatch_raw_propget(pDispatchListFormat,wdDISPID_LISTFORMAT_LISTSTRING,VT_BSTR,&listString)==S_OK&&listString) {
				if(SysStringLen(listString)>0) {
					IDispatchPtr pDispatchParagraphs=NULL;
					IDispatchPtr pDispatchParagraph=NULL;
					IDispatchPtr pDispatchParagraphRange=NULL;
					if(
						_com_dispatch_raw_propget(pDispatchRange,wdDISPID_RANGE_PARAGRAPHS,VT_DISPATCH,&pDispatchParagraphs)==S_OK&&pDispatchParagraphs\
						&&_com_dispatch_raw_method(pDispatchParagraphs,wdDISPID_PARAGRAPHS_ITEM,DISPATCH_METHOD,VT_DISPATCH,&pDispatchParagraph,L"\x0003",1)==S_OK&&pDispatchParagraph\
						&&_com_dispatch_raw_propget(pDispatchParagraph,wdDISPID_PARAGRAPH_RANGE,VT_DISPATCH,&pDispatchParagraphRange)==S_OK&&pDispatchParagraphRange\
						&&_com_dispatch_raw_propget(pDispatchParagraphRange,wdDISPID_RANGE_START,VT_I4,&iVal)==S_OK&&iVal==startOffset\
					) {
						formatAttribsStream<<L"line-prefix=\""<<listString<<L"\" ";
					}
				}
				SysFreeString(listString);
			}
		}
	}
	if((formatConfig&formatConfig_reportLinks)&&getHyperlinkCount(pDispatchRange)>0) {
		formatAttribsStream<<L"link=\"1\" ";
	}
	if(formatConfig&formatConfig_reportStyle) {
		IDispatchPtr pDispatchStyle=NULL;
		if(_com_dispatch_raw_propget(pDispatchRange,wdDISPID_RANGE_STYLE,VT_DISPATCH,&pDispatchStyle)==S_OK&&pDispatchStyle) {
			BSTR nameLocal=NULL;
			_com_dispatch_raw_propget(pDispatchStyle,wdDISPID_STYLE_NAMELOCAL,VT_BSTR,&nameLocal);
			if(nameLocal) {
				formatAttribsStream<<L"style=\""<<nameLocal<<L"\" ";
				SysFreeString(nameLocal);
			}
		}
	}
	if((formatConfig&formatConfig_reportComments)&&getCommentCount(pDispatchRange)>0) {
		formatAttribsStream<<L"comment=\"1\" ";
	}
	if(formatConfig&formatConfig_fontFlags) {
		IDispatchPtr pDispatchFont=NULL;
		if(_com_dispatch_raw_propget(pDispatchRange,wdDISPID_RANGE_FONT,VT_DISPATCH,&pDispatchFont)==S_OK&&pDispatchFont) {
			BSTR fontName=NULL;
			if((formatConfig&formatConfig_reportFontName)&&(_com_dispatch_raw_propget(pDispatchFont,wdDISPID_FONT_NAME,VT_BSTR,&fontName)==S_OK)&&fontName) {
				formatAttribsStream<<L"font-name=\""<<fontName<<L"\" ";
				SysFreeString(fontName);
			}
			if((formatConfig&formatConfig_reportFontSize)&&(_com_dispatch_raw_propget(pDispatchFont,wdDISPID_FONT_SIZE,VT_I4,&iVal)==S_OK)) {
				formatAttribsStream<<L"font-size=\""<<iVal<<L"pt\" ";
			}
			if((formatConfig&formatConfig_reportColor)&&(_com_dispatch_raw_propget(pDispatchFont,wdDISPID_FONT_COLOR,VT_I4,&iVal)==S_OK)) {
				formatAttribsStream<<L"color=\""<<iVal<<L"\" ";
			}
			if(formatConfig&formatConfig_reportFontAttributes) {
				if(_com_dispatch_raw_propget(pDispatchFont,wdDISPID_FONT_BOLD,VT_I4,&iVal)==S_OK&&iVal) {
					formatAttribsStream<<L"bold=\"1\" ";
				}
				if(_com_dispatch_raw_propget(pDispatchFont,wdDISPID_FONT_ITALIC,VT_I4,&iVal)==S_OK&&iVal) {
					formatAttribsStream<<L"italic=\"1\" ";
				}
				if(_com_dispatch_raw_propget(pDispatchFont,wdDISPID_FONT_UNDERLINE,VT_I4,&iVal)==S_OK&&iVal) {
					formatAttribsStream<<L"underline=\"1\" ";
				}
				if(_com_dispatch_raw_propget(pDispatchFont,wdDISPID_FONT_SUPERSCRIPT,VT_I4,&iVal)==S_OK&&iVal) {
					formatAttribsStream<<L"text-position=\"super\" ";
				} else if(_com_dispatch_raw_propget(pDispatchFont,wdDISPID_FONT_SUBSCRIPT,VT_I4,&iVal)==S_OK&&iVal) {
					formatAttribsStream<<L"text-position=\"sub\" ";
				}
			}
		}
	} 
	if(formatConfig&formatConfig_reportSpellingErrors) {
		IDispatchPtr pDispatchSpellingErrors=NULL;
		if(_com_dispatch_raw_propget(pDispatchRange,wdDISPID_RANGE_SPELLINGERRORS,VT_DISPATCH,&pDispatchSpellingErrors)==S_OK&&pDispatchSpellingErrors) {
			_com_dispatch_raw_propget(pDispatchSpellingErrors,wdDISPID_SPELLINGERRORS_COUNT,VT_I4,&iVal);
			if(iVal>0) {
				formatAttribsStream<<L"invalid-spelling=\""<<iVal<<L"\" ";
			}
		}
	}
	if (formatConfig&formatConfig_reportLanguage) {
		int languageId = 0;
		if (_com_dispatch_raw_propget(pDispatchRange,	wdDISPID_RANGE_LANGUAGEID, VT_I4, &languageId)==S_OK) {
			if (languageId != wdLanguageNone && languageId != wdNoProofing && languageId != wdLanguageUnknown) {
				formatAttribsStream<<L"wdLanguageId=\""<<languageId<<L"\" ";
			}
		}
	}
}

inline int getInlineShapesCount(IDispatch* pDispatchRange) {
	IDispatchPtr pDispatchShapes=NULL;
	int count=0;
	if(_com_dispatch_raw_propget(pDispatchRange,wdDISPID_RANGE_INLINESHAPES,VT_DISPATCH,&pDispatchShapes)!=S_OK||!pDispatchShapes) {
		return 0;
	}
	if(_com_dispatch_raw_propget(pDispatchShapes,wdDISPID_INLINESHAPES_COUNT,VT_I4,&count)!=S_OK||count<=0) {
		return 0;
	} 
	return count;
}

/**
 * Generates an opening tag for the first inline shape  in this range if one exists.
  * If the function is successful, the total number of inline shapes for this range is returned allowing the caller to then perhaps move the range forward a character and try again.
   */
inline int generateInlineShapeXML(IDispatch* pDispatchRange, wostringstream& XMLStream) {
	IDispatchPtr pDispatchShapes=NULL;
	IDispatchPtr pDispatchShape=NULL;
	int count=0;
	int shapeType=0;
	BSTR altText=NULL;
	if(_com_dispatch_raw_propget(pDispatchRange,wdDISPID_RANGE_INLINESHAPES,VT_DISPATCH,&pDispatchShapes)!=S_OK||!pDispatchShapes) {
		return 0;
	}
	if(_com_dispatch_raw_propget(pDispatchShapes,wdDISPID_INLINESHAPES_COUNT,VT_I4,&count)!=S_OK||count<=0) {
		return 0;
	}
	if(_com_dispatch_raw_method(pDispatchShapes,wdDISPID_INLINESHAPES_ITEM,DISPATCH_METHOD,VT_DISPATCH,&pDispatchShape,L"\x0003",1)!=S_OK||!pDispatchShape) {
		return 0;
	}
	if(_com_dispatch_raw_propget(pDispatchShape,wdDISPID_INLINESHAPE_TYPE,VT_I4,&shapeType)!=S_OK) {
		return 0;
	}
	if(_com_dispatch_raw_propget(pDispatchShape,wdDISPID_INLINESHAPE_ALTERNATIVETEXT,VT_BSTR,&altText)!=S_OK) {
		return 0;
	}
	XMLStream<<L"<control _startOfNode=\"1\" role=\""<<(shapeType==3?L"graphic":L"object")<<L"\" value=\""<<(altText?altText:L"")<<L"\">";
	if(altText) SysFreeString(altText);
	return count;
}

inline bool generateFootnoteEndnoteXML(IDispatch* pDispatchRange, wostringstream& XMLStream, bool footnote) {
	IDispatchPtr pDispatchNotes=NULL;
	IDispatchPtr pDispatchNote=NULL;
	int count=0;
	int index=0;
	if(_com_dispatch_raw_propget(pDispatchRange,(footnote?wdDISPID_RANGE_FOOTNOTES:wdDISPID_RANGE_ENDNOTES),VT_DISPATCH,&pDispatchNotes)!=S_OK||!pDispatchNotes) {
		return false;
	}
	if(_com_dispatch_raw_propget(pDispatchNotes,wdDISPID_FOOTNOTES_COUNT,VT_I4,&count)!=S_OK||count<=0) {
		return false;
	}
	if(_com_dispatch_raw_method(pDispatchNotes,wdDISPID_FOOTNOTES_ITEM,DISPATCH_METHOD,VT_DISPATCH,&pDispatchNote,L"\x0003",1)!=S_OK||!pDispatchNote) {
		return false;
	}
	if(_com_dispatch_raw_propget(pDispatchNote,wdDISPID_FOOTNOTE_INDEX,VT_I4,&index)!=S_OK) {
		return false;
	}
	XMLStream<<L"<control _startOfNode=\"1\" role=\""<<(footnote?L"footnote":L"endnote")<<L"\" value=\""<<index<<L"\">";
	return true;
}

UINT wm_winword_getTextInRange=0;
typedef struct {
	int startOffset;
	int endOffset;
	long formatConfig;
	BSTR text;
} winword_getTextInRange_args;
void winword_getTextInRange_helper(HWND hwnd, winword_getTextInRange_args* args) {
	//Fetch all needed objects
	//Get the window object
	IDispatchPtr pDispatchWindow=NULL;
	if(AccessibleObjectFromWindow(hwnd,OBJID_NATIVEOM,IID_IDispatch,(void**)&pDispatchWindow)!=S_OK) {
		LOG_DEBUGWARNING(L"AccessibleObjectFromWindow failed");
		return;
	}
		//Get the current selection
		IDispatchPtr pDispatchSelection=NULL;
	if(_com_dispatch_raw_propget(pDispatchWindow,wdDISPID_WINDOW_SELECTION,VT_DISPATCH,&pDispatchSelection)!=S_OK||!pDispatchSelection) {
		LOG_DEBUGWARNING(L"application.selection failed");
		return;
	}
	//Make a copy of the selection as an independent range
	IDispatchPtr pDispatchRange=NULL;
	if(_com_dispatch_raw_propget(pDispatchSelection,wdDISPID_SELECTION_RANGE,VT_DISPATCH,&pDispatchRange)!=S_OK||!pDispatchRange) {
		LOG_DEBUGWARNING(L"selection.range failed");
		return;
	}
	//Move the range to the requested offsets
	_com_dispatch_raw_method(pDispatchRange,wdDISPID_RANGE_SETRANGE,DISPATCH_METHOD,VT_EMPTY,NULL,L"\x0003\x0003",args->startOffset,args->endOffset);
	//A temporary stringstream for initial formatting
	wostringstream initialFormatAttribsStream;
	//Start writing the output xml to a stringstream
	wostringstream XMLStream;
	int neededClosingControlTagCount=0;
	int storyType=0;
	_com_dispatch_raw_propget(pDispatchRange,wdDISPID_RANGE_STORYTYPE,VT_I4,&storyType);
	XMLStream<<L"<control wdStoryType=\""<<storyType<<L"\">";
	neededClosingControlTagCount+=1;
	//Collapse the range
	int initialFormatConfig=(args->formatConfig)&formatConfig_initialFormatFlags;
	int formatConfig=(args->formatConfig)&(~formatConfig_initialFormatFlags);
	if((formatConfig&formatConfig_reportLinks)&&getHyperlinkCount(pDispatchRange)==0) {
		formatConfig&=~formatConfig_reportLinks;
	}
	if((formatConfig&formatConfig_reportComments)&&(storyType==wdCommentsStory||getCommentCount(pDispatchRange)==0)) {
		formatConfig&=~formatConfig_reportComments;
	}
	//Check for any inline shapes in the entire range to work out whether its worth checking for them by word
	bool hasInlineShapes=(getInlineShapesCount(pDispatchRange)>0);
	_com_dispatch_raw_method(pDispatchRange,wdDISPID_RANGE_COLLAPSE,DISPATCH_METHOD,VT_EMPTY,NULL,L"\x0003",wdCollapseStart);
	int chunkStartOffset=args->startOffset;
	int chunkEndOffset=chunkStartOffset;
	int unitsMoved=0;
	BSTR text=NULL;
	//Walk the range from the given start to end by characterFormatting or word units
	//And grab any text and formatting and generate appropriate xml
	bool firstLoop=true;
	do {
		//Move the end by word
		if(_com_dispatch_raw_method(pDispatchRange,wdDISPID_RANGE_MOVEEND,DISPATCH_METHOD,VT_I4,&unitsMoved,L"\x0003\x0003",wdWord,1)!=S_OK||unitsMoved<=0) {
			break;
		}
		_com_dispatch_raw_propget(pDispatchRange,wdDISPID_RANGE_END,VT_I4,&chunkEndOffset);
		//Make sure  that the end is not past the requested end after the move
		if(chunkEndOffset>(args->endOffset)) {
			_com_dispatch_raw_propput(pDispatchRange,wdDISPID_RANGE_END,VT_I4,args->endOffset);
			chunkEndOffset=args->endOffset;
		}
		//When using IME, the last moveEnd succeeds but the end does not really move
		if(chunkEndOffset<=chunkStartOffset) {
			LOG_DEBUGWARNING(L"moveEnd successfull but range did not expand! chunkStartOffset "<<chunkStartOffset<<L", chunkEndOffset "<<chunkEndOffset);
			break;
		}
		_com_dispatch_raw_propget(pDispatchRange,wdDISPID_RANGE_TEXT,VT_BSTR,&text);
		if(text) {
			//Force a new chunk before and after control+b (note characters)
			int noteCharOffset=-1;
			for(int i=0;text[i]!=L'\0';++i) {
				if(text[i]==L'\x0002') {
					noteCharOffset=i;
					if(i==0) text[i]=L' ';
					break;
				}
			}
			bool isNoteChar=(noteCharOffset==0);
			if(noteCharOffset==0) noteCharOffset=1;
			if(noteCharOffset>0) {
				text[noteCharOffset]=L'\0';
				_com_dispatch_raw_method(pDispatchRange,wdDISPID_RANGE_COLLAPSE,DISPATCH_METHOD,VT_EMPTY,NULL,L"\x0003",wdCollapseStart);
				if(_com_dispatch_raw_method(pDispatchRange,wdDISPID_RANGE_MOVEEND,DISPATCH_METHOD,VT_I4,&unitsMoved,L"\x0003\x0003",wdCharacter,noteCharOffset)!=S_OK||unitsMoved<=0) {
					break;
				}
				_com_dispatch_raw_propget(pDispatchRange,wdDISPID_RANGE_END,VT_I4,&chunkEndOffset);
			}
			if(firstLoop) {
				if(initialFormatConfig&formatConfig_reportTables) {
					neededClosingControlTagCount+=generateTableXML(pDispatchRange,args->startOffset,args->endOffset,XMLStream);
				}
				if(initialFormatConfig&formatConfig_reportHeadings) {
					neededClosingControlTagCount+=generateHeadingXML(pDispatchRange,XMLStream);
				}
				generateXMLAttribsForFormatting(pDispatchRange,chunkStartOffset,chunkEndOffset,initialFormatConfig,initialFormatAttribsStream);
			}
			if(isNoteChar) {
				isNoteChar=generateFootnoteEndnoteXML(pDispatchRange,XMLStream,true);
				if(!isNoteChar) isNoteChar=generateFootnoteEndnoteXML(pDispatchRange,XMLStream,false);
			}
			//If there are inline shapes somewhere, try getting and generating info for the first one hear.
			//We also get the over all count of shapes for this word so we know whether we need to check for more within this word
			int inlineShapesCount=hasInlineShapes?generateInlineShapeXML(pDispatchRange,XMLStream):0;
			if(inlineShapesCount>1) {
				_com_dispatch_raw_method(pDispatchRange,wdDISPID_RANGE_COLLAPSE,DISPATCH_METHOD,VT_EMPTY,NULL,L"\x0003",wdCollapseStart);
				if(_com_dispatch_raw_method(pDispatchRange,wdDISPID_RANGE_MOVEEND,DISPATCH_METHOD,VT_I4,&unitsMoved,L"\x0003\x0003",wdCharacter,1)!=S_OK||unitsMoved<=0) {
					break;
				}
				_com_dispatch_raw_propget(pDispatchRange,wdDISPID_RANGE_END,VT_I4,&chunkEndOffset);
			}
			XMLStream<<L"<text ";
			XMLStream<<initialFormatAttribsStream.str();
			generateXMLAttribsForFormatting(pDispatchRange,chunkStartOffset,chunkEndOffset,formatConfig,XMLStream);
			XMLStream<<L">";
			if(firstLoop) {
				formatConfig&=(~formatConfig_reportLists);
				firstLoop=false;
			}
			wstring tempText;
			if(inlineShapesCount>0) {
				tempText+=L" ";
			} else for(int i=0;text[i]!=L'\0';++i) {
				appendCharToXML(text[i],tempText);
			}
			XMLStream<<tempText;
			SysFreeString(text);
			text=NULL;
			XMLStream<<L"</text>";
			if(isNoteChar) XMLStream<<L"</control>";
			if(inlineShapesCount>0) XMLStream<<L"</control>";
		}
		_com_dispatch_raw_method(pDispatchRange,wdDISPID_RANGE_COLLAPSE,DISPATCH_METHOD,VT_EMPTY,NULL,L"\x0003",wdCollapseEnd);
		chunkStartOffset=chunkEndOffset;
	} while(chunkEndOffset<(args->endOffset));
	for(;neededClosingControlTagCount>0;--neededClosingControlTagCount) {
		XMLStream<<L"</control>";
	}
	args->text=SysAllocString(XMLStream.str().c_str());
}

LRESULT CALLBACK winword_callWndProcHook(int code, WPARAM wParam, LPARAM lParam) {
	CWPSTRUCT* pcwp=(CWPSTRUCT*)lParam;
	if(pcwp->message==wm_winword_expandToLine) {
		winword_expandToLine_helper(pcwp->hwnd,reinterpret_cast<winword_expandToLine_args*>(pcwp->wParam));
	} else if(pcwp->message==wm_winword_getTextInRange) {
		winword_getTextInRange_helper(pcwp->hwnd,reinterpret_cast<winword_getTextInRange_args*>(pcwp->wParam));
	}
	return 0;
}

error_status_t nvdaInProcUtils_winword_expandToLine(handle_t bindingHandle, const long windowHandle, const int offset, int* lineStart, int* lineEnd) {
	winword_expandToLine_args args={offset,-1,-1};
	DWORD_PTR wmRes=0;
	SendMessage((HWND)windowHandle,wm_winword_expandToLine,(WPARAM)&args,0);
	*lineStart=args.lineStart;
	*lineEnd=args.lineEnd;
	return RPC_S_OK;
}

error_status_t nvdaInProcUtils_winword_getTextInRange(handle_t bindingHandle, const long windowHandle, const int startOffset, const int endOffset, const long formatConfig, BSTR* text) { 
	winword_getTextInRange_args args={startOffset,endOffset,formatConfig,NULL};
	SendMessage((HWND)windowHandle,wm_winword_getTextInRange,(WPARAM)&args,0);
	*text=args.text;
	return RPC_S_OK;
}

void winword_inProcess_initialize() {
	wm_winword_expandToLine=RegisterWindowMessage(L"wm_winword_expandToLine");
	wm_winword_getTextInRange=RegisterWindowMessage(L"wm_winword_getTextInRange");
	registerWindowsHook(WH_CALLWNDPROC,winword_callWndProcHook);
}

void winword_inProcess_terminate() {
	unregisterWindowsHook(WH_CALLWNDPROC,winword_callWndProcHook);
}
