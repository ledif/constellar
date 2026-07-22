#include "CurrentActivityObject.h"

#include <utility>

CurrentActivityObject::CurrentActivityObject(QVariantMap bag, QObject* parent)
    : QObject(parent), m_bag(std::move(bag))
{
}
