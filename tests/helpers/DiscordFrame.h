#pragma once

#include <QByteArray>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalSocket>

inline QByteArray encodeFrame(qint32 opcode, QJsonObject const& payload)
{
    QByteArray const json = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    QByteArray frame;
    QDataStream stream(&frame, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << opcode << static_cast<qint32>(json.size());
    frame.append(json);
    return frame;
}

inline bool decodeFrame(QLocalSocket& socket, qint32& opcode, QJsonObject& payload)
{
    if (socket.bytesAvailable() < 8)
        return false;
    QByteArray const header = socket.peek(8);
    QDataStream headerStream(header);
    headerStream.setByteOrder(QDataStream::LittleEndian);
    qint32 length = 0;
    headerStream >> opcode >> length;

    if (socket.bytesAvailable() < 8 + length)
        return false;
    socket.read(8);
    QByteArray const json = socket.read(length);
    payload = QJsonDocument::fromJson(json).object();
    return true;
}
