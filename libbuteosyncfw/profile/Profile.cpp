/*
 * This file is part of buteo-syncfw package
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Sateesh Kavuri <sateesh.kavuri@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */


#include "Profile.h"
#include "Profile_p.h"

#include <QDomDocument>

#include "ProfileFactory.h"
#include "ProfileEngineDefs.h"
#include "LogMacros.h"

using namespace Buteo;

const QString Profile::TYPE_CLIENT("client");
const QString Profile::TYPE_SERVER("server");
const QString Profile::TYPE_STORAGE("storage");
//const QString Profile::TYPE_SERVICE("service");
const QString Profile::TYPE_SYNC("sync");


Profile::Profile()
    : d_ptr(new ProfilePrivate())
{
}

Profile::Profile(const QString &aName, const QString &aType)
    : d_ptr(new ProfilePrivate())
{
    d_ptr->iName = aName;
    d_ptr->iType = aType;
}

Profile::Profile(const QDomElement &aRoot)
    : d_ptr(new ProfilePrivate())
{
    d_ptr->iName = aRoot.attribute(ATTR_NAME);
    d_ptr->iType = aRoot.attribute(ATTR_TYPE);

    // Get keys.
    QDomElement key = aRoot.firstChildElement(TAG_KEY);
    for (; !key.isNull(); key = key.nextSiblingElement(TAG_KEY)) {
        QString name = key.attribute(ATTR_NAME);
        QString value = key.attribute(ATTR_VALUE);
        if (!name.isEmpty() && !value.isNull()) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        d_ptr->iLocalKeys.insertMulti(name, value);
#else
        d_ptr->iLocalKeys.insert(name, value);
#endif
        } else {
            // Invalid key
        }
    }

    // Get fields.
    QDomElement field = aRoot.firstChildElement(TAG_FIELD);
    for (; !field.isNull(); field = field.nextSiblingElement(TAG_FIELD)) {
        d_ptr->iLocalFields.append(new ProfileField(field));
    }

    // Get sub-profiles.
    ProfileFactory pf;
    QDomElement prof = aRoot.firstChildElement(TAG_PROFILE);
    for (; !prof.isNull(); prof = prof.nextSiblingElement(TAG_PROFILE)) {
        Profile *subProfile = pf.createProfile(prof);
        if (subProfile != 0) {
            d_ptr->iSubProfiles.append(subProfile);
        }
    }
}

Profile::Profile(const Profile &aSource)
    : d_ptr(new ProfilePrivate(*aSource.d_ptr))
{
}

Profile *Profile::clone() const
{
    return new Profile(*this);
}

Profile::~Profile()
{
    delete d_ptr;
    d_ptr = 0;
}

QString Profile::name() const
{
    return d_ptr->iName;
}

void Profile::setName(const QString &aName)
{
    d_ptr->iName = aName;
}

void Profile::setName(const QStringList &aKeys)
{

    d_ptr->iName = generateProfileId(aKeys);
}

QString Profile::type() const
{
    return d_ptr->iType;
}

QString Profile::key(const QString &aName, const QString &aDefault) const
{
    QString value;
    if (d_ptr->iLocalKeys.contains(aName)) {
        value = d_ptr->iLocalKeys[aName];
    } else if (d_ptr->iMergedKeys.contains(aName)) {
        value = d_ptr->iMergedKeys[aName];
    } else {
        value = aDefault;
    }
    return value;
}

QMap<QString, QString> Profile::allKeys() const
{
    QMap<QString, QString> keys(d_ptr->iMergedKeys);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    keys.unite(d_ptr->iLocalKeys);
#else
    for (auto it = d_ptr->iLocalKeys.begin(); it != d_ptr->iLocalKeys.end(); ++it)
        keys.insert(it.key(), it.value());
#endif

    return keys;
}

QMap<QString, QString> Profile::allNonStorageKeys() const
{
    QMap<QString, QString> keys;
    const QList<Profile *> &profiles = d_ptr->iSubProfiles;

    for(Profile * const p : profiles) {
        if (p != 0 && p->type() != Profile::TYPE_STORAGE) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            keys.unite(p->allKeys());
#else
        QMap<QString, QString> pKeys = p->allKeys();
        for (auto it = pKeys.begin(); it != pKeys.end(); ++it)
            keys.insert(it.key(), it.value());
#endif
        }
    }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    keys.unite(allKeys());
