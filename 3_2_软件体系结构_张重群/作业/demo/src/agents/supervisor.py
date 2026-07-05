"""
Supervisor Agent (监督者/路由器)
职责: 意图识别、任务路由、结果聚合
应用设计模式: Routing (路由模式) + Multi-Agent (多智能体协作)
"""

import re
from src.agents.base import BaseAgent
from src.agents.retrieval import RetrievalAgent
from src.agents.planner import PlannerAgent
from src.agents.summarizer import SummarizerAgent
from src.agents.quiz import QuizAgent
from src.core.memory import memory_manager


class SupervisorAgent(BaseAgent):
    """
    监督者Agent - 系统的入口路由
    负责:
    1. 分析用户意图 (LLM驱动的意图分类)
    2. 将请求路由到对应的专业Agent
    3. 聚合多个Agent的返回结果
    """

    # 意图类别与对应Agent的映射
    INTENT_MAP = {
        "question": "retrieval",      # 问答 → RetrievalAgent
        "summarize": "summarizer",    # 总结 → SummarizerAgent
        "plan": "planner",            # 规划 → PlannerAgent
        "quiz": "quiz",               # 测验 → QuizAgent
        "upload": None,               # 上传 → 直接处理
        "greeting": None,             # 问候 → 直接回复
    }

    # 安全护栏: 敏感词列表
    BLOCKED_PATTERNS = [
        r'作弊', r'代考', r'替考', r'抄袭', r'泄题',
        r'答案', r'抄.*作业',
    ]

    def __init__(self):
        super().__init__("SupervisorAgent", "意图识别与任务路由")
        # 注册专业Agent
        self.agents = {
            "retrieval": RetrievalAgent(),
            "planner": PlannerAgent(),
            "summarizer": SummarizerAgent(),
            "quiz": QuizAgent(),
        }

    def _safety_check(self, query: str) -> tuple[bool, str]:
        """
        安全护栏检查
        检测用户输入是否包含不当内容
        """
        for pattern in self.BLOCKED_PATTERNS:
            if re.search(pattern, query):
                return False, "抱歉，您的请求涉及不当内容，我无法处理。请提出与课程学习相关的问题。"
        return True, ""

    def classify_intent(self, query: str) -> str:
        """用 LLM 做意图分类"""
        prompt = (
            "将用户消息分类为以下意图之一。\n"
            "只能回复一个词: question 或 summarize 或 plan 或 quiz 或 greeting\n\n"
            "意图说明:\n"
            "- question: 提问概念、询问知识（如'什么是XX'、'XX的原理是什么'）\n"
            "- summarize: 要求总结归纳梳理（如'总结一下'、'归纳核心'、'梳理知识点'）\n"
            "- plan: 要求制定计划安排（如'制定复习计划'、'帮我安排7天学习'）\n"
            "- quiz: 要求出题测试（如'出几道题'、'生成选择题'）\n"
            "- greeting: 问候寒暄（如'你好'、'谢谢'）\n\n"
            f"用户: {query}\n\n"
            "回复:"
        )
        try:
            result = self.llm.chat(
                [{"role": "user", "content": prompt}],
                temperature=0.0, max_tokens=10,
            )
            result = result.strip().lower()
            for intent in ["summarize", "plan", "quiz", "greeting", "question"]:
                if intent in result:
                    return intent
            # 如果 LLM 返回了中文或其他格式，回退关键词
            return self._fallback_classify(query)
        except Exception:
            return self._fallback_classify(query)

    def _fallback_classify(self, query: str) -> str:
        """关键词回退（仅在 LLM 调用失败时使用）"""
        q = query.lower()
        # 总结/归纳类关键词
        if any(k in q for k in ["总结", "归纳", "梳理", "概述", "大纲", "笔记"]):
            return "summarize"
        # 规划类
        if any(k in q for k in ["计划", "复习安排", "日程", "规划"]):
            return "plan"
        # 出题类
        if any(k in q for k in ["出题", "题目", "测试题", "选择", "简答题"]):
            return "quiz"
        # 问候
        if len(q) <= 3 and any(k in q for k in ["你好", "hi", "谢谢"]):
            return "greeting"
        return "question"

    def process(self, input_data: dict) -> dict:
        """
        处理用户请求: 安全检查 → 意图分类 → 路由 → 结果聚合
        """
        query = input_data.get("query", "")
        session_id = input_data.get("session_id", "default")
        history = input_data.get("history", [])

        # Step 1: 安全护栏
        safe, message = self._safety_check(query)
        if not safe:
            return {
                "intent": "blocked",
                "agent": None,
                "response": message,
                "sources": [],
            }

        # Step 2: 意图分类
        intent = self.classify_intent(query)

        # Step 3: 根据意图路由
        if intent == "greeting":
            return {
                "intent": "greeting",
                "agent": None,
                "response": (
                    "你好！我是智能学习助手 🤖\n\n"
                    "我可以帮你:\n"
                    "📚 **智能问答** — 回答学习相关的问题\n"
                    "📝 **知识总结** — 对上传的课件生成结构化笔记\n"
                    "📋 **学习规划** — 制定考前复习计划\n"
                    "🎯 **自测练习** — 根据知识点生成练习题\n\n"
                    "直接输入问题即可开始！"
                ),
                "sources": [],
            }
        else:
            # Step 4: 路由到专业Agent
            agent_name = self.INTENT_MAP.get(intent, "retrieval")
            agent = self.agents.get(agent_name)

            if agent is None:
                return {
                    "intent": intent,
                    "agent": None,
                    "response": "抱歉，我无法处理这个请求。",
                    "sources": [],
                }

            # Step 5: 执行Agent任务
            agent_input = {
                "query": query,
                "session_id": session_id,
                "history": history,
            }
            result = agent.process(agent_input)

            return {
                "intent": intent,
                "agent": agent.name,
                "response": result.get("response", ""),
                "sources": result.get("sources", []),
                "plan": result.get("plan"),
                "quiz": result.get("quiz"),
            }

    async def process_with_memory(self, query: str, session_id: str) -> dict:
        """
        带记忆的请求处理
        保存对话历史到短期和长期记忆
        """
        # 获取历史
        context = memory_manager.get_short_term_context()

        result = self.process({
            "query": query,
            "session_id": session_id,
            "history": context,
        })

        # 保存到短期记忆
        memory_manager.add_message("user", query)
        memory_manager.add_message("assistant", result.get("response", ""))

        # 异步保存到长期记忆
        await memory_manager.save_message(session_id, "user", query)
        await memory_manager.save_message(
            session_id, "assistant",
            result.get("response", ""),
            {"intent": result.get("intent")},
        )

        # 记录学习行为
        await memory_manager.record_learning(
            session_id,
            result.get("intent", "unknown"),
            detail=query[:100],
        )

        return result


# 全局监督者
supervisor = SupervisorAgent()
