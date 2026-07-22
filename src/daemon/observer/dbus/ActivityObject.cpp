#include "ActivityObject.h"

#include <utility>

ActivityObject::ActivityObject(QVariantMap bag, QObject* parent)
    : QObject(parent), m_bag(std::move(bag))
{
}