#else
    QMap<QString, QString> allKeysMap = allKeys();
    for (auto it = allKeysMap.begin(); it != allKeysMap.end(); ++it)
        keys.insert(it.key(), it.value());
#endif
    return keys;
}

bool Profile::boolKey(const QString &aName, bool aDefault) const
{
    QString value = key(aName);
    if (!value.isNull()) {
        return (key(aName).compare(BOOLEAN_TRUE, Qt::CaseInsensitive) == 0);
    } else {
        return aDefault;
    }
}

QStringList Profile::keyValues(const QString &aName) const
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return (d_ptr->iLocalKeys.values(aName) +
            d_ptr->iMergedKeys.values(aName));
#else
    QStringList result;
    auto localIt = d_ptr->iLocalKeys.find(aName);
    if (localIt != d_ptr->iLocalKeys.end()) result.append(localIt.value());
    auto mergedIt = d_ptr->iMergedKeys.find(aName);
    if (mergedIt != d_ptr->iMergedKeys.end()) result.append(mergedIt.value());
    return result;
#endif
}

QStringList Profile::keyNames() const
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return d_ptr->iLocalKeys.uniqueKeys() + d_ptr->iMergedKeys.uniqueKeys();
#else
    return QStringList(d_ptr->iLocalKeys.keys()) + QStringList(d_ptr->iMergedKeys.keys());
#endif
}

void Profile::setKey(const QString &aName, const QString &aValue)
{
    if (aName.isEmpty())
        return;

    // Value is not checked, because it is allowed to have a key with empty
    // value.

    if (aValue.isNull()) {
        // Setting a key value to null removes the key.
        d_ptr->iLocalKeys.remove(aName);
        d_ptr->iMergedKeys.remove(aName);
    } else {
        d_ptr->iLocalKeys.insert(aName, aValue);
    }
}

void Profile::setKeyValues(const QString &aName, const QStringList &aValues)
{
    d_ptr->iLocalKeys.remove(aName);
    d_ptr->iMergedKeys.remove(aName);

    if (aValues.size() == 0)
        return;

    unsigned i = aValues.size();
    do {
        i--;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        d_ptr->iLocalKeys.insertMulti(aName, aValues[i]);
#else
        d_ptr->iLocalKeys.insert(aName, aValues[i]);
#endif
    } while (i > 0);
}

void Profile::setBoolKey(const QString &aName, bool aValue)
{
    d_ptr->iLocalKeys.insert(aName, aValue ? BOOLEAN_TRUE : BOOLEAN_FALSE);
}

void Profile::removeKey(const QString &aName)
{
    d_ptr->iLocalKeys.remove(aName);
    d_ptr->iMergedKeys.remove(aName);
}

const ProfileField *Profile::field(const QString &aName) const
{
    const QList<const ProfileField *> &fields = allFields();
    for(const ProfileField * const f : fields) {
        if (f->name() == aName)
            return f;
    }

    return 0;
}

QList<const ProfileField *> Profile::allFields() const
{
    QList<const ProfileField *> fields =
        d_ptr->iLocalFields + d_ptr->iMergedFields;
    return fields;
}

QList<const ProfileField *> Profile::visibleFields() const
{
    const QList<const ProfileField *> &fields = allFields();
    QList<const ProfileField *> visibleFields;

    for(const ProfileField * const f : fields) {
        // A field with VISIBLE_USER status is visible if a value for the field
        // does not come from a merged sub-profile, but from the top level
        // profile. This is the default visibility for a field.
        // In practice this means that the field value is not hard coded and
        // it should be possible for the user to modify it.
        if (f->visible() == ProfileField::VISIBLE_ALWAYS ||
                (f->visible() == ProfileField::VISIBLE_USER &&
                 !d_ptr->iMergedKeys.contains(f->name()))) {
            visibleFields.append(f);
        }
    }

    return visibleFields;
}

