"""
记忆管理器
实现智能体的短期记忆（对话上下文）和长期记忆（SQLite 持久化）
"""

import json
import aiosqlite
from datetime import datetime
from src.core.config import config


class MemoryManager:
    """
    智能体记忆管理器
    - 短期记忆: 内存中的对话消息列表 (会话级别)
    - 长期记忆: SQLite 中持久化的对话历史和学习记录
    """

    def __init__(self):
        self.short_term: list[dict] = []  # [{"role": "user"|"assistant", "content": "..."}]
        self.db_path = config.SQLITE_DB_PATH
        self.max_short_term = 20  # 短期记忆最多保留最近20条消息

    async def init_db(self):
        """初始化数据库表"""
        async with aiosqlite.connect(self.db_path) as db:
            await db.execute("""
                CREATE TABLE IF NOT EXISTS conversations (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    session_id TEXT NOT NULL,
                    role TEXT NOT NULL,
                    content TEXT NOT NULL,
                    metadata TEXT,
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                )
            """)
            await db.execute("""
                CREATE TABLE IF NOT EXISTS learning_records (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    session_id TEXT NOT NULL,
                    subject TEXT,
                    action TEXT NOT NULL,
                    detail TEXT,
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                )
            """)
            await db.execute("""
                CREATE INDEX IF NOT EXISTS idx_conv_session ON conversations(session_id, created_at)
            """)
            await db.execute("""
                CREATE INDEX IF NOT EXISTS idx_lr_session ON learning_records(session_id, created_at)
            """)
            await db.commit()

    def add_message(self, role: str, content: str, metadata: dict = None):
        """添加消息到短期记忆"""
        msg = {"role": role, "content": content, "metadata": metadata or {}}
        self.short_term.append(msg)
        # FIFO: 保留最近N条
        if len(self.short_term) > self.max_short_term:
            self.short_term = self.short_term[-self.max_short_term:]

    async def save_message(self, session_id: str, role: str, content: str, metadata: dict = None):
        """持久化消息到长期记忆"""
        async with aiosqlite.connect(self.db_path) as db:
            await db.execute(
                "INSERT INTO conversations (session_id, role, content, metadata) VALUES (?, ?, ?, ?)",
                (session_id, role, content, json.dumps(metadata or {})),
            )
            await db.commit()

    async def get_history(self, session_id: str, limit: int = 50) -> list[dict]:
        """从长期记忆获取对话历史"""
        async with aiosqlite.connect(self.db_path) as db:
            cursor = await db.execute(
                "SELECT role, content, metadata FROM conversations "
                "WHERE session_id = ? ORDER BY created_at DESC LIMIT ?",
                (session_id, limit),
            )
            rows = await cursor.fetchall()
            return [
                {"role": row[0], "content": row[1], "metadata": json.loads(row[2])}
                for row in reversed(rows)
            ]

    async def record_learning(self, session_id: str, action: str, subject: str = "", detail: str = ""):
        """记录学习行为"""
        async with aiosqlite.connect(self.db_path) as db:
            await db.execute(
                "INSERT INTO learning_records (session_id, subject, action, detail) VALUES (?, ?, ?, ?)",
                (session_id, subject, action, detail),
            )
            await db.commit()

    async def list_sessions(self, limit: int = 50) -> list[dict]:
        """列出所有会话（按时间倒序，每条返回首条用户消息作为标题）"""
        async with aiosqlite.connect(self.db_path) as db:
            cursor = await db.execute(
                "SELECT session_id, MIN(created_at) as started, COUNT(*) as msg_count "
                "FROM conversations GROUP BY session_id "
                "ORDER BY started DESC LIMIT ?",
                (limit,),
            )
            rows = await cursor.fetchall()
            sessions = []
            for row in rows:
                sid, started, count = row
                # 取该会话第一条用户消息作为标题
                cur2 = await db.execute(
                    "SELECT content FROM conversations "
                    "WHERE session_id = ? AND role = 'user' ORDER BY created_at ASC LIMIT 1",
                    (sid,),
                )
                first = await cur2.fetchone()
                title = first[0][:40] + ("..." if first[0] and len(first[0]) > 40 else "") if first else "新会话"
                sessions.append({
                    "session_id": sid,
                    "started": started,
                    "msg_count": count,
                    "title": title,
                })
            return sessions

    async def delete_session(self, session_id: str) -> bool:
        """删除整个会话"""
        async with aiosqlite.connect(self.db_path) as db:
            await db.execute("DELETE FROM conversations WHERE session_id = ?", (session_id,))
            await db.execute("DELETE FROM learning_records WHERE session_id = ?", (session_id,))
            await db.commit()
            return True

    async def get_learning_summary(self, session_id: str) -> dict:
        """获取学习统计摘要"""
        async with aiosqlite.connect(self.db_path) as db:
            cursor = await db.execute(
                "SELECT COUNT(*), action FROM learning_records "
                "WHERE session_id = ? GROUP BY action",
                (session_id,),
            )
            rows = await cursor.fetchall()
            return {row[1]: row[0] for row in rows}

    def get_short_term_context(self) -> list[dict]:
        """获取短期记忆上下文 (用于构建LLM消息)"""
        return [
            {"role": m["role"], "content": m["content"]}
            for m in self.short_term
        ]

    def clear_short_term(self):
        """清理短期记忆"""
        self.short_term = []


# 全局记忆管理器
memory_manager = MemoryManager()
