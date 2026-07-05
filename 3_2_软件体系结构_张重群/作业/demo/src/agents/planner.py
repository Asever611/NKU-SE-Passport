"""
PlannerAgent (规划智能体)
职责: 根据用户需求生成学习复习计划，知识库内容用于增强计划的针对性
应用设计模式: Planning (规划模式)

核心原则:
- LLM 自身具备规划能力，始终生成计划
- 知识库用于让计划更贴合用户已上传的课件内容
"""

from src.agents.base import BaseAgent
from src.core.knowledge_base import knowledge_base


class PlannerAgent(BaseAgent):
    """学习规划智能体 — 知识库增强计划的针对性"""

    def __init__(self):
        super().__init__("PlannerAgent", "学习计划生成 (Planning)")

    def process(self, input_data: dict) -> dict:
        query = input_data.get("query", "")
        history = input_data.get("history", [])

        # 获取知识库中可用课件列表（让计划参考）
        stats = knowledge_base.get_collection_stats()
        if stats["total_chunks"] > 0:
            kb_info = "用户已上传以下课件，制定计划时可参考:\n"
            for doc in stats.get("documents", []):
                kb_info += f"- {doc['filename']} ({doc['chunk_count']} 个知识块)\n"
            # 也拉取一些内容样本帮助LLM了解课件涵盖范围
            sample = knowledge_base.get_all_chunks(max_chars=2000)
            if sample:
                kb_info += f"\n课件内容摘要:\n{sample[:1500]}\n"
        else:
            kb_info = ("注意: 用户还未上传课件。请基于你的知识，"
                       "为该课程/主题制定通用复习计划。"
                       "最后提醒用户上传课件可让计划更精准。")

        system_prompt = (
            "你是一位专业的学习规划师。请根据用户需求制定详细的复习计划。\n\n"
            "规划原则:\n"
            "1. SMART原则: 具体、可衡量、可达成、相关、有时限\n"
            "2. 间隔重复: 重要知识点在不同日期安排复习\n"
            "3. 难度递进: 先基础后深入\n"
            "4. 劳逸结合: 合理分配每日学习量\n"
            "5. 自测环节: 关键节点安排自我测试\n\n"
            "输出格式 (Markdown):\n"
            "## 总体安排 (科目、天数、每日时长)\n"
            "## 分阶段计划 (3个阶段: 基础→重点→冲刺)\n"
            "## 每日任务清单 (每天: 学习内容 + 预估时长 + 完成标准)\n"
            "## 💡 学习建议 (3-5条实用建议)"
        )

        messages = [
            {"role": "system", "content": system_prompt},
            {"role": "user", "content": f"{kb_info}\n\n用户需求: {query}\n\n请制定详细的复习计划:"},
        ]

        response = self.llm.chat_with_retry(messages, temperature=0.7, max_tokens=4096)

        return {"response": response, "sources": []}