QDomElement Profile::toXml(QDomDocument &aDoc, bool aLocalOnly) const
{
    // Set profile name and type attributes.
    QDomElement root = aDoc.createElement(TAG_PROFILE);
    root.setAttribute(ATTR_NAME, d_ptr->iName);
    root.setAttribute(ATTR_TYPE, d_ptr->iType);

    // Set local keys.
    QMap<QString, QString>::const_iterator i;
    for (i = d_ptr->iLocalKeys.begin(); i != d_ptr->iLocalKeys.end(); i++) {
        QDomElement key = aDoc.createElement(TAG_KEY);
        key.setAttribute(ATTR_NAME, i.key());
        key.setAttribute(ATTR_VALUE, i.value());
        root.appendChild(key);
    }

    // Set local fields.
    const auto &localFields = d_ptr->iLocalFields;
    for(const ProfileField * const field : localFields) {
        root.appendChild(field->toXml(aDoc));
    }

    if (!aLocalOnly) {
        // Set merged keys.
        for (i = d_ptr->iMergedKeys.begin(); i != d_ptr->iMergedKeys.end(); i++) {
            QDomElement key = aDoc.createElement(TAG_KEY);
            key.setAttribute(ATTR_NAME, i.key());
            key.setAttribute(ATTR_VALUE, i.value());
            root.appendChild(key);
        }

        // Set merged fields.
        {
            const auto &mergedFields = d_ptr->iMergedFields;
            for(const ProfileField * const field : mergedFields) {
                root.appendChild(field->toXml(aDoc));
            }
        }
    }

        {
            const QList<Profile *> &subProfiles = d_ptr->iSubProfiles;
            for(Profile * const p : subProfiles) {
            if (!p->d_ptr->iMerged || !p->d_ptr->iLocalKeys.isEmpty() ||
                    !p->d_ptr->iLocalFields.isEmpty()) {
                root.appendChild(p->toXml(aDoc, aLocalOnly));
            }
        }
    }

    return root;
}

