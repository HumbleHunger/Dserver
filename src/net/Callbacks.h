#ifndef DJX_CALLBACKS_H
#define DJX_CALLBACKS_H

#include "../base/Timestamp.h"

#include <functional>
#include <memory>

namespace DJX
{
namespace net
{

class Buffer;
class TcpConnection;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void()> TimerCallback;

typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;
typedef std::function<void (const TcpConnectionPtr&)> WriteCompleteCallback;
typedef std::function<void (const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;

typedef std::function<void (const TcpConnectionPtr&,
                            Buffer*,
                            Timestamp)> MessageCallback;


} // namespace net
} // namespace DJX

#endif