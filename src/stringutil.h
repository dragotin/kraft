/***************************************************************************
                  String helper functions
                             -------------------
    begin                : July 2023
    copyright            : (C) 2023 by Klaas Freitag
    email                : kraft@freisturz.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef STRINGUTIL_H
#define STRINGUTIL_H

class QString;

namespace StringUtil {
    /**
     * @brief replaceTagsInString.
     * @param w - the string that includes the %-prepended tags
     * @param replaceMap - a map containing key/values that replace the tags.
     * @return the string with tags replaced
     *
     */
    QString replaceTagsInString( const QString& w, QMap<QString, QString>& replaceMap );
}


#endif // FORMAT_H