QString Profile::toString() const
{
    QDomDocument doc;
    QDomProcessingInstruction xmlHeading =
        doc.createProcessingInstruction("xml",
                                        "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xmlHeading);
    QDomElement root = toXml(doc, false);
    doc.appendChild(root);

    return doc.toString(PROFILE_INDENT);
}

bool Profile::isValid() const
{
    // Profile name and type must be set.
    if (d_ptr->iName.isEmpty()) {
        qCDebug(lcButeoCore) << "Error: Profile name is empty" ;
        return false;
    }

    if (d_ptr->iType.isEmpty()) {
        qCDebug(lcButeoCore) << "Error: Profile type is empty" ;
        return false;
    }

    // For each field a key with the same name must exist, and the
    // key values must be valid for the field.
    const QList<const ProfileField *> &fields = allFields();
    for(const ProfileField * const f : fields) {
        const QStringList values = keyValues(f->name());
        if (values.isEmpty()) {
            qCDebug(lcButeoCore) << "Error: Cannot find value for field" << f->name() <<
                       "for profile" << d_ptr->iName;
            return false;
        }
        for(const QString &value : values) { 
            if (!f->validate(value)) {
                qCDebug(lcButeoCore) << "Error: Value" << value <<
                           "is not valid for profile" << d_ptr->iName;
                return false;
            }

        }
    }

    // Enabled sub-profiles must be valid.
    {
        const QList<Profile *> &subProfiles = d_ptr->iSubProfiles;
        for(const Profile * const p : subProfiles) {
            if (p->isEnabled() && !p->isValid())
                return false;
        }
    }

    return true;
    }

    QStringList Profile::subProfileNames(const QString &aType) const
    {
    QStringList names;
    bool checkType = !aType.isEmpty();
    {
        const QList<Profile *> &subProfiles = d_ptr->iSubProfiles;
        for(const Profile * const p : subProfiles) {
            if (!checkType || aType == p->type()) {
                names.append(p->name());
            }
        }
    }

    return names;
}

Profile *Profile::subProfile(const QString &aName,
                             const QString &aType)
{
    bool checkType = !aType.isEmpty();
    {
        const QList<Profile *> &subProfiles = d_ptr->iSubProfiles;
        for (Profile * const p : subProfiles) {
            if (aName == p->name() && (!checkType || aType == p->type())) {
                return p;
            }
        }
    }

    return 0;
}

const Profile *Profile::subProfile(const QString &aName,
                                   const QString &aType) const
{
    bool checkType = !aType.isEmpty();
    {
        const QList<Profile *> &subProfiles = d_ptr->iSubProfiles;
        for(const Profile * const p : subProfiles) {
            if (aName == p->name() && (!checkType || aType == p->type())) {
                return p;
            }
        }
    }

    return 0;
}

const Profile *Profile::subProfileByKeyValue(const QString &aKey,
                                             const QString &aValue,
                                             const QString &aType,
                                             bool aEnabledOnly) const
{
    bool checkType = !aType.isEmpty();
    {
        const QList<Profile *> &subProfiles = d_ptr->iSubProfiles;
        for(const Profile * const p : subProfiles) {
            if ((!checkType || aType == p->type()) &&
                    (aValue.compare(p->key(aKey), Qt::CaseInsensitive) == 0) &&
                    (!aEnabledOnly || p->isEnabled())) {
                return p;
            }
        }
    }

    return 0;
}

QList<Profile *> Profile::allSubProfiles()
{
    return d_ptr->iSubProfiles;
}

QList<const Profile *> Profile::allSubProfiles() const
{
    QList<const Profile *> constProfiles;
    {
        const QList<Profile *> &subProfiles = d_ptr->iSubProfiles;
        for(const Profile * const p : subProfiles) {
            constProfiles.append(p);
        }
    }

    return constProfiles;
}

void Profile::merge(const Profile &aSource)
{
    // Get target sub-profile. Create new if not found.
    Profile *target = subProfile(aSource.name(), aSource.type());
    if (0 == target) {
        ProfileFactory pf;
        target = pf.createProfile(aSource.name(), aSource.type());
        if (target != 0) {
            target->d_ptr->iMerged = true;
            d_ptr->iSubProfiles.append(target);
        }
    }

    if (target != 0) {
        // Merge keys. Allow multiple keys with the same name.
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        target->d_ptr->iMergedKeys.unite(aSource.d_ptr->iLocalKeys);
        target->d_ptr->iMergedKeys.unite(aSource.d_ptr->iMergedKeys);
#else
        for (auto it = aSource.d_ptr->iLocalKeys.begin(); it != aSource.d_ptr->iLocalKeys.end(); ++it)
            target->d_ptr->iMergedKeys.insert(it.key(), it.value());
        for (auto it = aSource.d_ptr->iMergedKeys.begin(); it != aSource.d_ptr->iMergedKeys.end(); ++it)
            target->d_ptr->iMergedKeys.insert(it.key(), it.value());
#endif

        // Merge fields.
        const QList<const ProfileField *> &sourceFields = aSource.allFields();
        for(const ProfileField * const f : sourceFields) {
            if (0 == target->field(f->name())) {
                target->d_ptr->iMergedFields.append(new ProfileField(*f));
            }
        }
        }

        {
            const QList<Profile *> &subProfiles = aSource.d_ptr->iSubProfiles;
            for(const Profile * const p : subProfiles) {
                merge(*p);
            }
        }
}

bool Profile::isLoaded() const
{
    return d_ptr->iLoaded;
}

void Profile::setLoaded(bool aLoaded)
{
    d_ptr->iLoaded = aLoaded;
}

bool Profile::isEnabled() const
{
    return boolKey(KEY_ENABLED, true);
}

void Profile::setEnabled(bool aEnabled)
{
    setBoolKey(KEY_ENABLED, aEnabled);
}

bool Profile::isHidden() const
{
    return boolKey(KEY_HIDDEN);
}

bool Profile::isProtected() const
{
    return boolKey(KEY_PROTECTED);
}

QString Profile::displayname() const
{
    return key(KEY_DISPLAY_NAME);
}

QString Profile::generateProfileId(const QStringList &aKeys)
{
    if (aKeys.size() == 0)
        return QString();

    QString aId = QString::number(qHash(aKeys.join(QString())));
    return aId;
}
