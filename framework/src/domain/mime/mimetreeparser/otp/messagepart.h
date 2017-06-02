/*
   Copyright (c) 2015 Sandro Knauß <sknauss@kde.org>

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

#ifndef __MIMETREEPARSER_MESSAGEPART_H__
#define __MIMETREEPARSER_MESSAGEPART_H__

#include "util.h"
#include "enums.h"
#include "partmetadata.h"

#include <KMime/Message>

#include <gpgme++/verificationresult.h>
#include <gpgme++/decryptionresult.h>
#include <gpgme++/importresult.h>

#include <QString>
#include <QSharedPointer>

class QTextCodec;
class PartPrivate;

namespace GpgME
{
class ImportResult;
}

namespace QGpgME
{
class Protocol;
}

namespace KMime
{
class Content;
}

namespace MimeTreeParser
{
class ObjectTreeParser;
class HTMLBlock;
typedef QSharedPointer<HTMLBlock> HTMLBlockPtr;
class CryptoBodyPartMemento;
class MultiPartAlternativeBodyPartFormatter;
namespace Interface
{
class ObjectTreeSource;
}

class SignedMessagePart;
class EncryptedMessagePart;

class MessagePart : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool attachment READ isAttachment)
    Q_PROPERTY(bool root READ isRoot)
    Q_PROPERTY(bool isHtml READ isHtml)
    Q_PROPERTY(QString plaintextContent READ plaintextContent)
    Q_PROPERTY(QString htmlContent READ htmlContent)
public:
    enum Disposition {
        Inline,
        Attachment,
        Invalid
    };
    typedef QSharedPointer<MessagePart> Ptr;
    MessagePart(ObjectTreeParser *otp, const QString &text, KMime::Content *node = nullptr);

    virtual ~MessagePart();

    virtual QString text() const;
    void setText(const QString &text);
    bool isAttachment() const;

    void setIsRoot(bool root);
    bool isRoot() const;

    void setParentPart(MessagePart *parentPart);
    MessagePart *parentPart() const;

    virtual QString plaintextContent() const;
    virtual QString htmlContent() const;

    virtual bool isHtml() const;

    QByteArray mimeType() const;
    QByteArray charset() const;
    QString filename() const;
    Disposition disposition() const;
    bool isText() const;

    PartMetaData *partMetaData();

    void appendSubPart(const MessagePart::Ptr &messagePart);
    const QVector<MessagePart::Ptr> &subParts() const;
    bool hasSubParts() const;


    Interface::ObjectTreeSource *source() const;
    KMime::Content *node() const;

    QVector<SignedMessagePart*> signatures() const;
    QVector<EncryptedMessagePart*> encryptions() const;

protected:
    void parseInternal(KMime::Content *node, bool onlyOneMimePart);
    QString renderInternalText() const;

    QString mText;
    ObjectTreeParser *mOtp;
    PartMetaData mMetaData;
    MessagePart *mParentPart;
    KMime::Content *mNode;

private:
    QVector<MessagePart::Ptr> mBlocks;
    bool mRoot;
};

class MimeMessagePart : public MessagePart
{
    Q_OBJECT
public:
    typedef QSharedPointer<MimeMessagePart> Ptr;
    MimeMessagePart(MimeTreeParser::ObjectTreeParser *otp, KMime::Content *node, bool onlyOneMimePart);
    virtual ~MimeMessagePart();

    QString text() const Q_DECL_OVERRIDE;

    QString plaintextContent() const Q_DECL_OVERRIDE;
    QString htmlContent() const Q_DECL_OVERRIDE;
private:
    friend class AlternativeMessagePart;
    friend class ::PartPrivate;
};

class MessagePartList : public MessagePart
{
    Q_OBJECT
public:
    typedef QSharedPointer<MessagePartList> Ptr;
    MessagePartList(MimeTreeParser::ObjectTreeParser *otp, KMime::Content *node = nullptr);
    virtual ~MessagePartList();

    QString text() const Q_DECL_OVERRIDE;

    QString plaintextContent() const Q_DECL_OVERRIDE;
    QString htmlContent() const Q_DECL_OVERRIDE;
};

enum IconType {
    NoIcon = 0,
    IconExternal,
    IconInline
};

class TextMessagePart : public MessagePartList
{
    Q_OBJECT
public:
    typedef QSharedPointer<TextMessagePart> Ptr;
    TextMessagePart(MimeTreeParser::ObjectTreeParser *otp, KMime::Content *node);
    virtual ~TextMessagePart();

    KMMsgSignatureState signatureState() const;
    KMMsgEncryptionState encryptionState() const;

private:
    void parseContent();

    KMMsgSignatureState mSignatureState;
    KMMsgEncryptionState mEncryptionState;

    friend class DefaultRendererPrivate;
    friend class ObjectTreeParser;
    friend class ::PartPrivate;
};

class AttachmentMessagePart : public TextMessagePart
{
    Q_OBJECT
public:
    typedef QSharedPointer<AttachmentMessagePart> Ptr;
    AttachmentMessagePart(MimeTreeParser::ObjectTreeParser *otp, KMime::Content *node);
    virtual ~AttachmentMessagePart();

};

class HtmlMessagePart : public MessagePart
{
    Q_OBJECT
public:
    typedef QSharedPointer<HtmlMessagePart> Ptr;
    HtmlMessagePart(MimeTreeParser::ObjectTreeParser *otp, KMime::Content *node, MimeTreeParser::Interface::ObjectTreeSource *source);
    virtual ~HtmlMessagePart();

    QString text() const Q_DECL_OVERRIDE;

    bool isHtml() const Q_DECL_OVERRIDE;

private:
    Interface::ObjectTreeSource *mSource;
    QString mBodyHTML;
    QByteArray mCharset;

    friend class DefaultRendererPrivate;
    friend class ::PartPrivate;
};

class AlternativeMessagePart : public MessagePart
{
    Q_OBJECT
public:
    typedef QSharedPointer<AlternativeMessagePart> Ptr;
    AlternativeMessagePart(MimeTreeParser::ObjectTreeParser *otp, KMime::Content *node, Util::HtmlMode preferredMode);
    virtual ~AlternativeMessagePart();

    QString text() const Q_DECL_OVERRIDE;

    Util::HtmlMode preferredMode() const;

    bool isHtml() const Q_DECL_OVERRIDE;

    QString plaintextContent() const Q_DECL_OVERRIDE;
    QString htmlContent() const Q_DECL_OVERRIDE;

    QList<Util::HtmlMode> availableModes();
private:
    Util::HtmlMode mPreferredMode;

    QMap<Util::HtmlMode, KMime::Content *> mChildNodes;
    QMap<Util::HtmlMode, MimeMessagePart::Ptr> mChildParts;

    friend class DefaultRendererPrivate;
    friend class ObjectTreeParser;
    friend class MultiPartAlternativeBodyPartFormatter;
    friend class ::PartPrivate;
};

class CertMessagePart : public MessagePart
{
    Q_OBJECT
public:
    typedef QSharedPointer<CertMessagePart> Ptr;
    CertMessagePart(MimeTreeParser::ObjectTreeParser *otp, KMime::Content *node, const QGpgME::Protocol *cryptoProto, bool autoImport);
    virtual ~CertMessagePart();

    QString text() const Q_DECL_OVERRIDE;

private:
    bool mAutoImport;
    GpgME::ImportResult mImportResult;
    const QGpgME::Protocol *mCryptoProto;
    friend class DefaultRendererPrivate;
};

class EncapsulatedRfc822MessagePart : public MessagePart
{
    Q_OBJECT
public:
    typedef QSharedPointer<EncapsulatedRfc822MessagePart> Ptr;
    EncapsulatedRfc822MessagePart(MimeTreeParser::ObjectTreeParser *otp, KMime::Content *node, const KMime::Message::Ptr &message);
    virtual ~EncapsulatedRfc822MessagePart();

    QString text() const Q_DECL_OVERRIDE;
private:
    const KMime::Message::Ptr mMessage;

    friend class DefaultRendererPrivate;
};

class EncryptedMessagePart : public MessagePart
{
    Q_OBJECT
    Q_PROPERTY(bool isEncrypted READ isEncrypted)
    Q_PROPERTY(bool passphraseError READ passphraseError)
public:
    typedef QSharedPointer<EncryptedMessagePart> Ptr;
    EncryptedMessagePart(ObjectTreeParser *otp,
                         const QString &text,
                         const QGpgME::Protocol *cryptoProto,
                         const QString &fromAddress,
                         KMime::Content *node, KMime::Content *encryptedNode = nullptr);

    virtual ~EncryptedMessagePart();

    QString text() const Q_DECL_OVERRIDE;

    void setIsEncrypted(bool encrypted);
    bool isEncrypted() const;

    bool isDecryptable() const;

    bool passphraseError() const;

    void startDecryption(const QByteArray &text, const QTextCodec *aCodec);
    void startDecryption(KMime::Content *data = nullptr);

    QByteArray mDecryptedData;

    QString plaintextContent() const Q_DECL_OVERRIDE;
    QString htmlContent() const Q_DECL_OVERRIDE;

private:
    /** Handles the dectyptioon of a given content
     * returns true if the decryption was successfull
     * if used in async mode, check if mMetaData.inProgress is true, it inicates a running decryption process.
     */
    bool okDecryptMIME(KMime::Content &data);

