#include "ManagedTypes.h"

#include <QDBusMetaType>

void registerManagedTypes()
{
    qDBusRegisterMetaType<QVariantMapMap>();
    qDBusRegisterMetaType<DBusManagerStruct>();
}
