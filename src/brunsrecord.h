/***************************************************************************
                    brunsrecord.h - One Bruns Plant record
                             -------------------
    begin                : 2005-07
    copyright            : (C) 2005 by Klaas Freitag
    email                : freitag@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef BRUNSBRUNSRECORD_H
#define BRUNSBRUNSRECORD_H

#include <q3cstring.h>
#include <q3ptrlist.h>

#include "kraftcat_export.h"

/**
@author Klaas Freitag
*/
class KRAFTCAT_EXPORT BrunsSize {

public:
    BrunsSize();

    int getFormNo () { return formNo; }
    void setFormNo (int i) { formNo = i; }

    int getGrothNo () { return grothNo; }
    void setGrothNo (int i) { grothNo = i; }

    int getRootPack () { return rootPack; }
    void setRootPack (int i) { rootPack = i; }

    int getQualityAdd () { return qualityAdd; }
    void setQualityAdd (int i) { qualityAdd = i; }

    int getFormAdd () { return formAdd; }
    void setFormAdd (int i) { formAdd = i; }

    int getGoodsGroup () { return goodsGroup; }
    void setGoodsGroup (int i) { goodsGroup = i; }

    int getSize () { return sizeNo; }
    void setSize (int i) { sizeNo = i; }

    int getSizeAdd () { return sizeAddNo; }
    void setSizeAdd (int i) { sizeAddNo = i; }

    QString getPrimMatchcode() { return primMatch; }
    void setPrimMatchcode(const QString& str) { primMatch = str; }
private:
    int   formNo;      // 35-38
    int   grothNo;     // 39-42
    int   rootPack;    // 43-46
    int   qualityAdd;  // 53-56
    int   formAdd;     // 165-168
    int   goodsGroup;   // 268-271
    int   sizeNo;
    int   sizeAddNo;
    QString primMatch;
};
typedef QList<BrunsSize> BrunsSizeList;


class KRAFTCAT_EXPORT BrunsRecord{
public:
    // Construct with an artikelID
    BrunsRecord(){};
    BrunsRecord(int);
    ~BrunsRecord();

    int getArtNo() { return artNo; }

    bool isPassNeeded() { return passNeeded; }
    void setPassNeeded(bool b) { passNeeded = b; }

    int getPlantGroup() { return plantGroup; }
    void setPlantGroup (int i) { plantGroup = i; }

    int getArtId () { return artId; }
    void setArtId (int i) { artId = i; }

    Q3CString getArtMatch() const { return artMatch; }
    void setArtMatch( const Q3CString& str ) { artMatch = str; }

    Q3CString getLtName() const { return ltName; }
    void setLtName( const Q3CString& n ) { ltName = n; }

    Q3CString getDtName() const { return dtName; }
    void setDtName( const Q3CString& n ) { dtName = n; }
    
    void debugOut();

    void addSize( const BrunsSize& size );
    BrunsSizeList getSizes() { return m_sizes; }
    void clearSizes();
    
private:
    BrunsSizeList m_sizes;
    //                    character Index
    int   artNo;       // 1-6
    bool  passNeeded;  // 11
    int   plantGroup;  // 13-18
    int   artId;       // 19-24
    Q3CString artMatch; // 25-34
    Q3CString dtName;   // 272-331
    Q3CString ltName;   // 322-391
};

typedef QList<BrunsRecord*> BrunsRecordList;

#endif
