# ChatRoom Bug修复计划

## TL;DR

> **Quick Summary**: 修复ChatRoom局域网聊天项目的P0和P1级别bug，包括ThreadPool空实现、自动重连逻辑、线程同步、心跳超时处理、好友请求通知、连接数限制、消息ACK优化、群聊昵称显示、消息时间格式。
>
> **Deliverables**:
> - 完整实现的ThreadPool（使用QtConcurrent或QThreadPool）
> - 修复的自动重连机制（带退避策略）
> - 线程安全的共享数据结构（MessageModel, TcpClient, SessionManager）
> - 心跳超时自动断开和重连
> - 好友请求接受通知
> - 服务端最大连接数限制
> - O(1)消息ACK查找
> - 群聊发送者昵称显示
> - 消息时间完整格式（含日期）
>
> **Estimated Effort**: Medium
> **Parallel Execution**: YES - 4 waves
> **Critical Path**: Task 1 → Task 2 → Task 3 → Task 4 → Task 5 → Task 6 → Task 7 → Task 8 → Task 9 → F1-F4

---

## Context

### Original Request
修复ChatRoom项目的P0和P1级别bug，提升项目稳定性和评分。

### Interview Summary
**Key Discussions**:
- P0: ThreadPool空实现、自动重连缺陷、缺少线程同步
- P1: 心跳超时、好友请求通知、连接数限制、ACK效率、群聊昵称、时间格式
- 技术栈: Qt6.11 + C++17 + CMake + QML

### Metis Review
**Identified Gaps** (addressed):
- ThreadPool实现策略: 使用QThreadPool + QRunnable而非自定义线程管理
- 重连策略: 指数退避，最大重试10次
- 同步策略: QMutex for simple cases, QReadWriteLock for read-heavy data

---

## Work Objectives

### Core Objective
修复所有P0和P1级别bug，使项目达到生产可用状态，评分从74分提升至90+分。

### Concrete Deliverables
- `server/src/core/ThreadPool.h` - 完整实现
- `server/src/core/ThreadPool.cpp` - 完整实现
- `client/src/network/TcpClient.cpp` - 修复重连逻辑
- `client/src/network/HeartbeatManager.cpp` - 修复超时处理
- `client/src/models/MessageModel.cpp` - 添加线程同步
- `client/src/core/SessionManager.cpp` - 添加线程同步
- `server/src/core/TcpServer.cpp` - 添加连接数限制
- `server/src/core/MessageRouter.cpp` - 修复好友请求通知
- `client/src/qmlbridge/ChatController.cpp` - 优化ACK查找 + 群聊昵称
- `client/qml/components/MessageBubble.qml` - 修复时间显示

### Definition of Done
- [ ] 所有P0 bug修复并通过编译
- [ ] 所有P1 bug修复并通过编译
- [ ] 项目能正常编译运行
- [ ] 无新增编译警告

### Must Have
- ThreadPool实际工作（非空实现）
- 网络断开后自动重连
- 共享数据线程安全
- 心跳超时处理

### Must NOT Have (Guardrails)
- 不重构整个架构
- 不改变现有API接口（除非必要）
- 不添加新功能（只修复bug）
- 不修改数据库schema

---

## Verification Strategy

### Test Decision
- **Infrastructure exists**: NO（无现有测试框架）
- **Automated tests**: NO
- **Framework**: none
- **Agent-Executed QA**: 使用Bash编译验证 + Playwright UI验证（如适用）

### QA Policy
Every task MUST include agent-executed QA scenarios.
Evidence saved to `.sisyphus/evidence/task-{N}-{scenario-slug}.{ext}`.

---

## Execution Strategy

### Parallel Execution Waves