protected:
    bool mPassphraseError;
    bool mNoSecKey;
    const QGpgME::Protocol *mCryptoProto;
    QString mFromAddress;
    QByteArray mVerifiedText;
    std::vector<GpgME::DecryptionResult::Recipient> mDecryptRecipients;
    KMime::Content *mEncryptedNode;

    friend class DefaultRendererPrivate;
    friend class ::PartPrivate;
};

class SignedMessagePart : public MessagePart
{
    Q_OBJECT
    Q_PROPERTY(bool isSigned READ isSigned)
public:
    typedef QSharedPointer<SignedMessagePart> Ptr;
    SignedMessagePart(ObjectTreeParser *otp,
                      const QString &text,
                      const QGpgME::Protocol *cryptoProto,
                      const QString &fromAddress,
                      KMime::Content *node);

    virtual ~SignedMessagePart();

    void setIsSigned(bool isSigned);
    bool isSigned() const;

    void startVerification(const QByteArray &text, const QTextCodec *aCodec);
    void startVerificationDetached(const QByteArray &text, KMime::Content *textNode, const QByteArray &signature);

    QByteArray mDecryptedData;
    std::vector<GpgME::Signature> mSignatures;

    QString plaintextContent() const Q_DECL_OVERRIDE;
    QString htmlContent() const Q_DECL_OVERRIDE;

private:
    /** Handles the verification of data
     * If signature is empty it is handled as inline signature otherwise as detached signature mode.
     * Returns true if the verfication was successfull and the block is signed.
     * If used in async mode, check if mMetaData.inProgress is true, it inicates a running verification process.
     */
    bool okVerify(const QByteArray &data, const QByteArray &signature, KMime::Content *textNode);

    void sigStatusToMetaData();

    void setVerificationResult(const CryptoBodyPartMemento *m, KMime::Content *textNode);
protected:
    const QGpgME::Protocol *mCryptoProto;
    QString mFromAddress;
    QByteArray mVerifiedText;

    friend EncryptedMessagePart;
    friend class DefaultRendererPrivate;
    friend class ::PartPrivate;
};

}

#endif //__MIMETREEPARSER_MESSAGEPART_H__
