/*
    Copyright (c) 2016 Christian Mollekopf <mollekopf@kolabsys.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/
#include "messageparser.h"

#include "partmodel.h"
#include "attachmentmodel.h"
#include "modeltest.h"
#include <mimetreeparser/objecttreeparser.h>

#include <QDebug>
#include <QtConcurrent/QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>

class MessagePartPrivate
{
public:
    std::shared_ptr<MimeTreeParser::ObjectTreeParser> mParser;
};

MessageParser::MessageParser(QObject *parent)
    : QObject(parent)
    , d(std::unique_ptr<MessagePartPrivate>(new MessagePartPrivate))
{

}

MessageParser::~MessageParser()
{

}

QVariant MessageParser::message() const
{
    return QVariant();
}

void MessageParser::setMessage(const QVariant &message)
{
    mRawContent = message.toString();
    auto future = QtConcurrent::run([message] {
        auto parser = std::make_shared<MimeTreeParser::ObjectTreeParser>();
        parser->parseObjectTree(message.toByteArray());
        parser->decryptParts();
        return parser;
    });
    auto watcher = new QFutureWatcher<std::shared_ptr<MimeTreeParser::ObjectTreeParser>>;
    QObject::connect(watcher, &QFutureWatcher<std::shared_ptr<MimeTreeParser::ObjectTreeParser>>::finished, watcher, [this, watcher]() {
        d->mParser = watcher->future().result();
        delete watcher;
        emit htmlChanged();
    });
    watcher->setFuture(future);
}

QString MessageParser::rawContent() const
{
    return mRawContent;
}

QAbstractItemModel *MessageParser::parts() const
{
    if (!d->mParser) {
        return nullptr;
    }
    const auto model = new PartModel(d->mParser);
    // new ModelTest(model, model);
    return model;
}

QAbstractItemModel *MessageParser::attachments() const
{
    if (!d->mParser) {
        return nullptr;
    }
    const auto model = new AttachmentModel(d->mParser);
    // new ModelTest(model, model);
    return model;
}
