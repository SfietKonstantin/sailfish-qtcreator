/****************************************************************************
**
** Copyright (C) 2012 - 2014 Jolla Ltd.
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

#include "rpmvalidationcommand.h"

RpmValidationCommand::RpmValidationCommand()
{

}

QString RpmValidationCommand::name() const
{
    return QLatin1String("rpmvalidation");
}

int RpmValidationCommand::execute()
{
    QString command = QLatin1String("rpmvalidation.sh") +
                      QLatin1Char(' ') + arguments().join(QLatin1Char(' ')) + QLatin1Char(' ');
    MerRemoteProcess process;
    process.setSshParameters(sshParameters());
    process.setCommand(remotePathMapping(command));
    return process.executeAndWait();
}

bool RpmValidationCommand::isValid() const
{
    return Command::isValid() && !targetName().isEmpty();
}