```
Wave 1 (Foundation - Thread Safety + ThreadPool):
├── Task 1: Implement ThreadPool [unspecified-high]
├── Task 2: Add mutex to MessageModel [quick]
├── Task 3: Add mutex to SessionManager [quick]
└── Task 4: Add mutex to TcpClient::sendPacket [quick]

Wave 2 (Network Reliability):
├── Task 5: Fix auto-reconnect in TcpClient [deep]
└── Task 6: Fix heartbeat timeout handling [quick]

Wave 3 (Server Improvements):
├── Task 7: Add max connection limit to TcpServer [quick]
└── Task 8: Fix friend request accept notification [quick]

Wave 4 (Client Improvements):
├── Task 9: Optimize message ACK lookup [quick]
├── Task 10: Fix group chat senderNickname [quick]
└── Task 11: Fix message time format [quick]

Wave FINAL (Verification):
├── Task F1: Compile check (oracle)
├── Task F2: Code quality review (unspecified-high)
└── Task F3: Scope fidelity check (deep)
```

### Dependency Matrix
- **1-4**: None → can start immediately
- **5**: Depends on 4 (TcpClient mutex) → Wave 2
- **6**: Depends on 5 (reconnect logic) → Wave 2
- **7-8**: None → Wave 3 (parallel with Wave 2)
- **9-11**: None → Wave 4 (parallel with Wave 2-3)
- **F1-F3**: Depends on ALL tasks → Wave FINAL

---

## TODOs

