/****************************************************************************
**
** Copyright (C) 2012-2015,2018-2019 Jolla Ltd.
** Copyright (C) 2020 Open Mobile Platform LLC.
** Contact: http://jolla.com/
**
** This file is part of Qt Creator.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** Other Usage
**
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Digia.
**
****************************************************************************/

#include "rpmcommand.h"

RpmCommand::RpmCommand()
{

}

QString RpmCommand::name() const
{
    return QLatin1String("rpm");
}

int RpmCommand::execute()
{
    // FIXME Rename me as "package"
    return executeSfdk(QStringList{"package"} + arguments().mid(1));
}

bool RpmCommand::isValid() const
{
    return Command::isValid() && !targetName().isEmpty();
}