- [ ] 1. **Implement ThreadPool**

  **What to do**:
  - Replace empty ThreadPool.h/cpp with real implementation using QThreadPool + QRunnable
  - Support task submission with std::function
  - Support graceful shutdown
  - Integrate with MessageRouter to process packets in thread pool

  **Must NOT do**:
  - Don't use raw QThread management (too complex)
  - Don't change MessageRouter public API

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high`
  - **Skills**: [`nodejs-best-practices`]
    - Actually: need C++ Qt expert, but use `unspecified-high` with clear instructions

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1
  - **Blocks**: Task 8 (MessageRouter may use ThreadPool)
  - **Blocked By**: None

  **References**:
  - `server/src/core/ThreadPool.h` - Current empty header
  - `server/src/core/ThreadPool.cpp` - Current empty implementation
  - `server/src/core/MessageRouter.cpp:36-44` - Where ThreadPool should be used

  **Acceptance Criteria**:
  - [ ] ThreadPool can submit and execute tasks
  - [ ] ThreadPool has configurable thread count
  - [ ] ThreadPool supports graceful shutdown
  - [ ] MessageRouter uses ThreadPool for packet processing

  **QA Scenarios**:
  ```
  Scenario: ThreadPool executes submitted tasks
    Tool: Bash (compile + grep)
    Preconditions: Clean build
    Steps:
      1. Build server target
      2. Grep ThreadPool.cpp for "QThreadPool" or "QRunnable"
      3. Grep MessageRouter.cpp for "threadPool" or "ThreadPool"
    Expected Result: Found implementation references
    Evidence: .sisyphus/evidence/task-1-threadpool-impl.log
  ```

- [ ] 2. **Add Mutex to MessageModel**

  **What to do**:
  - Add QMutex to MessageModel class
  - Lock in addMessage, loadMessages, clear, updateMessageStatus, updateMessageContent
  - Use QMutexLocker for RAII

  **Must NOT do**:
  - Don't change public API
  - Don't use recursive mutex unless necessary

  **Recommended Agent Profile**:
  - **Category**: `quick`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1
  - **Blocked By**: None

  **References**:
  - `client/src/models/MessageModel.h` - Header to modify
  - `client/src/models/MessageModel.cpp` - Implementation to modify

  **Acceptance Criteria**:
  - [ ] QMutex member added to MessageModel
  - [ ] All modifying methods use QMutexLocker
  - [ ] data() method uses QMutexLocker for read safety

  **QA Scenarios**:
  ```
  Scenario: MessageModel has thread safety
    Tool: Bash (grep)
    Preconditions: None
    Steps:
      1. Grep MessageModel.h for "QMutex"
      2. Grep MessageModel.cpp for "QMutexLocker"
    Expected Result: Found mutex usage in all modifying methods
    Evidence: .sisyphus/evidence/task-2-mutex-check.log
  ```

- [ ] 3. **Add Mutex to SessionManager**

  **What to do**:
  - Add QMutex to SessionManager class
  - Lock in storeSessionKey, getSessionKey, hasSessionKey, removeSessionKey
  - Lock in setLoggedIn, logout (for m_sessionKeys clear)

  **Must NOT do**:
  - Don't lock simple getters (userId, username) - atomic reads are safe

  **Recommended Agent Profile**:
  - **Category**: `quick`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1
  - **Blocked By**: None

  **References**:
  - `client/src/core/SessionManager.h` - Header to modify
  - `client/src/core/SessionManager.cpp` - Implementation to modify

  **Acceptance Criteria**:
  - [ ] QMutex member added to SessionManager
  - [ ] m_sessionKeys access is protected by mutex

  **QA Scenarios**:
  ```
  Scenario: SessionManager session keys are thread-safe
    Tool: Bash (grep)
    Steps:
      1. Grep SessionManager.h for "QMutex"
      2. Grep SessionManager.cpp for "QMutexLocker" near m_sessionKeys
    Expected Result: Found mutex protection
    Evidence: .sisyphus/evidence/task-3-session-mutex.log
  ```

- [ ] 4. **Add Mutex to TcpClient::sendPacket**

  **What to do**:
  - Add QMutex to TcpClient class
  - Lock in sendRaw and sendPacket methods
  - Lock in processBuffer for m_readBuffer access

  **Must NOT do**:
  - Don't lock isConnected() (atomic state check)

  **Recommended Agent Profile**:
  - **Category**: `quick`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1
  - **Blocks**: Task 5 (reconnect logic depends on thread-safe send)
  - **Blocked By**: None

  **References**:
  - `client/src/network/TcpClient.h` - Header to modify
  - `client/src/network/TcpClient.cpp` - Implementation to modify

  **Acceptance Criteria**:
  - [ ] QMutex member added to TcpClient
  - [ ] sendRaw/sendPacket use QMutexLocker
  - [ ] processBuffer uses QMutexLocker for buffer access

  **QA Scenarios**:
  ```
  Scenario: TcpClient send is thread-safe
    Tool: Bash (grep)
    Steps:
      1. Grep TcpClient.h for "QMutex"
      2. Grep TcpClient.cpp for "QMutexLocker"
    Expected Result: Found mutex in send and buffer methods
    Evidence: .sisyphus/evidence/task-4-tcp-mutex.log
  ```

- [ ] 5. **Fix Auto-Reconnect in TcpClient**

  **What to do**:
  - Add QTimer for reconnection with exponential backoff
  - Max retry: 10 attempts
  - Initial delay: 1s, max delay: 30s
  - On manual disconnect, stop reconnection attempts
  - Limit pending queue to 100 messages (drop oldest)

  **Must NOT do**:
  - Don't block UI thread during reconnection
  - Don't retry indefinitely

  **Recommended Agent Profile**:
  - **Category**: `deep`

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Blocked By**: Task 4 (TcpClient mutex)
  - **Blocks**: Task 6 (heartbeat depends on reconnect)

  **References**:
  - `client/src/network/TcpClient.h` - Add reconnect timer
  - `client/src/network/TcpClient.cpp:63-79` - Current broken logic

  **Acceptance Criteria**:
  - [ ] Reconnect timer added with exponential backoff
  - [ ] Max retry limit enforced
  - [ ] Pending queue limited to 100 messages
  - [ ] Manual disconnect stops reconnection

  **QA Scenarios**:
  ```
  Scenario: Auto-reconnect works with backoff
    Tool: Bash (grep + code review)
    Steps:
      1. Check TcpClient.h for QTimer* m_reconnectTimer
      2. Check TcpClient.cpp for exponential backoff logic
      3. Verify pending queue size limit
    Expected Result: Found reconnect implementation with backoff
    Evidence: .sisyphus/evidence/task-5-reconnect.log
  ```

- [ ] 6. **Fix Heartbeat Timeout Handling**

  **What to do**:
  - In HeartbeatManager::sendHeartbeat, when timeout detected:
    - Emit heartbeatTimeout (already done)
    - Also force disconnect socket to trigger reconnect logic
  - Connect heartbeatTimeout signal in AppManager or TcpClient

  **Must NOT do**:
  - Don't directly reconnect from HeartbeatManager (separation of concerns)

  **Recommended Agent Profile**:
  - **Category**: `quick`

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Blocked By**: Task 5 (reconnect logic)

  **References**:
  - `client/src/network/HeartbeatManager.cpp:47-51` - Timeout handling
  - `client/src/network/TcpClient.cpp` - Where disconnect should be triggered

  **Acceptance Criteria**:
  - [ ] Heartbeat timeout forces socket disconnect
  - [ ] Disconnect triggers reconnect logic (Task 5)

  **QA Scenarios**:
  ```
  Scenario: Heartbeat timeout triggers disconnect
    Tool: Bash (grep)
    Steps:
      1. Grep HeartbeatManager.cpp for "disconnect" or "abort"
      2. Check that heartbeatTimeout is connected somewhere
    Expected Result: Found disconnect call on timeout
    Evidence: .sisyphus/evidence/task-6-heartbeat.log
  ```

- [ ] 7. **Add Max Connection Limit to TcpServer**

  **What to do**:
  - Add maxConnections property (default: 100)
  - In incomingConnection, check current count vs max
  - Reject new connections if at limit
  - Log warning when limit reached

  **Must NOT do**:
  - Don't change existing connection handling logic

  **Recommended Agent Profile**:
  - **Category**: `quick`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 3
  - **Blocked By**: None

  **References**:
  - `server/src/core/TcpServer.h` - Add maxConnections
  - `server/src/core/TcpServer.cpp:96-126` - incomingConnection

  **Acceptance Criteria**:
  - [ ] maxConnections property added
  - [ ] incomingConnection checks limit
  - [ ] Excess connections rejected with log warning

  **QA Scenarios**:
  ```
  Scenario: Connection limit enforced
    Tool: Bash (grep)
    Steps:
      1. Grep TcpServer.h for "maxConnections"
      2. Grep TcpServer.cpp for limit check in incomingConnection
    Expected Result: Found connection limit logic
    Evidence: .sisyphus/evidence/task-7-connection-limit.log
  ```

- [ ] 8. **Fix Friend Request Accept Notification**

  **What to do**:
  - In MessageRouter::handleFriendAccept, after successful accept:
    - Get request details (fromUserId, toUserId)
    - If fromUser is online, send FriendAcceptNotify
    - Include accepter's info in notification

  **Must NOT do**:
  - Don't change database schema

  **Recommended Agent Profile**:
  - **Category**: `quick`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 3
  - **Blocked By**: None

  **References**:
  - `server/src/core/MessageRouter.cpp:583-597` - handleFriendAccept
  - `common/src/protocol/MessageTypes.h` - Check for FriendAcceptNotify type

  **Acceptance Criteria**:
  - [ ] After accept, request sender gets notification
  - [ ] Notification includes accepter info

  **QA Scenarios**:
  ```
  Scenario: Friend accept sends notification
    Tool: Bash (grep)
    Steps:
      1. Grep MessageRouter.cpp for "FriendAcceptNotify"
      2. Verify notification is sent to fromUserId
    Expected Result: Found notification logic in handleFriendAccept
    Evidence: .sisyphus/evidence/task-8-friend-notify.log
  ```

- [ ] 9. **Optimize Message ACK Lookup**

  **What to do**:
  - Replace QMap with QHash for m_pendingAcks (already QMap, but verify)
  - Add reverse index: QHash<messageId, sequence> for O(1) lookup
  - Or: maintain both indices

  **Must NOT do**:
  - Don't break existing ACK matching logic

  **Recommended Agent Profile**:
  - **Category**: `quick`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 4
  - **Blocked By**: None

  **References**:
  - `client/src/qmlbridge/ChatController.cpp:382-409` - ACK logic

  **Acceptance Criteria**:
  - [ ] ACK lookup is O(1) instead of O(n)
  - [ ] Existing functionality preserved

  **QA Scenarios**:
  ```
  Scenario: ACK lookup optimized
    Tool: Bash (grep)
    Steps:
      1. Check ChatController.h for data structure change
      2. Verify no linear search in acknowledgeMessage
    Expected Result: Found QHash or direct index usage
    Evidence: .sisyphus/evidence/task-9-ack-optimize.log
  ```

- [ ] 10. **Fix Group Chat senderNickname**

  **What to do**:
  - In ChatController::sendGroupMessage, set senderNickname from SessionManager
  - Use m_appManager->sessionManager()->nickname()

  **Must NOT do**:
  - Don't change message protocol

  **Recommended Agent Profile**:
  - **Category**: `quick`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 4
  - **Blocked By**: None

  **References**:
  - `client/src/qmlbridge/ChatController.cpp:133-148` - sendGroupMessage

  **Acceptance Criteria**:
  - [ ] senderNickname is set to current user's nickname

  **QA Scenarios**:
  ```
  Scenario: Group message has sender nickname
    Tool: Bash (grep)
    Steps:
      1. Grep ChatController.cpp for "senderNickname"
      2. Verify it's set from sessionManager
    Expected Result: Found nickname assignment
    Evidence: .sisyphus/evidence/task-10-group-nickname.log
  ```

- [ ] 11. **Fix Message Time Format**

  **What to do**:
  - In MessageBubble.qml, show full date for messages older than today
  - Format: "昨天 HH:mm" or "MM-dd HH:mm" for older messages

  **Must NOT do**:
  - Don't break existing time display for today's messages

  **Recommended Agent Profile**:
  - **Category**: `quick`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 4
  - **Blocked By**: None

  **References**:
  - `client/qml/components/MessageBubble.qml:89` - Current time display

  **Acceptance Criteria**:
  - [ ] Today's messages show "HH:mm"
  - [ ] Yesterday's messages show "昨天 HH:mm"
  - [ ] Older messages show "MM-dd HH:mm"

  **QA Scenarios**:
  ```
  Scenario: Message time shows date when needed
    Tool: Bash (grep)
    Steps:
      1. Grep MessageBubble.qml for date formatting logic
      2. Verify conditional formatting exists
    Expected Result: Found date-aware time formatting
    Evidence: .sisyphus/evidence/task-11-time-format.log
  ```

---

## Final Verification Wave

- [ ] F1. **Compile Check** — `oracle`
  Run cmake build for both client and server. Verify zero errors.
  Output: `Build [PASS/FAIL]`

- [ ] F2. **Code Quality Review** — `unspecified-high`
  Check for: `as any`/`@ts-ignore`, empty catches, console.log in prod, commented-out code, unused imports.
  Output: `Files [N clean/N issues]`

- [ ] F3. **Scope Fidelity Check** — `deep`
  Verify only bug fixes were made, no feature creep.
  Output: `Tasks [N/N compliant]`

---

## Commit Strategy

- **Wave 1**: `fix(server): implement ThreadPool and add thread safety`
- **Wave 2**: `fix(client): fix auto-reconnect and heartbeat timeout`
- **Wave 3**: `fix(server): add connection limit and friend request notification`
- **Wave 4**: `fix(client): optimize ACK lookup and fix UI issues`

---

## Success Criteria

### Verification Commands
```bash
# Build client
cd build && cmake --build . --target ChatRoomClient

# Build server
cd build && cmake --build . --target ChatRoomServer
```

### Final Checklist
- [ ] All P0 bugs fixed
- [ ] All P1 bugs fixed
- [ ] Project compiles without errors
- [ ] No new warnings introduced
